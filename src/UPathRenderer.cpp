#include "UPathRenderer.hpp"
#include <filesystem>
#include <glad/glad.h>
#include <iostream>

const char* default_path_vtx_shader_source = "#version 330\n\
layout (location = 0) in vec3 position;\n\
layout (location = 1) in vec4 color;\n\
out vec4 mPath_color;\n\
uniform mat4 gpu_ModelViewProjectionMatrix;\n\
void main()\n\
{\n\
    gl_Position = gpu_ModelViewProjectionMatrix * vec4(position, 1.0);\n\
    gl_PointSize = min(16000, 16000 / gl_Position.w);\n\
    mPath_color = color;\n\
}\
";

const char* default_path_frg_shader_source = "#version 330\n\
uniform sampler2D spriteTexture;\n\
in vec4 mPath_color;\n\
uniform bool pointMode;\n\
void main()\n\
{\n\
    if(pointMode){\n\
        vec2 p = gl_PointCoord * 2.0 - vec2(1.0);\n\
        float r = sqrt(dot(p,p));\n\
        if(dot(p,p) > r){\n\
            discard;\n\
        } else {\n\
            gl_FragColor = mPath_color;\n\
        }\n\
    } else {\n\
        gl_FragColor = mPath_color;\n\
    }\n\
}\
";

void CPathRenderer::Init() {
    isClosed = false;
	//Compile Shaders
	{
	    char glErrorLogBuffer[4096];
	    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	
	    glShaderSource(vs, 1, &default_path_vtx_shader_source, NULL);
	    glShaderSource(fs, 1, &default_path_frg_shader_source, NULL);
	
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

    mMVPUniform = glGetUniformLocation(mShaderID, "gpu_ModelViewProjectionMatrix");
    mPointModeUniform = glGetUniformLocation(mShaderID, "pointMode");

    glGenVertexArrays(1, &mVao);
    glBindVertexArray(mVao);

    glGenBuffers(1, &mVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CPathPoint), (void*)offsetof(CPathPoint, Position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(CPathPoint), (void*)offsetof(CPathPoint, Color));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(CPathPoint), (void*)offsetof(CPathPoint, LeftHandle));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(CPathPoint), (void*)offsetof(CPathPoint, RightHandle));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenVertexArrays(1, &mPointsVao);
    glBindVertexArray(mPointsVao);

    glGenBuffers(1, &mPointsVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mPointsVbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CPathPoint), (void*)offsetof(CPathPoint, Position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(CPathPoint), (void*)offsetof(CPathPoint, Color));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(CPathPoint), (void*)offsetof(CPathPoint, LeftHandle));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(CPathPoint), (void*)offsetof(CPathPoint, RightHandle));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

CPathRenderer::CPathRenderer() {}

CPathRenderer::~CPathRenderer() {
    glDeleteBuffers(1, &mVbo);
    glDeleteVertexArrays(1, &mVao);

    glDeleteBuffers(1, &mPointsVbo);
    glDeleteVertexArrays(1, &mPointsVao);
}

void CPathRenderer::UpdateData(){
    std::vector<CPathPoint> points, circles;

    for(int i = 0; i < mPath.size(); i++){
        circles.push_back(mPath.at(i));
        circles.push_back((CPathPoint){mPath.at(i).RightHandle, mPath.at(i).Color, {0,0,0}, {0,0,0}});
        circles.push_back((CPathPoint){mPath.at(i).LeftHandle, mPath.at(i).Color, {0,0,0}, {0,0,0}});

        points.push_back(mPath.at(i));
        points.push_back((CPathPoint){mPath.at(i).RightHandle, mPath.at(i).Color, {0,0,0}, {0,0,0}});
        points.push_back((CPathPoint){mPath.at(i).LeftHandle, mPath.at(i).Color, {0,0,0}, {0,0,0}});
        points.push_back(mPath.at(i));
        
        if(i == mPath.size() - 1 && isClosed == false) break;

        for(float t = 0.01f; t < 1.0f; t += 0.01f){
            float p1t = (1.0f - t) * (1.0f - t) * (1.0f - t);
            float p2t = 3.0f * t * (1.0f - t) * (1.0f - t);
            float p3t = 3.0f * t * t * (1.0f - t);
            float p4t = t * t * t;
            
            float x = mPath.at(i).Position.x * p1t + mPath.at(i).RightHandle.x * p2t + mPath.at((i + 1) % mPath.size()).LeftHandle.x * p3t + mPath.at((i + 1) % mPath.size()).Position.x * p4t;
            float y = mPath.at(i).Position.y * p1t + mPath.at(i).RightHandle.y * p2t + mPath.at((i + 1) % mPath.size()).LeftHandle.y * p3t + mPath.at((i + 1) % mPath.size()).Position.y * p4t;
            float z = mPath.at(i).Position.z * p1t + mPath.at(i).RightHandle.z * p2t + mPath.at((i + 1) % mPath.size()).LeftHandle.z * p3t + mPath.at((i + 1) % mPath.size()).Position.z * p4t;
            
            points.push_back((CPathPoint){glm::vec3(x,y,z), mPath.at(i).Color, {0,0,0}, {0,0,0}});
            points.push_back((CPathPoint){glm::vec3(x,y,z), mPath.at(i).Color, {0,0,0}, {0,0,0}});
        }
        points.push_back(mPath.at((i + 1) % mPath.size()));
    }

    mRenderPathSize = points.size();

    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CPathPoint) * points.size(), &points[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, mPointsVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CPathPoint) * mPath.size() * 3, &circles[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CPathRenderer::Draw(USceneCamera *Camera, glm::mat4 ReferenceFrame) {
    glEnable(GL_PROGRAM_POINT_SIZE);
    //glEnable(GL_POINT_SPRITE);

    glLineWidth(1.5f);

	glm::mat4 mvp;
	mvp = Camera->GetProjectionMatrix() * Camera->GetViewMatrix() * ReferenceFrame;

    glUseProgram(mShaderID);

    glUniformMatrix4fv(mMVPUniform, 1, 0, (float*)&mvp[0]);

    glBindVertexArray(mPointsVao);
    glUniform1i(mPointModeUniform, GL_TRUE);
    glDrawArrays(GL_POINTS, 0, mPath.size() * 3);
    
    glBindVertexArray(0);

    glBindVertexArray(mVao);
    glUniform1i(mPointModeUniform, GL_FALSE);
    glDrawArrays(GL_LINE_STRIP, 0, mRenderPathSize);

    glBindVertexArray(0);


}