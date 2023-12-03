#include "DOM/ObjectDOMNode.hpp"
#include "ModelCache.hpp"
#include "ResUtil.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <fmt/core.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <LightConfigs.hpp>
#include "IconsForkAwesome.h"

static nlohmann::json objectDB;

void SObjectDOMNode::LoadObjectDB(nlohmann::json db){
    objectDB = db;
}

SObjectDOMNode::SObjectDOMNode() : Super("Object") {
    mType = EDOMNodeType::Object;
    mTransform = glm::mat4(1);
    mObjArgNames = {"Obj_arg0","Obj_arg1","Obj_arg2","Obj_arg3","Obj_arg4","Obj_arg5","Obj_arg6","Obj_arg7"};
    mVisible = true;
}

SObjectDOMNode::~SObjectDOMNode(){
    
}

void SObjectDOMNode::Deserialize(SBcsvIO* bcsv, int entry){
    mName = SGenUtility::SjisToUtf8(bcsv->GetString(entry, "name"));
    mRenderable = nullptr;
    
    if(ModelCache.count(mName) == 0){
        GCResourceManager.CacheModel(mName);
    }

    glm::vec3 position = {bcsv->GetFloat(entry, "pos_x"), bcsv->GetFloat(entry, "pos_y"), bcsv->GetFloat(entry, "pos_z")};
    glm::vec3 rotation = {bcsv->GetFloat(entry, "dir_x"), bcsv->GetFloat(entry, "dir_y"), bcsv->GetFloat(entry, "dir_z")};
    glm::vec3 scale = {bcsv->GetFloat(entry, "scale_x"), bcsv->GetFloat(entry, "scale_y"), bcsv->GetFloat(entry, "scale_z")};

    mTransform = SGenUtility::CreateMTX(scale, rotation, position);

    for (size_t i = 0; i < 8; i++){
        mObjArgs[i] = bcsv->GetSignedInt(entry, fmt::format("Obj_arg{0}", i));
    }
    
    for (size_t i = 0; i < 8; i++){
        mPathArgs[i] = bcsv->GetSignedInt(entry, fmt::format("Path_arg{0}", i));
    }

    for(auto& obj : objectDB["Classes"]){
        if(obj["InternalName"] == mName){
            for (size_t i = 0; i < 8; i++){
                if(!obj["Parameters"].contains(fmt::format("Obj_arg{0}", i))) continue;
                mObjArgNames[i] = obj["Parameters"][fmt::format("Obj_arg{0}", i)]["Name"];
            }
        }
    }

    //Collect arg types from object DB and load data accordingly into array of obj/path args

    mLinkID = bcsv->GetSignedInt(entry, "l_id");
    mCameraSetID = bcsv->GetSignedInt(entry, "CameraSetId");
    mSW_Appear = bcsv->GetSignedInt(entry, "SW_APPEAR");
    mSW_Dead = bcsv->GetSignedInt(entry, "SW_DEAD");
    mSW_A = bcsv->GetSignedInt(entry, "SW_A");
    mSW_B = bcsv->GetSignedInt(entry, "SW_B");
    mSW_Sleep = bcsv->GetSignedInt(entry, "SW_SLEEP");
    mMessageID = bcsv->GetSignedInt(entry, "MessageId");
    mCastID = bcsv->GetSignedInt(entry, "CastId");
    mViewGroupID = bcsv->GetSignedInt(entry, "ViewGroupId");

    mShapeModelNo = bcsv->GetShort(entry, "ShapeModelNo");
    mCommonPathID = bcsv->GetShort(entry, "CommonPath_ID");
    mClippingGroupID = bcsv->GetShort(entry, "ClippingGroupId");
    mGroupID = bcsv->GetShort(entry, "GroupId");
    mDemoGroupID = bcsv->GetShort(entry, "DemoGroupId");
    mMapPartID = bcsv->GetShort(entry, "MapParts_ID");

    mSwitchAwake = bcsv->GetSignedInt(entry, "SW_AWAKE");
    mSwitchParam = bcsv->GetSignedInt(entry, "SW_PARAM");
    mParamScale = bcsv->GetFloat(entry, "ParamScale");
    mObjID = bcsv->GetShort(entry, "Obj_ID");
    mGeneratorID = bcsv->GetShort(entry, "GeneratorID");

    if(ModelCache.count(mName) != 0){
        mRenderable = ModelCache[mName]->CreateInstance();
        mRenderable->SetLight(LightingConfigs["Strong"].Light0, 0);
        mRenderable->SetLight(LightingConfigs["Strong"].Light1, 1);
        mRenderable->SetLight(LightingConfigs["Strong"].Light2, 2);
    }

}

