#include "UAreaRenderer.hpp"
#include <filesystem>
#include <glad/glad.h>
#include <cmath>
#include <iostream>
#define M_PI 3.14159265358979323846 // why is this not defined on windows :)

const std::vector<CShapeVertex> Box = {
    // Bottom
    {{-1, -1, -1}},  {{-1, -1, 1}}, // Back Line
    {{1, -1, -1}}, {{1, -1, 1}},    // Front Line
    {{-1, -1, 1}}, {{1, -1, 1}},    //Left Side
    {{-1, -1, -1}}, {{1, -1, -1}},  // Right Side


    // Top
    {{-1, 1, -1}},  {{-1, 1, 1}}, // Back Line
    {{1, 1, -1}}, {{1, 1, 1}},    // Front Line
    {{-1, 1, 1}}, {{1, 1, 1}},    //Left Side
    {{-1, 1, -1}}, {{1, 1, -1}},  // Right Side

    // Sides
    {{1, -1, 1}},  {{1, 1, 1}},
    {{1, -1, -1}},  {{1, 1, -1}},
    {{-1, -1, 1}},  {{-1, 1, 1}},
    {{-1, -1, -1}},  {{-1, 1, -1}},
    
};

static std::vector<CShapeVertex> Sphere;
static std::vector<CShapeVertex> Bowl;
static std::vector<CShapeVertex> Cylinder;

const std::vector<CShapeVertex>* Shapes[SHAPES_COUNT] = {
    &Box,
    &Box,
    &Sphere,
    &Cylinder,
    &Bowl
};

const char* default_area_vtx_shader_source = "#version 330\n\
layout (location = 0) in vec3 position;\n\
uniform float y_offset;\n\
uniform mat4 gpu_ModelViewProjectionMatrix;\n\
void main()\n\
{\n\
    gl_Position = gpu_ModelViewProjectionMatrix * vec4(((position * vec3(500,500,500)) + vec3(0, y_offset, 0)), 1.0);\n\
}\
";

const char* default_area_frg_shader_source = "#version 330\n\
uniform int id;\n\
uniform vec4 modColor;\n\
out vec4 outColor;\n\
out int outPick;\n\
void main()\n\
{\n\
    outColor = modColor;\n\
    outPick = id;\n\
}\
";

