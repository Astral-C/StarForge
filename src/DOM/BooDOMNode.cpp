#include "DOM/BooDOMNode.hpp"
#include "ModelCache.hpp"
#include "ResUtil.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <fmt/core.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <LightConfigs.hpp>

SBooDOMNode::SBooDOMNode() : Super() {
    mType = EDOMNodeType::Boo;
    mTransform = glm::mat4(1);
}

SBooDOMNode::~SBooDOMNode(){
    
}

void SBooDOMNode::Deserialize(SBcsvIO* bcsv, int entry){
    mName = SGenUtility::SjisToUtf8(bcsv->GetString(entry, "name"));
    
    mRenderable = nullptr;
    
    if(ModelCache.count("Teresa") == 0){
        GCResourceManager.CacheModel("Teresa");
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

    if(ModelCache.count("Teresa") != 0){
        mRenderable = ModelCache["Teresa"]->CreateInstance();

        mRenderable->SetLight(LightingConfigs["Strong"].Light0, 0);
        mRenderable->SetLight(LightingConfigs["Strong"].Light1, 1);
        mRenderable->SetLight(LightingConfigs["Strong"].Light2, 2);

        auto anim = GCResourceManager.LoadColorAnimation("Teresa", "teresa.brk");
        if(anim == nullptr){
            anim = GCResourceManager.LoadColorAnimation("Teresa", "Teresa.brk");
        }

        mRenderable->SetRegisterColorAnimation(anim);
        mRenderable->GetRegisterColorAnimation()->SetFrame(0, true);

        auto wait_anim = GCResourceManager.LoadJointAnimation("Teresa", "wait.bck");

        if(wait_anim == nullptr){
            wait_anim = GCResourceManager.LoadJointAnimation("Teresa", "Wait.bck");
        }

        mRenderable->SetJointAnimation(wait_anim);
        mRenderable->GetJointAnimation()->SetFrame(rand() % 20);
    }

}

void SBooDOMNode::Render(std::vector<std::shared_ptr<J3DModelInstance>>& renderables, glm::mat4 transform, float dt){
    if(mRenderable != nullptr) {
        mRenderable->SetReferenceFrame(transform * mTransform);
     
        if(mRenderable->GetJointAnimation()) mRenderable->GetJointAnimation()->Tick(dt);

        mRenderable->UpdateAnimations(dt);
        renderables.push_back(mRenderable);
    }
}