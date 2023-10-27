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

class SPlanetDOMNode : public SObjectDOMNode {
    std::shared_ptr<J3DModelInstance> mWaterRenderable, mBloomRenderable; // add bloom and indirect
public:
    typedef SObjectDOMNode Super; 


    SPlanetDOMNode();
    ~SPlanetDOMNode();

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::Planet){
            return true;
        }

        return Super::IsNodeType(type);
    }

    void Deserialize(SBcsvIO* bcsv, int entry) override;

    void Render(std::vector<std::shared_ptr<J3DModelInstance>>& renderables, glm::mat4 transform, float dt) override;

};