#ifndef __CAMERA_PARAMETERS_HPP__
#define __CAMERA_PARAMETERS_HPP__

#include <project2/common.hpp>
#include <project2/geometry/frustum.hpp>

struct CameraParameters
{
    struct ShaderParameters
    {
        glm::mat4 V;
        glm::mat4 P;
        glm::mat4 VP;

        glm::vec3 CameraPosition;
        glm::vec4 frustumPlanes[6];
    } shaderParameters;

    GLuint    buffer;

    GLint     width;
    GLint     height;
    float     aspect;
    float     yFov;

    glm::vec3 up;
    glm::vec3 position;
    glm::vec2 rotation;

    glm::quat RotationAsQuaternion() const
    {
        return glm::angleAxis(rotation.y, glm::vec3(0,1,0)) * 
               glm::angleAxis(rotation.x, glm::vec3(1,0,0));
    }

    void Update()
    {
        // Compute camera parameters
        glm::vec3 center = position + RotationAsQuaternion()*glm::vec3(0,0,-1);

        // Build frustum for culling the terrain
        Frustum cameraFrustum;
        cameraFrustum.SetCameraProperties(yFov + (25.f/180.f * glm::pi<float>()), aspect, 0.1f, 70000.f);
        cameraFrustum.SetCameraOrientation(position, center, up);

        // Extract planes
        for(int i = 0; i < 6; i++)
            cameraFrustum.ExtractPlane(i, shaderParameters.frustumPlanes[i]);

        // Compute OpenGL matrices
        shaderParameters.CameraPosition = position;
        shaderParameters.P              = glm::perspective(yFov, aspect, 0.01f, 20000.f);
        shaderParameters.V              = glm::lookAt(position, center, up);
        shaderParameters.VP             = shaderParameters.P * shaderParameters.V;

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
