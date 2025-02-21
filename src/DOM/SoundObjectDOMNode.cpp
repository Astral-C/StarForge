#include "DOM/SoundObjectDOMNode.hpp"
#include "ResUtil.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <format>
#include <glm/gtx/matrix_decompose.hpp>
#include "IconsForkAwesome.h"


SSoundObjDOMNode::SSoundObjDOMNode() : Super() {
    mType = EDOMNodeType::SoundObj;
    mTransform = glm::mat4(1);
}

SSoundObjDOMNode::~SSoundObjDOMNode(){
    
}

void SSoundObjDOMNode::Deserialize(SBcsvIO* bcsv, int entry){
    mName = SGenUtility::SjisToUtf8(bcsv->GetString(entry, "name"));

    glm::vec3 position = {bcsv->GetFloat(entry, "pos_x"), bcsv->GetFloat(entry, "pos_y"), bcsv->GetFloat(entry, "pos_z")};
    glm::vec3 rotation = {bcsv->GetFloat(entry, "dir_x"), bcsv->GetFloat(entry, "dir_y"), bcsv->GetFloat(entry, "dir_z")};
    glm::vec3 scale = {bcsv->GetFloat(entry, "scale_x"), bcsv->GetFloat(entry, "scale_y"), bcsv->GetFloat(entry, "scale_z")};

    mTransform = SGenUtility::CreateMTX(scale, rotation, position);

    for (size_t i = 0; i < 3; i++){
        mObjArgs[i] = bcsv->GetSignedInt(entry, std::format("Obj_arg{0}", i));
    }

    //Collect arg types from object DB and load data accordingly into array of obj/path args

    mLinkID = bcsv->GetSignedInt(entry, "l_id");
    mSW_Appear = bcsv->GetSignedInt(entry, "SW_APPEAR");
    mSW_A = bcsv->GetSignedInt(entry, "SW_A");
    mSW_B = bcsv->GetSignedInt(entry, "SW_B");
    mCommonPathID = bcsv->GetShort(entry, "CommonPath_ID");

    // check smg2 for these
    mSwitchAwake = bcsv->GetSignedInt(entry, "SW_AWAKE");
    mSwitchParam = bcsv->GetSignedInt(entry, "SW_PARAM");
    mParamScale = bcsv->GetFloat(entry, "ParamScale");
    mObjID = bcsv->GetShort(entry, "Obj_ID");
    mGeneratorID = bcsv->GetShort(entry, "GeneratorID");

}

void SSoundObjDOMNode::RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected){
    ImGui::Text((mVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH));
    if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
        mVisible = !mVisible;
    }
    
    ImGui::SameLine();

    if(selected == GetSharedPtr<SSoundObjDOMNode>(EDOMNodeType::Object)){
        ImGui::TextColored(ImColor(0,255,0), mName.data());
    } else if(selected == mLinkedObject.lock()) {
        ImGui::TextColored(ImColor(0,255,150), std::format("{0} [Linked]", mName.data()).c_str());
    } else {
        ImGui::Text(mName.data());
    }
    if(ImGui::IsItemClicked(0)){
        selected = GetSharedPtr<SSoundObjDOMNode>(EDOMNodeType::SoundObj);
    }
    ImGui::SameLine();

    ImGui::Text(ICON_FK_MINUS_CIRCLE);
    if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
        //should check this lock but whatever
        GetParentOfType<SDOMNodeBase>(EDOMNodeType::ZoneLayer).lock()->RemoveChild(GetSharedPtr<SSoundObjDOMNode>(EDOMNodeType::SoundObj));
    }
}

void SSoundObjDOMNode::RenderDetailsUI(){
    glm::vec3 pos(mTransform[3]);
    ImGui::Text(std::format("Position: {0},{1},{2}", pos.x,pos.y,pos.z).c_str());

    //ImGui::InputText("Name", &mName);
    
    for (size_t i = 1; i < 3; i++){
        ImGui::InputInt(mObjArgNames[i].data(), &mObjArgs[i]);
    }
}
