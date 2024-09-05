#include "UProject.hpp"
#include <fmt/format.h>
#include <imgui.h>
#include "stb_image.h"
#include "glad/glad.h"
#include "IconsForkAwesome.h"
#include "util/UUIUtil.hpp"
#include "../lib/imgui/misc/cpp/imgui_stdlib.h"
#include "ImGuiFileDialog.h"

UStarForgeProject::UStarForgeProject(){}

UStarForgeProject::UStarForgeProject(nlohmann::json projectJson){

    mName = projectJson["name"];
    mDescription = projectJson["description"];
    mProjectRoot = std::filesystem::path(projectJson["root"].get<std::string>());
    mGame = (EGameType)projectJson["game"];
    mIsDolphinRoot = projectJson["isDolphinRoot"];

    if(mIsDolphinRoot && mProjectRoot.parent_path() != "DATA") {
        mProjectRoot = mProjectRoot / "DATA" / "files"; // this is a hack  - just to support most non-dolphin roots not having a files dir
    }

    if(projectJson.contains("icon") && std::filesystem::exists(projectJson["icon"])){
        int w, h, c;
        unsigned char* defaultProjImg = stbi_load(projectJson["icon"].get<std::string>().c_str(), &w, &h, &c, 4);

        glGenTextures(1, &mProjImageID);
        glBindTexture(GL_TEXTURE_2D, mProjImageID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultProjImg);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(defaultProjImg);
    }

    mGalaxies = projectJson["galaxies"];
}

void UStarForgeProject::LoadThumbs(){
    stbi_set_flip_vertically_on_load(1);
    for(auto galaxy : mGalaxies){
        if(std::filesystem::exists("./res/thumb/" + galaxy["internalName"].get<std::string>() + ".png")){
            std::string path = "./res/thumb/" + galaxy["internalName"].get<std::string>() + ".png";

            int w, h, c;
            unsigned char* defaultProjImg = stbi_load(path.c_str(), &w, &h, &c, 4);

            uint32_t thumbId;
            glGenTextures(1, &thumbId);
            glBindTexture(GL_TEXTURE_2D, thumbId);
            
            mGalaxyThumbnails.push_back(thumbId);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultProjImg);
            glBindTexture(GL_TEXTURE_2D, 0);

            stbi_image_free(defaultProjImg);

        } else {
            mGalaxyThumbnails.push_back(0xFFFFFFFF);
        }
    }
    stbi_set_flip_vertically_on_load(0);
}

uint32_t UStarForgeProject::GetThumbnail(std::string name){
    for(int i = 0; i < mGalaxies.size(); i++){
        if(mGalaxies.at(i)["internalName"] == name && i < mGalaxyThumbnails.size()) return mGalaxyThumbnails.at(i);
    }
    return 0xFFFFFFFF;
}

UStarForgeProject::~UStarForgeProject(){}

UStarForgeProjectManager::UStarForgeProjectManager(){
    if(!std::filesystem::exists(Options.mProjectsPath)){
        std::filesystem::create_directory(std::filesystem::current_path() / "projects");
        Options.mProjectsPath = std::filesystem::current_path() / "projects";
    }

    if(!std::filesystem::exists(Options.mProjectsPath / "projects.json")){
        std::fstream projectsTemplateFile(Options.mProjectsPath / "projects.json", std::ios::out);
        nlohmann::json projectsTemplate;
        projectsTemplate["projects"] = nlohmann::json::array();
        projectsTemplate >> projectsTemplateFile;
    }

    nlohmann::json projectsJson = nlohmann::json::parse(std::fstream(Options.mProjectsPath / "projects.json"));

    for(auto project : projectsJson["projects"]){
        mProjects.push_back(std::make_unique<UStarForgeProject>(project));
    }
}

void UStarForgeProjectManager::Init(){
    int w, h, c;
    unsigned char* defaultProjImg = stbi_load("./res/default_project.png", &w, &h, &c, 4);

    glGenTextures(1, &mProjImageID);
    glBindTexture(GL_TEXTURE_2D, mProjImageID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultProjImg);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(defaultProjImg);
}

UStarForgeProjectManager::~UStarForgeProjectManager(){}

