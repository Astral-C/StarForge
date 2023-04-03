#pragma once

#include "UCamera.hpp"

#include <vector>
#include <filesystem>
#include <memory>
#include <UGrid.hpp>
#include "DOM/GalaxyDOMNode.hpp"

namespace bStream { class CStream; }
class J3DModelData;
class J3DMaterial;
class J3DModelInstance;


class UStarForgeContext {
	
	std::shared_ptr<SGalaxyDOMNode> mRoot;

	USceneCamera mCamera;
	UGrid mGrid;

	uint32_t mMainDockSpaceID;
	uint32_t mDockNodeLeftID;
	uint32_t mDockNodeUpLeftID;
	uint32_t mDockNodeDownLeftID;
	
	bool bIsDockingSetUp { false };
	bool bIsFileDialogOpen { false };
	bool bIsSaveDialogOpen { false };
	bool mSetLights { false };
	bool mTextEditorActive { false };

	void RenderMainWindow(float deltaTime);
	void RenderPanels(float deltaTime);
	void RenderMenuBar();

	void OpenModelCB();
	void SaveModelCB();

	void SetLights();
	void LoadFromPath(std::filesystem::path filePath);

	void SaveModel(std::filesystem::path filePath);

public:
	UStarForgeContext();
	~UStarForgeContext() {}

	bool Update(float deltaTime);
	void Render(float deltaTime);
};
