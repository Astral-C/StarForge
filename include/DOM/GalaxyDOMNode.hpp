#pragma once
#include <filesystem>
#include "context.h"
#include "archive.h"
#include <J3D/Data/J3DModelInstance.hpp>
#include "DOM/DOMNodeBase.hpp"
#include "DOM/ZoneDOMNode.hpp"
#include "DOM/ScenarioDOMNode.hpp"
#include "io/BcsvIO.hpp"

class SGalaxyDOMNode : public SDOMNodeBase {
    EGameType mGame;
    std::string mGalaxyName;
    std::string mScenarioArchivePath;
    GCarchive mScenarioArchive;
    bool mGalaxyLoaded { false };

    // BCSV files loaded for scenario and zone list
    SBcsvIO mScenarioData, mZoneListData;

    // The current scenario selected by the user. Separate from the normal selection since it controls state for rendering etc
    std::shared_ptr<SScenarioDOMNode> mSelectedScenario { nullptr };

    //std::map<uint32_t, > mLightingConfigs;

public:
    typedef SDOMNodeBase Super;
    
    SGalaxyDOMNode();
    ~SGalaxyDOMNode();

    void RenderScenarios(std::shared_ptr<SDOMNodeBase>& selected);
    void RenderZones(std::shared_ptr<SDOMNodeBase>& selected);
    void Render(std::vector<std::shared_ptr<J3DModelInstance>>& renderables, float dt);

    void RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected);
    void RenderDetailsUI();

    void SaveGalaxy();
    void AddZone(std::filesystem::path zonePath);
    void RemoveZone(std::shared_ptr<SZoneDOMNode> zone);
    bool LoadGalaxy(std::filesystem::path galaxy_path, EGameType game);
    bool GetGalaxyLoaded() { return mGalaxyLoaded; }

    std::shared_ptr<SScenarioDOMNode> GetSelectedScenario() { return mSelectedScenario; }
    void SetSelectedScenario(std::shared_ptr<SScenarioDOMNode> scenario) { mSelectedScenario = scenario; }

    EGameType GetGame() { return mGame; }

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::Galaxy){
            return true;
        }

        return Super::IsNodeType(type);
    }
};
