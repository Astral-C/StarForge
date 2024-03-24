#pragma once

#include <glm/glm.hpp>
#include "DOM/DOMNodeBase.hpp"
#include "DOM/ObjectDOMNode.hpp"
#include "io/BcsvIO.hpp"
#include <json.hpp>
#include <tuple>
#include <array>

#include <UPointSpriteManager.hpp>

class SSoundObjDOMNode : public SObjectDOMNode {
    CPointSprite mPointSprite;

public:
    typedef SObjectDOMNode Super; 

    void RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected) override;
    void RenderDetailsUI() override;

    SSoundObjDOMNode();
    ~SSoundObjDOMNode();

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::SoundObj){
            return true;
        }

        return Super::IsNodeType(type);
    }

    void Deserialize(SBcsvIO* bcsv, int entry) override;

};