#include "DOM/ScenarioDOMNode.hpp"
#include <fmt/core.h>
#include "GenUtil.hpp"
#include "imgui.h"

SScenarioDOMNode::SScenarioDOMNode() : Super("scenario") {
    mType = EDOMNodeType::Scenario;
}

SScenarioDOMNode::~SScenarioDOMNode(){

}

void SScenarioDOMNode::Deserialize(SBcsvIO* bcsv, int entry){
    mScenarioName = bcsv->GetString(entry, "ScenarioName");
    mAppearPowerStarObj =  bcsv->GetString(entry, "AppearPowerStarObj");
    mScenarioNo = bcsv->GetUnsignedInt(entry, "ScenarioNo");
    mComet = bcsv->GetString(entry, "Comet");
}

void SScenarioDOMNode::Serialize(SBcsvIO* bcsv, int entry){

}


void SScenarioDOMNode::RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected){
    ImGui::Text(fmt::format("{0} : {1} {2}", mScenarioNo, mScenarioName, mComet.empty() ? "" : "["+mComet+"]").data());
}

void SScenarioDOMNode::RenderDetailsUI(){
    
}