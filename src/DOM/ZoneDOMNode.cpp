#include <filesystem>
#include "ResUtil.hpp"
#include "DOM/ZoneDOMNode.hpp"
#include "DOM/ObjectDOMNode.hpp"
#include "compression.h"
#include "archive.h"
#include "imgui.h"
#include "UStarForgeContext.hpp"
#include <fmt/core.h>

SZoneLayerDOMNode::SZoneLayerDOMNode() : Super("Layer") {
    mType = EDOMNodeType::ZoneLayer;
    mVisible = true;
}

SZoneLayerDOMNode::~SZoneLayerDOMNode() {

}

void SZoneLayerDOMNode::SaveLayer(GCarchive* zoneArchive){
    std::string layerPath = fmt::format("jmp/placement/{0}/objinfo", mName);
    GCarcfile* objInfoFile = GCResourceManager.GetFile(zoneArchive, std::filesystem::path(layerPath));

    std::vector<std::shared_ptr<SDOMNodeSerializable>> objects = GetChildrenOfType<SDOMNodeSerializable>(EDOMNodeType::Object);
    if(objects.size() == 0) {
        return;
    }


    bStream::CMemoryStream saveStream(mObjInfo.CalculateNewFileSize(objects.size()), bStream::Endianess::Big, bStream::OpenMode::Out);
    mObjInfo.Save(objects, saveStream);

    if(objInfoFile != nullptr){
        std::cout << "Replacing file " << layerPath << std::endl;
        gcReplaceFileData(objInfoFile, saveStream.getBuffer(), saveStream.getSize());
    }

}

void SZoneLayerDOMNode::LoadLayer(GCarchive* zoneArchive, GCarcfile* layerDir, std::string layerName){
    mName = std::string(layerDir->name);
    
	for (GCarcfile* layer_file = &zoneArchive->files[zoneArchive->dirs[layerDir->size].fileoff]; layer_file < &zoneArchive->files[zoneArchive->dirs[layerDir->size].fileoff] + zoneArchive->dirs[layerDir->size].filenum; layer_file++){
		if((strcmp(layer_file->name, "objinfo") == 0 || strcmp(layer_file->name, "ObjInfo") == 0) && layer_file->data != nullptr){
			bStream::CMemoryStream ObjInfoStream((uint8_t*)layer_file->data, (size_t)layer_file->size, bStream::Endianess::Big, bStream::OpenMode::In);
			mObjInfo.Load(&ObjInfoStream);
			for(size_t objEntry = 0; objEntry < mObjInfo.GetEntryCount(); objEntry++){
                auto object = std::make_shared<SObjectDOMNode>();
                object->Deserialize(&mObjInfo, objEntry);
                AddChild(object);
			}
		}
	}
}

void SZoneLayerDOMNode::RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected) {
    ImGui::Checkbox(fmt::format("##isVisible{}", mName).data(), &mVisible);
    ImGui::SameLine();
    if(ImGui::TreeNode(mName.data())){
        for (auto node : GetChildrenOfType<SObjectDOMNode>(EDOMNodeType::Object)){
            node->RenderHeirarchyUI(selected);
        }
        ImGui::TreePop();
    }
}

void SZoneLayerDOMNode::RenderDetailsUI() {

}

void SZoneLayerDOMNode::Render(glm::mat4 transform, float dt){
    for(auto& object : GetChildrenOfType<SObjectDOMNode>(EDOMNodeType::Object)){
        object->Render(transform, dt);
    }
}

SZoneDOMNode::SZoneDOMNode() : Super("Zone") {
    mTransform = glm::mat4(1);
    mType = EDOMNodeType::Zone;
}

SZoneDOMNode::~SZoneDOMNode(){
    if(mZoneArchiveLoaded){
        gcFreeArchive(&mZoneArchive);
    }
}

void SZoneDOMNode::RenderDetailsUI(){
    
}

void SZoneDOMNode::RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected){
    bool opened = ImGui::TreeNodeEx(mName.data(), GetIsSelected() ? ImGuiTreeNodeFlags_Selected : 0);
    if(ImGui::IsItemClicked()){
        SetIsSelected(true);
        selected = GetSharedPtr<SZoneDOMNode>(EDOMNodeType::Zone);
    } else {
        SetIsSelected(false);
    }
    if(opened){
        for (auto node : GetChildrenOfType<SZoneLayerDOMNode>(EDOMNodeType::ZoneLayer)){
            node->RenderHeirarchyUI(selected);
        }
        ImGui::TreePop();
    }
}

