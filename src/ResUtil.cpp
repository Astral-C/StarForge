#include "ResUtil.hpp"
#include "imgui.h"
#include <filesystem>
#include <fmt/core.h>
#include <bstream.h>
#include <ImGuiFileDialog.h>

#include <J3D/J3DModelLoader.hpp>
#include <J3D/J3DModelData.hpp>
#include <J3D/J3DUniformBufferObject.hpp>
#include <J3D/J3DLight.hpp>
#include <J3D/J3DModelInstance.hpp>
#include "ModelCache.hpp"

#include "ini.h"

SResUtility::SGCResourceManager GCResourceManager;
SResUtility::SOptions Options;

void SResUtility::SGCResourceManager::Init()
{
	GCerror err;
	if ((err = gcInitContext(&mResManagerContext)) != GC_ERROR_SUCCESS)
	{
		printf("Error initing arc loader context: %s\n", gcGetErrorMessage(err));
	}

	mInitialized = true;
}

GCarcfile* SResUtility::SGCResourceManager::GetFile(GCarchive* archive, std::filesystem::path filepath){
	int dirID = 0;
	for(std::string component : filepath){
		for (GCarcfile* file = &archive->files[archive->dirs[dirID].fileoff]; file < &archive->files[archive->dirs[dirID].fileoff] + archive->dirs[dirID].filenum; file++){
			if(strcmp(file->name, component.c_str()) == 0 && (file->attr & 0x02)){
				dirID = file->size;
				break;
			} else if(strcmp(file->name, component.c_str()) == 0 && !(file->attr & 0x02)) {
				return file;
			}
		}
	}
	return nullptr;
}

bool SResUtility::SGCResourceManager::LoadArchive(const char* path, GCarchive* archive)
{
	if(!mInitialized) return false;
	
	GCerror err;

	FILE* f = fopen(path, "rb");
	if (f == nullptr)
	{
		printf("Error opening file \"%s\"\n", path);
		return false;
	}

	fseek(f, 0L, SEEK_END);
	GCsize size = (GCsize)ftell(f);
	rewind(f);

	void* file = malloc(size);
	if (file == nullptr)
	{
		printf("Error allocating buffer for file \"%s\"\n", path);
		return false;
	}

	fread(file, 1, size, f);
	fclose(f);

	// If the file starts with 'Yay0', it's Yay0 compressed.
	if (*((uint32_t*)file) == 0x30796159)
	{
		GCsize compressedSize = gcDecompressedSize(&mResManagerContext, (GCuint8*)file, 0);

		void* decompBuffer = malloc(compressedSize);
		gcYay0Decompress(&mResManagerContext, (GCuint8*)file, (GCuint8*)decompBuffer, compressedSize, 0);

		free(file);
		size = compressedSize;
		file = decompBuffer;
	}
	// Likewise, if the file starts with 'Yaz0' it's Yaz0 compressed.
	else if (*((uint32_t*)file) == 0x307A6159)
	{
		GCsize compressedSize = gcDecompressedSize(&mResManagerContext, (GCuint8*)file, 0);

		void* decompBuffer = malloc(compressedSize);
		gcYaz0Decompress(&mResManagerContext, (GCuint8*)file, (GCuint8*)decompBuffer, compressedSize, 0);
		free(file);
		size = compressedSize;
		file = decompBuffer;
	}

	gcInitArchive(archive, &mResManagerContext);
	if ((err = gcLoadArchive(archive, file, size)) != GC_ERROR_SUCCESS) {
		printf("Error Loading Archive: %s\n", gcGetErrorMessage(err));
		return false;
	}

	return true;
}

