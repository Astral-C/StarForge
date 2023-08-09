#pragma once

#include <glm/glm.hpp>
#include "DOM/DOMNodeBase.hpp"
#include "io/BcsvIO.hpp"
#include <json.hpp>
#include <tuple>
#include <array>

#include <J3D/Animation/J3DColorAnimationInstance.hpp>
#include <J3D/Animation/J3DAnimationInstance.hpp>
#include <J3D/J3DModelInstance.hpp>

class SObjectDOMNode : public SDOMNodeSerializable {
    uint32_t mID;

    uint32_t mLinkID;
    uint32_t mCameraSetID;
    uint32_t mSW_Appear;
    uint32_t mSW_Dead;
    uint32_t mSW_A, mSW_B, mSW_Sleep;
    uint32_t mMessageID;
    uint32_t mCastID;
    uint32_t mViewGroupID;

    uint16_t mShapeModelNo;
    uint16_t mCommonPathID;
    uint16_t mClippingGroupID;
    uint16_t mGroupID;
    uint16_t mDemoGroupID;
    uint16_t mMapPartID;

    // Galaxy 2
    uint32_t mSwitchAwake;
    uint32_t mSwitchParam;
    float mParamScale;
    uint16_t mObjID;
    uint16_t mGeneratorID;
    

    //TODO: this should be a config instead of just a name, that way the UI can dynamically change based on the config

    std::array<std::string, 8> mPathArgNames;
    std::array<int, 8> mPathArgs;

    std::weak_ptr<SObjectDOMNode> mLinkedObject;

    std::shared_ptr<J3DModelInstance> mRenderable;

public:
    std::array<std::string, 8> mObjArgNames;
    std::array<int, 8> mObjArgs;

    glm::mat4 mTransform;
    typedef SDOMNodeSerializable Super; 

    void RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected);
    void RenderDetailsUI();

    SObjectDOMNode();
    ~SObjectDOMNode();

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::Object){
            return true;
        }

        return Super::IsNodeType(type);
    }

    void Deserialize(SBcsvIO* bcsv, int entry);
    void Serialize(SBcsvIO* bcsv, int entry);

   void Render(std::vector<std::shared_ptr<J3DModelInstance>>& renderables, glm::mat4 transform, float dt);
    uint32_t GetLinkID() { return mLinkID; }

    std::weak_ptr<SObjectDOMNode> GetLinked() { return mLinkedObject; }
    void SetLinked(std::shared_ptr<SObjectDOMNode> node) { mLinkedObject = node; }

    static void LoadObjectDB(nlohmann::json db);

};