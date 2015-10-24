
#pragma once
#include "GLCamera.h"
#include "GLProgram.h"
#include "GLModel.h"
#include "GLLighting.h"

class GL3DProgram : public GLProgram {

	//All the attributes used for drawing voxels
	//Not all are valid on all platforms
	GLint attributeIndexTexture;
	GLint attributeIndexVertex;
	GLint attributeIndexNormal;
	GLint attributeIndexPosition;
    GLint uniformMaterialReflectivity;
    
    // Modelview uniform
    GLint uniformMatrixModelView;
    GLint uniformMatrixModelViewProjection;

public:
	GL3DProgram(GLCommonShaderFile * commonShader, string vertexShaderPath, string fragmentShaderPath);

	void UseProgram(void) override;

	GLLighting Lights;
	GLModel Model;
	GLCamera Camera;
    
	//I've elected for manual management of textures
	//since the material manager concept wasn't really doing anything for me
	//GLMaterialManager Materials;

	//All the attributes used for drawing voxels
	//Not all are valid on all platforms
	const GLint AttributePosition();
	const GLint AttributeTexture();
	const GLint AttributeVertex();
	const GLint AttributeNormal();
    const GLint UniformMaterialReflectivity();
    
    // Apply combined matrices
    void Apply();
};