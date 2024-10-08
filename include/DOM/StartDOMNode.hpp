#pragma once

#include <glm/glm.hpp>
#include "DOM/DOMNodeBase.hpp"
#include "io/BcsvIO.hpp"
#include <json.hpp>
#include <tuple>
#include <array>

class SStartObjDOMNode : public SDOMNodeSerializable {
    uint32_t mID;
protected:
    bool mVisible;

    uint32_t mMarioNo;
    int32_t mObjArg0;
    uint32_t mCameraId;
public:
    glm::mat4 mTransform;
    typedef SDOMNodeSerializable Super; 

    void RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected);
    void RenderDetailsUI();

    SStartObjDOMNode();
    ~SStartObjDOMNode();

    int32_t GetPickID() { return mPickId; }

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::StartObj){
            return true;
        }

        return Super::IsNodeType(type);
    }

    void Deserialize(SBcsvIO* bcsv, int entry);
    void Serialize(SBcsvIO* bcsv, int entry);
};