std::string UStarForgeProjectManager::RenderGalaxySelectUi(bool& galaxySelectOpen){
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImVec2 size = ImGui::GetMainViewport()->Size;
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(size.x * 0.35f, size.y * 0.8f));
    bool shouldClosePopup = false;
    std::string selectedGalaxy = "";

    if (ImGui::BeginPopupModal("Galaxy Select", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)){
        if(mNewGalaxyDialogOpen){
            ImGui::BeginChild("Add Galaxy", ImVec2(0.0f, 164.0f), ImGuiChildFlags_Border);
            ImGui::InputText("Name", &mNewGalaxyName);
            ImGui::InputText("Internal Name", &mNewGalaxyInternalName);
            ImGui::EndChild();
            if(UIUtil::CenteredButton(ICON_FK_PLUS " Add")){
                nlohmann::json newGalaxy;

                newGalaxy["name"] = mNewGalaxyName;
                newGalaxy["internalName"] = mNewGalaxyInternalName;

                mCurrentProject->AddGalaxy(newGalaxy);
                std::cout << mCurrentProject->GetGalaxies() << std::endl;

                std::fstream projectsFileIn(Options.mProjectsPath / "projects.json", std::ios::in);
                nlohmann::json projectsJson = nlohmann::json::parse(projectsFileIn);

                for(int i = 0; i < mProjects.size(); i++){
                    if(mProjects.at(i) == mCurrentProject){
                        projectsJson["projects"].at(i)["galaxies"] = mCurrentProject->GetGalaxies();
                    }
                }

                std::fstream projectsFileOut(Options.mProjectsPath / "projects.json", std::ios::out);
                projectsJson >> projectsFileOut;

                mNewGalaxyDialogOpen = false;
            }
        } else {
            for(auto galaxy : mCurrentProject->GetGalaxies()){
                ImGui::BeginChild(fmt::format("##{}", galaxy["name"].get<std::string>()).data(), ImVec2(ImGui::GetContentRegionAvail().x, 80.0f), ImGuiChildFlags_Border);
                    if(ImGui::IsWindowHovered()){
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);

                        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
                            selectedGalaxy = galaxy["internalName"];
                            shouldClosePopup = true;
                        }
                    }

                    uint32_t imgId = mCurrentProject->GetThumbnail(galaxy["internalName"].get<std::string>());
                    if(imgId != 0xFFFFFFFF){
                        ImGui::Image((ImTextureID)imgId, ImVec2(84, 64));
                    } else {
                        ImGui::Image((ImTextureID)mProjImageID, ImVec2(64, 64));
                    }
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImGui::Text(galaxy["name"].get<std::string>().data());
                    ImGui::EndGroup();
                ImGui::EndChild();
            }
        
                ImGui::BeginChild("##newGalaxyBtn", ImVec2(ImGui::GetContentRegionAvail().x, 32.0f), ImGuiChildFlags_Border);
                    if(ImGui::IsWindowHovered()){
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);

                        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
                            // set the projectmanager mode to create project
                            mNewGalaxyDialogOpen = true;
                        }
                    }


                    ImGui::BeginGroup();
                    ImGui::Text(ICON_FK_PLUS);
                    ImGui::SameLine();
                    ImGui::Text("New");
                    ImGui::EndGroup();
                ImGui::EndChild();

            if(shouldClosePopup){
                ImGui::CloseCurrentPopup();
                galaxySelectOpen = false;
            }
        }

		ImGui::EndPopup();

    }

    return selectedGalaxy;
}

