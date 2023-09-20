#pragma once
#include "context.h"
#include "archive.h"
#include <J3D/J3DModelInstance.hpp>
#include "DOM/DOMNodeBase.hpp"
#include "io/BcsvIO.hpp"
#include <filesystem>

class SGalaxyDOMNode : public SDOMNodeBase {
    EGameType mGame;
    std::string mGalaxyName;
    std::string mScenarioArchivePath;
    GCarchive mScenarioArchive;
    bool mGalaxyLoaded { false };

    SBcsvIO mScenarioData, mZoneListData;

    //std::map<uint32_t, > mLightingConfigs;

public:
    typedef SDOMNodeBase Super;
    
    SGalaxyDOMNode();
    ~SGalaxyDOMNode();

    void RenderScenarios(std::shared_ptr<SDOMNodeBase>& selected);
    void RenderZones(std::shared_ptr<SDOMNodeBase>& selected);
    void Render(std::vector<std::weak_ptr<J3DModelInstance>>& renderables, float dt);

    void RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected);
    void RenderDetailsUI();

    void SaveGalaxy();
    void AddZone(std::filesystem::path zonePath);
    bool LoadGalaxy(std::filesystem::path galaxy_path, EGameType game);
    bool GetGalaxyLoaded() { return mGalaxyLoaded; }

    EGameType GetGame() { return mGame; }

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::Galaxy){
            return true;
        }

        return Super::IsNodeType(type);
    }
};
