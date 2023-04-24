#include "DOM/ScenarioDOMNode.hpp"
#include "DOM/GalaxyDOMNode.hpp"
#include "DOM/ZoneDOMNode.hpp"
#include <fmt/core.h>
#include "GenUtil.hpp"
#include "imgui.h"

const char* StarTypeNames[] = {
    "Normal",
    "Hidden",
    "Green"
};

const char* CometTypeNames[] = {
    "",
    "Red",
    "Purple",
    "Dark",
    "Exterminate",
    "Mimic",
    "Quick"
};

SScenarioDOMNode::SScenarioDOMNode() : Super("scenario") {
    mType = EDOMNodeType::Scenario;
}

SScenarioDOMNode::~SScenarioDOMNode(){

}

void SScenarioDOMNode::Deserialize(SBcsvIO* bcsv, int entry){
    mScenarioName = bcsv->GetString(entry, "ScenarioName");
    mAppearPowerStarObj =  bcsv->GetString(entry, "AppearPowerStarObj");
    mScenarioNo = bcsv->GetUnsignedInt(entry, "ScenarioNo");
    mPowerStarType = bcsv->GetUnsignedInt(entry, "Type");
    mComet = bcsv->GetString(entry, "Comet");

    auto zones = GetParentOfType<SGalaxyDOMNode>(EDOMNodeType::Galaxy).lock()->GetChildrenOfType<SZoneDOMNode>(EDOMNodeType::Zone);

    for(auto& zone : zones){
        if(!mZoneLayers.contains(zone->GetName())){
            mZoneLayers.insert({zone->GetName(), bcsv->GetUnsignedInt(entry, zone->GetName())});
        }
    }
}

void SScenarioDOMNode::Serialize(SBcsvIO* bcsv, int entry){

}


void SScenarioDOMNode::RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected){
    if(selected == GetSharedPtr<SScenarioDOMNode>(EDOMNodeType::Scenario)){
        ImGui::TextColored(ImColor(0,255,0),fmt::format("{0} : {1} {2}", mScenarioNo, mScenarioName, mComet.empty() ? "" : "["+mComet+"]").data());
    } else {
        ImGui::Text(fmt::format("{0} : {1} {2}", mScenarioNo, mScenarioName, mComet.empty() ? "" : "["+mComet+"]").data());
    }

    if(ImGui::IsItemClicked(0)){
        selected = GetSharedPtr<SScenarioDOMNode>(EDOMNodeType::Scenario);
        //show and hide layers based on stuff

        // Get all the zones in the galaxy
        
        auto zones = GetParentOfType<SGalaxyDOMNode>(EDOMNodeType::Galaxy).lock()->GetChildrenOfType<SZoneDOMNode>(EDOMNodeType::Zone);
        for(auto& zone : zones){
            if(mZoneLayers.contains(zone->GetName())){
                auto layers = zone->GetChildrenOfType<SZoneLayerDOMNode>(EDOMNodeType::ZoneLayer);
                // assume theyre in order for now. they should be.
                uint32_t l = 0;
                for(auto& layer : layers){
                    if(layer->GetName() != "common" && layer->GetName() != "Common"){
                        layer->SetVisible(mZoneLayers[zone->GetName()] & (1 << l++));
                    }
                }
            }
        }
        
    }

}

void SScenarioDOMNode::RenderDetailsUI(){
    /*
    if(ImGui::BeginCombo("Star Type", StarTypeNames[mPowerStarType], 0)){
        for(int type = 0; type < IM_ARRAYSIZE(StarTypeNames); type++){
            bool is_selected = (mPowerStarType == type);
            if (ImGui::Selectable(StarTypeNames[type], is_selected)){
                mPowerStarType = type;
            }
            if (is_selected){
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    */

    ImGui::Text("Comet Type");
    ImGui::SameLine();
    if(ImGui::BeginCombo("##cometType", mComet.c_str(), 0)){
        for(auto cometType : CometTypeNames){
            bool is_selected = (mComet == std::string(cometType));
            if (ImGui::Selectable(std::string(cometType) == "" ? "##None" : cometType, is_selected)){
                mComet = std::string(cometType);
            }
            if (is_selected){
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}