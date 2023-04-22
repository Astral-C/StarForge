#include "DOM/ObjectDOMNode.hpp"
#include "ModelCache.hpp"
#include "ResUtil.hpp"
#include "imgui.h"
#include <fmt/core.h>
#include <glm/gtx/matrix_decompose.hpp>

static nlohmann::json objectDB;

void SObjectDOMNode::LoadObjectDB(nlohmann::json db){
    objectDB = db;
}

SObjectDOMNode::SObjectDOMNode() : Super("Object") {
    mType = EDOMNodeType::Object;
    mTransform = glm::mat4(1);
    mObjArgNames = {"Obj_arg0","Obj_arg1","Obj_arg2","Obj_arg3","Obj_arg4","Obj_arg5","Obj_arg6","Obj_arg7"};
}

SObjectDOMNode::~SObjectDOMNode(){
    
}

void SObjectDOMNode::Deserialize(SBcsvIO* bcsv, int entry){
    mName = bcsv->GetString(entry, "name");
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
}

void SObjectDOMNode::Serialize(SBcsvIO* bcsv, int entry){
    glm::vec3 pos, scale, skew;
    glm::quat dir;
    glm::vec4 persp;

    /*

    glm::decompose(mTransform, scale, dir, pos, skew, persp);

    glm::vec3 rotation = glm::eulerAngles(dir);

    bcsv->SetString(entry, "name", mName, stringTable);
    bcsv->SetFloat(entry, "pos_x", pos.x);
    bcsv->SetFloat(entry, "pos_y", pos.y);
    bcsv->SetFloat(entry, "pos_z", pos.z);

    bcsv->SetFloat(entry, "dir_x", dir.x);
    bcsv->SetFloat(entry, "dir_y", dir.y);
    bcsv->SetFloat(entry, "dir_z", dir.z);

    bcsv->SetFloat(entry, "scale_x", scale.x);
    bcsv->SetFloat(entry, "scale_y", scale.y);
    bcsv->SetFloat(entry, "scale_z", scale.z);

    for (size_t i = 0; i < 8; i++){
        bcsv->SetSignedInt(entry, fmt::format("Obj_arg{0}", i), mObjArgs[i]);
    }

    for (size_t i = 0; i < 8; i++){
        bcsv->SetSignedInt(entry, fmt::format("Path_arg{0}", i), mPathArgs[i]);
    }
    */
}

void SObjectDOMNode::RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected){
    ImGui::Text(fmt::format("{0}", mName.data()).c_str());
    if(ImGui::IsItemClicked(0)){
        selected = GetSharedPtr<SObjectDOMNode>(EDOMNodeType::Object);
    }
}

void SObjectDOMNode::RenderDetailsUI(){
    for (size_t i = 0; i < 8; i++){
        ImGui::InputInt(mObjArgNames[i].data(), &mObjArgs[i]);
    }
}

void SObjectDOMNode::Render(glm::mat4 transform, float dt){
    if(ModelCache.count(mName) != 0) {
		J3DUniformBufferObject::SetEnvelopeMatrices(ModelCache.at(mName)->GetRestPose().data(), ModelCache.at(mName)->GetRestPose().size());

        glm::mat4 drawTransform = transform * mTransform;

		J3DUniformBufferObject::SetModelMatrix(&drawTransform);

		ModelCache.at(mName)->Render(dt);
    }
}