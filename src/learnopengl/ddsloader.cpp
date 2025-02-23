#include "ddsloader.hpp"

#include <glad/glad.h>

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <memory>

/*
    Can load easier and more in-depth with https://github.com/Hydroque/DDSLoader
    Because a lot of crappy, weird DDS file loader files were found online. The resources are actually VERY VERY limited.
    Written in C, can very easily port to C++ through casting mallocs (ensure your imports are correct), goto can be replaced.
    https://www.gamedev.net/forums/topic/637377-loading-dds-textures-in-opengl-black-texture-showing/
    http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/
    ^ Two examples of terrible code
    https://gist.github.com/Hydroque/d1a8a46853dea160dc86aa48618be6f9
    ^ My first look and clean up 'get it working'
    https://ideone.com/WoGThC
    ^ Improvement details
    File Structure:
      Section     Length
      ///////////////////
      FILECODE    4
      HEADER      124
      HEADER_DX10* 20  (https://msdn.microsoft.com/en-us/library/bb943983(v=vs.85).aspx)
      PIXELS      fseek(f, 0, SEEK_END); (ftell(f) - 128) - (fourCC == "DX10" ? 17 or 20 : 0)
    * the link tells you that this section isn't written unless it's a DX10 file
    Supports DXT1, DXT3, DXT5.
    The problem with supporting DX10 is you need to know what it is used for and how OpenGL would use it.
    File Byte Order:
    typedef unsigned int DWORD;           // 32bits little endian
      type   index    attribute           // description
    ///////////////////////////////////////////////////////////////////////////////////////////////
      DWORD  0        file_code;          //. always `DDS `, or 0x20534444
      DWORD  4        size;               //. size of the header, always 124 (includes PIXELFORMAT)
      DWORD  8        flags;              //. bitflags that tell you if data is present in the file
                                          //      CAPS         0x1
                                          //      HEIGHT       0x2
                                          //      WIDTH        0x4
                                          //      PITCH        0x8
                                          //      PIXELFORMAT  0x1000
                                          //      MIPMAPCOUNT  0x20000
                                          //      LINEARSIZE   0x80000
                                          //      DEPTH        0x800000
      DWORD  12       height;             //. height of the base image (largest mipmap)
      DWORD  16       width;              //. width of the base image (largest mipmap)
      DWORD  20       pitchOrLinearSize;  //. bytes per scan line in an uncompressed texture, or bytes in the top level texture for a compressed texture
                                          //     D3DX11.lib and other similar libraries unreliably or inconsistently provide the pitch, convert with
                                          //     DX* && BC*: max( 1, ((width+3)/4) ) * block-size
                                          //     *8*8_*8*8 && UYVY && YUY2: ((width+1) >> 1) * 4
                                          //     (width * bits-per-pixel + 7)/8 (divide by 8 for byte alignment, whatever that means)
      DWORD  24       depth;              //. Depth of a volume texture (in pixels), garbage if no volume data
      DWORD  28       mipMapCount;        //. number of mipmaps, garbage if no pixel data
      DWORD  32       reserved1[11];      //. unused
      DWORD  76       Size;               //. size of the following 32 bytes (PIXELFORMAT)
      DWORD  80       Flags;              //. bitflags that tell you if data is present in the file for the following 28 bytes
                                          //      ALPHAPIXELS  0x1
                                          //      ALPHA        0x2
                                          //      FOURCC       0x4
                                          //      RGB          0x40
                                          //      YUV          0x200
                                          //      LUMINANCE    0x20000
      DWORD  84       FourCC;             //. File format: DXT1, DXT2, DXT3, DXT4, DXT5, DX10. 
      DWORD  88       RGBBitCount;        //. Bits per pixel
      DWORD  92       RBitMask;           //. Bit mask for R channel
      DWORD  96       GBitMask;           //. Bit mask for G channel
      DWORD  100      BBitMask;           //. Bit mask for B channel
      DWORD  104      ABitMask;           //. Bit mask for A channel
      DWORD  108      caps;               //. 0x1000 for a texture without mipmaps
                                          //      0x401008 for a texture with mipmaps
                                          //      0x1008 for a cube map
      DWORD  112      caps2;              //. bitflags that tell you if data is present in the file
                                          //      CUBEMAP           0x200     Required for a cube map.
                                          //      CUBEMAP_POSITIVEX 0x400     Required when these surfaces are stored in a cube map.
                                          //      CUBEMAP_NEGATIVEX 0x800     ^
                                          //      CUBEMAP_POSITIVEY 0x1000    ^
                                          //      CUBEMAP_NEGATIVEY 0x2000    ^
                                          //      CUBEMAP_POSITIVEZ 0x4000    ^
                                          //      CUBEMAP_NEGATIVEZ 0x8000    ^
                                          //      VOLUME            0x200000  Required for a volume texture.
      DWORD  114      caps3;              //. unused
      DWORD  116      caps4;              //. unused
      DWORD  120      reserved2;          //. unused
*/

