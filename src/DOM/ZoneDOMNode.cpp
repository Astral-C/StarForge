#include <filesystem>
#include "ResUtil.hpp"
#include "DOM/ZoneDOMNode.hpp"
#include "DOM/ObjectDOMNode.hpp"
#include "DOM/AreaObjectDOMNode.hpp"
#include "DOM/PlanetDOMNode.hpp"
#include "DOM/ToadDOMNode.hpp"
#include "DOM/TicoDOMNode.hpp"
#include "DOM/BlackHoleDOMNode.hpp"
#include "DOM/AstroObjectDOMNode.hpp"
#include "DOM/BooDOMNode.hpp"
#include "DOM/PathDOMNode.hpp"
#include <Archive.hpp>
#include "imgui.h"
#include "UStarForgeContext.hpp"
#include "IconsForkAwesome.h"
#include <fmt/core.h>
#include <glm/gtx/matrix_decompose.hpp>

SZoneLayerDOMNode::SZoneLayerDOMNode(std::string name) : Super("Layer") {
    mName = name;
    mType = EDOMNodeType::ZoneLayer;
    mVisible = true;
    mLayerName = "";
}

SZoneLayerDOMNode::~SZoneLayerDOMNode() {

}

void SZoneLayerDOMNode::SaveLayer(){
    std::vector<std::shared_ptr<SDOMNodeSerializable>> objects;

    if(Children.size() == 0) return;

    if(mObjInfoFile == nullptr){
        auto zone = GetParentOfType<SZoneDOMNode>(EDOMNodeType::Zone);
        std::shared_ptr<Archive::Folder> layerFolder = nullptr;

        if(zone.lock()->mGameType == EGameType::SMG1){
            layerFolder = zone.lock()->mZoneArchive->GetFolder(mLayerName);
            if(layerFolder == nullptr){
                std::shared_ptr<Archive::Folder> newLayerFolder = Archive::Folder::Create(zone.lock()->mZoneArchive);
                newLayerFolder->SetName(std::filesystem::path(mLayerName).stem());

                std::shared_ptr<Archive::File> newLayerObjInfo = Archive::File::Create();
                newLayerObjInfo->SetName("objinfo");
                newLayerFolder->AddFile(newLayerObjInfo);
                mObjInfo.FromTemplate(zone.lock()->mObjInfoTemplate);
                mObjInfoFile = newLayerObjInfo;

                zone.lock()->mZoneArchive->GetFolder("jmp/placement")->AddSubdirectory(newLayerFolder);
            }
        } else {
            layerFolder = zone.lock()->mZoneArchive->GetFolder(mLayerName);
            if(layerFolder == nullptr){
                std::shared_ptr<Archive::Folder> newLayerFolder = Archive::Folder::Create(zone.lock()->mZoneArchive);
                newLayerFolder->SetName(std::filesystem::path(mLayerName).stem());
                
                std::shared_ptr<Archive::File> newLayerObjInfo = Archive::File::Create();
                newLayerObjInfo->SetName("ObjInfo");
                newLayerFolder->AddFile(newLayerObjInfo);
                mObjInfo.FromTemplate(zone.lock()->mObjInfoTemplate);
                mObjInfoFile = newLayerObjInfo;

                zone.lock()->mZoneArchive->GetFolder("jmp/Placement")->AddSubdirectory(newLayerFolder);
            }
        }

    }

    objects.reserve(Children.size());

    for(auto child : GetChildrenOfType<SObjectDOMNode>(EDOMNodeType::Object)){
        std::cout << fmt::format("[Save Zone Layer {0}]: Adding object {1} type {2}", mName, child->GetName(), child->GetNodeTypeString()) << std::endl;
        objects.push_back(std::dynamic_pointer_cast<SDOMNodeSerializable>(child));
    }

    if(objects.size() == 0) {
        return;
    }

    bStream::CMemoryStream saveStream(mObjInfo.CalculateNewFileSize(objects.size()), bStream::Endianess::Big, bStream::OpenMode::Out);
    mObjInfo.Save(objects, saveStream);

    if(mObjInfoFile != nullptr){
        std::cout << "Saving objinfo" << std::endl;
        mObjInfoFile->SetData(saveStream.getBuffer(), saveStream.getSize());
    }

}

