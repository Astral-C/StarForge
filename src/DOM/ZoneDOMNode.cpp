#include <filesystem>
#include "ResUtil.hpp"
#include "DOM/ZoneDOMNode.hpp"
#include "DOM/ObjectDOMNode.hpp"
#include "DOM/AreaObjectDOMNode.hpp"
#include "DOM/PlanetDOMNode.hpp"
#include "DOM/ToadDOMNode.hpp"
#include "DOM/TicoDOMNode.hpp"
#include "DOM/AstroObjectDOMNode.hpp"
#include "DOM/BooDOMNode.hpp"
#include "compression.h"
#include "archive.h"
#include "imgui.h"
#include "UStarForgeContext.hpp"
#include "IconsForkAwesome.h"
#include <fmt/core.h>
#include <glm/gtx/matrix_decompose.hpp>

SZoneLayerDOMNode::SZoneLayerDOMNode() : Super("Layer") {
    mType = EDOMNodeType::ZoneLayer;
    mVisible = true;
}

SZoneLayerDOMNode::~SZoneLayerDOMNode() {

}

void SZoneLayerDOMNode::SaveLayer(GCarchive* zoneArchive){
    std::vector<std::shared_ptr<SDOMNodeSerializable>> objects;

    objects.reserve(Children.size());

    for(auto child : Children){
        std::cout << fmt::format("[Save Zone Layer {0}]: Adding object {1} type {2}", mName, child->GetName(), child->GetNodeTypeString()) << std::endl;
        objects.push_back(std::dynamic_pointer_cast<SDOMNodeSerializable>(child));
    }

    if(objects.size() == 0) {
        return;
    }

    bStream::CMemoryStream saveStream(mObjInfo.CalculateNewFileSize(objects.size()), bStream::Endianess::Big, bStream::OpenMode::Out);
    mObjInfo.Save(objects, saveStream);

    if(mObjInfoFile != nullptr){
        gcReplaceFileData(mObjInfoFile, saveStream.getBuffer(), saveStream.getSize());
    }

}

void SZoneLayerDOMNode::LoadLayer(GCarchive* zoneArchive, GCarcfile* layerDir, std::string layerName){
    mName = std::string(layerDir->name);
    
	for (GCarcfile* layer_file = &zoneArchive->files[zoneArchive->dirs[layerDir->size].fileoff]; layer_file < &zoneArchive->files[zoneArchive->dirs[layerDir->size].fileoff] + zoneArchive->dirs[layerDir->size].filenum; layer_file++){
		if((strcmp(layer_file->name, "objinfo") == 0 || strcmp(layer_file->name, "ObjInfo") == 0) && layer_file->data != nullptr){
            mObjInfoFile = layer_file;
			bStream::CMemoryStream ObjInfoStream((uint8_t*)layer_file->data, (size_t)layer_file->size, bStream::Endianess::Big, bStream::OpenMode::In);
			mObjInfo.Load(&ObjInfoStream);
			for(size_t objEntry = 0; objEntry < mObjInfo.GetEntryCount(); objEntry++){
                // This section needs to be redone to actually properly load models somehow. Maybe a premape map for name->archive?
                std::string ActorName = SGenUtility::SjisToUtf8(mObjInfo.GetString(objEntry, "name"));
                if(ActorName.find("Tico") != std::string::npos && ActorName != "TicoCoin" && ActorName != "TicoComet"){
                    auto object = std::make_shared<STicoDOMNode>();
                    object->Deserialize(&mObjInfo, objEntry);
                    AddChild(object);
                } else if(ActorName.find("Kinopio") != std::string::npos){
                    auto object = std::make_shared<SToadDOMNode>();
                    object->Deserialize(&mObjInfo, objEntry);
                    AddChild(object);
                } else if(ActorName.find("Teresa") != std::string::npos){
                    auto object = std::make_shared<SBooDOMNode>();
                    object->Deserialize(&mObjInfo, objEntry);
                    AddChild(object);
                } else if (ActorName.find("Planet") != std::string::npos || ActorName == "AstroDomeComet") {
                    auto object = std::make_shared<SPlanetDOMNode>();
                    object->Deserialize(&mObjInfo, objEntry);
                    AddChild(object);                    
                } else if(ActorName.find("Astro") != std::string::npos){
                    auto object = std::make_shared<SAstroObjectDOMNode>();
                    object->Deserialize(&mObjInfo, objEntry);
                    AddChild(object);
                } else {
                    auto object = std::make_shared<SObjectDOMNode>();
                    object->Deserialize(&mObjInfo, objEntry);
                    AddChild(object);
                }
			}

		}
		if((strcmp(layer_file->name, "areaobjinfo") == 0 || strcmp(layer_file->name, "AreaObjInfo") == 0) && layer_file->data != nullptr){
            mAreaObjInfoFile = layer_file;
			bStream::CMemoryStream mAreaObjInfoFile((uint8_t*)layer_file->data, (size_t)layer_file->size, bStream::Endianess::Big, bStream::OpenMode::In);
			mAreaObjInfo.Load(&mAreaObjInfoFile);
			for(size_t areaObjEntry = 0; areaObjEntry < mAreaObjInfo.GetEntryCount(); areaObjEntry++){
                auto object = std::make_shared<SAreaObjectDOMNode>();
                object->Deserialize(&mAreaObjInfo, areaObjEntry);
                AddChild(object);  
            }
        }
    }
}

