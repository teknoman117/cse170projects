#ifndef __CAMERA_PARAMETERS_HPP__
#define __CAMERA_PARAMETERS_HPP__

#include <project2/common.hpp>

struct CameraParameters
{
    struct ShaderParameters
    {
        glm::mat4 V;
        glm::mat4 P;
        glm::mat4 VP;

        glm::vec3 CameraPosition;
        float     padding;
        
        glm::vec4 frustumPlanes[6];
        glm::vec4 viewport;
    } shaderParameters;

    GLuint    buffer;

    GLint     width;
    GLint     height;
    float     aspect;
    float     yFov;

    glm::vec3 up;
    glm::vec3 position;
    glm::vec2 rotation;
    glm::vec4 viewport;

    glm::quat RotationAsQuaternion() const
    {
        return glm::angleAxis(rotation.y, glm::vec3(0,1,0)) * 
               glm::angleAxis(rotation.x, glm::vec3(1,0,0));
    }

    void Update()
    {
        // Compute camera parameters
        glm::vec3 center = position + RotationAsQuaternion()*glm::vec3(0,0,-1);

        // Compute OpenGL matrices
        shaderParameters.CameraPosition = position;
        shaderParameters.viewport       = viewport;
        shaderParameters.V              = glm::lookAt(position, center, up);
        shaderParameters.P              = glm::perspective(yFov, aspect, 0.1f, 70000.f);
        shaderParameters.VP             = shaderParameters.P * shaderParameters.V;

        // Extract frustum planes from view projection matrix (increase vertical fov a bit)
        glm::mat4 cullVP = glm::perspective(yFov+(2.5f/18.f * glm::pi<float>()), aspect, 0.1f, 70000.f) * shaderParameters.V;
        shaderParameters.frustumPlanes[0] = row(cullVP, 3) + row(cullVP, 0); // left
        shaderParameters.frustumPlanes[1] = row(cullVP, 3) - row(cullVP, 0); // right
        shaderParameters.frustumPlanes[2] = row(cullVP, 3) + row(cullVP, 1); // bottom
        shaderParameters.frustumPlanes[3] = row(cullVP, 3) - row(cullVP, 1); // top
        shaderParameters.frustumPlanes[4] = row(cullVP, 3) + row(cullVP, 2); // far
        shaderParameters.frustumPlanes[5] = row(cullVP, 3) - row(cullVP, 2); // near

        // Bind the UBO
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, buffer);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderParameters), (GLvoid *) this);
    }

    CameraParameters()
        : yFov(75.0f/180.f * glm::pi<float>()), up(glm::vec3(0,1,0))
    {
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, buffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderParameters), (GLvoid *) this, GL_STREAM_DRAW);
    }
    ~CameraParameters()
    {
        glDeleteBuffers(1, &buffer);
    }
};

#endif
