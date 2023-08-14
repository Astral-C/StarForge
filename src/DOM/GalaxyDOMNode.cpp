#include "DOM/GalaxyDOMNode.hpp"
#include "DOM/ScenarioDOMNode.hpp"
#include "DOM/ObjectDOMNode.hpp"
#include "DOM/ZoneDOMNode.hpp"
#include "ResUtil.hpp"
#include "imgui.h"
#include <map>

#include <LightConfigs.hpp>

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

    GCarcfile* scenarioFile;
    if(mGame == EGameType::SMG1){
        scenarioFile = GCResourceManager.GetFile(&mScenarioArchive, std::filesystem::path("scenariodata.bcsv"));
    } else {
        scenarioFile = GCResourceManager.GetFile(&mScenarioArchive, std::filesystem::path("ScenarioData.bcsv"));
    }
    auto scenarios = GetChildrenOfType<SDOMNodeSerializable>(EDOMNodeType::Scenario);


    bStream::CMemoryStream saveStream(mScenarioData.CalculateNewFileSize(scenarios.size()), bStream::Endianess::Big, bStream::OpenMode::Out);
    mScenarioData.Save(scenarios, saveStream);

    if(scenarioFile != nullptr){
        gcReplaceFileData(scenarioFile, saveStream.getBuffer(), saveStream.tell());
    }

    GCResourceManager.SaveArchive(mScenarioArchivePath.c_str(), &mScenarioArchive);

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

void SGalaxyDOMNode::Render(std::vector<std::shared_ptr<J3DModelInstance>>& renderables, float dt){
    for (auto& zone : GetChildrenOfType<SZoneDOMNode>(EDOMNodeType::Zone)){
        zone->Render(renderables, dt);
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

    mScenarioArchivePath = (galaxy_path / (mName + "Scenario.arc")).c_str();

	if(!std::filesystem::exists(mScenarioArchivePath)){
		std::cout << "Couldn't open scenario archive " << mScenarioArchivePath << std::endl;
		return false;
	}

    GCResourceManager.LoadArchive((galaxy_path / (mName + "Scenario.arc")).c_str(), &mScenarioArchive);
    mGalaxyLoaded = true;

    GCarcfile* zoneFile = nullptr, *scenarioFile = nullptr;
    if(mGame == EGameType::SMG1){
        zoneFile = GCResourceManager.GetFile(&mScenarioArchive, std::filesystem::path("zonelist.bcsv"));
        scenarioFile = GCResourceManager.GetFile(&mScenarioArchive, std::filesystem::path("scenariodata.bcsv"));

        // Load Lighting Configs
        GCarchive lightDataArc = {0};
        GCResourceManager.LoadArchive((Options.mRootPath / "files" / "ObjectData" / "LightData.arc").string().c_str(), &lightDataArc);

        GCarcfile* lightDataFile = GCResourceManager.GetFile(&lightDataArc, std::filesystem::path("lightdata.bcsv"));
        if(lightDataFile != nullptr){
            SBcsvIO lightData;
            bStream::CMemoryStream LightDataStream((uint8_t*)lightDataFile->data, (size_t)lightDataFile->size, bStream::Endianess::Big, bStream::OpenMode::In);
            lightData.Load(&LightDataStream);

            LoadLightConfig("Weak", &lightData);
            LoadLightConfig("Strong", &lightData);
            LoadLightConfig("Planet", &lightData);
            LoadLightConfig("Player", &lightData);

        }
        

        gcFreeArchive(&lightDataArc);

    } else {
        zoneFile = GCResourceManager.GetFile(&mScenarioArchive, std::filesystem::path("ZoneList.bcsv"));
        scenarioFile = GCResourceManager.GetFile(&mScenarioArchive, std::filesystem::path("ScenarioData.bcsv"));
    }


    // Load all zones and all zone layers
    {
        SBcsvIO ZoneData;
        bStream::CMemoryStream ZoneDataStream((uint8_t*)zoneFile->data, (size_t)zoneFile->size, bStream::Endianess::Big, bStream::OpenMode::In);
        ZoneData.Load(&ZoneDataStream);

        // Manually load the main galaxy zone so we can get a list of zone transforms
        auto mainZone = std::make_shared<SZoneDOMNode>();
        mainZone->Deserialize(&ZoneData, 0);
        
        std::filesystem::path mainZonePath;

        if(mGame == EGameType::SMG1){
            mainZonePath = galaxy_path.parent_path() / (mainZone->GetName() + ".arc");
        } else {
            mainZonePath = galaxy_path.parent_path() / mainZone->GetName() / (mainZone->GetName() + "Map.arc");
        }

        auto zoneTransforms = mainZone->LoadMainZone(mainZonePath);
        AddChild(mainZone);

        for(size_t entry = 1; entry < ZoneData.GetEntryCount(); entry++){
            auto zone = std::make_shared<SZoneDOMNode>();
            zone->Deserialize(&ZoneData, entry);

            if(mGame == EGameType::SMG1){
                zone->LoadZone(galaxy_path.parent_path() / (zone->GetName() + ".arc"));
            } else {
                zone->LoadZone(galaxy_path.parent_path() / zone->GetName() / (zone->GetName() + "Map.arc"));
            }

            if(zoneTransforms.count(zone->GetName())){
                zone->SetTransform(zoneTransforms.at(zone->GetName()).first);
                zone->SetLinkID(zoneTransforms.at(zone->GetName()).second); 
            }

            AddChild(zone);
        }

        //Link Objects
        std::vector<std::shared_ptr<SObjectDOMNode>> objects = GetChildrenOfType<SObjectDOMNode>(EDOMNodeType::Object);

        for(auto& object : objects){

            auto linked = std::find_if(objects.begin(), objects.end(), [&object](std::shared_ptr<SObjectDOMNode> searchObj){
                //This should really check for connection ID in the objects argument list
                return searchObj->GetLinkID() == object->GetLinkID() || (searchObj->GetName() == "EarthenPipe" && searchObj->mObjArgs[3] == object->mObjArgs[3]);
            });

            if(linked != objects.end()){
                object->SetLinked(*linked);
                (*linked)->SetLinked(object);
            }
        }

    }

    // Load Scenarios
    {
        bStream::CMemoryStream ScenarioDataStream((uint8_t*)scenarioFile->data, (size_t)scenarioFile->size, bStream::Endianess::Big, bStream::OpenMode::In);
        mScenarioData.Load(&ScenarioDataStream);
        for(size_t entry = 0; entry < mScenarioData.GetEntryCount(); entry++){

            auto scenario = std::make_shared<SScenarioDOMNode>();
            AddChild(scenario);

            scenario->Deserialize(&mScenarioData, entry);
        }
    }
    return true;
}