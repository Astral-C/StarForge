#include "UStarForgeContext.hpp"

#include "util/UUIUtil.hpp"

#include <J3D/J3DModelLoader.hpp>
#include <J3D/J3DModelData.hpp>
#include <J3D/J3DUniformBufferObject.hpp>
#include <J3D/J3DLight.hpp>
#include <J3D/J3DModelInstance.hpp>

#include <ImGuiFileDialog.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <bstream.h>
#include <ResUtil.hpp>
#include "ImGuizmo.h"

#include "DOM/ZoneDOMNode.hpp"
#include "DOM/ObjectDOMNode.hpp"


UStarForgeContext::UStarForgeContext(){
	mGrid.Init();

	auto objectDBPath = std::filesystem::current_path() / std::filesystem::path("res/objectdb.json");
	if(std::filesystem::exists(objectDBPath)){
		std::ifstream objectDBStream(objectDBPath);
		SObjectDOMNode::LoadObjectDB(nlohmann::json::parse(objectDBStream));
	}

	ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF((std::filesystem::current_path() / "res/NotoSansJP-Regular.otf").c_str(), 16.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	Options.LoadOptions();

	mRoot = std::make_shared<SGalaxyDOMNode>();
}

bool UStarForgeContext::Update(float deltaTime) {
	mCamera.Update(deltaTime);

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
	
	ImGui::Begin("mainWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
	ImGui::Text("Scenarios");
		ImGui::Separator();
		mRoot->RenderScenarios(selected);
	ImGui::End();

	ImGui::SetNextWindowClass(&mainWindowOverride);
	ImGui::Begin("zoneView", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
		ImGui::Text("Zones");
		ImGui::Separator();
		mRoot->RenderZones(selected);
		if(ImGui::Button("Add Zone")){
			// mRoot AddZone method. Needs to make sure the added zone has all empty stageobjinfo entries!
		}
		ImGui::SameLine();
		if(ImGui::Button("Remove Zone")){
			// Make sure we have at least one zone with stageobjinfo entries!
		}
	ImGui::End();

	ImGui::SetNextWindowClass(&mainWindowOverride);

	ImGuizmo::BeginFrame();
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

	ImGui::Begin("detailWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
		ImGui::Text("Object Settings");
		ImGui::Separator();
		if(selected != nullptr){
			selected->RenderDetailsUI();
			if(selected->IsNodeType(EDOMNodeType::Object)){
				std::shared_ptr<SObjectDOMNode> object = std::static_pointer_cast<SObjectDOMNode>(selected);
				glm::mat4 transform = object->GetParentOfType<SZoneDOMNode>(EDOMNodeType::Zone).lock()->mTransform * object->mTransform, delta;

				ImGuizmo::Manipulate(&mCamera.GetViewMatrix()[0][0], &mCamera.GetProjectionMatrix()[0][0], ImGuizmo::TRANSLATE, ImGuizmo::WORLD, &transform[0][0], &delta[0][0]);
				
				object->mTransform = glm::translate(object->mTransform, glm::vec3(delta[3]));
			} else if (selected->IsNodeType(EDOMNodeType::Zone)){
				std::shared_ptr<SZoneDOMNode> zone = std::static_pointer_cast<SZoneDOMNode>(selected);
				ImGuizmo::Manipulate(&mCamera.GetViewMatrix()[0][0], &mCamera.GetProjectionMatrix()[0][0], ImGuizmo::TRANSLATE, ImGuizmo::WORLD, &zone->mTransform[0][0]);
			}
		}
		//TODO: Once selection is set up again call the selected node's render function
	ImGui::End();

	glm::mat4 projection, view;
	projection = mCamera.GetProjectionMatrix();
	view = mCamera.GetViewMatrix();

	if(!mSetLights) SetLights();
	J3DUniformBufferObject::SetProjAndViewMatrices(&projection, &view);
	
	//Render Models here
	mRoot->Render(deltaTime);

	mGrid.Render(mCamera.GetPosition(), mCamera.GetProjectionMatrix(), mCamera.GetViewMatrix());
}

void UStarForgeContext::RenderMainWindow(float deltaTime) {


}

void UStarForgeContext::RenderMenuBar() {
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("Open...")) {
			OpenModelCB();
		}
		if (ImGui::MenuItem("Save...")) {
			if(mRoot != nullptr){
				mRoot->SaveGalaxy();
			}
		}

		ImGui::Separator();
		ImGui::MenuItem("Close");

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Edit")) {
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("About")) {
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();

	if (bIsFileDialogOpen) {
		ImGuiFileDialog::Instance()->OpenDialog("OpenFileDialog", "Choose Folder", nullptr, ".");
	}
	if (bIsSaveDialogOpen) {
		ImGuiFileDialog::Instance()->OpenDialog("SaveFileDialog", "Choose File", "J3D Models (*.bmd *.bdl){.bmd,.bdl}", ".", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
	}

	if (ImGuiFileDialog::Instance()->Display("OpenFileDialog")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			std::string FilePath = ImGuiFileDialog::Instance()->GetFilePathName();

			try {
				LoadFromPath(FilePath);
			}
			catch (std::exception e) {
				std::cout << "Failed to load galaxy " << FilePath << "! Exception: " << e.what() << "\n";
			}

			bIsFileDialogOpen = false;
		}

		ImGuiFileDialog::Instance()->Close();
	}

	if (ImGuiFileDialog::Instance()->Display("SaveFileDialog")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			std::string FilePath = ImGuiFileDialog::Instance()->GetFilePathName();

			try {
			}
			catch (std::exception e) {
				std::cout << "Failed to save galaxy " << FilePath << "! Exception: " << e.what() << "\n";
			}

			bIsSaveDialogOpen = false;
		}

		ImGuiFileDialog::Instance()->Close();
	}
}

void UStarForgeContext::OpenModelCB() {
	bIsFileDialogOpen = true;
}

void UStarForgeContext::SaveModelCB() {
	bIsSaveDialogOpen = true;
}

void UStarForgeContext::SetLights() {

	J3DLight lights[8];

	lights[0].Position = glm::vec4(00000, 00000, 00000, 1);
	lights[0].AngleAtten = glm::vec4(1.0, 0.0, 0.0, 1);
	lights[0].DistAtten = glm::vec4(1.0, 0.0, 0.0, 1);

	lights[1].Position = glm::vec4(1.0, 0.0, 0.0, 1);
	lights[1].AngleAtten = glm::vec4(1.0, 0.0, 0.0, 1);
	lights[1].DistAtten = glm::vec4(1.0, 0., 0.0, 1);

	lights[2].Position = glm::vec4(0.0, 0.0, 0.0, 1);
	lights[2].AngleAtten = glm::vec4(0, 0, 1, 1);
	lights[2].DistAtten = glm::vec4(25.0, 0.0, -24.0, 1);
	lights[2].Direction = glm::vec4(1.0, -0.868448, 0.239316, 1);

	for (int i = 0; i < 8; i++)
		lights[i].Color = glm::vec4(1, 1, 1, 1);

	J3DUniformBufferObject::SetLights(lights);
}

void UStarForgeContext::LoadFromPath(std::filesystem::path filePath) {
	//TODO: Make game a setting
	mRoot->LoadGalaxy(filePath, EGameType::SMG1);
}