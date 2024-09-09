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
    mVisible = true;
}

SPathDOMNode::~SPathDOMNode(){
    
}

void SPathDOMNode::Render(USceneCamera* cam, glm::mat4 referenceFrame){
    if(mVisible) mRenderer.Draw(cam, mPickId, referenceFrame);
}

void SPathDOMNode::Update(){
	mRenderer.mPath.clear();
    mRenderer.isClosed = mIsClosed;
    auto points = GetChildrenOfType<SPathPointDOMNode>(EDOMNodeType::PathPoint);
    mRenderer.mPath.reserve(points.size());
    for(auto point : points){
		CPathPoint pnt = {
			.Position = point->GetPosition(),
			.Color = glm::vec4((float)(mColor >> 16 & 0xFF) / 255.0f, (float)(mColor >> 8 & 0xFF) / 255.0f, (float)(mColor & 0xFF) / 255.0f, 1.0f),
			.LeftHandle = point->GetLeftHandle(),
			.RightHandle = point->GetRightHandle(),
            .PickID = point->GetPickID()
		};
        mRenderer.mPath.push_back(pnt);
    }
	mRenderer.UpdateData();
}

void SPathDOMNode::Deserialize(SBcsvIO* bcsv, int entry){
    mName = bcsv->GetString(entry, "name");
    mPathType = bcsv->GetString(entry, "type");

    mIsClosed = bcsv->GetString(entry, "closed") != "OPEN";
    mLinkID = bcsv->GetUnsignedInt(entry, "l_id");
    
    for(int i = 0; i < 8; i++){
        mPathArgs[i] = bcsv->GetUnsignedInt(entry, fmt::format("path_arg{}", i));
    }

    mUsage = bcsv->GetString(entry, "usage");
    mNo = bcsv->GetUnsignedInt(entry, "no");
    mPathID = bcsv->GetUnsignedInt(entry, "Path_ID");
}

void SPathDOMNode::Serialize(SBcsvIO* bcsv, int entry){
    // TODO
}

void SPathDOMNode::RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected){
    ImGui::Text((mVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH));
    if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
        mVisible = !mVisible;
    }
    ImGui::SameLine();
    bool treeOpen = ImGui::TreeNode(mName.data());

    if(treeOpen){
        for (auto node : GetChildrenOfType<SPathPointDOMNode>(EDOMNodeType::PathPoint)){
            node->RenderHeirarchyUI(selected);
        }
        
        ImGui::Text(ICON_FK_PLUS_CIRCLE);
        if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
            auto object = std::make_shared<SPathPointDOMNode>();
            AddChild(object);
            selected = object;
            Update();
        }
        ImGui::TreePop();
    }
    
    
    if(ImGui::IsItemClicked(0)){
        selected = GetSharedPtr<SPathDOMNode>(EDOMNodeType::Path);
    }
}

void SPathDOMNode::RenderDetailsUI(){
    ImGui::Text(fmt::format("Name: {}", mName).c_str());
    ImGui::Text("Path Type: %s", mPathType.c_str());
    ImGui::Text("Usage: %s", mUsage.c_str());
    ImGui::Checkbox("Is Closed", &mIsClosed);

    ImVec4 color = ImColor(mColor);
    ImGui::ColorPicker4("Path Color Pick", (float*)&color);
    

    uint32_t newColor = ImGui::GetColorU32(color);
    if(newColor != mColor){
        mColor = newColor;
        //Update();
    }
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
    ImGui::Text("Path Point");
    if(ImGui::IsItemClicked(0)){
        selected = GetSharedPtr<SPathPointDOMNode>(EDOMNodeType::PathPoint);
    }
}

void SPathPointDOMNode::RenderDetailsUI(){

}