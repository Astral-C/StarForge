#include "ResUtil.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <filesystem>
#include <fmt/core.h>
#include <bstream.h>
#include "../lib/ImGuiFileDialog/ImGuiFileDialog.h"

#include <J3D/J3DModelLoader.hpp>
#include <J3D/Animation/J3DAnimationLoader.hpp>
#include <J3D/Data/J3DModelData.hpp>
#include <J3D/Material/J3DUniformBufferObject.hpp>
#include <J3D/Rendering/J3DLight.hpp>
#include <J3D/Data/J3DModelInstance.hpp>
#include "ModelCache.hpp"

#include <curl/curl.h>

#include "DOM/ObjectDOMNode.hpp"

#include "ini.h"

SResUtility::SGCResourceManager GCResourceManager;
SResUtility::SOptions Options;

void SResUtility::SGCResourceManager::Init()
{
	mInitialized = true;
}

void SResUtility::SGCResourceManager::CacheModel(std::string modelName){
	std::filesystem::path modelPath = std::filesystem::path(Options.mRootPath) / "files" / "ObjectData" / (modelName + ".arc");
	//std::cout << "Trying to load archive" << modelPath << std::endl;
	if(std::filesystem::exists(modelPath)){
		std::shared_ptr<Archive::Rarc> archive = Archive::Rarc::Create();
		bStream::CFileStream modelArchive(modelPath, bStream::Endianess::Big, bStream::OpenMode::In);

		if(!archive->Load(&modelArchive)){
			std::cout << "Couldn't load archive " << modelPath << std::endl; 
			return;
		}

		std::string modelNameLower = modelName;
		std::transform(modelNameLower.begin(), modelNameLower.end(), modelNameLower.begin(), [](unsigned char c){ return std::tolower(c); });
		std::shared_ptr<Archive::File> modelFile = archive->Get<Archive::File>(modelNameLower+".bdl");

		if(modelFile == nullptr){
			std::cout << "Couldn't find model " << modelName << ".bdl" << std::endl;
			return;
		}

		J3DModelLoader Loader;
		bStream::CMemoryStream modelStream(modelFile->GetData(), modelFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
				
		std::shared_ptr<J3DModelData> data = Loader.Load(&modelStream, NULL);
		ModelCache.insert({modelName, data});
	} else {
		std::cout << "Couldn't find model " << modelName << std::endl;
	}
}

std::shared_ptr<J3DAnimation::J3DColorAnimationInstance> SResUtility::SGCResourceManager::LoadColorAnimation(std::string modelName, std::string animName){
	std::filesystem::path modelPath = std::filesystem::path(Options.mRootPath) / "files" / "ObjectData" / (modelName + ".arc");
	//std::cout << "Trying to load archive" << modelPath << std::endl;
	if(std::filesystem::exists(modelPath)){
		std::shared_ptr<Archive::Rarc> archive = Archive::Rarc::Create();
		bStream::CFileStream modelArchive(modelPath, bStream::Endianess::Big, bStream::OpenMode::In);

		if(!archive->Load(&modelArchive)){
			std::cout << "Couldn't load archive " << modelPath << std::endl; 
			return nullptr;
		}

		std::shared_ptr<Archive::File> modelFile = archive->Get<Archive::File>(animName);

		if(modelFile == nullptr){
			std::cout << "Couldn't find color anim " << animName << std::endl;
			return nullptr;
		}

		J3DAnimation::J3DAnimationLoader Loader;
		bStream::CMemoryStream animStream(modelFile->GetData(), modelFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);

		std::shared_ptr<J3DAnimation::J3DColorAnimationInstance> instance = Loader.LoadAnimation<J3DAnimation::J3DColorAnimationInstance>(animStream);
		return instance;
	} else {
		std::cout << "Couldn't find animation " << animName << std::endl;
	}
	return nullptr;
}

std::shared_ptr<J3DAnimation::J3DJointAnimationInstance> SResUtility::SGCResourceManager::LoadJointAnimation(std::string modelName, std::string animName){
	std::filesystem::path modelPath = std::filesystem::path(Options.mRootPath) / "files" / "ObjectData" / (modelName + ".arc");
	//std::cout << "Trying to load archive" << modelPath << std::endl;
	if(std::filesystem::exists(modelPath)){
		std::shared_ptr<Archive::Rarc> archive = Archive::Rarc::Create();
		bStream::CFileStream modelArchive(modelPath, bStream::Endianess::Big, bStream::OpenMode::In);

		if(!archive->Load(&modelArchive)){
			std::cout << "Couldn't load archive " << modelPath << std::endl; 
			return nullptr;
		}

		std::shared_ptr<Archive::File> modelFile = archive->Get<Archive::File>(animName);

		if(modelFile == nullptr){
			std::cout << "Couldn't find joint anim " << animName << std::endl;
			return nullptr;
		}

		J3DAnimation::J3DAnimationLoader Loader;
		bStream::CMemoryStream animStream(modelFile->GetData(), modelFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);

		std::shared_ptr<J3DAnimation::J3DJointAnimationInstance> instance = Loader.LoadAnimation<J3DAnimation::J3DJointAnimationInstance>(animStream);
		return instance;
	} else {
		std::cout << "Couldn't find animation " << animName << std::endl;
	}
	return nullptr;
}

std::shared_ptr<J3DAnimation::J3DTexMatrixAnimationInstance> SResUtility::SGCResourceManager::LoadTextureAnimation(std::string modelName, std::string animName){
	std::filesystem::path modelPath = std::filesystem::path(Options.mRootPath) / "files" / "ObjectData" / (modelName + ".arc");
	//std::cout << "Trying to load archive" << modelPath << std::endl;
	if(std::filesystem::exists(modelPath)){
		std::shared_ptr<Archive::Rarc> archive = Archive::Rarc::Create();
		bStream::CFileStream modelArchive(modelPath, bStream::Endianess::Big, bStream::OpenMode::In);

		if(!archive->Load(&modelArchive)){
			std::cout << "Couldn't load archive " << modelPath << std::endl; 
			return nullptr;
		}

		std::shared_ptr<Archive::File> modelFile = archive->Get<Archive::File>(animName);

		if(modelFile == nullptr){
			std::cout << "Couldn't find texmatrix anim " << modelName << std::endl;
			return nullptr;
		}

		J3DAnimation::J3DAnimationLoader Loader;
		bStream::CMemoryStream animStream(modelFile->GetData(), modelFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);

		std::shared_ptr<J3DAnimation::J3DTexMatrixAnimationInstance> instance = Loader.LoadAnimation<J3DAnimation::J3DTexMatrixAnimationInstance>(animStream);
		return instance;
	} else {
		std::cout << "Couldn't find animation " << animName << std::endl;
	}
	return nullptr;
}

void SResUtility::SOptions::LoadOptions(){
	auto optionsPath = std::filesystem::current_path() / "settings.ini";
	if(std::filesystem::exists(optionsPath)){
		ini_t* config = ini_load(optionsPath.string().c_str());
		if(config == nullptr) return;

		const char* path = ini_get(config, "settings", "root");
		if(path != nullptr) mRootPath = std::filesystem::path(path);

		const char* url = ini_get(config, "settings", "objectdb_url");
		if(url != nullptr) mObjectDBUrl = std::string(url);

		ini_free(config);
	}
}

void WriteObjectDBChunk(void* ptr, size_t size, size_t nmemb, FILE* file){
	fwrite(ptr, size, nmemb, file);
}

// I need to add proper error codes to this
void SResUtility::SOptions::UpdateObjectDB(){
	if(mObjectDBUrl == "") return;

	CURL* curl;
	CURLcode result;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if(!curl){
		curl_global_cleanup();
		return;
	}

	FILE* objectDBFile = fopen((std::filesystem::current_path() / "res" / "objectdb.json").string().c_str(), "w");

	curl_easy_setopt(curl, CURLOPT_URL, mObjectDBUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteObjectDBChunk);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, objectDBFile);

	result = curl_easy_perform(curl);

	fclose(objectDBFile);

	if(result == CURLE_OK){
		auto objectDBPath = std::filesystem::current_path() / "res" / "objectdb.json";
		if(std::filesystem::exists(objectDBPath)){
			std::ifstream objectDBStream(objectDBPath);
			SObjectDOMNode::LoadObjectDB(nlohmann::json::parse(objectDBStream));
		}
	}

	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

void SResUtility::SOptions::RenderOptionMenu(){
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Options", NULL, ImGuiWindowFlags_AlwaysAutoResize)){
		ImGui::Text(fmt::format("Root Path: {0}", mRootPath == "" ? "(Not Set)" : mRootPath.string()).data());
		ImGui::SameLine();
		if(ImGui::Button("Open")){
			mSelectRootDialogOpen = true;
		}

		ImGui::Text("ObjectDB Url");
		ImGui::InputText("##objDBUrl", &mObjectDBUrl);
		ImGui::SameLine();
		if(ImGui::Button("Update")){
			UpdateObjectDB();
		}

		if(ImGui::Button("Save")){
			std::ofstream settingsFile(std::filesystem::current_path() / "settings.ini");
			settingsFile << fmt::format("[settings]\nroot={0}\nobjectdb_url={1}", mRootPath.string(), mObjectDBUrl);
			settingsFile.close();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if(ImGui::Button("Close")){
			ImGui::CloseCurrentPopup();
		}


		if(mSelectRootDialogOpen){
			IGFD::FileDialogConfig config;
			config.path = ".";
			ImGuiFileDialog::Instance()->OpenDialog("OpenRootDialog", "Choose Game Root", nullptr, config);
		}

		if (ImGuiFileDialog::Instance()->Display("OpenRootDialog")) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				mRootPath = ImGuiFileDialog::Instance()->GetCurrentPath();

				mSelectRootDialogOpen = false;
			} else {
				mSelectRootDialogOpen = false;
			}

			ImGuiFileDialog::Instance()->Close();
		}
		ImGui::EndPopup();
	}
}