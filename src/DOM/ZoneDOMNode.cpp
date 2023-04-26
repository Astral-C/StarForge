#include <filesystem>
#include "ResUtil.hpp"
#include "DOM/ZoneDOMNode.hpp"
#include "DOM/ObjectDOMNode.hpp"
#include "compression.h"
#include "archive.h"
#include "imgui.h"
#include "UStarForgeContext.hpp"
#include <fmt/core.h>
#include <glm/gtx/matrix_decompose.hpp>

SZoneLayerDOMNode::SZoneLayerDOMNode() : Super("Layer") {
    mType = EDOMNodeType::ZoneLayer;
    mVisible = true;
}

SZoneLayerDOMNode::~SZoneLayerDOMNode() {

}

void SZoneLayerDOMNode::SaveLayer(GCarchive* zoneArchive){
    // hop up two parents to sgalaxydomnode and check game type
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
    if(ImGui::IsItemClicked() && !mIsMainZone){
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

    mIsMainZone = false;

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

    if(mIsMainZone){

        //get parent and check game type

        GCarcfile* stageObjInfo = GCResourceManager.GetFile(&mZoneArchive, std::filesystem::path("jmp/placement/common/stageobjinfo"));

        std::vector<std::shared_ptr<SZoneDOMNode>> zoneNodes = Parent.lock()->GetChildrenOfType<SZoneDOMNode>(EDOMNodeType::Zone);
        std::vector<std::shared_ptr<SDOMNodeSerializable>> zones;

        for(auto& zone : zoneNodes){
            if(zone->mLinkID != 0) zones.push_back(zone);
        }


        std::cout << "Saving Stage Object Info" << std::endl;
        bStream::CMemoryStream saveStream(mStageObjInfo.CalculateNewFileSize(zones.size()), bStream::Endianess::Big, bStream::OpenMode::Out);
        mStageObjInfo.Save(zones, saveStream);

        if(stageObjInfo != nullptr){
            gcReplaceFileData(stageObjInfo, saveStream.getBuffer(), saveStream.tell());
        }
    }

    std::cout << "Saving Zone " << mName << std::endl;
    for(auto& layer : GetChildrenOfType<SZoneLayerDOMNode>(EDOMNodeType::ZoneLayer)){
        layer->SaveLayer(&mZoneArchive);
    }

    GCResourceManager.SaveArchive(mZoneArchivePath.c_str(), &mZoneArchive);

}

std::map<std::string, std::pair<glm::mat4, int32_t>> SZoneDOMNode::LoadMainZone(std::string zonePath){
    if(!std::filesystem::exists(zonePath)){
        std::cout << "Couldn't find zone " << zonePath << std::endl;
        return {};
    }

    mIsMainZone = true;
    mZoneArchivePath = std::filesystem::path(zonePath);

    GCResourceManager.LoadArchive(zonePath.data(), &mZoneArchive);

    mZoneArchiveLoaded = true;

    std::map<std::string, std::pair<glm::mat4, int32_t>> zoneTransforms;

    // get parent and check game type
    GCarcfile* file = GCResourceManager.GetFile(&mZoneArchive, std::filesystem::path("jmp/placement/common/stageobjinfo"));
    
    bStream::CMemoryStream StageObjInfoStream((uint8_t*)file->data, (size_t)file->size, bStream::Endianess::Big, bStream::OpenMode::In);
    mStageObjInfo.Load(&StageObjInfoStream);
    for(size_t stageObjEntry = 0; stageObjEntry < mStageObjInfo.GetEntryCount(); stageObjEntry++){
        std::string zoneName = mStageObjInfo.GetString(stageObjEntry, "name");
        glm::vec3 position = {mStageObjInfo.GetFloat(stageObjEntry, "pos_x"), mStageObjInfo.GetFloat(stageObjEntry, "pos_y"), mStageObjInfo.GetFloat(stageObjEntry, "pos_z")};
        glm::vec3 rotation = {mStageObjInfo.GetFloat(stageObjEntry, "dir_x"), mStageObjInfo.GetFloat(stageObjEntry, "dir_y"), mStageObjInfo.GetFloat(stageObjEntry, "dir_z")};

        zoneTransforms.insert({zoneName, {SGenUtility::CreateMTX({1,1,1}, rotation, position), mStageObjInfo.GetUnsignedInt(stageObjEntry, "l_id")}});
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
    mName = bcsv->GetString(entry, "ZoneName");
}

void SZoneDOMNode::Serialize(SBcsvIO* bcsv, int entry){
    glm::vec3 pos, scale, skew;
    glm::quat dir;
    glm::vec4 persp;


    glm::decompose(mTransform, scale, dir, pos, skew, persp);

    glm::vec3 rotation = glm::eulerAngles(dir);

    bcsv->SetString(entry, "name", mName);
    bcsv->SetUnsignedInt(entry, "l_id", mLinkID);

    bcsv->SetFloat(entry, "pos_x", pos.x);
    bcsv->SetFloat(entry, "pos_y", pos.y);
    bcsv->SetFloat(entry, "pos_z", pos.z);

    bcsv->SetFloat(entry, "dir_x", glm::degrees(dir.x));
    bcsv->SetFloat(entry, "dir_y", glm::degrees(dir.y));
    bcsv->SetFloat(entry, "dir_z", glm::degrees(dir.z));
}


void SZoneDOMNode::Render(float dt){
    std::vector<std::shared_ptr<SZoneLayerDOMNode>> zoneLayers = GetChildrenOfType<SZoneLayerDOMNode>(EDOMNodeType::ZoneLayer);

    for(auto& layer : zoneLayers){
        if(layer->GetVisible()) layer->Render(mTransform, dt);
    }
}