#include <filesystem>
#include "ResUtil.hpp"
#include "DOM/ZoneDOMNode.hpp"
#include "compression.h"
#include "archive.h"
#include "imgui.h"

SZoneDOMNode::SZoneDOMNode() : Super("zone") {

}

SZoneDOMNode::~SZoneDOMNode(){

}

void SZoneDOMNode::RenderDetailsUI(){
    
}

void SZoneDOMNode::RenderHeirarchyUI(){
    if(ImGui::TreeNode(mName.data())){
        for (auto node : Children){
            node->RenderHeirarchyUI();
        }
        ImGui::TreePop();
    }
}

void SZoneDOMNode::LoadZoneArchive(std::string zone_path){
    if(!std::filesystem::exists(zone_path)) return;
    
    GCarchive zone_archive;
    GCResourceManager.LoadArchive(zone_path.data(), &zone_archive);

    //for(GCarcdir* dir = zone_archive.dirs; dir < zone_archive.dirs + zone_archive.dirnum; dir++){    
    //    if((std::string(dir->name).find("layer") != std::string::npos || std::string(dir->name).find("common") != std::string::npos) &&){
    //        //Load all layers
    //        
    //    }
    //}

    gcFreeArchive(&zone_archive);
}

void SZoneDOMNode::Deserialize(SBcsvIO* bcsv, int entry){
    mName = LGenUtility::SjisToUtf8(bcsv->GetString(entry, "ZoneName"));

}

void SZoneDOMNode::Serialize(){
    
}