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
    mPowerStarId = bcsv->GetUnsignedInt(entry, "PowerStarId");
    mAppearPowerStarObj =  bcsv->GetString(entry, "AppearPowerStarObj");
    mScenarioNo = bcsv->GetUnsignedInt(entry, "ScenarioNo");
    mPowerStarType = bcsv->GetUnsignedInt(entry, "Type");
    mComet = bcsv->GetString(entry, "Comet");

    mLuigiModeTimer = bcsv->GetUnsignedInt(entry, "LuigiModeTimer");

    mIsHidden = bcsv->GetUnsignedInt(entry, "IsHidden");
    mErrorCheck = bcsv->GetUnsignedInt(entry, "ErrorCheck");


    auto zones = GetParentOfType<SGalaxyDOMNode>(EDOMNodeType::Galaxy).lock()->GetChildrenOfType<SZoneDOMNode>(EDOMNodeType::Zone);

    for(auto& zone : zones){
        if(!mZoneLayers.contains(zone->GetName())){
            mZoneLayers.insert({zone->GetName(), bcsv->GetUnsignedInt(entry, zone->GetName())});
        }
    }
    mSelectedZone = mZoneLayers.begin()->first;
}

void SScenarioDOMNode::Serialize(SBcsvIO* bcsv, int entry){
    bcsv->SetString(entry, "ScenarioName", mScenarioName);
    bcsv->SetUnsignedInt(entry, "PowerStarId", mPowerStarId);
    bcsv->SetString(entry, "AppearPowerStarObj", mAppearPowerStarObj);
    bcsv->SetUnsignedInt(entry, "ScenarioNo", mScenarioNo);
    bcsv->SetUnsignedInt(entry, "Type", mPowerStarType);
    bcsv->SetString(entry, "Comet", mComet);
    
    bcsv->SetUnsignedInt(entry, "LuigiModeTimer", mLuigiModeTimer);

    auto zones = GetParentOfType<SGalaxyDOMNode>(EDOMNodeType::Galaxy).lock()->GetChildrenOfType<SZoneDOMNode>(EDOMNodeType::Zone);

    for(auto& zone : zones){
        bcsv->SetUnsignedInt(entry, zone->GetName(), mZoneLayers[zone->GetName()]);
    }

    bcsv->SetUnsignedInt(entry, "IsHidden", mIsHidden);
    bcsv->SetUnsignedInt(entry, "ErrorCheck", mErrorCheck);
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

    ImGui::Text("Enabled Layers");
    ImGui::Separator();
    if(ImGui::BeginCombo("##zoneSelected", mSelectedZone.c_str(), 0)){
        for(auto [zoneName, enabled] : mZoneLayers){
            bool is_selected = (mSelectedZone == zoneName);
            if (ImGui::Selectable(zoneName.c_str(), is_selected)){
                mSelectedZone = zoneName;
            }
            if (is_selected){
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }


    if(ImGui::BeginTable("##layerEnable", 4)){
        for(int layer = 0; layer < 16; layer++){

            ImGui::TableNextColumn();

            bool enabled = mZoneLayers[mSelectedZone] & (1 << layer);
            if(ImGui::Checkbox(fmt::format("{}", char('A' + layer)).c_str(), &enabled)){
                mZoneLayers[mSelectedZone] ^= (1 << layer);

                // Reshow-hide layers
                auto zones = GetParentOfType<SGalaxyDOMNode>(EDOMNodeType::Galaxy).lock()->GetChildrenOfType<SZoneDOMNode>(EDOMNodeType::Zone);
                for(auto& zone : zones){
                    if(zone->GetName() == mSelectedZone){
                        auto layers = zone->GetChildrenOfType<SZoneLayerDOMNode>(EDOMNodeType::ZoneLayer);
                        // assume theyre in order for now. they should be.
                        uint32_t l = 0;
                        for(auto& layer : layers){
                            if(layer->GetName() != "common" && layer->GetName() != "Common"){
                                layer->SetVisible(mZoneLayers[mSelectedZone] & (1 << l++));
                            }
                        }
                    }
                }
            }

        }
        ImGui::EndTable();
    }
}