void SZoneLayerDOMNode::RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected) {
    //ImGui::Checkbox(fmt::format("##isVisible{}", mName).data(), &mVisible);
    ImGui::Text((mVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH));
    if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
        mVisible = !mVisible;
    }

    ImGui::SameLine();
    if(ImGui::TreeNode(mName.data())){

        bool treeOpen = ImGui::TreeNode("Objects");
        ImGui::SameLine();
        ImGui::Text(ICON_FK_PLUS_CIRCLE);
        if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
            auto object = std::make_shared<SObjectDOMNode>();
            AddChild(object);
            selected = object;
        }
        
        if(treeOpen){
            for (auto node : GetChildrenOfType<SObjectDOMNode>(EDOMNodeType::Object)){
                node->RenderHeirarchyUI(selected);
            }
            ImGui::TreePop();
        }

        treeOpen = ImGui::TreeNode("Area Objects");
        ImGui::SameLine();
        ImGui::Text(ICON_FK_PLUS_CIRCLE);
        if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
            auto object = std::make_shared<SAreaObjectDOMNode>();
            AddChild(object);
            selected = object;
        }

        if(treeOpen){
            for (auto node : GetChildrenOfType<SAreaObjectDOMNode>(EDOMNodeType::AreaObject)){
                node->RenderHeirarchyUI(selected);
            }
            
            ImGui::TreePop();
        }
        
        ImGui::TreePop();
    }

}

void SZoneLayerDOMNode::RenderDetailsUI() {

}

void SZoneLayerDOMNode::Render(std::vector<std::shared_ptr<J3DModelInstance>>& renderables, glm::mat4 transform, float dt){
    for(auto& object : GetChildrenOfType<SObjectDOMNode>(EDOMNodeType::Object)){
        object->Render(renderables, transform, dt);
    }
}

SZoneDOMNode::SZoneDOMNode(std::string name) : Super("Zone") {
    mTransform = glm::mat4(1);
    mType = EDOMNodeType::Zone;
    mName = name;
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
    ImGui::Text((mVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH));
    if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
        mVisible = !mVisible;
    }
    
    ImGui::SameLine();

    bool opened = ImGui::TreeNodeEx(mName.data(), GetIsSelected() ? ImGuiTreeNodeFlags_Selected : 0);
    if(ImGui::IsItemClicked() && !mIsMainZone){
        SetIsSelected(true);
        selected = GetSharedPtr<SZoneDOMNode>(EDOMNodeType::Zone);
    } else {
        SetIsSelected(false);
    }


    if(opened){
        ImGui::SameLine();

        ImGui::Text(ICON_FK_MINUS_CIRCLE);
        if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
            //Remove Zone Code goes here. Should be a call to Galaxy->RemoveZone

            GetParentOfType<SGalaxyDOMNode>(EDOMNodeType::Galaxy).lock()->RemoveZone(GetSharedPtr<SZoneDOMNode>(EDOMNodeType::Zone));
        }

        for (auto node : GetChildrenOfType<SZoneLayerDOMNode>(EDOMNodeType::ZoneLayer)){
            node->RenderHeirarchyUI(selected);
        }
        
        ImGui::TreePop();
    }
}

