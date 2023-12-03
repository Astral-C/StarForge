#include "DOM/PlanetDOMNode.hpp"
#include "ModelCache.hpp"
#include "ResUtil.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <fmt/core.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <LightConfigs.hpp>

SPlanetDOMNode::SPlanetDOMNode() : Super() {
    mType = EDOMNodeType::Planet;
    mTransform = glm::mat4(1);
}

SPlanetDOMNode::~SPlanetDOMNode(){
    
}

void SPlanetDOMNode::Deserialize(SBcsvIO* bcsv, int entry){
    mName = SGenUtility::SjisToUtf8(bcsv->GetString(entry, "name"));
    
    mRenderable = nullptr;
    
    if(ModelCache.count(mName) == 0){
        GCResourceManager.CacheModel(mName);
        GCResourceManager.CacheModel(mName+"Water");
        GCResourceManager.CacheModel(mName+"Bloom");
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


    if(ModelCache.count(mName) != 0){
        mRenderable = ModelCache[mName]->CreateInstance();

        mRenderable->SetLight(LightingConfigs["Planet"].Light0, 0);
        mRenderable->SetLight(LightingConfigs["Planet"].Light1, 1);
        mRenderable->SetLight(LightingConfigs["Planet"].Light2, 2);

    }
    
    if(ModelCache.count(mName+"Water")){
        mWaterRenderable = ModelCache[mName+"Water"]->CreateInstance();
        
        mRenderable->SetLight(LightingConfigs["Planet"].Light0, 0);
        mRenderable->SetLight(LightingConfigs["Planet"].Light1, 1);
        mRenderable->SetLight(LightingConfigs["Planet"].Light2, 2);
    }

    if(ModelCache.count(mName+"Bloom")){
        mWaterRenderable = ModelCache[mName+"Bloom"]->CreateInstance();
        
        mRenderable->SetLight(LightingConfigs["Planet"].Light0, 0);
        mRenderable->SetLight(LightingConfigs["Planet"].Light1, 1);
        mRenderable->SetLight(LightingConfigs["Planet"].Light2, 2);
    }
}

void SPlanetDOMNode::Render(std::vector<std::shared_ptr<J3DModelInstance>>& renderables, glm::mat4 transform, float dt){
    if(mRenderable != nullptr) {
        glm::mat4 drawPos = transform * mTransform;
        
        mRenderable->SetReferenceFrame(drawPos);
        
        if(mWaterRenderable != nullptr){
            mWaterRenderable->SetReferenceFrame(drawPos);
            renderables.push_back(mWaterRenderable);
        }

         if(mBloomRenderable != nullptr){
            mBloomRenderable->SetReferenceFrame(drawPos);
            renderables.push_back(mBloomRenderable);
        }

        renderables.push_back(mRenderable);
    }
}