#pragma once
#include "context.h"
#include "archive.h"
#include "DOM/DOMNodeBase.hpp"
#include <filesystem>

class SGalaxyDOMNode : public SDOMNodeBase {
    EGameType mGame;
    std::string mGalaxyName;
    GCarchive mScenarioArchive;
    bool mGalaxyLoaded { false };


public:
    typedef SDOMNodeBase Super;
    
    SGalaxyDOMNode();
    ~SGalaxyDOMNode();

    void RenderScenarios(std::shared_ptr<SDOMNodeBase>& selected);
    void RenderZones(std::shared_ptr<SDOMNodeBase>& selected);
    void Render(float dt);

    void RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected);
    void RenderDetailsUI();

    void SaveGalaxy();
    bool LoadGalaxy(std::filesystem::path galaxy_path, EGameType game);

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::Galaxy){
            return true;
        }

        return Super::IsNodeType(type);
    }
};