void SZoneDOMNode::LoadZone(std::string zonePath){
    if(!std::filesystem::exists(zonePath)) return;

    mZoneArchivePath = std::filesystem::path(zonePath);

    GCResourceManager.LoadArchive(zonePath.data(), &mZoneArchive);

	for (GCarcfile* file = mZoneArchive.files; file < mZoneArchive.files + mZoneArchive.filenum; file++){
		if(file->parent != nullptr && (strcmp(file->parent->name, "placement") == 0 || strcmp(file->parent->name, "Placement") == 0) && (file->attr & 0x02) && strcmp(file->name, ".") != 0 && strcmp(file->name, "..") != 0){
			auto layer = std::make_shared<SZoneLayerDOMNode>();
            
            layer->LoadLayer(&mZoneArchive, file, std::string(file->name));

            AddChild(layer);
		}
	}

    mZoneArchiveLoaded = true;

}

void SZoneDOMNode::SaveZone(){

    if(!mZoneArchiveLoaded) return;

    std::cout << "Saving Zone " << mName << std::endl;
    for(auto& layer : GetChildrenOfType<SZoneLayerDOMNode>(EDOMNodeType::ZoneLayer)){
        layer->SaveLayer(&mZoneArchive);
    }

    GCResourceManager.SaveArchive(mZoneArchivePath.c_str(), &mZoneArchive);

}

std::map<std::string, glm::mat4> SZoneDOMNode::LoadMainZone(std::string zonePath){
    if(!std::filesystem::exists(zonePath)){
        std::cout << "Couldn't find zone " << zonePath << std::endl;
        return {};
    }

    mIsMainZone = true;
    mZoneArchivePath = std::filesystem::path(zonePath);
    

    GCResourceManager.LoadArchive(zonePath.data(), &mZoneArchive);

    std::map<std::string, glm::mat4> zoneTransforms;

    GCarcfile* file = GCResourceManager.GetFile(&mZoneArchive, std::filesystem::path("jmp/placement/common/stageobjinfo"));
    
    SBcsvIO StageObjInfo;
    bStream::CMemoryStream StageObjInfoStream((uint8_t*)file->data, (size_t)file->size, bStream::Endianess::Big, bStream::OpenMode::In);
    StageObjInfo.Load(&StageObjInfoStream);
    for(size_t stageObjEntry = 0; stageObjEntry < StageObjInfo.GetEntryCount(); stageObjEntry++){
        std::string zoneName = StageObjInfo.GetString(stageObjEntry, "name");
        glm::vec3 position = {StageObjInfo.GetFloat(stageObjEntry, "pos_x"), StageObjInfo.GetFloat(stageObjEntry, "pos_y"), StageObjInfo.GetFloat(stageObjEntry, "pos_z")};
        glm::vec3 rotation = {StageObjInfo.GetFloat(stageObjEntry, "dir_x"), StageObjInfo.GetFloat(stageObjEntry, "dir_y"), StageObjInfo.GetFloat(stageObjEntry, "dir_z")};

        zoneTransforms.insert({zoneName, SGenUtility::CreateMTX({1,1,1}, rotation, position)});
    }

	for (GCarcfile* file = mZoneArchive.files; file < mZoneArchive.files + mZoneArchive.filenum; file++){
		if(file->parent != nullptr && (strcmp(file->parent->name, "placement") == 0 || strcmp(file->parent->name, "Placement") == 0) && (file->attr & 0x02) && strcmp(file->name, ".") != 0 && strcmp(file->name, "..") != 0){
			auto layer = std::make_shared<SZoneLayerDOMNode>();
            
            layer->LoadLayer(&mZoneArchive, file, std::string(file->name));

            AddChild(layer);
		}
	}

    return zoneTransforms;
}

void SZoneDOMNode::Deserialize(SBcsvIO* bcsv, int entry){
    mName = SGenUtility::SjisToUtf8(bcsv->GetString(entry, "ZoneName"));

}

void SZoneDOMNode::Serialize(SBcsvIO* bcsv, int entry){
    
}


void SZoneDOMNode::Render(float dt){
    std::vector<std::shared_ptr<SZoneLayerDOMNode>> zoneLayers = GetChildrenOfType<SZoneLayerDOMNode>(EDOMNodeType::ZoneLayer);

    for(auto& layer : zoneLayers){
        if(layer->GetVisible()) layer->Render(mTransform, dt);
    }
}