void SObjectDOMNode::Serialize(SBcsvIO* bcsv, int entry){
    glm::vec3 pos, scale, skew;
    glm::quat dir;
    glm::vec4 persp;


    glm::decompose(mTransform, scale, dir, pos, skew, persp);

    glm::vec3 rotation = glm::eulerAngles(dir);

    bcsv->SetString(entry, "name", SGenUtility::Utf8ToSjis(mName));
    bcsv->SetFloat(entry, "pos_x", pos.x);
    bcsv->SetFloat(entry, "pos_y", pos.y);
    bcsv->SetFloat(entry, "pos_z", pos.z);

    bcsv->SetFloat(entry, "dir_x", glm::degrees(rotation.x));
    bcsv->SetFloat(entry, "dir_y", glm::degrees(rotation.y));
    bcsv->SetFloat(entry, "dir_z", glm::degrees(rotation.z));

    bcsv->SetFloat(entry, "scale_x", scale.x);
    bcsv->SetFloat(entry, "scale_y", scale.y);
    bcsv->SetFloat(entry, "scale_z", scale.z);

    for (size_t i = 0; i < 8; i++){
        bcsv->SetSignedInt(entry, fmt::format("Obj_arg{0}", i), mObjArgs[i]);
    }

    for (size_t i = 0; i < 8; i++){
        bcsv->SetSignedInt(entry, fmt::format("Path_arg{0}", i), mPathArgs[i]);
    }

    bcsv->SetSignedInt(entry, "l_id", mLinkID);
    bcsv->SetSignedInt(entry, "CameraSetId", mCameraSetID);
    bcsv->SetSignedInt(entry, "SW_APPEAR", mSW_Appear);
    bcsv->SetSignedInt(entry, "SW_DEAD", mSW_Dead);
    bcsv->SetSignedInt(entry, "SW_A", mSW_A);
    bcsv->SetSignedInt(entry, "SW_B", mSW_B);
    bcsv->SetSignedInt(entry, "SW_SLEEP", mSW_Sleep);
    bcsv->SetSignedInt(entry, "MessageId", mMessageID);
    bcsv->SetSignedInt(entry, "CastId", mCastID);
    bcsv->SetSignedInt(entry, "ViewGroupId", mViewGroupID);

    bcsv->SetShort(entry, "ShapeModelNo", mShapeModelNo);
    bcsv->SetShort(entry, "CommonPath_ID", mCommonPathID);
    bcsv->SetShort(entry, "ClippingGroupId", mClippingGroupID);
    bcsv->SetShort(entry, "GroupId", mGroupID);
    bcsv->SetShort(entry, "DemoGroupId", mDemoGroupID);
    bcsv->SetShort(entry, "MapParts_ID", mMapPartID);


    bcsv->SetSignedInt(entry, "SW_AWAKE", mSwitchAwake);
    bcsv->SetSignedInt(entry, "SW_PARAM", mSwitchParam);
    bcsv->SetFloat(entry, "ParamScale", mParamScale);
    bcsv->SetShort(entry, "Obj_ID", mObjID);
    bcsv->SetShort(entry, "GeneratorID", mGeneratorID);

}

void SObjectDOMNode::RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected){
    ImGui::Text((mVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH));
    if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
        mVisible = !mVisible;
    }
    ImGui::SameLine();
    if(selected == GetSharedPtr<SObjectDOMNode>(EDOMNodeType::Object)){
        ImGui::TextColored(ImColor(0,255,0), fmt::format("{0}", mName.data()).c_str());
        ImGui::SameLine();

        ImGui::Text(ICON_FK_MINUS_CIRCLE);
        if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
            //should check this lock but whatever
            GetParentOfType<SDOMNodeBase>(EDOMNodeType::ZoneLayer).lock()->RemoveChild(GetSharedPtr<SObjectDOMNode>(EDOMNodeType::Object));
        }
    } else if(selected == mLinkedObject.lock()) {
        ImGui::TextColored(ImColor(0,255,150), fmt::format("{0} [Linked]", mName.data()).c_str());
    } else {
        ImGui::Text(fmt::format("{0}", mName.data()).c_str());
    }
    if(ImGui::IsItemClicked(0)){
        selected = GetSharedPtr<SObjectDOMNode>(EDOMNodeType::Object);
    }

}

void SObjectDOMNode::RenderDetailsUI(){
    glm::vec3 pos(mTransform[3]);
    ImGui::Text(fmt::format("Position: {0},{1},{2}", pos.x,pos.y,pos.z).c_str());

    ImGui::InputText("Name", &mName);

    for (size_t i = 0; i < 8; i++){
        ImGui::InputInt(mObjArgNames[i].data(), &mObjArgs[i]);
    }
}

void SObjectDOMNode::Render(std::vector<std::shared_ptr<J3DModelInstance>>& renderables, glm::mat4 transform, float dt){
    if(mRenderable != nullptr && mVisible) {
        mRenderable->SetReferenceFrame(transform * mTransform);
        renderables.push_back(mRenderable);
    }
}