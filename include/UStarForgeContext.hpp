#pragma once

#include "UCamera.hpp"

#include <vector>
#include <filesystem>
#include <memory>
#include <UGrid.hpp>
#include "DOM/GalaxyDOMNode.hpp"
#include "ModelCache.hpp"
#include "UPathRenderer.hpp"

class UStarForgeContext {
	
	std::shared_ptr<SGalaxyDOMNode> mRoot;
	std::shared_ptr<SDOMNodeBase> selected;

	std::vector<std::shared_ptr<J3DModelInstance>> mRenderables;
	CPathRenderer mPathRenderer;

	uint32_t mGizmoOperation { 0 };

	USceneCamera mCamera;
	UGrid mGrid;

	uint32_t mMainDockSpaceID;
	uint32_t mDockNodeLeftID;
	uint32_t mDockNodeUpLeftID;
	uint32_t mDockNodeDownLeftID;
	
	std::string mSelectedAddZone { "" };

	bool mOptionsOpen { false };
	bool mAboutOpen { false };

	bool bIsDockingSetUp { false };
	bool bIsFileDialogOpen { false };
	bool bIsSaveDialogOpen { false };
	bool mSetLights { false };
	bool mTextEditorActive { false };

	void RenderMainWindow(float deltaTime);
	void RenderPanels(float deltaTime);
	void RenderMenuBar();

	void SetLights();
	void LoadFromPath(std::filesystem::path filePath);

	void SaveModel(std::filesystem::path filePath);

public:
	UStarForgeContext();
	~UStarForgeContext();

	void HandleSelect();
	bool Update(float deltaTime);
	void Render(float deltaTime);
};
