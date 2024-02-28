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
#include <UPathRenderer.hpp>

#include <UCamera.hpp>

class SPathDOMNode : public SDOMNodeSerializable {
private:
    std::string mName;
    std::string mPathType;    

    bool mIsClosed;

    uint32_t mLinkID;
    std::array<std::string, 8> mPathArgNames;
    std::array<int, 8> mPathArgs;

    std::string mUsage;
    uint16_t mNo;
    uint16_t mPathID;

    CPathRenderer mRenderer;
    uint32_t mColor;

public:

    glm::mat4 mTransform;
    typedef SDOMNodeSerializable Super; 

    uint16_t GetPathNumber() { return mNo; }
    std::string GetName() { return mName; }

    void Update();
    void Render(USceneCamera* cam, glm::mat4 referenceFrame);
    void RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected);
    void RenderDetailsUI();

    SPathDOMNode();
    ~SPathDOMNode();

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::Path){
            return true;
        }

        return Super::IsNodeType(type);
    }

    void Deserialize(SBcsvIO* bcsv, int entry);
    void Serialize(SBcsvIO* bcsv, int entry);

};

class SPathPointDOMNode : public SDOMNodeSerializable {
private: 
    glm::vec3 mLeftHandle;
    glm::vec3 mRightHandle;
    
public:
    std::array<int, 8> mPointArgs;

    glm::mat4 mTransform;
    typedef SDOMNodeSerializable Super; 

    std::string GetName() { return mName; }

    void RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected);
    void RenderDetailsUI();

    SPathPointDOMNode();
    ~SPathPointDOMNode();

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::PathPoint){
            return true;
        }

        return Super::IsNodeType(type);
    }

    void Deserialize(SBcsvIO* bcsv, int entry);
    void Serialize(SBcsvIO* bcsv, int entry);

    glm::vec3 GetPosition() { 
        glm::vec3 pos, scale, skew;
        glm::quat dir;
        glm::vec4 persp;


        glm::decompose(mTransform, scale, dir, pos, skew, persp);
        return pos;
    }
    glm::vec3 GetLeftHandle() { return mLeftHandle; }
    glm::vec3 GetRightHandle() { return mRightHandle; }


};