#include "DOM/GalaxyDOMNode.hpp"
#include "DOM/ScenarioDOMNode.hpp"
#include "DOM/ObjectDOMNode.hpp"
#include "DOM/ZoneDOMNode.hpp"
#include "ResUtil.hpp"
#include "imgui.h"
#include <map>

#include <LightConfigs.hpp>

#include "IconsForkAwesome.h"

SGalaxyDOMNode::SGalaxyDOMNode() : Super("galaxy") {
    mType = EDOMNodeType::Galaxy;
}

SGalaxyDOMNode::~SGalaxyDOMNode(){}

void SGalaxyDOMNode::SaveGalaxy(){
    auto zones = GetChildrenOfType<SDOMNodeSerializable>(EDOMNodeType::Zone);
    auto scenarios = GetChildrenOfType<SDOMNodeSerializable>(EDOMNodeType::Scenario);

    for(auto& zone : GetChildrenOfType<SZoneDOMNode>(EDOMNodeType::Zone)){
        zone->SaveZone();
    }

    std::shared_ptr<Archive::File> scenarioFile;
    std::shared_ptr<Archive::File> zoneFile;

    if(mGame == EGameType::SMG1){
        scenarioFile = mScenarioArchive->Get<Archive::File>(std::filesystem::path("scenariodata.bcsv"));///GCResourceManager.GetFile(&mScenarioArchive, std::filesystem::path("scenariodata.bcsv"));
    } else {
        scenarioFile = mScenarioArchive->Get<Archive::File>(std::filesystem::path("ScenarioData.bcsv"));
    }

    if(mGame == EGameType::SMG1){
        zoneFile = mScenarioArchive->Get<Archive::File>(std::filesystem::path("zonelist.bcsv"));
    } else {
        zoneFile = mScenarioArchive->Get<Archive::File>(std::filesystem::path("ZoneList.bcsv"));
    }


    bStream::CMemoryStream scenarioSaveStream(mScenarioData.CalculateNewFileSize(scenarios.size()), bStream::Endianess::Big, bStream::OpenMode::Out);
    mScenarioData.Save(scenarios, scenarioSaveStream);

    if(scenarioFile != nullptr){
        scenarioFile->SetData(scenarioSaveStream.getBuffer(), scenarioSaveStream.getSize());
    }


    bStream::CMemoryStream zoneSaveStream(mZoneListData.CalculateNewFileSize(zones.size()), bStream::Endianess::Big, bStream::OpenMode::Out);
    mZoneListData.Save(zones, zoneSaveStream, [](SBcsvIO* bcsv, int entry, std::shared_ptr<SDOMNodeSerializable> node){  
        bcsv->SetString(entry, "ZoneName", std::dynamic_pointer_cast<SZoneDOMNode>(node)->GetName());
    });
    

    if(zoneFile != nullptr){
        zoneFile->SetData(zoneSaveStream.getBuffer(), zoneSaveStream.getSize());
    }

    // write and compress
    mScenarioArchive->SaveToFile(mScenarioArchivePath, Compression::Format::YAZ0);

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

void SGalaxyDOMNode::AddZone(std::filesystem::path zonePath){
	std::shared_ptr<SZoneDOMNode> newZone = std::make_shared<SZoneDOMNode>(zonePath.stem().string());

	newZone->LoadZone(zonePath, mGame);

	auto scenarios = GetChildrenOfType<SScenarioDOMNode>(EDOMNodeType::Scenario);
	for(auto scenario : scenarios){
        std::cout << "[Add Zone]: Adding Zone " << zonePath.stem().string() << std::endl;
		scenario->AddZone(zonePath.stem().string());
	}

    mScenarioData.AddField(zonePath.stem().string(), EJmpFieldType::Integer);

	AddChild(newZone);
}

void SGalaxyDOMNode::RemoveZone(std::shared_ptr<SZoneDOMNode> zone){
        
    auto scenarios = GetChildrenOfType<SScenarioDOMNode>(EDOMNodeType::Scenario);
    for(auto scenario : scenarios){
        scenario->RemoveZone(zone->GetName());
    }

    mScenarioData.RemoveField(zone->GetName());

    RemoveChild(zone);
}

bool SGalaxyDOMNode::LoadGalaxy(std::filesystem::path galaxy_path, EGameType game){
    //Load Scenario Nodes
    // What the fuck?
    mName = (galaxy_path / std::string(".")).parent_path().filename().string();
    mGame = game;

    SBcsvIO scenarios;
    SBcsvIO zones;

    mScenarioArchivePath = (galaxy_path / (mName + "Scenario.arc")).string().c_str();
    mScenarioArchive = Archive::Rarc::Create();

	if(!std::filesystem::exists(mScenarioArchivePath)){
		std::cout << "Couldn't open scenario archive " << mScenarioArchivePath << std::endl;
		return false;
	}

    {
        bStream::CFileStream scenarioArchive((galaxy_path / (mName + "Scenario.arc")).string(), bStream::Endianess::Big, bStream::OpenMode::In);
        if(!mScenarioArchive->Load(&scenarioArchive)){
            std::cout << "Couldn't mount scenario archive" << mScenarioArchivePath << std::endl;
            return false;
        }
    }

    std::shared_ptr<Archive::File> zoneFile, scenarioFile;

    if(mGame == EGameType::SMG1){
        zoneFile = mScenarioArchive->Get<Archive::File>(std::filesystem::path("zonelist.bcsv"));
        scenarioFile = mScenarioArchive->Get<Archive::File>(std::filesystem::path("scenariodata.bcsv"));

        // Load Lighting Configs
        std::shared_ptr<Archive::Rarc> lightDataArc = Archive::Rarc::Create();

        {
            bStream::CFileStream lightDataArcFile((Options.mRootPath / "files" / "ObjectData" / "LightData.arc").string(), bStream::Endianess::Big, bStream::OpenMode::In);

            lightDataArc->Load(&lightDataArcFile);
        }

        std::shared_ptr<Archive::File> lightDataFile = lightDataArc->Get<Archive::File>(std::filesystem::path("lightdata.bcsv"));
        
        if(lightDataFile != nullptr){
            SBcsvIO lightData;
            bStream::CMemoryStream LightDataStream(lightDataFile->GetData(), lightDataFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
            lightData.Load(&LightDataStream);

            LoadLightConfig("Weak", &lightData);
            LoadLightConfig("Strong", &lightData);
            LoadLightConfig("Planet", &lightData);
            LoadLightConfig("Player", &lightData);

        }

    } else {
        zoneFile = mScenarioArchive->Get<Archive::File>(std::filesystem::path("ZoneList.bcsv"));
        scenarioFile = mScenarioArchive->Get<Archive::File>(std::filesystem::path("ScenarioData.bcsv"));


        // Load Lighting Configs
        std::shared_ptr<Archive::Rarc> lightDataArc = Archive::Rarc::Create();

        {
            bStream::CFileStream lightDataArcFile((Options.mRootPath / "files" / "LightData" / "LightData.arc").string(), bStream::Endianess::Big, bStream::OpenMode::In);

            lightDataArc->Load(&lightDataArcFile);
        }

        std::shared_ptr<Archive::File> lightDataFile = lightDataArc->Get<Archive::File>(std::filesystem::path("LightData.bcsv"));
        
        if(lightDataFile != nullptr){
            SBcsvIO lightData;
            bStream::CMemoryStream LightDataStream(lightDataFile->GetData(), lightDataFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
            lightData.Load(&LightDataStream);

            LoadLightConfig("Weak", &lightData);
            LoadLightConfig("Strong", &lightData);
            LoadLightConfig("Planet", &lightData);
            LoadLightConfig("Player", &lightData);

        }
    }


    // Load all zones and all zone layers
    {
        bStream::CMemoryStream ZoneDataStream(zoneFile->GetData(), zoneFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
        mZoneListData.Load(&ZoneDataStream);

        // Manually load the main galaxy zone so we can get a list of zone transforms
        auto mainZone = std::make_shared<SZoneDOMNode>();
        mainZone->Deserialize(&mZoneListData, 0);
        
        std::filesystem::path mainZonePath;

        if(mGame == EGameType::SMG1){
            mainZonePath = galaxy_path.parent_path() / (mainZone->GetName() + ".arc");
        } else {
            mainZonePath = galaxy_path.parent_path() / mainZone->GetName() / (mainZone->GetName() + "Map.arc");
        }

        auto zoneTransforms = mainZone->LoadMainZone(mainZonePath);
        AddChild(mainZone);

        for(size_t entry = 1; entry < mZoneListData.GetEntryCount(); entry++){
            auto zone = std::make_shared<SZoneDOMNode>();
            zone->Deserialize(&mZoneListData, entry);

            if(mGame == EGameType::SMG1){
                zone->LoadZone(galaxy_path.parent_path() / (zone->GetName() + ".arc"), mGame);
            } else {
                zone->LoadZone(galaxy_path.parent_path() / zone->GetName() / (zone->GetName() + "Map.arc"), mGame);
            }

            if(zoneTransforms.count(zone->GetName())){
                zone->SetTransform(zoneTransforms.at(zone->GetName()).first);
                zone->SetLinkID(zoneTransforms.at(zone->GetName()).second); 
            }

            AddChild(zone);
        }

        // Link Objects
        // TODO: Come back to this 
        /*
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
        */

    }

    // Load Scenarios
    {
        bStream::CMemoryStream ScenarioDataStream(scenarioFile->GetData(), scenarioFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
        mScenarioData.Load(&ScenarioDataStream);
        for(size_t entry = 0; entry < mScenarioData.GetEntryCount(); entry++){

            auto scenario = std::make_shared<SScenarioDOMNode>();
            AddChild(scenario);

            scenario->Deserialize(&mScenarioData, entry);
        }
    }
    mGalaxyLoaded = true;
    return true;
}