void SZoneDOMNode::LoadZone(std::filesystem::path zonePath){
    if(!std::filesystem::exists(zonePath)) return;

    mIsMainZone = false;

    mZoneArchivePath = zonePath;

    GCResourceManager.LoadArchive(zonePath.string().c_str(), &mZoneArchive);

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
        GCarcfile* stageObjInfo = GCResourceManager.GetFile(&mZoneArchive, std::filesystem::path("jmp/placement/common/stageobjinfo"));
        if(stageObjInfo == nullptr){
            stageObjInfo = GCResourceManager.GetFile(&mZoneArchive, std::filesystem::path("jmp/Placement/Common/StageObjInfo"));
        }

        std::vector<std::shared_ptr<SZoneDOMNode>> zoneNodes = Parent.lock()->GetChildrenOfType<SZoneDOMNode>(EDOMNodeType::Zone);
        std::vector<std::shared_ptr<SDOMNodeSerializable>> zones;

        for(auto& zone : zoneNodes){
            if(zone->GetName() != GetName()) zones.push_back(zone);
        }


        std::cout << "[Save Main Zone]: Saving Stage Object Info" << std::endl;
        bStream::CMemoryStream saveStream(mStageObjInfo.CalculateNewFileSize(zones.size()), bStream::Endianess::Big, bStream::OpenMode::Out);
        mStageObjInfo.Save(zones, saveStream);

        if(stageObjInfo != nullptr){
            gcReplaceFileData(stageObjInfo, saveStream.getBuffer(), saveStream.tell());
        }
    }

    std::cout << "[Save Zone]: Saving Zone " << mName << std::endl;
    for(auto& layer : GetChildrenOfType<SZoneLayerDOMNode>(EDOMNodeType::ZoneLayer)){
        layer->SaveLayer(&mZoneArchive);
    }

    GCResourceManager.SaveArchive(mZoneArchivePath.string().c_str(), &mZoneArchive);

}

std::map<std::string, std::pair<glm::mat4, int32_t>> SZoneDOMNode::LoadMainZone(std::filesystem::path zonePath){
    if(!std::filesystem::exists(zonePath)){
        std::cout << "Couldn't find zone " << zonePath << std::endl;
        return {};
    }

    mIsMainZone = true;
    mZoneArchivePath = zonePath;

    GCResourceManager.LoadArchive(zonePath.string().c_str(), &mZoneArchive);

    mZoneArchiveLoaded = true;

    std::map<std::string, std::pair<glm::mat4, int32_t>> zoneTransforms;

    // get parent and check game type
    GCarcfile* file = GCResourceManager.GetFile(&mZoneArchive, std::filesystem::path("jmp/placement/common/stageobjinfo"));
    if(file == nullptr){
        file = GCResourceManager.GetFile(&mZoneArchive, std::filesystem::path("jmp/Placement/Common/StageObjInfo"));
    }

    if(file == nullptr){
        std::cout << "Couldn't find " << std::string(file->name) << " for Galaxy" << std::endl;
        return zoneTransforms;
    }

    if(file->size != 0) {
        bStream::CMemoryStream StageObjInfoStream((uint8_t*)file->data, (size_t)file->size, bStream::Endianess::Big, bStream::OpenMode::In);
        mStageObjInfo.Load(&StageObjInfoStream);
        for(size_t stageObjEntry = 0; stageObjEntry < mStageObjInfo.GetEntryCount(); stageObjEntry++){
            std::string zoneName = mStageObjInfo.GetString(stageObjEntry, "name");
            glm::vec3 position = {mStageObjInfo.GetFloat(stageObjEntry, "pos_x"), mStageObjInfo.GetFloat(stageObjEntry, "pos_y"), mStageObjInfo.GetFloat(stageObjEntry, "pos_z")};
            glm::vec3 rotation = {mStageObjInfo.GetFloat(stageObjEntry, "dir_x"), mStageObjInfo.GetFloat(stageObjEntry, "dir_y"), mStageObjInfo.GetFloat(stageObjEntry, "dir_z")};

            zoneTransforms.insert({zoneName, {SGenUtility::CreateMTX({1,1,1}, rotation, position), mStageObjInfo.GetUnsignedInt(stageObjEntry, "l_id")}});
        }
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

void SZoneDOMNode::Render(std::vector<std::shared_ptr<J3DModelInstance>>& renderables, float dt){
    if(mVisible){
        std::vector<std::shared_ptr<SZoneLayerDOMNode>> zoneLayers = GetChildrenOfType<SZoneLayerDOMNode>(EDOMNodeType::ZoneLayer);

        for(auto& layer : zoneLayers){
            if(layer->GetVisible()) layer->Render(renderables, mTransform, dt);
        }
    }
}