#include "DOM/StartDOMNode.hpp"
#include "ModelCache.hpp"
#include "ResUtil.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <format>
#include <glm/gtx/matrix_decompose.hpp>
#include <LightConfigs.hpp>
#include "IconsForkAwesome.h"
#include <J3D/Picking/J3DPicking.hpp>

// yippiee circular includes!
#include "UStarForgeContext.hpp"

SStartObjDOMNode::SStartObjDOMNode() : Super("Start") {
    mType = EDOMNodeType::StartObj;
}

SStartObjDOMNode::~SStartObjDOMNode(){
    
}

void SStartObjDOMNode::Deserialize(SBcsvIO* bcsv, int entry){
    glm::vec3 position = {bcsv->GetFloat(entry, "pos_x"), bcsv->GetFloat(entry, "pos_y"), bcsv->GetFloat(entry, "pos_z")};
    glm::vec3 rotation = {bcsv->GetFloat(entry, "dir_x"), bcsv->GetFloat(entry, "dir_y"), bcsv->GetFloat(entry, "dir_z")};
    glm::vec3 scale = {bcsv->GetFloat(entry, "scale_x"), bcsv->GetFloat(entry, "scale_y"), bcsv->GetFloat(entry, "scale_z")};

    mTransform = SGenUtility::CreateMTX(scale, rotation, position);

    mMarioNo = bcsv->GetUnsignedInt(entry, "MarioNo");
    mObjArg0 = bcsv->GetUnsignedInt(entry, "Obj_arg0");
    mCameraId = bcsv->GetUnsignedInt(entry, "Camera_id");

    mName = std::format("Start {}", mMarioNo);

}

void SStartObjDOMNode::Serialize(SBcsvIO* bcsv, int entry){
    glm::vec3 pos, scale, skew;
    glm::quat dir;
    glm::vec4 persp;


    glm::decompose(mTransform, scale, dir, pos, skew, persp);

    glm::vec3 rotation = glm::eulerAngles(dir);

    bcsv->SetFloat(entry, "pos_x", pos.x);
    bcsv->SetFloat(entry, "pos_y", pos.y);
    bcsv->SetFloat(entry, "pos_z", pos.z);

    bcsv->SetFloat(entry, "dir_x", glm::degrees(rotation.x));
    bcsv->SetFloat(entry, "dir_y", glm::degrees(rotation.y));
    bcsv->SetFloat(entry, "dir_z", glm::degrees(rotation.z));

    bcsv->SetFloat(entry, "scale_x", scale.x);
    bcsv->SetFloat(entry, "scale_y", scale.y);
    bcsv->SetFloat(entry, "scale_z", scale.z);


    bcsv->SetUnsignedInt(entry, "MarioNo", mMarioNo);
    bcsv->SetUnsignedInt(entry, "Obj_arg0", mObjArg0);
    bcsv->SetUnsignedInt(entry, "Camera_id", mCameraId);

}

void SStartObjDOMNode::RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected){
    ImGui::Text((mVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH));
    if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
        mVisible = !mVisible;
    }
    ImGui::SameLine();
    if(selected == GetSharedPtr<SStartObjDOMNode>(EDOMNodeType::StartObj)){
        ImGui::TextColored(ImColor(0,255,0), std::format("{0}", mName).c_str());
        ImGui::SameLine();

        ImGui::Text(ICON_FK_MINUS_CIRCLE);
        if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
            //should check this lock but whatever
            GetParentOfType<SDOMNodeBase>(EDOMNodeType::ZoneLayer).lock()->RemoveChild(GetSharedPtr<SStartObjDOMNode>(EDOMNodeType::StartObj));
        }
    } else {
        ImGui::Text(std::format("{0}", mName).c_str());
    }
    if(ImGui::IsItemClicked(0)){
        selected = GetSharedPtr<SStartObjDOMNode>(EDOMNodeType::StartObj);
    }

}

void SStartObjDOMNode::RenderDetailsUI(){
    glm::vec3 pos(mTransform[3]);
    ImGui::Text(std::format("Position: {0},{1},{2}", pos.x,pos.y,pos.z).c_str());

    //bcsv->SetUnsignedInt(entry, "MarioNo", mMarioNo);
    //bcsv->SetUnsignedInt(entry, "Obj_arg0", mObjArg0);
    //bcsv->SetUnsignedInt(entry, "Camera_id", mCameraId);
    ImGui::InputInt("Obj Arg0", &mObjArg0);

}
