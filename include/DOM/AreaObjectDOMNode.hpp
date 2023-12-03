#pragma once

#include <glm/glm.hpp>
#include "DOM/DOMNodeBase.hpp"
#include "io/BcsvIO.hpp"
#include <json.hpp>
#include <tuple>
#include <array>

#include <J3D/Animation/J3DColorAnimationInstance.hpp>
#include <J3D/Animation/J3DAnimationInstance.hpp>
#include <J3D/Data/J3DModelInstance.hpp>

class SAreaObjectDOMNode : public SDOMNodeSerializable {
    uint32_t mID;
protected:

    uint32_t mLinkID;
    uint32_t mSW_Appear;
    uint32_t mSW_A, mSW_B, mSW_Sleep;

    uint16_t mPathID;
    uint16_t mClippingGroupID;
    uint16_t mGroupID;
    uint16_t mDemoGroupID;
    uint16_t mMapPartsID;

    uint16_t mChildObjID;
    uint16_t mObjID;
    
    // Galaxy 2
    uint32_t mSwitchAwake;
    uint16_t mAreaShapeNo;
    uint32_t mPriority;

public:
    std::array<std::string, 8> mObjArgNames;
    std::array<int, 8> mObjArgs;

    glm::mat4 mTransform;
    typedef SDOMNodeSerializable Super; 

    void RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected);
    void RenderDetailsUI();

    SAreaObjectDOMNode();
    ~SAreaObjectDOMNode();

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::AreaObject){
            return true;
        }

        return Super::IsNodeType(type);
    }

    void Deserialize(SBcsvIO* bcsv, int entry);
    void Serialize(SBcsvIO* bcsv, int entry);

    virtual void Render(std::vector<std::shared_ptr<J3DModelInstance>>& renderables, glm::mat4 transform, float dt);
    uint32_t GetLinkID() { return mLinkID; }


};