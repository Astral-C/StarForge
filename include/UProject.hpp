#include "json.hpp"
#include "DOM/DOMNodeBase.hpp"
#include "DOM/GalaxyDOMNode.hpp"
#include <filesystem>
#include "ResUtil.hpp"


class UStarForgeProject
{
private:
    std::string mName { "" };
    std::string mDescription { "" };
    std::filesystem::path mProjectRoot { "." };
    bool mIsDolphinRoot { false };
    EGameType mGame { EGameType::SMG1 };
    EGameSystem mSystem { EGameSystem::Wii };

    nlohmann::json mGalaxies;
    std::vector<uint32_t> mGalaxyThumbnails;

    uint32_t mProjImageID { 0xFFFFFFFF };

public:

    std::string GetName() { return mName; }
    std::string GetDescription() { return mDescription; }
    std::filesystem::path GetRootPath() { return mProjectRoot; }
    nlohmann::json GetGalaxies() { return mGalaxies; }
    uint32_t GetImage() { return mProjImageID; }
    EGameType GetGame() { return mGame; }
    EGameSystem GetSystem() { return mSystem; }

    uint32_t GetThumbnail(std::string name);

    void LoadThumbs();
    void AddGalaxy(nlohmann::json newGalaxy) { mGalaxies.push_back(newGalaxy); }

    UStarForgeProject(nlohmann::json projectJson);
    UStarForgeProject();
    ~UStarForgeProject();
};

class UStarForgeProjectManager {
private:
    uint32_t mProjImageID = -1;

    std::shared_ptr<UStarForgeProject> mCurrentProject { nullptr };
    std::vector<std::shared_ptr<UStarForgeProject>> mProjects;
    bool mNewProjectDialogOpen { false };
    bool mSelectRootDialogOpen { false };
    bool mSelectIconDialogOpen { false };
    bool mNewGalaxyDialogOpen { false } ;

    std::string mNewGalaxyName { "Readable Galaxy Name" };
    std::string mNewGalaxyInternalName { "Main Zone Archive Name" };

    std::string mNewProjectName { "StarForge Project" };
    std::string mNewProjectDescription { "A Project for StarForge!" };
    std::string mNewProjectRoot { "." };
    std::string mNewProjectIconPath { "" };
    uint32_t mNewProjectGame { 0 };
    uint32_t mNewProjectSystem { 0 };
    bool mNewProjectDolphinRoot { false };


public:
    void Init();

    std::shared_ptr<UStarForgeProject> CurrentProject() { return mCurrentProject; }

    std::string RenderGalaxySelectUi(bool& galaxySelectOpen);
    void RenderUi(bool& projectManagerOpen);

    UStarForgeProjectManager();
    ~UStarForgeProjectManager();
};
