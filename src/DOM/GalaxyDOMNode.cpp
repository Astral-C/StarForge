#include "DOM/GalaxyDOMNode.hpp"
#include "DOM/ScenarioDOMNode.hpp"
#include "DOM/ZoneDOMNode.hpp"
#include "io/BcsvIO.hpp"
#include "ResUtil.hpp"
#include "imgui.h"

SGalaxyDOMNode::SGalaxyDOMNode() : Super("galaxy") {
    mType = EDOMNodeType::Galaxy;
}

SGalaxyDOMNode::~SGalaxyDOMNode(){
    if(mScenarioArchive.ctx != nullptr){
        gcFreeArchive(&mScenarioArchive);
    }
}

void SGalaxyDOMNode::RenderScenarios(){
    auto scenarios = GetChildrenOfType<SScenarioDOMNode>(EDOMNodeType::Scenario);  

    for (int i = 0; i < scenarios.size(); i++){
        scenarios.at(i)->RenderHeirarchyUI();
    }
        
}

void SGalaxyDOMNode::RenderZones(){
    auto zone = GetChildrenOfType<SZoneDOMNode>(EDOMNodeType::Zone);    

    for (int i = 0; i < zone.size(); i++){
        zone.at(i)->RenderHeirarchyUI();
    }
}

void SGalaxyDOMNode::RenderHeirarchyUI(){
	if (ImGui::TreeNode(mName.c_str())){
        auto scenarios = GetChildrenOfType<SScenarioDOMNode>(EDOMNodeType::Scenario);
        auto zone = GetChildrenOfType<SScenarioDOMNode>(EDOMNodeType::Zone);
        
        if(ImGui::TreeNode("Scenarios")){
            for (int i = 0; i < scenarios.size(); i++){
                scenarios.at(i)->RenderHeirarchyUI();
            }
            ImGui::TreePop();
        }
        
        if(ImGui::TreeNode("Zones")){
            for (int i = 0; i < zone.size(); i++){
                zone.at(i)->RenderHeirarchyUI();
            }
            ImGui::TreePop();
        }
        
        ImGui::TreePop();
    }
}

void SGalaxyDOMNode::RenderDetailsUI(){

}

bool SGalaxyDOMNode::LoadGalaxy(std::filesystem::path galaxy_path, EGameType game){
    //Load Scenario Nodes
    // What the fuck?
    mName = (galaxy_path / std::string(".")).parent_path().filename();
    mGame = game;

    SBcsvIO scenarios;
    SBcsvIO zones;

    //Get scenario bcsv (its the only file in galaxy_path)
    GCResourceManager.LoadArchive((galaxy_path / (mName + "Scenario.arc")).c_str(), &mScenarioArchive);

    for(GCarcfile* file = mScenarioArchive.files; file < mScenarioArchive.files + mScenarioArchive.filenum; file++){
        
        // Load Scenarios

        if(strcmp(file->name, "scenariodata.bcsv") == 0){
            SBcsvIO ScenarioData;
            bStream::CMemoryStream ScenarioDataStream((uint8_t*)file->data, (size_t)file->size, bStream::Endianess::Big, bStream::OpenMode::In);
            ScenarioData.Load(&ScenarioDataStream);
            for(size_t entry = 0; entry < ScenarioData.GetEntryCount(); entry++){
                auto scenario = std::make_shared<SScenarioDOMNode>();
                scenario->Deserialize(&ScenarioData, entry);
                AddChild(scenario);
            }
        }

        // Load all zones and all zone layers

        if(strcmp(file->name, "zonelist.bcsv") == 0){
            SBcsvIO ZoneData;
            bStream::CMemoryStream ZoneDataStream((uint8_t*)file->data, (size_t)file->size, bStream::Endianess::Big, bStream::OpenMode::In);
            ZoneData.Load(&ZoneDataStream);
            for(size_t entry = 0; entry < ZoneData.GetEntryCount(); entry++){
                auto zone = std::make_shared<SZoneDOMNode>();
                zone->Deserialize(&ZoneData, entry);
                zone->LoadZoneArchive((galaxy_path / (mName + ".arc")));
                AddChild(zone);
            }
        }
    }
}