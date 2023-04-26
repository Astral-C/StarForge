#include "DOM/GalaxyDOMNode.hpp"
#include "DOM/ScenarioDOMNode.hpp"
#include "DOM/ZoneDOMNode.hpp"
#include "io/BcsvIO.hpp"
#include "ResUtil.hpp"
#include "imgui.h"
#include <map>

SGalaxyDOMNode::SGalaxyDOMNode() : Super("galaxy") {
    mType = EDOMNodeType::Galaxy;
}

SGalaxyDOMNode::~SGalaxyDOMNode(){
    if(mGalaxyLoaded){
        gcFreeArchive(&mScenarioArchive);
    }
}

void SGalaxyDOMNode::SaveGalaxy(){
    for(auto& zone : GetChildrenOfType<SZoneDOMNode>(EDOMNodeType::Zone)){
        zone->SaveZone();
    }
}

void SGalaxyDOMNode::RenderScenarios(std::shared_ptr<SDOMNodeBase>& selected){
    for (auto& scenario : GetChildrenOfType<SScenarioDOMNode>(EDOMNodeType::Scenario)){
        scenario->RenderHeirarchyUI(selected);
    }
        
}

void SGalaxyDOMNode::RenderZones(std::shared_ptr<SDOMNodeBase>& selected){
    for (auto& zone : GetChildrenOfType<SZoneDOMNode>(EDOMNodeType::Zone)){
        zone->RenderHeirarchyUI(selected);
    }
}

void SGalaxyDOMNode::RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected){
}

void SGalaxyDOMNode::Render(float dt){
    for (auto& zone : GetChildrenOfType<SZoneDOMNode>(EDOMNodeType::Zone)){
        zone->Render(dt);
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
    mGalaxyLoaded = true;

    GCarcfile* zoneFile = GCResourceManager.GetFile(&mScenarioArchive, std::filesystem::path("zonelist.bcsv"));
    GCarcfile* scenarioFile = GCResourceManager.GetFile(&mScenarioArchive, std::filesystem::path("scenariodata.bcsv"));


    // Load all zones and all zone layers
    {
        SBcsvIO ZoneData;
        bStream::CMemoryStream ZoneDataStream((uint8_t*)zoneFile->data, (size_t)zoneFile->size, bStream::Endianess::Big, bStream::OpenMode::In);
        ZoneData.Load(&ZoneDataStream);

        // Manually load the main galaxy zone so we can get a list of zone transforms
        auto mainZone = std::make_shared<SZoneDOMNode>();
        mainZone->Deserialize(&ZoneData, 0);
        
        auto zoneTransforms = mainZone->LoadMainZone(galaxy_path.parent_path() / (mainZone->GetName() + ".arc"));
        AddChild(mainZone);

        for(size_t entry = 1; entry < ZoneData.GetEntryCount(); entry++){
            auto zone = std::make_shared<SZoneDOMNode>();
            zone->Deserialize(&ZoneData, entry);
            zone->LoadZone(galaxy_path.parent_path() / (zone->GetName() + ".arc"));

            if(zoneTransforms.count(zone->GetName())){
                zone->SetTransform(zoneTransforms.at(zone->GetName()).first);
                zone->SetLinkID(zoneTransforms.at(zone->GetName()).second); 
            }

            AddChild(zone);
        }
    }

    // Load Scenarios
    {
        SBcsvIO ScenarioData;
        bStream::CMemoryStream ScenarioDataStream((uint8_t*)scenarioFile->data, (size_t)scenarioFile->size, bStream::Endianess::Big, bStream::OpenMode::In);
        ScenarioData.Load(&ScenarioDataStream);
        for(size_t entry = 0; entry < ScenarioData.GetEntryCount(); entry++){

            auto scenario = std::make_shared<SScenarioDOMNode>();
            AddChild(scenario);

            scenario->Deserialize(&ScenarioData, entry);
        }
    }
}