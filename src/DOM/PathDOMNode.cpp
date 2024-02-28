#include "DOM/PathDOMNode.hpp"
#include "ModelCache.hpp"
#include "ResUtil.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <fmt/core.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <LightConfigs.hpp>
#include "IconsForkAwesome.h"
#include <J3D/Picking/J3DPicking.hpp>

SPathDOMNode::SPathDOMNode() : Super("Path") {
    mType = EDOMNodeType::Path;
    mTransform = glm::mat4(1);
    mColor = ~rand() & 0xFFFFFF;
    mRenderer.Init();
}

SPathDOMNode::~SPathDOMNode(){
    
}

void SPathDOMNode::Render(USceneCamera* cam, glm::mat4 referenceFrame){
    mRenderer.Draw(cam, referenceFrame);
}

void SPathDOMNode::Update(){
	mRenderer.mPath.clear();
    mRenderer.isClosed = mIsClosed;
    for(auto point : GetChildrenOfType<SPathPointDOMNode>(EDOMNodeType::PathPoint)){
		mRenderer.mPath.push_back(
			(CPathPoint){
				point->GetPosition(),
				glm::vec4((float)(mColor >> 16 & 0xFF) / 255.0f, (float)(mColor >> 8 & 0xFF) / 255.0f, (float)(mColor & 0xFF) / 255.0f, 1.0f),
				point->GetLeftHandle(),
				point->GetRightHandle()
			}
		);
    }
	mRenderer.UpdateData();
}

void SPathDOMNode::Deserialize(SBcsvIO* bcsv, int entry){
    mName = SGenUtility::SjisToUtf8(bcsv->GetString(entry, "name"));
    std::cout << mName << std::endl;
    mPathType = SGenUtility::SjisToUtf8(bcsv->GetString(entry, "type"));

    mIsClosed = SGenUtility::SjisToUtf8(bcsv->GetString(entry, "closed")) != "OPEN";
    mLinkID = bcsv->GetUnsignedInt(entry, "l_id");
    
    for(int i = 0; i < 8; i++){
        mPathArgs[i] = bcsv->GetUnsignedInt(entry, fmt::format("path_arg{}", i));
    }

    mUsage = SGenUtility::SjisToUtf8(bcsv->GetString(entry, "usage"));
    mNo = bcsv->GetUnsignedInt(entry, "no");
    mPathID = bcsv->GetUnsignedInt(entry, "Path_ID");
}

void SPathDOMNode::Serialize(SBcsvIO* bcsv, int entry){
    // TODO
}

void SPathDOMNode::RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected){
    /*
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
    */
}

void SPathDOMNode::RenderDetailsUI(){
    /*
    glm::vec3 pos(mTransform[3]);
    ImGui::Text(fmt::format("Position: {0},{1},{2}", pos.x,pos.y,pos.z).c_str());

    ImGui::InputText("Name", &mName);

    for (size_t i = 0; i < 8; i++){
        ImGui::InputInt(mObjArgNames[i].data(), &mObjArgs[i]);
    }
    */
}

SPathPointDOMNode::SPathPointDOMNode() : Super("PathPoint") {
    mType = EDOMNodeType::PathPoint;
    mTransform = glm::mat4(1);
}

SPathPointDOMNode::~SPathPointDOMNode(){}

void SPathPointDOMNode::Deserialize(SBcsvIO* bcsv, int entry){
    for(int i = 0; i < 8; i++){
        mPointArgs[i] = bcsv->GetUnsignedInt(entry, fmt::format("point_arg{}", i));
    }
    
    glm::vec3 pos;

    pos.x = bcsv->GetFloat(entry, "pnt0_x");
    pos.y = bcsv->GetFloat(entry, "pnt0_y");
    pos.z = bcsv->GetFloat(entry, "pnt0_z");

    mLeftHandle.x = bcsv->GetFloat(entry, "pnt1_x");
    mLeftHandle.y = bcsv->GetFloat(entry, "pnt1_y");
    mLeftHandle.z = bcsv->GetFloat(entry, "pnt1_z");

    mRightHandle.x = bcsv->GetFloat(entry, "pnt2_x");
    mRightHandle.y = bcsv->GetFloat(entry, "pnt2_y");
    mRightHandle.z = bcsv->GetFloat(entry, "pnt2_z");

    mTransform = glm::translate(glm::identity<glm::mat4>(), pos);
}

void SPathPointDOMNode::Serialize(SBcsvIO* bcsv, int entry){
    //TODO
}

void SPathPointDOMNode::RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected){
}

void SPathPointDOMNode::RenderDetailsUI(){
}