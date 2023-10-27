#include "DOM/AreaObjectDOMNode.hpp"
#include "ModelCache.hpp"
#include "ResUtil.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <fmt/core.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <LightConfigs.hpp>


SAreaObjectDOMNode::SAreaObjectDOMNode() : Super("Area") {
    mType = EDOMNodeType::AreaObject;
    mTransform = glm::mat4(1);
    mObjArgNames = {"Obj_arg0","Obj_arg1","Obj_arg2","Obj_arg3","Obj_arg4","Obj_arg5","Obj_arg6","Obj_arg7"};
}

SAreaObjectDOMNode::~SAreaObjectDOMNode(){
    
}

void SAreaObjectDOMNode::Deserialize(SBcsvIO* bcsv, int entry){
    mName = bcsv->GetString(entry, "name");

    glm::vec3 position = {bcsv->GetFloat(entry, "pos_x"), bcsv->GetFloat(entry, "pos_y"), bcsv->GetFloat(entry, "pos_z")};
    glm::vec3 rotation = {bcsv->GetFloat(entry, "dir_x"), bcsv->GetFloat(entry, "dir_y"), bcsv->GetFloat(entry, "dir_z")};
    glm::vec3 scale = {bcsv->GetFloat(entry, "scale_x"), bcsv->GetFloat(entry, "scale_y"), bcsv->GetFloat(entry, "scale_z")};

    mTransform = SGenUtility::CreateMTX(scale, rotation, position);

    mLinkID = bcsv->GetUnsignedInt(entry, "l_id"); // this should just be regular ID, what?
    
    mSW_Appear = bcsv->GetUnsignedInt(entry, "SW_APPEAR");
    mSW_A = bcsv->GetUnsignedInt(entry, "SW_A");
    mSW_B = bcsv->GetUnsignedInt(entry, "SW_B");

    mChildObjID = bcsv->GetShort(entry, "ChildObjId");
    mSW_Sleep = bcsv->GetUnsignedInt(entry, "SW_SLEEP");

    mPathID = bcsv->GetShort(entry, "CommonPath_ID");
    mClippingGroupID = bcsv->GetShort(entry, "ClippingGroupId");
    mGroupID = bcsv->GetShort(entry, "GroupId");
    mDemoGroupID = bcsv->GetShort(entry, "DemoGroupId");
    mMapPartsID = bcsv->GetShort(entry, "MapParts_ID");
    mObjID = bcsv->GetShort(entry, "Obj_ID");


    // SMG2
    mPriority = bcsv->GetUnsignedInt(entry, "Priority");
    mSwitchAwake = bcsv->GetUnsignedInt(entry, "SW_AWAKE");
    mAreaShapeNo = bcsv->GetShort(entry, "AreaShapeNo");
}

void SAreaObjectDOMNode::Serialize(SBcsvIO* bcsv, int entry){
    glm::vec3 pos, scale, skew;
    glm::quat dir;
    glm::vec4 persp;


    glm::decompose(mTransform, scale, dir, pos, skew, persp);

    glm::vec3 rotation = glm::eulerAngles(dir);

    bcsv->SetString(entry, "name", SGenUtility::Utf8ToSjis(mName));
    bcsv->SetFloat(entry, "pos_x", pos.x);
    bcsv->SetFloat(entry, "pos_y", pos.y);
    bcsv->SetFloat(entry, "pos_z", pos.z);

    bcsv->SetFloat(entry, "dir_x", dir.x);
    bcsv->SetFloat(entry, "dir_y", dir.y);
    bcsv->SetFloat(entry, "dir_z", dir.z);

    bcsv->SetFloat(entry, "scale_x", scale.x);
    bcsv->SetFloat(entry, "scale_y", scale.y);
    bcsv->SetFloat(entry, "scale_z", scale.z);

    bcsv->SetUnsignedInt(entry, "l_id", mLinkID); // this should just be regular ID, what?
    
    bcsv->SetUnsignedInt(entry, "SW_APPEAR", mSW_Appear);
    bcsv->SetUnsignedInt(entry, "SW_A", mSW_A);
    bcsv->SetUnsignedInt(entry, "SW_B", mSW_B);

    bcsv->SetShort(entry, "ChildObjId", mChildObjID);
    bcsv->SetUnsignedInt(entry, "SW_SLEEP", mSW_Sleep);

    bcsv->SetShort(entry, "CommonPath_ID", mPathID);
    bcsv->SetShort(entry, "ClippingGroupId", mClippingGroupID);
    bcsv->SetShort(entry, "GroupId", mGroupID);
    bcsv->SetShort(entry, "DemoGroupId", mDemoGroupID);
    bcsv->SetShort(entry, "MapParts_ID", mMapPartsID);
    bcsv->SetShort(entry, "Obj_ID", mObjID);


    // SMG2
    bcsv->SetUnsignedInt(entry, "Priority", mPriority);
    bcsv->SetUnsignedInt(entry, "SW_AWAKE", mSwitchAwake);
    bcsv->SetShort(entry, "AreaShapeNo", mAreaShapeNo);
}

void SAreaObjectDOMNode::RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected){
    if(selected == GetSharedPtr<SAreaObjectDOMNode>(EDOMNodeType::AreaObject)){
        ImGui::TextColored(ImColor(0,255,0), fmt::format("{0}", mName.data()).c_str());
    } else {
        ImGui::Text(fmt::format("{0}", mName.data()).c_str());
    }
    if(ImGui::IsItemClicked(0)){
        selected = GetSharedPtr<SAreaObjectDOMNode>(EDOMNodeType::AreaObject);
    }

}

void SAreaObjectDOMNode::RenderDetailsUI(){
    glm::vec3 pos(mTransform[3]);
    ImGui::Text(fmt::format("Position: {0},{1},{2}", pos.x,pos.y,pos.z).c_str());
}

void SAreaObjectDOMNode::Render(std::vector<std::shared_ptr<J3DModelInstance>>& renderables, glm::mat4 transform, float dt){
}