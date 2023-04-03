#pragma once
#include "context.h"
#include "archive.h"
#include "DOM/DOMNodeBase.hpp"
#include <filesystem>

class SGalaxyDOMNode : public SDOMNodeBase {
    EGameType mGame;
    std::string mGalaxyName;
    GCarchive mScenarioArchive;


public:
    typedef SDOMNodeBase Super;

    SGalaxyDOMNode();
    ~SGalaxyDOMNode();

    void RenderScenarios();
    void RenderZones();

    void RenderHeirarchyUI();
    void RenderDetailsUI();

    bool LoadGalaxy(std::filesystem::path galaxy_path, EGameType game);

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::Galaxy){
            return true;
        }

        return Super::IsNodeType(type);
    }
};
