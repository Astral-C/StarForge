#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <filesystem>
#include <UCamera.hpp>

enum AreaRenderShape {
    BOX_BASE,
    BOX_CENTER,
    SPHERE,
    CYLINDER,
    BOWL
};

class CAreaRenderer {
    uint32_t mShaderID;

    uint32_t mVaoCube, mVboCube;
    uint32_t mVaoSphere, mVboSphere;
    uint32_t mVaoCylinder, mVboCylinder;
    uint32_t mVaoBowl, mVboBowl;

public:
	
    void DrawShape(USceneCamera* Camera, AreaRenderShape shape);

	void Init();
	CAreaRenderer();
	~CAreaRenderer();

};