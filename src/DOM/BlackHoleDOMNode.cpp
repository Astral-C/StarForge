#include "DOM/BlackHoleDOMNode.hpp"
#include "ModelCache.hpp"
#include "ResUtil.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <format>
#include <glm/gtx/matrix_decompose.hpp>
#include <LightConfigs.hpp>

SBlackHoleDOMNode::SBlackHoleDOMNode() : Super() {
    mType = EDOMNodeType::BlackHole;
    mTransform = glm::mat4(1);
}

SBlackHoleDOMNode::~SBlackHoleDOMNode(){
    
}

void SBlackHoleDOMNode::Deserialize(SBcsvIO* bcsv, int entry){
    mName = SGenUtility::SjisToUtf8(bcsv->GetString(entry, "name"));
    
    mRenderable = nullptr;
    
    if(ModelCache.count("BlackHole") == 0){
        GCResourceManager.CacheModel("BlackHole");
    }

    glm::vec3 position = {bcsv->GetFloat(entry, "pos_x"), bcsv->GetFloat(entry, "pos_y"), bcsv->GetFloat(entry, "pos_z")};
    glm::vec3 rotation = {bcsv->GetFloat(entry, "dir_x"), bcsv->GetFloat(entry, "dir_y"), bcsv->GetFloat(entry, "dir_z")};
    glm::vec3 scale = {bcsv->GetFloat(entry, "scale_x"), bcsv->GetFloat(entry, "scale_y"), bcsv->GetFloat(entry, "scale_z")};

    mTransform = SGenUtility::CreateMTX(scale, rotation, position);

    for (size_t i = 0; i < 8; i++){
        mObjArgs[i] = bcsv->GetSignedInt(entry, std::format("Obj_arg{0}", i));
    }
    
    for (size_t i = 0; i < 8; i++){
        mPathArgs[i] = bcsv->GetSignedInt(entry, std::format("Path_arg{0}", i));
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

    if(ModelCache.count("BlackHole") != 0){
        mRenderable = ModelCache["BlackHole"]->CreateInstance();

        mRenderable->SetLight(LightingConfigs["Strong"].Light0, 0);
        mRenderable->SetLight(LightingConfigs["Strong"].Light1, 1);
        mRenderable->SetLight(LightingConfigs["Strong"].Light2, 2);

        auto anim = GCResourceManager.LoadTextureAnimation("BlackHole", "blackhole.btk");

        if(anim == nullptr){
            anim = GCResourceManager.LoadTextureAnimation("BlackHole", "BlackHole.btk");
        }

        mRenderable->SetTexMatrixAnimation(anim);
    }

}

void SBlackHoleDOMNode::Render(std::vector<std::shared_ptr<J3DModelInstance>>& renderables, glm::mat4 transform, float dt){
    if(mRenderable != nullptr) {
        mRenderable->SetReferenceFrame(transform * mTransform);
     
        if(mRenderable->GetTexMatrixAnimation()) mRenderable->GetTexMatrixAnimation()->Tick(dt);

        mRenderable->UpdateAnimations(dt);
        renderables.push_back(mRenderable);
    }
}