void UStarForgeProjectManager::RenderUi(bool& projectManagerOpen){
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImVec2 size = ImGui::GetMainViewport()->Size;
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(size.x * 0.35f, size.y * 0.8f), ImGuiCond_Always);
    bool shouldClosePopup = false;

	if (ImGui::BeginPopupModal("Projects", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)){
		// ImGui::CloseCurrentPopup();
        if(mNewProjectDialogOpen) {
            ImGui::BeginChild("New Project", ImVec2(0.0f, 164.0f), ImGuiChildFlags_Border);
            ImGui::InputText("Name", &mNewProjectName);
            ImGui::InputText("Description", &mNewProjectDescription);
            ImGui::Text("Root: %s", mNewProjectRoot.data());
            ImGui::SameLine();
            if(ImGui::Button("Open Root")){
                mSelectRootDialogOpen = true;
            }
            ImGui::Text("Icon Path: %s", mNewProjectIconPath.data());
            ImGui::SameLine();
            if(ImGui::Button("Select")){
                mSelectIconDialogOpen = true;
            }
            ImGui::Checkbox("Dolphin Root", &mNewProjectDolphinRoot);
            ImGui::EndChild();
            if(UIUtil::CenteredButton(ICON_FK_PLUS " Create")){
                nlohmann::json newProject;

                newProject["name"] = mNewProjectName;
                newProject["description"] = mNewProjectDescription;
                newProject["root"] = mNewProjectRoot;
                newProject["icon"] = mNewProjectIconPath;
                newProject["game"] = mNewProjectGame;
                newProject["isDolphinRoot"] = mNewProjectDolphinRoot;

                mProjects.push_back(std::make_shared<UStarForgeProject>(newProject));

                std::fstream projectsFileIn(Options.mProjectsPath / "projects.json", std::ios::in);
                nlohmann::json projectsJson = nlohmann::json::parse(projectsFileIn);
                projectsJson["projects"].push_back(newProject);

                std::fstream projectsFileOut(Options.mProjectsPath / "projects.json", std::ios::out);
                projectsJson >> projectsFileOut;


                mNewProjectDialogOpen = false;
            }

        } else {
            std::shared_ptr<UStarForgeProject> markDelete = nullptr;
            for(auto project : mProjects){
                ImGui::BeginChild(fmt::format("##{}", project->GetName()).data(), ImVec2(0.0f, 64.0f), ImGuiChildFlags_Border);
                    if(ImGui::IsWindowHovered()){

                        float ypos = ImGui::GetCursorPosY();
                        ImGui::SetCursorPosY(ypos + 16);
                        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 10);
                        ImGui::Text(ICON_FK_TRASH);
                        if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
                            markDelete = project;
                        } else if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
                            Options.mRootPath = project->GetRootPath();
                            mCurrentProject = project;
                            shouldClosePopup = true;
                        }
                        ImGui::SetCursorPosY(ypos);
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);


                    }

                    uint32_t imgId = project->GetImage();
                    if(imgId != 0xFFFFFFFF){
                        ImGui::Image((ImTextureID)imgId, ImVec2(48, 48));
                    } else {
                        ImGui::Image((ImTextureID)mProjImageID, ImVec2(48, 48));
                    }
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImGui::Text(project->GetName().data());
                    ImGui::Text(project->GetDescription().data());
                    ImGui::EndGroup();
                ImGui::EndChild();
            }

            ImGui::BeginChild("##newProjectBtn", ImVec2(ImGui::GetContentRegionAvail().x, 32.0f), ImGuiChildFlags_Border);
                if(ImGui::IsWindowHovered()){
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);

                    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
                        // set the projectmanager mode to create project
                        mNewProjectDialogOpen = true;
                    }
                }


                ImGui::BeginGroup();
                ImGui::Text(ICON_FK_PLUS);
                ImGui::SameLine();
                ImGui::Text("New");
                ImGui::EndGroup();
            ImGui::EndChild();

            if(markDelete != nullptr){
                std::fstream projectsFileIn(Options.mProjectsPath / "projects.json", std::ios::in);
                nlohmann::json projectsJson = nlohmann::json::parse(projectsFileIn);
                for(int i = 0; i < mProjects.size(); i++){
                    if(mProjects.at(i) == markDelete){
                        projectsJson["projects"].erase(i);
                        mProjects.erase(mProjects.begin() + i);
                    }
                }

                std::fstream projectsFileOut(Options.mProjectsPath / "projects.json", std::ios::out);
                projectsJson >> projectsFileOut;
            }
        }

        if(shouldClosePopup){
            mCurrentProject->LoadThumbs();
            ImGui::CloseCurrentPopup();
            projectManagerOpen = false;
        }

		if(mSelectRootDialogOpen){
			IGFD::FileDialogConfig config;
			config.path = ".";
			ImGuiFileDialog::Instance()->OpenDialog("OpenRootDialog", "Choose Root Path", nullptr, config);
		}


		if (ImGuiFileDialog::Instance()->Display("OpenRootDialog")) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				mNewProjectRoot = ImGuiFileDialog::Instance()->GetCurrentPath();

				mSelectRootDialogOpen = false;
			} else {
				mSelectRootDialogOpen = false;
			}

			ImGuiFileDialog::Instance()->Close();
		}

		if(mSelectIconDialogOpen){
			IGFD::FileDialogConfig config;
			config.path = ".";
			ImGuiFileDialog::Instance()->OpenDialog("OpenIconDialog", "Choose 48x48 Project Icon Path", ".png", config);
		}
        
		if (ImGuiFileDialog::Instance()->Display("OpenIconDialog")) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				mNewProjectIconPath = (std::filesystem::path(ImGuiFileDialog::Instance()->GetCurrentPath()) / ImGuiFileDialog::Instance()->GetCurrentFileName()).string();

				mSelectIconDialogOpen = false;
			} else {
				mSelectIconDialogOpen = false;
			}

			ImGuiFileDialog::Instance()->Close();
		}

		ImGui::EndPopup();
	}

}