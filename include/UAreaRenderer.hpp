#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <filesystem>
#include <UCamera.hpp>

struct CShapeVertex {
    glm::vec3 Position;
};

enum AreaRenderShape {
    BOX_BASE,
    BOX_CENTER,
    SPHERE,
    CYLINDER,
    BOWL,
    SHAPES_COUNT
};

class CAreaRenderer {
    uint32_t mShaderID;

    uint32_t mShapeArrays[SHAPES_COUNT];
    uint32_t mShapeBuffers[SHAPES_COUNT];

    uint32_t mMVPUniform;
    uint32_t mYOffsetUniform;
    uint32_t mPickUniform;

public:
	
    void DrawShape(USceneCamera* camera, AreaRenderShape shape, int32_t id, glm::mat4 transform);

	void Init();
	CAreaRenderer();
	~CAreaRenderer();

};