bool SResUtility::SGCResourceManager::ReplaceArchiveFileData(GCarcfile* file, uint8_t* new_data, size_t new_data_size){
	if(!mInitialized) return false;
	
	size_t paddedSize = (new_data_size + 31) & ~31;

	//allocate size of new file
	uint8_t* newFileData = (uint8_t*)gcAllocMem(&mResManagerContext, paddedSize);
	memcpy(newFileData, new_data, paddedSize);
		
	gcFreeMem(&mResManagerContext, file->data);

	//copy new jmp to file buffer for arc
	file->data = newFileData;

	//set size properly
	file->size = paddedSize;

	return true;
}

bool SResUtility::SGCResourceManager::SaveArchiveCompressed(const char* path, GCarchive* archive)
{
	if(!mInitialized) return false;

	GCsize outSize = gcSaveArchive(archive, NULL);
	GCuint8* archiveOut = new GCuint8[outSize];
	GCuint8* archiveCmp = new GCuint8[outSize];

	gcSaveArchive(archive, archiveOut);
	GCsize cmpSize = gcYay0Compress(&mResManagerContext, archiveOut, archiveCmp, outSize);
	
	std::ofstream fileStream;
	fileStream.open(path, std::ios::binary | std::ios::out);
	fileStream.write((const char*)archiveCmp, cmpSize);
	fileStream.close();

	delete archiveOut;
	delete archiveCmp;

	return true;
}

bool SResUtility::SGCResourceManager::SaveArchive(const char* path, GCarchive* archive)
{
	if(!mInitialized) return false;

	GCsize outSize = gcSaveArchive(archive, NULL);
	GCuint8* archiveOut = new GCuint8[outSize];

	gcSaveArchive(archive, archiveOut);
	
	std::ofstream fileStream;
	fileStream.open(path, std::ios::binary | std::ios::out);
	fileStream.write((const char*)archiveOut, outSize);
	fileStream.close();

	delete[] archiveOut;

	return true;
}

void SResUtility::SGCResourceManager::CacheModel(std::string modelName){
	std::filesystem::path modelPath = std::filesystem::path(Options.mRootPath) / "DATA" / "files" / "ObjectData" / (modelName + ".arc");
	
	if(std::filesystem::exists(modelPath)){
		GCarchive modelArc;
		GCResourceManager.LoadArchive(modelPath.c_str(), &modelArc);
		for (GCarcfile* file = modelArc.files; file < modelArc.files + modelArc.filenum; file++){
			if(std::filesystem::path(file->name).extension() == ".bdl"){
				J3DModelLoader Loader;
				bStream::CMemoryStream modelStream((uint8_t*)file->data, file->size, bStream::Endianess::Big, bStream::OpenMode::In);
				
				auto data = std::make_shared<J3DModelData>();
				data = Loader.Load(&modelStream, NULL);
				ModelCache.insert({modelName, data});
			}
		}
	}
}

void SResUtility::SOptions::LoadOptions(){
	auto optionsPath = std::filesystem::current_path() / "settings.ini";
	if(std::filesystem::exists(optionsPath)){
		ini_t* config = ini_load(optionsPath.c_str());
		if(config == nullptr) return;

		const char* path = ini_get(config, "settings", "root");
		if(path != nullptr) mRootPath = std::filesystem::path(path);
		ini_free(config);
	}
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

		if(ImGui::Button("Save")){
			std::ofstream settingsFile(std::filesystem::current_path() / "settings.ini");
			settingsFile << fmt::format("[settings]\nroot={0}", mRootPath.string());
			settingsFile.close();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if(ImGui::Button("Close")){
			ImGui::CloseCurrentPopup();
		}

		if(mSelectRootDialogOpen) ImGuiFileDialog::Instance()->OpenDialog("OpenRootDialog", "Choose Game Root", nullptr, ".");

		if (ImGuiFileDialog::Instance()->Display("OpenRootDialog")) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				mRootPath = ImGuiFileDialog::Instance()->GetFilePathName();

				mSelectRootDialogOpen = false;
			} else {
				mSelectRootDialogOpen = false;
			}

			ImGuiFileDialog::Instance()->Close();
		}
		ImGui::EndPopup();
	}
}