GLuint texture_loadDDS(const std::string path, const std::string &directory, bool gammaCorrection) {
    // Initialize variables to be used
    unsigned char* header = nullptr;
    unsigned char* buffer = nullptr;

    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int mipMapCount = 1; // Ensure at least one mipmap level

    unsigned int blockSize = 0;
    unsigned int format = 0;

    GLuint tid = 0;

    // Open the DDS file for binary reading and get file size
    FILE* f = fopen((directory + "/" + path).c_str(), "rb");
    if (!f) {
        std::cerr << "Failed to open DDS file: " << directory + "/" + path << std::endl;
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Allocate memory for header and read the first 128 bytes
    header = (unsigned char*)malloc(128);
    if (!header || fread(header, 1, 128, f) != 128) {
        std::cerr << "Failed to read DDS header." << std::endl;
        fclose(f);
        free(header);
        return 0;
    }

    // Verify the `DDS ` signature
    if (memcmp(header, "DDS ", 4) != 0) {
        std::cerr << "Invalid DDS signature." << std::endl;
        fclose(f);
        free(header);
        return 0;
    }

    // Extract height, width, and mipMapCount from header
    height = *(unsigned int*)&header[12];
    width = *(unsigned int*)&header[16];
    mipMapCount = *(unsigned int*)&header[28];
    if (mipMapCount == 0) mipMapCount = 1; // Ensure at least one mipmap level

    // Determine format and block size based on FourCC code
    if (header[84] == 'D') {
        switch (header[87]) {
            case '1': // DXT1
                format = gammaCorrection ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                blockSize = 8;
                break;
            case '3': // DXT3
                format = gammaCorrection ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT : GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
                blockSize = 16;
                break;
            case '5': // DXT5
                format = gammaCorrection ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                blockSize = 16;
                break;
            default:
                std::cerr << "Unsupported DDS format: " << header[84] << header[85] << header[86] << header[87] << std::endl;
                fclose(f);
                free(header);
                return 0;
        }
    } else {
        std::cerr << "Unsupported DDS FourCC: " << header[84] << header[85] << header[86] << header[87] << std::endl;
        fclose(f);
        free(header);
        return 0;
    }

    // Allocate memory for pixel data and read the rest of the file
    long pixel_data_size = file_size - 128;
    buffer = (unsigned char*)malloc(pixel_data_size);
    if (!buffer || fread(buffer, 1, pixel_data_size, f) != (size_t)pixel_data_size) {
        std::cerr << "Failed to read DDS pixel data." << std::endl;
        fclose(f);
        free(header);
        free(buffer);
        return 0;
    }

    fclose(f);

    // Generate and bind OpenGL texture
    glGenTextures(1, &tid);
    if (tid == 0) {
        std::cerr << "Failed to generate OpenGL texture." << std::endl;
        free(header);
        free(buffer);
        return 0;
    }


    glBindTexture(GL_TEXTURE_2D, tid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    GLfloat value, max_anisotropy = 8.0f; /* don't exceed this value...*/
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, & value);
    value = (value > max_anisotropy) ? max_anisotropy : value;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, value);

    // Upload mipmap levels
    unsigned int offset = 0;
    unsigned int size = 0;
    unsigned int w = width;
    unsigned int h = height;
    unsigned int actualMipMapCount = 0;

    for (unsigned int i = 0; i < mipMapCount; ++i) {
        // Ensure width and height are at least 1
        unsigned int mipWidth = (w > 1) ? w : 1;
        unsigned int mipHeight = (h > 1) ? h : 1;

        size = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * blockSize;

        // Check if there is enough data for the current mipmap level
        if (offset + size > (unsigned int)pixel_data_size) {
            std::cerr << "Insufficient pixel data for mipmap level " << i << std::endl;
            break;
        }

        glCompressedTexImage2D(GL_TEXTURE_2D, i, format, mipWidth, mipHeight, 0, size, buffer + offset);
        offset += size;
        actualMipMapCount++;
        w /= 2;
        h /= 2;
    }

    // Set the actual mipmap level
    if (actualMipMapCount > 1) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, actualMipMapCount - 1);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    // Clean up allocated memory
    free(header);
    free(buffer);

    return tid;
}