void CAreaRenderer::Init() {
	//Compile Shaders
	{
	    char glErrorLogBuffer[4096];
	    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	
	    glShaderSource(vs, 1, &default_area_vtx_shader_source, NULL);
	    glShaderSource(fs, 1, &default_area_frg_shader_source, NULL);
	
	    glCompileShader(vs);
	
	    GLint status;
	    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	    if(status == GL_FALSE){
	        GLint infoLogLength;
	        glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLogLength);
	
	        glGetShaderInfoLog(vs, infoLogLength, NULL, glErrorLogBuffer);
	
	        printf("Compile failure in vertex shader:\n%s\n", glErrorLogBuffer);
	    }
	
	    glCompileShader(fs);
	
	    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
	    if(status == GL_FALSE){
	        GLint infoLogLength;
	        glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLogLength);
	
	        glGetShaderInfoLog(fs, infoLogLength, NULL, glErrorLogBuffer);
	
	        printf("Compile failure in fragment shader:\n%s\n", glErrorLogBuffer);
	    }
	
	    mShaderID = glCreateProgram();
	
	    glAttachShader(mShaderID, vs);
	    glAttachShader(mShaderID, fs);
	
	    glLinkProgram(mShaderID);
	
	    glGetProgramiv(mShaderID, GL_LINK_STATUS, &status); 
	    if(GL_FALSE == status) {
	        GLint logLen; 
	        glGetProgramiv(mShaderID, GL_INFO_LOG_LENGTH, &logLen); 
	        glGetProgramInfoLog(mShaderID, logLen, NULL, glErrorLogBuffer); 
	        printf("Shader Program Linking Error:\n%s\n", glErrorLogBuffer);
	    } 
	
	    glDetachShader(mShaderID, vs);
	    glDetachShader(mShaderID, fs);
	
	    glDeleteShader(vs);
	    glDeleteShader(fs);

	}

    glGenVertexArrays(SHAPES_COUNT, mShapeArrays);
    glGenBuffers(SHAPES_COUNT, mShapeBuffers);

    for(int shape = 0; shape < SHAPES_COUNT; shape++){
        glBindVertexArray(mShapeArrays[shape]);
        glBindBuffer(GL_ARRAY_BUFFER, mShapeBuffers[shape]);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CShapeVertex), (void*)offsetof(CShapeVertex, Position));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }


    // Generate Meshes
    // Cylinder
    float secStep = (2 * M_PI) / 16;
    for(float sector = 0.0f; sector < 16; sector += secStep){
        float x = cos(sector), z = sin(sector);
        float x1 = cos(sector+secStep), z1 = sin(sector+secStep); 

        Cylinder.push_back({.Position = {x, -1, z}});
        Cylinder.push_back({.Position = {x, 1, z}});

        Cylinder.push_back({.Position = {x, 1, z}});
        Cylinder.push_back({.Position = {x1, 1, z1}});

        Cylinder.push_back({.Position = {x, -1, z}});
        Cylinder.push_back({.Position = {x1, -1, z1}});
    }

    // Sphere
    secStep = (2 * M_PI) / 64;
    float arcStep = ((2*M_PI) / 8);
    
    for(float sector = 0.0f; sector < 64; sector += secStep){
        float x = cos(sector), z = sin(sector);
        float x1 = cos(sector+secStep), z1 = sin(sector+secStep); 

        Sphere.push_back({.Position = {x, 0, z}});
        Sphere.push_back({.Position = {x1, 0, z1}});
    }

    for(float arc = 0.0f; arc < 8; arc += arcStep){
        float r = cos(arc), t = sin(arc);
        for(float sector = 0.0f; sector < 64; sector += secStep){
            float x = cos(sector), z = sin(sector);
            float x1 = cos(sector+secStep), z1 = sin(sector+secStep); 

            Sphere.push_back({.Position = {x * t, z, r * x}});
            Sphere.push_back({.Position = {x1 * t, z1, r * x1}});
        }
    }
    
    // Bowl
    for(float sector = 0.0f; sector < 64; sector += secStep){
        float x = cos(sector), z = sin(sector);
        float x1 = cos(sector+secStep), z1 = sin(sector+secStep); 

        Bowl.push_back({.Position = {x, 0, z}});
        Bowl.push_back({.Position = {x1, 0, z1}});
    }

    for(float arc = 0.0f; arc < 8; arc += arcStep){
        float r = cos(arc), t = sin(arc);
        for(float sector = 0.0f; sector < 64; sector += secStep){
            float x = cos(sector), z = sin(sector);
            float x1 = cos(sector+secStep), z1 = sin(sector+secStep); 

            if(z > 0 || z1 > 0) continue;

            Bowl.push_back({.Position = {x * t, z, r * x}});
            Bowl.push_back({.Position = {x1 * t, z1, r * x1}});
        }
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, mShapeBuffers[BOX_BASE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CShapeVertex) * Box.size(), &Box[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, mShapeBuffers[BOX_CENTER]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CShapeVertex) * Box.size(), &Box[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, mShapeBuffers[SPHERE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CShapeVertex) * Sphere.size(), &Sphere[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, mShapeBuffers[CYLINDER]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CShapeVertex) * Cylinder.size(), &Cylinder[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, mShapeBuffers[BOWL]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CShapeVertex) * Bowl.size(), &Bowl[0], GL_STATIC_DRAW);

    mMVPUniform = glGetUniformLocation(mShaderID, "gpu_ModelViewProjectionMatrix");
    mYOffsetUniform = glGetUniformLocation(mShaderID, "y_offset");
    mPickUniform = glGetUniformLocation(mShaderID, "id");
    mColorUniform = glGetUniformLocation(mShaderID, "modColor");
    
}

CAreaRenderer::CAreaRenderer() {}

CAreaRenderer::~CAreaRenderer() {
    glDeleteVertexArrays(SHAPES_COUNT, mShapeArrays);
    glDeleteBuffers(SHAPES_COUNT, mShapeBuffers);
}

void CAreaRenderer::DrawShape(USceneCamera* camera, AreaRenderShape shape,  int32_t id, glm::mat4 transform, glm::vec4 color) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glLineWidth(2.15f);

	glm::mat4 mvp;
	mvp = camera->GetProjectionMatrix() * camera->GetViewMatrix() * transform;

    glUseProgram(mShaderID);

    // Set Uniforms
    glUniformMatrix4fv(mMVPUniform, 1, 0, (float*)&mvp[0]);
    glUniform4fv(mColorUniform, 1, (float*)&color[0]);
    glUniform1i(mPickUniform, id);

    if(shape == BOX_BASE){ // bowl is also not centered, but its y offset is the radius
        glUniform1f(mYOffsetUniform, 500.0f);
    } else {
        glUniform1f(mYOffsetUniform, 0.0f);
    }

    glBindVertexArray(mShapeArrays[shape]);
    glDrawArrays(GL_LINES, 0, Shapes[shape]->size());

    glBindVertexArray(0);
}