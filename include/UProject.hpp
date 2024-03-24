#include "json.hpp"
#include "DOM/DOMNodeBase.hpp"
#include "DOM/GalaxyDOMNode.hpp"
#include <filesystem>
#include "ResUtil.hpp"


class UStarForgeProject
{
private:
    std::string mName;
    std::string mDescription;
    std::filesystem::path mProjectRoot;
    bool mIsDolphinRoot;
    EGameType mGame;

    nlohmann::json mGalaxies;

    uint32_t mProjImageID { 0xFFFFFFFF };

public:

    std::string GetName() { return mName; }
    std::string GetDescription() { return mDescription; }
    std::filesystem::path GetRootPath() { return mProjectRoot; }
    nlohmann::json GetGalaxies() { return mGalaxies; }
    uint32_t GetImage() { return mProjImageID; }
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
    uint32_t mNewProjectGame { 1 };
    bool mNewProjectDolphinRoot { false };


public:
    void Init();

    std::string RenderGalaxySelectUi(bool& galaxySelectOpen);
    void RenderUi(bool& projectManagerOpen);

    UStarForgeProjectManager();
    ~UStarForgeProjectManager();
};