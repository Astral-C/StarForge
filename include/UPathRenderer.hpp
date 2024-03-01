#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <filesystem>
#include <UCamera.hpp>

typedef struct {
    glm::vec3 Position;
    glm::vec4 Color;
    glm::vec3 LeftHandle;
    glm::vec3 RightHandle;
} CPathPoint;


class CPathRenderer {
    uint32_t mShaderID;
    uint32_t mMVPUniform;
    uint32_t mPointModeUniform;
    uint32_t mIsOrthoUniform;
    uint32_t mTextureID; //single texture for points

    uint32_t mVao;
    uint32_t mVbo;

    uint32_t mPointsVao;
    uint32_t mPointsVbo;

    uint32_t mRenderPathSize;
    
public:
    bool isClosed;
    std::vector<CPathPoint> mPath;

    void UpdateData();
	void Draw(USceneCamera* Camera, glm::mat4 ReferenceFrame);

	void Init();
	CPathRenderer();
	~CPathRenderer();

};