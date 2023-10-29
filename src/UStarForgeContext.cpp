#include "UStarForgeContext.hpp"

#include "util/UUIUtil.hpp"

#include <J3D/J3DModelLoader.hpp>
#include <J3D/J3DModelData.hpp>
#include <J3D/J3DUniformBufferObject.hpp>
#include <J3D/J3DLight.hpp>
#include <J3D/J3DModelInstance.hpp>
#include <J3D/J3DRendering.hpp>

#include <ImGuiFileDialog.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <bstream.h>
#include <ResUtil.hpp>
#include "ImGuizmo.h"

#include "DOM/ZoneDOMNode.hpp"
#include "DOM/ScenarioDOMNode.hpp"
#include "DOM/ObjectDOMNode.hpp"

#include "IconsForkAwesome.h"

void GalaxySort(J3DRendering::SortFunctionArgs packets) {
    std::sort(
        packets.begin(),
        packets.end(),
        [](const J3DRenderPacket& a, const J3DRenderPacket& b) -> bool {
			if((a.SortKey & 0x01000000) != (b.SortKey & 0x01000000)){
				return (a.SortKey & 0x01000000) > (b.SortKey & 0x01000000);
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
}

UStarForgeContext::UStarForgeContext(){
	mGrid.Init();

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

	J3DRendering::SetSortFunction(GalaxySort);

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

	return true;
}

void UStarForgeContext::Render(float deltaTime) {

	RenderMenuBar();
	
	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

	ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoDockingInCentralNode;
	mMainDockSpaceID = ImGui::DockSpaceOverViewport(mainViewport, dockFlags);
	
	if(!bIsDockingSetUp){
		ImGui::DockBuilderRemoveNode(mMainDockSpaceID); // clear any previous layout
		ImGui::DockBuilderAddNode(mMainDockSpaceID, dockFlags | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(mMainDockSpaceID, mainViewport->Size);


		mDockNodeLeftID = ImGui::DockBuilderSplitNode(mMainDockSpaceID, ImGuiDir_Left, 0.25f, nullptr, &mMainDockSpaceID);
		mDockNodeDownLeftID = ImGui::DockBuilderSplitNode(mDockNodeLeftID, ImGuiDir_Down, 0.5f, nullptr, &mDockNodeUpLeftID);


		ImGui::DockBuilderDockWindow("mainWindow", mDockNodeUpLeftID);
		ImGui::DockBuilderDockWindow("zoneView", ImGui::DockBuilderSplitNode(mDockNodeUpLeftID, ImGuiDir_Down, 0.5f, nullptr, nullptr));
		ImGui::DockBuilderDockWindow("detailWindow", mDockNodeDownLeftID);

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

	ImGuizmo::BeginFrame();
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

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
		if(selected != nullptr){
			selected->RenderDetailsUI();
			if(selected->IsNodeType(EDOMNodeType::Object)){
				std::shared_ptr<SObjectDOMNode> object = std::static_pointer_cast<SObjectDOMNode>(selected);
				glm::mat4 zoneTransform = object->GetParentOfType<SZoneDOMNode>(EDOMNodeType::Zone).lock()->mTransform;
				glm::mat4 transform = (zoneTransform * object->mTransform), delta = glm::identity<glm::mat4>();

				if(ImGuizmo::Manipulate(&mCamera.GetViewMatrix()[0][0], &mCamera.GetProjectionMatrix()[0][0], (ImGuizmo::OPERATION)mGizmoOperation, ImGuizmo::WORLD, &transform[0][0], &delta[0][0])){
					object->mTransform = glm::inverse(zoneTransform) * transform;
				}
			} else if (selected->IsNodeType(EDOMNodeType::Zone)){
				std::shared_ptr<SZoneDOMNode> zone = std::static_pointer_cast<SZoneDOMNode>(selected);
				ImGuizmo::Manipulate(&mCamera.GetViewMatrix()[0][0], &mCamera.GetProjectionMatrix()[0][0], (ImGuizmo::OPERATION)mGizmoOperation, ImGuizmo::WORLD, &zone->mTransform[0][0]);
			}
		}
		//TODO: Once selection is set up again call the selected node's render function
	ImGui::End();

	glm::mat4 projection, view;
	projection = mCamera.GetProjectionMatrix();
	view = mCamera.GetViewMatrix();

	//if(!mSetLights) SetLights();
	J3DUniformBufferObject::SetProjAndViewMatrices(&projection, &view);
	
	//Render Models here
	
	mRenderables.clear();
	mRoot->Render(mRenderables, deltaTime);

	J3DRendering::Render(deltaTime, mCamera.GetPosition(), view, projection, mRenderables);

	//mGrid.Render(mCamera.GetPosition(), mCamera.GetProjectionMatrix(), mCamera.GetViewMatrix());
}

void UStarForgeContext::RenderMainWindow(float deltaTime) {


}

void UStarForgeContext::RenderMenuBar() {
	mOptionsOpen = false;
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem(ICON_FK_FOLDER_OPEN " Open...")) {
			bIsFileDialogOpen = true;
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
	if (ImGui::BeginMenu(ICON_FK_QUESTION_CIRCLE)) {
		if(ImGui::MenuItem("About")){
			mAboutOpen = true;
		}
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();

	if (bIsFileDialogOpen) {
		ImGuiFileDialog::Instance()->OpenDialog("OpenGalaxyDialog", "Choose Stage Directory", nullptr, Options.mRootPath.string() == "" ? "." : (Options.mRootPath / "files" / "StageData" / ".").string(), "");
	}

	if (ImGuiFileDialog::Instance()->Display("OpenGalaxyDialog")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			std::string FilePath = ImGuiFileDialog::Instance()->GetFilePathName();

			try {
				selected = nullptr;
				mRoot = std::make_shared<SGalaxyDOMNode>();
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
		ImGui::SetNextWindowSize(ImVec2(250, 110), ImGuiCond_Appearing);
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.4f));
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

		textWidth = ImGui::CalcTextSize("Made by SpaceCats/Veebs\n\n").x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text("Made by SpaceCats/Veebs");

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