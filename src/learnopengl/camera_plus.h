#ifndef CAMERA_PLUS_H
#define CAMERA_PLUS_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>


    
#define CAM_INIT_YAW -90.0f
#define CAM_INIT_PITCH 0.0f
#define CAM_INIT_SPEED 5.5f
#define CAM_INIT_SENSITIVITY 0.1f
#define CAM_INIT_ZOOM 70.0f

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    CAMERA_FORWARD,
    CAMERA_BACKWARD,
    CAMERA_LEFT,
    CAMERA_RIGHT,
    CAMERA_UP,
    CAMERA_DOWN
};

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera {
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 SightFront;
    glm::vec3 BodyFront;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = CAM_INIT_YAW, float pitch = CAM_INIT_PITCH):
           SightFront(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(CAM_INIT_SPEED), MouseSensitivity(CAM_INIT_SENSITIVITY), Zoom(CAM_INIT_ZOOM) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : 
           SightFront(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(CAM_INIT_SPEED), MouseSensitivity(CAM_INIT_SENSITIVITY), Zoom(CAM_INIT_ZOOM) {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + SightFront, Up);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == CAMERA_FORWARD)
            Position += BodyFront * velocity;
        if (direction == CAMERA_BACKWARD)
            Position -= BodyFront * velocity;
        if (direction == CAMERA_LEFT)
            Position -= Right * velocity;
        if (direction == CAMERA_RIGHT)
            Position += Right * velocity;
        if (direction == CAMERA_UP)
            Position += WorldUp * velocity;
        if (direction == CAMERA_DOWN)
            Position -= WorldUp * velocity;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch) {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset) {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > CAM_INIT_ZOOM)
            Zoom = CAM_INIT_ZOOM;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors() {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        // The direction camera looking at
        SightFront = glm::normalize(front);
        // The direction camera face to. Asume that the body is on the xz plane, we won't have y value
        BodyFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(SightFront, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, SightFront));
    }

};
#endif