#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <filesystem>
#include <UCamera.hpp>
#include <DOM/GalaxyDOMNode.hpp>
#include <DOM/DOMNodeSerializable.hpp>

typedef struct {
    glm::vec3 Position;
    int32_t SpriteSize;
    int32_t Texture;
    int32_t SizeFixed;
    int32_t ID;
} CPointSprite;

class CPointSpriteManager {
    uint32_t mShaderID;
    uint32_t mMVPUniform;
	int mBillboardResolution, mTextureCount;
    uint32_t mTextureID;

    uint32_t mVao;
    uint32_t mVbo;

	std::vector<CPointSprite> mBillboards;
public:
	
	void SetBillboardTexture(std::filesystem::path ImagePath, int TextureIndex);
	void Draw(USceneCamera* Camera);

    void UpdateData(std::shared_ptr<SGalaxyDOMNode> Root);
	void Init(int BillboardResolution, int BillboardImageCount);

	CPointSpriteManager();
	~CPointSpriteManager();

};