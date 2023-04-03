#pragma once

#include <glm/glm.hpp>
#include "DOM/DOMNodeBase.hpp"
#include "io/BcsvIO.hpp"

class SZoneLayerDOMNode : public SDOMNodeBase {

public:
    typedef SDOMNodeBase Super; 

    void RenderHeirarchyUI();
    void RenderDetailsUI();

    SZoneLayerDOMNode();
    ~SZoneLayerDOMNode();

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::ZoneLayer){
            return true;
        }

        return Super::IsNodeType(type);
    }


};

class SZoneDOMNode : public SDOMNodeBase {
    glm::mat4 transform;


public:
    typedef SDOMNodeBase Super; 

    void RenderHeirarchyUI();
    void RenderDetailsUI();

    SZoneDOMNode();
    ~SZoneDOMNode();

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::Zone){
            return true;
        }

        return Super::IsNodeType(type);
    }

    void LoadZoneArchive(std::string zone_path);
    void Deserialize(SBcsvIO* bcsv, int entry);
    void Serialize();

};