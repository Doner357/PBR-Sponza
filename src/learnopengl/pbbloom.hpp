#ifndef PHYSIC_BLOOM_HPP
#define PHYSIC_BLOOM_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <vector>
#include <limits>
#include <climits>

#include "custom_helper.h"
#include "shader_m.h"

struct  BloomMip {
    glm::vec2 size;
    glm::ivec2 int_size;
    unsigned int texture;
};

class BloomFBO {
    public:
        BloomFBO() : has_init(false) {}

        ~BloomFBO() {}

        bool init(unsigned int window_width, unsigned int window_height, unsigned int mip_chain_length) {
            if (has_init) {
                return true;
            }

            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);

            glm::vec2 mip_size(static_cast<float>(window_width), static_cast<float>(window_height));
            glm::ivec2 mip_int_size(static_cast<int>(window_width), static_cast<int>(window_height));

            if (window_width > static_cast<unsigned int>(INT_MAX) || window_height > static_cast<unsigned int>(INT_MAX)) {
                std::cerr << "ERROR::BloomFBO::Init: Window size conversion overflow - cannot build bloom FBO!\n";
                return false;
            }

            for (unsigned int i = 0; i < mip_chain_length; ++i) {
                BloomMip mip;

                mip_size     *= 0.5f;
                mip_int_size /= 2;
                mip.size      = mip_size;
                mip.int_size  = mip_int_size;

                glGenTextures(1, &mip.texture);
                glBindTexture(GL_TEXTURE_2D, mip.texture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, static_cast<int>(mip_size.x), static_cast<int>(mip_size.y), 0, GL_RGB, GL_FLOAT, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                mip_chain.emplace_back(mip);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip_chain[0].texture, 0);

            unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
            glDrawBuffers(1, attachments);

            int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE) {
                printf("gbuffer FBO error, status: 0x\%x\n", status);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                return false;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            has_init = true;
            return true;
        }

        void destroy() {
            for (int i = 0; i< mip_chain.size(); ++i) {
                glDeleteTextures(1, &mip_chain[i].texture);
                mip_chain[i].texture = 0;
            }
            glDeleteFramebuffers(1, &fbo);
            fbo = 0;
            has_init = false;
        }

        void bindForWriting() {
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        }

        const std::vector<BloomMip>& getMipChain() const {
            return mip_chain;
        }

        unsigned int getFbo() {
            return fbo;
        }


    private:
        bool has_init;
        unsigned int fbo;
        std::vector<BloomMip> mip_chain;

};


class BloomRenderer {
    public:
        BloomRenderer(unsigned int quad_vao) : has_init(false), quad_vao(quad_vao) {}
        ~BloomRenderer() {}

        bool init(unsigned int window_width, unsigned int window_height) {
            if (has_init) {
                return true;
            }

            src_viewport_size = glm::ivec2(window_width, window_height);
            src_viewport_size_float = glm::vec2(static_cast<float>(window_width), static_cast<float>(window_height));

            const unsigned int num_bloom_mips = 5;
            bool status = fbo.init(window_width, window_height, num_bloom_mips);
            if (!status) {
                std::cerr << "ERROR::BloomRenderer::init: Failed to initialize bloom FBO - cannot create bloom renderer!\n";
                return false;
            }

            downsample_shader = new Shader("shaders/post-processing/vertex/regular_screen.vert", "shaders/post-processing/fragment/physic_bloom_downsample.frag");
            upsample_shader = new Shader("shaders/post-processing/vertex/regular_screen.vert", "shaders/post-processing/fragment/physic_bloom_upsample.frag");

            downsample_shader->use();
            downsample_shader->setInt("image", 0);
            downsample_shader->deactivate();

            upsample_shader->use();
            upsample_shader->setInt("image", 0);
            upsample_shader->deactivate();

            has_init = true;
            return true;
        }

        void destroy() {
            fbo.destroy();
            delete downsample_shader;
            delete upsample_shader;
            has_init = false;
        }

        void renderBloomTexture(unsigned int src_texture, float filter_radius) {
            fbo.bindForWriting();

            renderDownsamples(src_texture);
            renderUpdamples(filter_radius);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glViewport(0, 0, src_viewport_size.x, src_viewport_size.y);
        }

        unsigned int getBloomTexture() {
            return fbo.getMipChain()[0].texture;
        }

        unsigned int getBloomFbo() {
            return fbo.getFbo();
        }

    
    private:
        void renderDownsamples(unsigned int src_texture) {
            const std::vector<BloomMip>& mip_chain = fbo.getMipChain();

            downsample_shader->use();
            downsample_shader->setVec2("imgResolution", src_viewport_size_float);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, src_texture);

            for (int i = 0; i < mip_chain.size(); ++i) {
                const BloomMip& mip = mip_chain[i];
                glViewport(0, 0, mip.size.x, mip.size.y);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);

                glBindVertexArray(quad_vao);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);

                downsample_shader->setVec2("imgResolution", mip.size);
                downsample_shader->setInt("mipLevel", i);
                glBindTexture(GL_TEXTURE_2D, mip.texture);
            }

            downsample_shader->deactivate();
        }

        void renderUpdamples(float filter_radius) {
            const std::vector<BloomMip>& mip_chain = fbo.getMipChain();

            upsample_shader->use();
            upsample_shader->setFloat("filterRadius", filter_radius);

            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);

            for (int i = mip_chain.size() - 1; i > 0; --i) {
                const BloomMip& mip = mip_chain[i];
                const BloomMip& next_mip = mip_chain[i - 1];

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mip.texture);

                glViewport(0, 0, next_mip.size.x, next_mip.size.y);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, next_mip.texture, 0);

                glBindVertexArray(quad_vao);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);
            }

            glDisable(GL_BLEND);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip_chain[0].texture, 0);

            upsample_shader->deactivate();
        }


        bool has_init;
        BloomFBO fbo;
        glm::ivec2 src_viewport_size;
        glm::vec2 src_viewport_size_float;
        Shader* downsample_shader;
        Shader* upsample_shader;
        unsigned int quad_vao;
};

#endif