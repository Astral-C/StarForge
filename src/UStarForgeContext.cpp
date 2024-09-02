#include "UStarForgeContext.hpp"

#include "util/UUIUtil.hpp"

#include <J3D/J3DModelLoader.hpp>
#include <J3D/Data/J3DModelData.hpp>
#include <J3D/Material/J3DUniformBufferObject.hpp>
#include <J3D/Rendering/J3DLight.hpp>
#include <J3D/Data/J3DModelInstance.hpp>
#include <J3D/Picking/J3DPicking.hpp>
#include <J3D/Rendering/J3DRendering.hpp>

#include "../lib/ImGuiFileDialog/ImGuiFileDialog.h"
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <bstream.h>
#include <ResUtil.hpp>
#include "ImGuizmo.h"
#include <fmt/format.h>

#include "DOM/ZoneDOMNode.hpp"
#include "DOM/ScenarioDOMNode.hpp"
#include "DOM/ObjectDOMNode.hpp"
#include "DOM/PathDOMNode.hpp"
#include "DOM/AreaObjectDOMNode.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#include "IconsForkAwesome.h"

void GalaxySort(J3D::Rendering::RenderPacketVector& packets) {
    std::sort(
        packets.begin(),
        packets.end(),
        [](const J3DRenderPacket& a, const J3DRenderPacket& b) -> bool {
			uint8_t aSortKey = static_cast<uint8_t>((a.SortKey & 0x800000) >> 23);
			uint8_t bSortKey = static_cast<uint8_t>((b.SortKey & 0x800000) >> 23);
			if(aSortKey != bSortKey){
				return aSortKey > bSortKey;
			} else {
            	return a.Material->Name < b.Material->Name;
			}
        }
    );
}

UStarForgeContext::~UStarForgeContext(){
	//J3DRendering::Cleanup();
	mRenderables.erase(mRenderables.begin(), mRenderables.end());
	ModelCache.clear();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glDeleteFramebuffers(1, &mFbo);
	glDeleteRenderbuffers(1, &mRbo);
	glDeleteTextures(1, &mViewTex);
	glDeleteTextures(1, &mPickTex);
}

