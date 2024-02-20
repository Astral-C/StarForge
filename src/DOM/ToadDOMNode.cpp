#include "DOM/ToadDOMNode.hpp"
#include "ModelCache.hpp"
#include "ResUtil.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <fmt/core.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <LightConfigs.hpp>
#include "IconsForkAwesome.h"

enum ToadColors {
	Blue,
	Green,
	Purple,
	Red,
	Yellow,
	TOAD_COLOR_MAX
};

const char* ToadColorNames[] = {
	"Blue",
	"Green",
	"Purple",
	"Red",
	"Yellow"
};

SToadDOMNode::SToadDOMNode() : Super() {
	mType = EDOMNodeType::Toad;
	mTransform = glm::mat4(1);
}

SToadDOMNode::~SToadDOMNode(){
	
}

void SToadDOMNode::Deserialize(SBcsvIO* bcsv, int entry){
	mName = SGenUtility::SjisToUtf8(bcsv->GetString(entry, "name"));
	
	mRenderable = nullptr;
	mGoodsModel = nullptr;
	
	if(ModelCache.count("Kinopio") == 0){
		GCResourceManager.CacheModel("Kinopio");
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

	if(ModelCache.count("Kinopio") != 0){
		mRenderable = ModelCache["Kinopio"]->CreateInstance();

		auto anim = GCResourceManager.LoadColorAnimation("Kinopio", "colorchange.brk");

        mRenderable->SetLight(LightingConfigs["Strong"].Light0, 0);
        mRenderable->SetLight(LightingConfigs["Strong"].Light1, 1);
        mRenderable->SetLight(LightingConfigs["Strong"].Light2, 2);

		mRenderable->SetRegisterColorAnimation(anim);
		mRenderable->GetRegisterColorAnimation()->SetFrame(glm::max(mObjArgs[1], 0), true);

        auto wait_anim = GCResourceManager.LoadJointAnimation("Kinopio", "wait.bck");
        mRenderable->SetJointAnimation(wait_anim);
        mRenderable->GetJointAnimation()->SetFrame(rand() % 20);
	}

}

void SToadDOMNode::RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected){
    ImGui::Text((mVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH));
    if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
        mVisible = !mVisible;
    }
    ImGui::SameLine();
	if(selected == GetSharedPtr<SToadDOMNode>(EDOMNodeType::Object)){
		ImGui::TextColored(ImColor(0,255,0), fmt::format("{0} Toad", ToadColorNames[glm::max(mObjArgs[0], 0)]).c_str());
	} else if(selected == mLinkedObject.lock()) {
		ImGui::TextColored(ImColor(0,255,150), fmt::format("{0} [Linked]", mName.data()).c_str());
	} else {
		ImGui::Text(fmt::format("{0} Toad", ToadColorNames[glm::max(mObjArgs[1], 0)]).c_str());
	}
	if(ImGui::IsItemClicked(0)){
		selected = GetSharedPtr<SToadDOMNode>(EDOMNodeType::Toad);
	}

}

void SToadDOMNode::RenderDetailsUI(){
	glm::vec3 pos(mTransform[3]);
	ImGui::Text(fmt::format("Position: {0},{1},{2}", pos.x,pos.y,pos.z).c_str());

	//ImGui::InputText("Name", &mName);

	if(ImGui::BeginCombo("Color", ToadColorNames[glm::max(mObjArgs[0], 0)], ImGuiComboFlags_None)){
		for(int i = 0; i < ToadColors::TOAD_COLOR_MAX; i++){
			ImGui::PushID(i);
				bool is_selected = (mObjArgs[1] == i);
				if (ImGui::Selectable(ToadColorNames[i], is_selected))
				{
					mObjArgs[1] = i;
					mRenderable->GetRegisterColorAnimation()->SetFrame(mObjArgs[1], true);
				}            
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	
	ImGui::InputInt(mObjArgNames[0].data(), &mObjArgs[0]);
	for (size_t i = 2; i < 8; i++){
		ImGui::InputInt(mObjArgNames[i].data(), &mObjArgs[i]);
	}

    ImGui::SameLine();

    ImGui::Text(ICON_FK_MINUS_CIRCLE);
    if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
        //should check this lock but whatever
        GetParentOfType<SDOMNodeBase>(EDOMNodeType::ZoneLayer).lock()->RemoveChild(GetSharedPtr<SToadDOMNode>(EDOMNodeType::Toad));
    }
}

void SToadDOMNode::Render(std::vector<std::shared_ptr<J3DModelInstance>>& renderables, glm::mat4 transform, float dt){
	if(mRenderable != nullptr && mVisible) {
		mRenderable->SetReferenceFrame(transform * mTransform);
		
        mRenderable->GetJointAnimation()->Tick(dt);
		mRenderable->UpdateAnimations(dt);
		renderables.push_back(mRenderable);
	}
}