void SZoneLayerDOMNode::LoadLayerPaths(std::shared_ptr<Archive::Folder> layer){

    std::shared_ptr<Archive::File> commonPathInfoFile = layer->GetFile("commonpathinfo");
    if(commonPathInfoFile == nullptr){
        commonPathInfoFile = layer->GetFile("CommonPathInfo");
    }

    if(commonPathInfoFile == nullptr) return; // couldnt load path
    
    SBcsvIO commonPathInfo;
	bStream::CMemoryStream commonPathInfoStream(commonPathInfoFile->GetData(), commonPathInfoFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
	commonPathInfo.Load(&commonPathInfoStream);

    for(size_t pathEntry = 0; pathEntry < commonPathInfo.GetEntryCount(); pathEntry++){            
        auto path = std::make_shared<SPathDOMNode>();
        path->Deserialize(&commonPathInfo, pathEntry);
        std::cout << "Path " << path->GetName() << " Deserialized" << std::endl;
        
        std::shared_ptr<Archive::File> pathFile =  layer->GetFile(fmt::format("commonpathpointinfo.{}", path->GetPathNumber()));

        if(pathFile == nullptr){
            pathFile = layer->GetFile(fmt::format("CommonPathPointInfo.{}", path->GetPathNumber()));
        }

        if(pathFile == nullptr) continue;

        std::cout << "Loading Path " <<  path->GetName() << std::endl;

        SBcsvIO pathInfo;
	    bStream::CMemoryStream pathInfoStream(pathFile->GetData(), pathFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
	    pathInfo.Load(&pathInfoStream);

        for(size_t pointEntry = 0; pointEntry < pathInfo.GetEntryCount(); pointEntry++){
            auto point = std::make_shared<SPathPointDOMNode>();
            point->Deserialize(&pathInfo, pointEntry);
            path->AddChild(point);
        }
    
        path->Update();
        AddChild(path);
    }

}

void SZoneLayerDOMNode::LoadLayerObjects(std::shared_ptr<Archive::Folder> layer){
    if(layer == nullptr) return;

    mObjInfoFile = layer->GetFile("objinfo");
    if(mObjInfoFile == nullptr){
        mObjInfoFile = layer->GetFile("ObjInfo");
    }

    if(mObjInfoFile == nullptr) return; // couldnt load path
    
	bStream::CMemoryStream ObjInfoStream(mObjInfoFile->GetData(), mObjInfoFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
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
        } else if(ActorName == "BlackHole"){
            auto object = std::make_shared<SBlackHoleDOMNode>();
            object->Deserialize(&mObjInfo, objEntry);
            AddChild(object);
        } else {
            auto object = std::make_shared<SObjectDOMNode>();
            object->Deserialize(&mObjInfo, objEntry);
            AddChild(object);
        }
	}


    mAreaObjInfoFile = layer->GetFile("areaobjinfo");
    if(mAreaObjInfoFile == nullptr){
        mAreaObjInfoFile = layer->GetFile("AreaObjInfo");
    }

    if(mAreaObjInfoFile == nullptr) return; // couldnt load path
    
	bStream::CMemoryStream AreaInfoStream(mAreaObjInfoFile->GetData(), mAreaObjInfoFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
	mAreaObjInfo.Load(&AreaInfoStream);

	for(size_t areaObjEntry = 0; areaObjEntry < mAreaObjInfo.GetEntryCount(); areaObjEntry++){
        auto object = std::make_shared<SAreaObjectDOMNode>();
        object->Deserialize(&mAreaObjInfo, areaObjEntry);
        AddChild(object);  
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
        if(treeOpen){
            for (auto node : GetChildrenOfType<SObjectDOMNode>(EDOMNodeType::Object)){
                node->RenderHeirarchyUI(selected);
            }
            ImGui::Text(ICON_FK_PLUS_CIRCLE);
            if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
                auto object = std::make_shared<SObjectDOMNode>();
                AddChild(object);
                selected = object;
            }
            ImGui::TreePop();
        }

        treeOpen = ImGui::TreeNode("Area Objects");

        if(treeOpen){
            for (auto node : GetChildrenOfType<SAreaObjectDOMNode>(EDOMNodeType::AreaObject)){
                node->RenderHeirarchyUI(selected);
            }
            
            ImGui::Text(ICON_FK_PLUS_CIRCLE);
            if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
                auto object = std::make_shared<SAreaObjectDOMNode>();
                AddChild(object);
                selected = object;
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

SZoneDOMNode::~SZoneDOMNode(){}

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

        bool treeOpen = ImGui::TreeNode("Paths");

        if(treeOpen){
            for (auto node : GetChildrenOfType<SPathDOMNode>(EDOMNodeType::Path)){
                node->RenderHeirarchyUI(selected);
            }
            
            ImGui::Text(ICON_FK_PLUS_CIRCLE);
            if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
                auto object = std::make_shared<SPathDOMNode>();
                AddChild(object);
                selected = object;
            }
            ImGui::TreePop();
        }
        
        ImGui::TreePop();
    }
}

void SZoneDOMNode::LoadZone(std::filesystem::path zonePath, EGameType game){
    if(!std::filesystem::exists(zonePath)) return;

    mIsMainZone = false;

    mZoneArchivePath = zonePath;
    mZoneArchive = Archive::Rarc::Create();

    bStream::CFileStream zoneFileStream(zonePath, bStream::Endianess::Big, bStream::OpenMode::In);
    if(!mZoneArchive->Load(&zoneFileStream)){
        std::cout << "Error Loading Zone Archive" << std::endl;
        return;
    }

    auto layer = std::make_shared<SZoneLayerDOMNode>("Common");

    std::shared_ptr<Archive::Folder> layerCommonObjects = mZoneArchive->Get<Archive::Folder>("/jmp/placement/common");
    std::shared_ptr<Archive::Folder> layerCommonPaths = mZoneArchive->Get<Archive::Folder>("/jmp/path");

    mGameType = game;
    
    if(layerCommonObjects == nullptr){
        layerCommonObjects = mZoneArchive->Get<Archive::Folder>("/jmp/Placement/Common");
    }
    
    if(layerCommonPaths == nullptr){
        layerCommonPaths = mZoneArchive->Get<Archive::Folder>("/jmp/Path");
    }

    if(layerCommonObjects == nullptr || layerCommonPaths == nullptr) return;
    
    layer->LoadLayerObjects(layerCommonObjects);
    layer->LoadLayerPaths(layerCommonPaths);
    
    AddChild(layer);

    // Can layers other than common have paths?
    for(int l = 0; l < 16; l++){
        auto layer = std::make_shared<SZoneLayerDOMNode>(fmt::format("Layer {}", char('A'+l)));

        if(mGameType == EGameType::SMG1){
            layer->mLayerName = fmt::format("/jmp/placement/layer{}", char('a'+l));
        } else {
            layer->mLayerName = fmt::format("/jmp/Placement/Layer{}", char('A'+l));
        }

        std::shared_ptr<Archive::Folder> layerObjects = mZoneArchive->Get<Archive::Folder>(fmt::format("/jmp/placement/layer{}", char('a'+l)));

        if(layerObjects == nullptr){
            layerObjects = mZoneArchive->Get<Archive::Folder>(fmt::format("/jmp/Placement/Layer{}", char('A'+l)));
        }

        layer->LoadLayerObjects(layerObjects);
        
        AddChild(layer);
    }

    auto objInfoFile = mZoneArchive->Get<Archive::File>("/jmp/placement/common/objinfo");
    bStream::CMemoryStream objInfoStream(objInfoFile->GetData(), objInfoFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
    mObjInfoTemplate.Load(&objInfoStream);
    mObjInfoTemplate.Clear();

    mZoneArchiveLoaded = true;

}

void SZoneDOMNode::SaveZone(){

    if(!mZoneArchiveLoaded) return;

    if(mIsMainZone){ // Todo: per scenario!! same as regar objects
        std::shared_ptr<Archive::File> stageObjInfoFile = mZoneArchive->GetFile("jmp/placement/common/stageobjinfo");
        if(stageObjInfoFile == nullptr){
            stageObjInfoFile = mZoneArchive->GetFile("jmp/Placement/Common/StageObjInfo");
        }

        std::vector<std::shared_ptr<SZoneDOMNode>> zoneNodes = Parent.lock()->GetChildrenOfType<SZoneDOMNode>(EDOMNodeType::Zone);
        std::vector<std::shared_ptr<SDOMNodeSerializable>> zones;

        for(auto& zone : zoneNodes){
            if(zone->GetName() != GetName()) zones.push_back(zone);
        }


        std::cout << "[Save Main Zone]: Saving Stage Object Info" << std::endl;
        bStream::CMemoryStream saveStream(mStageObjInfo.CalculateNewFileSize(zones.size()), bStream::Endianess::Big, bStream::OpenMode::Out);
        mStageObjInfo.Save(zones, saveStream);

        if(stageObjInfoFile != nullptr){
            stageObjInfoFile->SetData(saveStream.getBuffer(), saveStream.getSize());
        }
    }

    std::cout << "[Save Zone]: Saving Zone " << mName << std::endl;
    for(auto& layer : GetChildrenOfType<SZoneLayerDOMNode>(EDOMNodeType::ZoneLayer)){
        layer->SaveLayer();
    }

    mZoneArchive->SaveToFile(mZoneArchivePath, Compression::Format::YAZ0, 9);

}

std::map<std::string, std::pair<glm::mat4, int32_t>> SZoneDOMNode::LoadMainZone(std::filesystem::path zonePath){
    if(!std::filesystem::exists(zonePath)){
        std::cout << "Couldn't find zone " << zonePath << std::endl;
        return {};
    }

    mIsMainZone = true;
    mZoneArchivePath = zonePath;
    mZoneArchive = Archive::Rarc::Create();

    bStream::CFileStream zoneFileStream(zonePath, bStream::Endianess::Big, bStream::OpenMode::In);
    if(!mZoneArchive->Load(&zoneFileStream)){
        std::cout << "Couldn't load zone archive " << zonePath << std::endl;
        return {};
    }

    mZoneArchiveLoaded = true;

    std::map<std::string, std::pair<glm::mat4, int32_t>> zoneTransforms;

    // get parent and check game type
    std::shared_ptr<Archive::File> file = mZoneArchive->Get<Archive::File>(std::filesystem::path("jmp/placement/common/stageobjinfo"));
    mGameType = EGameType::SMG1;

    if(file == nullptr){
        file = mZoneArchive->Get<Archive::File>(std::filesystem::path("jmp/Placement/Common/StageObjInfo"));
    }

    if(file == nullptr){
        std::cout << "Couldn't find Stage Obj file for Galaxy" << std::endl;
        return zoneTransforms;
    }

    auto layer = std::make_shared<SZoneLayerDOMNode>("Common");

    std::shared_ptr<Archive::File> layerCommonStageObj = mZoneArchive->Get<Archive::File>("/jmp/placement/common/stageobjinfo");
    std::shared_ptr<Archive::Folder> layerCommonObjects = mZoneArchive->Get<Archive::Folder>("/jmp/placement/common");
    std::shared_ptr<Archive::Folder> layerCommonPaths = mZoneArchive->Get<Archive::Folder>("/jmp/path");
    layer->mLayerName = fmt::format("/jmp/placement/common");    

    if(layerCommonStageObj == nullptr){
        layerCommonStageObj = mZoneArchive->Get<Archive::File>("/jmp/Placement/Common/StageObjInfo");
        layer->mLayerName = fmt::format("/jmp/Placement/Common");
    }

    if(layerCommonObjects == nullptr){
        layerCommonObjects = mZoneArchive->Get<Archive::Folder>("/jmp/Placement/Common");
    }
    
    if(layerCommonPaths == nullptr){
        layerCommonPaths = mZoneArchive->Get<Archive::Folder>("/jmp/Path");
    }

    if(layerCommonObjects == nullptr || layerCommonPaths == nullptr) return {};
    
    layer->LoadLayerObjects(layerCommonObjects);
    layer->LoadLayerPaths(layerCommonPaths);
    
    bStream::CMemoryStream StageObjInfoStream(layerCommonStageObj->GetData(), layerCommonStageObj->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
    mStageObjInfo.Load(&StageObjInfoStream);
    for(size_t stageObjEntry = 0; stageObjEntry < mStageObjInfo.GetEntryCount(); stageObjEntry++){
        std::string zoneName = mStageObjInfo.GetString(stageObjEntry, "name");
        
        glm::vec3 position = {mStageObjInfo.GetFloat(stageObjEntry, "pos_x"), mStageObjInfo.GetFloat(stageObjEntry, "pos_y"), mStageObjInfo.GetFloat(stageObjEntry, "pos_z")};
        glm::vec3 rotation = {mStageObjInfo.GetFloat(stageObjEntry, "dir_x"), mStageObjInfo.GetFloat(stageObjEntry, "dir_y"), mStageObjInfo.GetFloat(stageObjEntry, "dir_z")};
        
        zoneTransforms.insert({zoneName, {SGenUtility::CreateMTX({1,1,1}, rotation, position), mStageObjInfo.GetUnsignedInt(stageObjEntry, "l_id")}});
    }
    
    auto objInfoFile = mZoneArchive->Get<Archive::File>("/jmp/placement/common/objinfo");
    bStream::CMemoryStream objInfoStream(objInfoFile->GetData(), objInfoFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
    mObjInfoTemplate.Load(&objInfoStream);
    mObjInfoTemplate.Clear();

    AddChild(layer);

    // Can layers other than common have paths?
    for(int l = 0; l < 16; l++){
        auto layer = std::make_shared<SZoneLayerDOMNode>(fmt::format("Layer {}", char('A'+l)));

        std::shared_ptr<Archive::File> layerStageObj = mZoneArchive->Get<Archive::File>(fmt::format("jmp/placement/layer{}/stageobjinfo", char('a'+l)));
        std::shared_ptr<Archive::Folder> layerObjects = mZoneArchive->Get<Archive::Folder>(fmt::format("jmp/placement/layer{}", char('a'+l)));
        layer->mLayerName = fmt::format("layer{}", char('a'+l));

        if(layerStageObj == nullptr){
            layer->mLayerName = fmt::format("Layer{}", char('A'+l));
            layerStageObj = mZoneArchive->Get<Archive::File>(fmt::format("/jmp/Placement/Layer{}/StageObjInfo",  char('A'+l)));
        }

        if(layerObjects == nullptr){
            layerObjects = mZoneArchive->Get<Archive::Folder>(fmt::format("/jmp/Placement/Layer{}", char('A'+l)));
        }

        if(layerObjects != nullptr) layer->LoadLayerObjects(layerObjects);
        
        if(layerStageObj != nullptr){
            bStream::CMemoryStream LayerStageObjInfoStream(layerStageObj->GetData(), layerStageObj->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
            SBcsvIO layerStageObjInfo;
            layerStageObjInfo.Load(&LayerStageObjInfoStream);
            for(size_t stageObjEntry = 0; stageObjEntry < layerStageObjInfo.GetEntryCount(); stageObjEntry++){
                std::string zoneName = layerStageObjInfo.GetString(stageObjEntry, "name");
                
                glm::vec3 position = {layerStageObjInfo.GetFloat(stageObjEntry, "pos_x"), layerStageObjInfo.GetFloat(stageObjEntry, "pos_y"), layerStageObjInfo.GetFloat(stageObjEntry, "pos_z")};
                glm::vec3 rotation = {layerStageObjInfo.GetFloat(stageObjEntry, "dir_x"), layerStageObjInfo.GetFloat(stageObjEntry, "dir_y"), layerStageObjInfo.GetFloat(stageObjEntry, "dir_z")};
                
                zoneTransforms.insert({zoneName, {SGenUtility::CreateMTX({1,1,1}, rotation, position), layerStageObjInfo.GetUnsignedInt(stageObjEntry, "l_id")}});
            }
        }

        AddChild(layer);
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