UStarForgeContext::UStarForgeContext(){
	mGrid.Init();
	mProjects.Init();
	mAreaRenderer.Init();
	srand(time(0));

	auto objectDBPath = std::filesystem::current_path() / "res" / "objectdb.json";
	if(std::filesystem::exists(objectDBPath)){
		std::ifstream objectDBStream(objectDBPath);
		SObjectDOMNode::LoadObjectDB(nlohmann::json::parse(objectDBStream));
	}

	ImGuiIO& io = ImGui::GetIO();
	
	if(std::filesystem::exists((std::filesystem::current_path() / "res" / "NotoSansJP-Regular.otf"))){
		io.Fonts->AddFontFromFileTTF((std::filesystem::current_path() / "res" / "NotoSansJP-Regular.otf").string().c_str(), 16.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	}

	if(std::filesystem::exists((std::filesystem::current_path() / "res" / "forkawesome.ttf"))){
		static const ImWchar icons_ranges[] = { ICON_MIN_FK, ICON_MAX_16_FK, 0 };
		ImFontConfig icons_config; 
		icons_config.MergeMode = true; 
		icons_config.PixelSnapH = true; 
		icons_config.GlyphMinAdvanceX = 16.0f;
		io.Fonts->AddFontFromFileTTF((std::filesystem::current_path() / "res" / "forkawesome.ttf").string().c_str(), icons_config.GlyphMinAdvanceX, &icons_config, icons_ranges );
	}
	
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	Options.LoadOptions();

	mGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;

	J3D::Rendering::SetSortFunction(GalaxySort);

	mRoot = std::make_shared<SGalaxyDOMNode>();
}

bool UStarForgeContext::Update(float deltaTime) {
	mCamera.Update(deltaTime);

	if(ImGui::IsKeyPressed(ImGuiKey_1)){
		mGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	}

	if(ImGui::IsKeyPressed(ImGuiKey_2)){
		mGizmoOperation = ImGuizmo::OPERATION::ROTATE;
	}

	if(ImGui::IsKeyPressed(ImGuiKey_3)){
		mGizmoOperation = ImGuizmo::OPERATION::SCALE;
	}

	if(ImGui::IsKeyPressed(ImGuiKey_Escape)){
		selected = nullptr;
	}

	if(ImGui::IsKeyPressed(ImGuiKey_O)){
		mCamera.ToggleOrtho();
	}

	return true;
}

void UStarForgeContext::Render(float deltaTime) {
	ImGuiIO& io = ImGui::GetIO();

	RenderMenuBar();
	
	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

	ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoDockingInCentralNode;
	mMainDockSpaceID = ImGui::DockSpaceOverViewport(0, mainViewport, dockFlags);
	
	if(!bIsDockingSetUp){

		glGenFramebuffers(1, &mFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, mFbo);

		glGenRenderbuffers(1, &mRbo);
		glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);

		glGenTextures(1, &mViewTex);
		glGenTextures(1, &mPickTex);

		glBindTexture(GL_TEXTURE_2D, mViewTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		glBindTexture(GL_TEXTURE_2D, mPickTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 1280, 720, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mViewTex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mPickTex, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRbo);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		ImGui::DockBuilderRemoveNode(mMainDockSpaceID); // clear any previous layout
		ImGui::DockBuilderAddNode(mMainDockSpaceID, dockFlags | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(mMainDockSpaceID, mainViewport->Size);


		mDockNodeLeftID = ImGui::DockBuilderSplitNode(mMainDockSpaceID, ImGuiDir_Left, 0.20f, nullptr, &mMainDockSpaceID);
		mDockNodeDownLeftID = ImGui::DockBuilderSplitNode(mDockNodeLeftID, ImGuiDir_Down, 0.5f, nullptr, &mDockNodeUpLeftID);


		ImGui::DockBuilderDockWindow("mainWindow", mDockNodeUpLeftID);
		ImGui::DockBuilderDockWindow("zoneView", ImGui::DockBuilderSplitNode(mDockNodeUpLeftID, ImGuiDir_Down, 0.5f, nullptr, nullptr));
		ImGui::DockBuilderDockWindow("detailWindow", mDockNodeDownLeftID);
		ImGui::DockBuilderDockWindow("viewportWindow", mMainDockSpaceID);

		ImGui::DockBuilderFinish(mMainDockSpaceID);
		bIsDockingSetUp = true;
	}


	ImGuiWindowClass mainWindowOverride;
	mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&mainWindowOverride);
	
	ImGui::Begin("mainWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	ImGui::Text("Scenarios");
		ImGui::SameLine();
        ImGui::Text(ICON_FK_PLUS_CIRCLE);
		if(ImGui::IsItemClicked(ImGuiMouseButton_Left) && mRoot->GetGalaxyLoaded()){
			std::shared_ptr<SScenarioDOMNode> scenarioNode = std::make_shared<SScenarioDOMNode>(mRoot);
			mRoot->AddChild(scenarioNode);
		}

		ImGui::Separator();
		mRoot->RenderScenarios(selected);

	ImGui::End();

	ImGui::SetNextWindowClass(&mainWindowOverride);
	ImGui::Begin("zoneView", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		ImGui::Text("Zones");
		ImGui::SameLine();
        ImGui::Text(ICON_FK_PLUS_CIRCLE);
        
		if(ImGui::IsItemClicked(ImGuiMouseButton_Left) && mRoot->GetGalaxyLoaded()){
			// Add Zone code goes here. Should be a call to Galaxy->AddZone
			ImGui::OpenPopup("addZoneDialog");
		}

		ImGui::Separator();
		mRoot->RenderZones(selected);

		bool addZoneDialogOpen = true;
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x / 8, ImGui::GetMainViewport()->Size.y / 3.45));

		if(ImGui::BeginPopupModal("addZoneDialog", &addZoneDialogOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)){
			float windowWidth = ImGui::GetContentRegionAvail().x;
			float textWidth = ImGui::CalcTextSize("Zone Archives").x;

			ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
	
			ImGui::Text("Zone Archives");
			
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.98f);
			ImGui::SetCursorPosX(0.2f);
			ImGui::BeginListBox("##zoneArchives");
				for (const auto & entry : std::filesystem::directory_iterator(std::filesystem::path(Options.mRootPath) / "files" / "StageData")){
					if(entry.path().string().find(".arc") != std::string::npos){
						if(ImGui::Selectable(entry.path().stem().string().c_str())){
							mRoot->AddZone(entry.path());

							ImGui::CloseCurrentPopup();
						}
					}
				}
			ImGui::EndListBox();

			textWidth = ImGui::CalcTextSize(ICON_FK_WINDOW_CLOSE " Cancel").x;
			ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
			ImGui::Text(ICON_FK_WINDOW_CLOSE " Cancel");
			if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();

		}
	ImGui::End();

	ImGui::SetNextWindowClass(&mainWindowOverride);


	ImGui::Begin("detailWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		if(selected != nullptr){
			switch (selected->GetNodeType())
			{
			case EDOMNodeType::Scenario:
				ImGui::Text("Scenario Properties");
				break;

			case EDOMNodeType::Zone:
				ImGui::Text("Zone Properties");
				break;

			case EDOMNodeType::Object:
				ImGui::Text("Object Properties");
				break;
			
			default:
				ImGui::Text("Selection Properties");
				break;
			}
		} else {
			ImGui::Text("Properties");
		}

		ImGui::Separator();
		if(selected != nullptr) selected->RenderDetailsUI();
	ImGui::End();

		
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));

	ImGui::SetNextWindowClass(&mainWindowOverride);
	ImGui::Begin("viewportWindow");

		std::string markDelete = "";

		ImGui::BeginTabBar("openedGalaxies", ImGuiTabBarFlags_Reorderable);
		for(auto [galaxyName, galaxy] : mOpenGalaxies){
			bool shouldClose = true;
			if(ImGui::BeginTabItem(galaxyName.c_str(), &shouldClose)){
				if(galaxy != mRoot){
					mRoot = galaxy;
				}
				ImGui::EndTabItem();
			}
			if(!shouldClose) markDelete = galaxyName;
		}
		ImGui::EndTabBar();


		ImVec2 winSize = ImGui::GetContentRegionAvail();
		ImVec2 cursorPos = ImGui::GetCursorScreenPos();

		glBindFramebuffer(GL_FRAMEBUFFER, mFbo);

		if(winSize.x != mPrevWinWidth || winSize.y != mPrevWinHeight){
			std::cout << "Regenerating textures..." << std::endl;
			glDeleteTextures(1, &mViewTex);
			glDeleteTextures(1, &mPickTex);
			glDeleteRenderbuffers(1, &mRbo);

			glGenRenderbuffers(1, &mRbo);
			glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (uint32_t)winSize.x, (uint32_t)winSize.y);

			glGenTextures(1, &mViewTex);
			glGenTextures(1, &mPickTex);

			glBindTexture(GL_TEXTURE_2D, mViewTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (uint32_t)winSize.x, (uint32_t)winSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glBindTexture(GL_TEXTURE_2D, mPickTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, (uint32_t)winSize.x, (uint32_t)winSize.y, 0, GL_RED_INTEGER, GL_INT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mViewTex, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mPickTex, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRbo);

			GLenum attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, attachments);

			assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		}
		
		glViewport(0, 0, (uint32_t)winSize.x, (uint32_t)winSize.y);

		
		glClearColor(0.100f, 0.261f, 0.402f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int32_t unused = 0;
		glClearTexImage(mPickTex, 0, GL_RED_INTEGER, GL_INT, &unused);

		mPrevWinWidth = winSize.x;
		mPrevWinHeight = winSize.y;
		
		glm::mat4 projection, view;
		projection = mCamera.GetProjectionMatrix();
		view = mCamera.GetViewMatrix();

		//if(!mSetLights) SetLights();
		J3DUniformBufferObject::SetProjAndViewMatrices(projection, view);
		
		//Render Models here
		
		mRenderables.clear();
		mRoot->Render(mRenderables, deltaTime);


		J3D::Rendering::RenderPacketVector packets = J3D::Rendering::SortPackets(mRenderables, mCamera.GetPosition());
		J3D::Rendering::Render(deltaTime, view, projection, packets);

		// Combine these two into one

		for(std::shared_ptr<SPathDOMNode> path : mRoot->GetChildrenOfType<SPathDOMNode>(EDOMNodeType::Path)){
			std::shared_ptr<SZoneDOMNode> zone = path->GetParentOfType<SZoneDOMNode>(EDOMNodeType::Zone).lock();
			if(zone->isVisible()) path->Render(&mCamera, zone->mTransform);
		}

		for(std::shared_ptr<SAreaObjectDOMNode> area : mRoot->GetChildrenOfType<SAreaObjectDOMNode>(EDOMNodeType::AreaObject)){
			std::shared_ptr<SZoneLayerDOMNode> layer = area->GetParentOfType<SZoneLayerDOMNode>(EDOMNodeType::ZoneLayer).lock();
			if(layer->GetVisible()) area->Render(&mCamera, &mAreaRenderer, layer->GetParentOfType<SZoneDOMNode>(EDOMNodeType::Zone).lock()->mTransform, mRoot->GetGame());
		}

		cursorPos = ImGui::GetCursorScreenPos();
		ImGui::Image(reinterpret_cast<void*>(static_cast<uintptr_t>(mViewTex)), winSize, {0.0f, 1.0f}, {1.0f, 0.0f});

		if(ImGui::IsItemClicked(0) && !ImGuizmo::IsOver()){
			J3D::Picking::RenderPickingScene(view, projection, packets);
			ImVec2 mousePos = ImGui::GetMousePos();

			// Check picking FB for paths, areas, billboards, etc
			// Exit early if we found a selection here


			// Check picking for J3DUltra 
			uint16_t modelID = std::get<0>(J3D::Picking::Query((uint32_t)mousePos.x - (uint32_t)cursorPos.x, (uint32_t)mousePos.y - (uint32_t)cursorPos.y));

			std::cout << "Readback model id at " << (uint32_t)mousePos.x - (uint32_t)cursorPos.x << "," << (uint32_t)mousePos.y - (uint32_t)cursorPos.y << 
				" is " << modelID << std::endl;
			for(auto object : mRoot->GetChildrenOfType<SObjectDOMNode>(EDOMNodeType::Object)){
				if(object->GetModel() != nullptr && object->GetModel()->GetModelId()== modelID){
					std::cout << "Selected model " << object->GetName() << std::endl;
					selected = object;
					break;
				}
			}
		}

		ImGuizmo::BeginFrame();
		ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
		ImGuizmo::SetRect(cursorPos.x, cursorPos.y, winSize.x, winSize.y);

		if(selected != nullptr){
			//TODO: make this a switch please
			if(selected->IsNodeType(EDOMNodeType::Object)){
				std::shared_ptr<SObjectDOMNode> object = std::static_pointer_cast<SObjectDOMNode>(selected);
				glm::mat4 zoneTransform = object->GetParentOfType<SZoneDOMNode>(EDOMNodeType::Zone).lock()->mTransform;
				glm::mat4 transform = (zoneTransform * object->mTransform), delta = glm::identity<glm::mat4>();

				if(ImGuizmo::Manipulate(&mCamera.GetViewMatrix()[0][0], &mCamera.GetProjectionMatrix()[0][0], (ImGuizmo::OPERATION)mGizmoOperation, ImGuizmo::WORLD, &transform[0][0], &delta[0][0])){
					object->mTransform = glm::inverse(zoneTransform) * transform;
				}
			} else if(selected->IsNodeType(EDOMNodeType::AreaObject)){
				std::shared_ptr<SAreaObjectDOMNode> object = std::static_pointer_cast<SAreaObjectDOMNode>(selected);
				glm::mat4 zoneTransform = object->GetParentOfType<SZoneDOMNode>(EDOMNodeType::Zone).lock()->mTransform;
				glm::mat4 transform = (zoneTransform * object->mTransform), delta = glm::identity<glm::mat4>();

				if(ImGuizmo::Manipulate(&mCamera.GetViewMatrix()[0][0], &mCamera.GetProjectionMatrix()[0][0], (ImGuizmo::OPERATION)mGizmoOperation, ImGuizmo::WORLD, &transform[0][0], &delta[0][0])){
					object->mTransform = glm::inverse(zoneTransform) * transform;
				}
			}  else if(selected->IsNodeType(EDOMNodeType::PathPoint)) {
				std::shared_ptr<SPathPointDOMNode> pathpoint = std::static_pointer_cast<SPathPointDOMNode>(selected);
				std::shared_ptr<SPathDOMNode> path = pathpoint->GetParentOfType<SPathDOMNode>(EDOMNodeType::Path).lock();
				glm::mat4 zoneTransform = pathpoint->GetParentOfType<SZoneDOMNode>(EDOMNodeType::Zone).lock()->mTransform;
				glm::mat4 transform, delta = glm::identity<glm::mat4>();

				if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl)){
					transform = (zoneTransform * glm::translate(glm::identity<glm::mat4>(), pathpoint->GetLeftHandle()));
				} else if(ImGui::IsKeyDown(ImGuiKey_LeftShift)){
					transform = (zoneTransform * glm::translate(glm::identity<glm::mat4>(), pathpoint->GetRightHandle()));
				} else {
					transform = (zoneTransform * pathpoint->mTransform);
				}

				if(ImGuizmo::Manipulate(&mCamera.GetViewMatrix()[0][0], &mCamera.GetProjectionMatrix()[0][0], (ImGuizmo::OPERATION)mGizmoOperation, ImGuizmo::WORLD, &transform[0][0], &delta[0][0])){
					glm::mat4 out = glm::inverse(zoneTransform) * transform;
				
					if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl)){
						pathpoint->SetLeftHandle(out[3]);
					} else if(ImGui::IsKeyDown(ImGuiKey_LeftShift)){
						pathpoint->SetRightHandle(out[3]);
					} else {
						pathpoint->mTransform = out;
					}
				
					path->Update();
				}

			} else if (selected->IsNodeType(EDOMNodeType::Zone)){
				std::shared_ptr<SZoneDOMNode> zone = std::static_pointer_cast<SZoneDOMNode>(selected);
				ImGuizmo::Manipulate(&mCamera.GetViewMatrix()[0][0], &mCamera.GetProjectionMatrix()[0][0], (ImGuizmo::OPERATION)mGizmoOperation, ImGuizmo::WORLD, &zone->mTransform[0][0]);
			}
		}

		glm::mat4 viewMtx = mCamera.GetViewMatrix();
		ImVec4 forward = {FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX}, up = {FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX};
		// [veebs]: Fix this, imguizmo update broke it
		//ImGuizmo::ViewManipulate(&viewMtx[0][0], 64, ImVec2(mainViewport->Size.x - 74, mainViewport->Size.y - 74), ImVec2(64, 64), ImColor(ImVec4(0.35,0.2,0.35,0.35)), forward, up);

		if(forward.x != FLT_MAX && forward.y != FLT_MAX && forward.z != FLT_MAX){
			mCamera.SetForward(glm::vec3(forward.x, forward.y, forward.z));
			mCamera.SetUp(glm::vec3(up.x, up.y, up.z));
		}


		if(markDelete != ""){

			mOpenGalaxies.erase(markDelete);

			if(!std::filesystem::exists("./res/thumb")){
				std::filesystem::create_directory("./res/thumb");
			}

			int wsx = (int)winSize.x;
			int wsy = (int)winSize.y;

			unsigned char* imgData = new unsigned char[wsx * wsy * 4]{0};
			unsigned char* imgDataScaled = new unsigned char[84 * 64 *4] {0};

			glReadPixels(0, 0, wsx, wsy, GL_RGBA, GL_UNSIGNED_BYTE, imgData);

			stbir_resize_uint8_linear(imgData, wsx, wsy, 0, imgDataScaled, 84, 64, 0, STBIR_RGBA_NO_AW);

			stbi_write_png(fmt::format("./res/thumb/{}.png", markDelete).c_str(), 84, 64, 4,  imgDataScaled, 84 * 4);
			delete imgData;
			delete imgDataScaled;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ImGui::End();
	ImGui::PopStyleVar();

	//mGrid.Render(mCamera.GetPosition(), mCamera.GetProjectionMatrix(), mCamera.GetViewMatrix());
}

