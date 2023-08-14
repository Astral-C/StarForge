#pragma once

#include <glm/glm.hpp>
#include "DOM/DOMNodeBase.hpp"
#include "DOM/ObjectDOMNode.hpp"
#include "io/BcsvIO.hpp"
#include <json.hpp>
#include <tuple>
#include <array>

#include <J3D/Animation/J3DColorAnimationInstance.hpp>
#include <J3D/Animation/J3DAnimationInstance.hpp>
#include <J3D/J3DModelInstance.hpp>

class SToadDOMNode : public SObjectDOMNode {
    std::shared_ptr<J3DModelInstance> mGoodsModel;
public:
    typedef SObjectDOMNode Super; 

    void RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected) override;
    void RenderDetailsUI() override;

    SToadDOMNode();
    ~SToadDOMNode();

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::Toad){
            return true;
        }

        return Super::IsNodeType(type);
    }

    void Deserialize(SBcsvIO* bcsv, int entry) override;

    void Render(std::vector<std::shared_ptr<J3DModelInstance>>& renderables, glm::mat4 transform, float dt) override;

};