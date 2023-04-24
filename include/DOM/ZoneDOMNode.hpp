#pragma once

#include <glm/glm.hpp>
#include "DOM/DOMNodeBase.hpp"
#include "io/BcsvIO.hpp"
#include <map>

class SZoneLayerDOMNode : public SDOMNodeBase {
    bool mVisible;
    SBcsvIO mObjInfo;
    GCarcfile* mObjInfoFile;

public:
    typedef SDOMNodeBase Super; 

    void RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected);
    void RenderDetailsUI();
    void Render(glm::mat4 transform, float dt);

    void LoadLayer(GCarchive* zoneArchive, GCarcfile* layerDir, std::string layerName);
    void SaveLayer(GCarchive* zoneArchive);
    bool GetVisible(){ return mVisible; }
    void SetVisible(bool v) { mVisible = v; }

    SZoneLayerDOMNode();
    ~SZoneLayerDOMNode();


    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::ZoneLayer){
            return true;
        }

        return Super::IsNodeType(type);
    }


};

class SZoneDOMNode : public SDOMNodeSerializable {
    bool mIsMainZone { false };
    bool mZoneArchiveLoaded { false };
    int32_t mLinkID;
    
    SBcsvIO mStageObjInfo;
    GCarchive mZoneArchive;
    std::filesystem::path mZoneArchivePath;
    

public:
    glm::mat4 mTransform;
    typedef SDOMNodeSerializable Super; 

    void RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected);
    void RenderDetailsUI();
    void Render(float dt);

    SZoneDOMNode();
    ~SZoneDOMNode();

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::Zone){
            return true;
        }

        return Super::IsNodeType(type);
    }

    void LoadZone(std::string zonePath);
    void SaveZone();
    void SetTransform(glm::mat4 transform) { mTransform = transform; }
    void SetLinkID(int32_t link_id) { mLinkID = link_id; }

    // Special loader function for loading the main zone archive
    std::map<std::string, std::pair<glm::mat4, int32_t>> LoadMainZone(std::string zonePath);

    void Deserialize(SBcsvIO* bcsv, int entry);
    void Serialize(SBcsvIO* bcsv, int entry);

};