void UStarForgeContext::RenderMainWindow(float deltaTime) {


}

void UStarForgeContext::RenderMenuBar() {
	mOptionsOpen = false;
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem(ICON_FK_FOLDER_OPEN " Open...")) {
			//bIsFileDialogOpen = true;
			mGalaxySelectOpen = true;
		}
		if (ImGui::MenuItem(ICON_FK_FLOPPY_O " Save...")) {
			if(mRoot != nullptr){
				mRoot->SaveGalaxy();
			}
		}

		ImGui::Separator();
		ImGui::MenuItem(ICON_FK_WINDOW_CLOSE " Close");

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Edit")) {
		if(ImGui::MenuItem(ICON_FK_COG " Settings")){
			mOptionsOpen = true;
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Projects")) {
		mProjectManagerOpen = true;
		if(mRoot != nullptr){
			mRoot = std::make_shared<SGalaxyDOMNode>(); // delete by disconnecting root and setting it to a blank galaxy
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu(ICON_FK_QUESTION_CIRCLE)) {
		if(ImGui::MenuItem("About")){
			mAboutOpen = true;
		}
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();

	if (bIsFileDialogOpen) {
		IGFD::FileDialogConfig config;
		config.path = Options.mRootPath.string() == "" ? "." : (Options.mRootPath / "files" / "StageData" / ".").string();
		ImGuiFileDialog::Instance()->OpenDialog("OpenGalaxyDialog", "Choose Stage Directory", nullptr, config);
	}

	if (ImGuiFileDialog::Instance()->Display("OpenGalaxyDialog")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			std::string FilePath = ImGuiFileDialog::Instance()->GetCurrentPath();
			std::cout << FilePath << std::endl;

			try {
				selected = nullptr;
				if(mRoot == nullptr) mRoot = std::make_shared<SGalaxyDOMNode>();
				// TODO: add way to set galaxy type
				// copy from cammie again? or infer based on root structure
				if(!mRoot->LoadGalaxy(FilePath, std::filesystem::exists(Options.mRootPath / "files" / "SystemData" / "ObjNameTable.arc") ? EGameType::SMG2 : EGameType::SMG1)){
					ImGui::OpenPopup("Galaxy Load Error");
				} else {
					ModelCache.erase(ModelCache.begin(), ModelCache.end());
					mRenderables.erase(mRenderables.begin(), mRenderables.end());

					// Get a good enough guesstimation of how many renderables we will need so we aren't reallocating a bunch every frame 
					size_t renderableCountEstimation = 0;
					for(auto& child : mRoot->Children){
						renderableCountEstimation += child->Children.size();
					}

					mRenderables.reserve(renderableCountEstimation*3);
				}
			}
			catch (std::runtime_error e) {
				std::cout << "Failed to load galaxy " << FilePath << "! Exception: " << e.what() << "\n";
			}
			catch (std::exception e) {
				std::cout << "Failed to load galaxy " << FilePath << "! Exception: " << e.what() << "\n";
			}

			bIsFileDialogOpen = false;
		} else {
			bIsFileDialogOpen = false;
		}

		ImGuiFileDialog::Instance()->Close();
	}

	if (ImGui::BeginPopupModal("Galaxy Load Error", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)){
		ImGui::Text("Error Loading Stage\n\n");
		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120,0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if(mAboutOpen){
		ImGui::OpenPopup("About Window");
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowSize(ImVec2(250, 130), ImGuiCond_Always);
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.4f));
	}

	if (ImGui::BeginPopupModal("About Window", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove)){
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth = ImGui::CalcTextSize("StarForge").x;
		ImGuiStyle* style = &ImGui::GetStyle();

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text("StarForge");
		
		ImGui::Separator();

		textWidth = ImGui::CalcTextSize("https://github.com/Astral-C/StarForge").x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text("https://github.com/Astral-C/StarForge");

		textWidth = ImGui::CalcTextSize("Made by SpaceCats").x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text("Made by SpaceCats");

		textWidth = ImGui::CalcTextSize("INI Parser by rxi").x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text("INI Parser by rxi");

		ImGui::Separator();

		float size = 120 + style->FramePadding.x * 2.0f;
		float avail = ImGui::GetContentRegionAvail().x;

		float off = (avail - size) * 0.5;
		if (off > 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

		if (ImGui::Button("Close", ImVec2(120, 0))) {
			mAboutOpen = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if(mOptionsOpen){
		ImGui::OpenPopup("Options");
	}
	Options.RenderOptionMenu();

	if(mProjectManagerOpen){
		ImGui::OpenPopup("Projects");
	}

	if(mGalaxySelectOpen) {
		ImGui::OpenPopup("Galaxy Select");
	}

	mProjects.RenderUi(mProjectManagerOpen);
	auto galaxyPath = mProjects.RenderGalaxySelectUi(mGalaxySelectOpen);
	if(galaxyPath != ""){
		try {
			selected = nullptr;
			mRoot = std::make_shared<SGalaxyDOMNode>();
			mOpenGalaxies[galaxyPath] = mRoot;
			// TODO: add way to set galaxy type
			// copy from cammie again? or infer based on root structure
			if(!mRoot->LoadGalaxy(Options.mRootPath / "files" / "StageData" / galaxyPath, std::filesystem::exists(Options.mRootPath / "files" / "SystemData" / "ObjNameTable.arc") ? EGameType::SMG2 : EGameType::SMG1)){
				ImGui::OpenPopup("Galaxy Load Error");
			} else {
				ModelCache.erase(ModelCache.begin(), ModelCache.end());
				mRenderables.erase(mRenderables.begin(), mRenderables.end());

				// Get a good enough guesstimation of how many renderables we will need so we aren't reallocating a bunch every frame 
				size_t renderableCountEstimation = 0;
				for(auto& child : mRoot->Children){
					renderableCountEstimation += child->Children.size();
				}

				mRenderables.reserve(renderableCountEstimation*3);
			}
		}
		catch (std::runtime_error e) {
			std::cout << "Failed to load galaxy " << Options.mRootPath / "files" / "StageData" / galaxyPath << "! Exception: " << e.what() << "\n";
		}
		catch (std::exception e) {
			std::cout << "Failed to load galaxy " << Options.mRootPath / "files" / "StageData" / galaxyPath << "! Exception: " << e.what() << "\n";
		}
	}
}

void UStarForgeContext::SetLights() {

	J3DLight lights[8];

	lights[0].Position = glm::vec4(100000.0f, 100000.0f, 100000.0f, 1);
	lights[0].Color = glm::vec4(0.5f, 0.5f, 0.5f, 0.0f);
	lights[0].AngleAtten = glm::vec4(1.0, 0.0, 0.0, 1);
	lights[0].DistAtten = glm::vec4(1.0, 0.0, 0.0, 1);
	lights[0].Direction = glm::vec4(1.0, 0.0, 0.0, 1);

	lights[1].Position = glm::vec4(-100000.0f, -100000.0f, 100000.0f, 1);
	lights[1].Color = glm::vec4(64/255, 62/255, 64/255, 0.0f);
	lights[1].AngleAtten = glm::vec4(1.0, 0.0, 0.0, 1);
	lights[1].DistAtten = glm::vec4(1.0, 0.0, 0.0, 1);
	lights[1].Direction = glm::vec4(1.0, 0.0, 0.0, 1);

	lights[2].Position = glm::vec4(0, 0, 0, 0);
	lights[2].AngleAtten = glm::vec4(1.0, 0, 0, 1);
	lights[2].DistAtten = glm::vec4(1.0, 0.0, 0.0, 1);
	lights[2].Direction = glm::vec4(0, -1, 0, 1);
	lights[2].Color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);

	J3DUniformBufferObject::SetLights(lights);
}