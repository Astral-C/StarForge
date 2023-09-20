#pragma once

#include "DOM/DOMNodeBase.hpp"
#include "io/BcsvIO.hpp"

enum EStarType {
    Normal,
    Hidden,
    Green
};

enum ECometType {
    None,
    Red,
    Purple,
    Dark,
    Exterminate,
    Mimic,
    Quick
};


//TODO: Make sure everything here has the proper signedness and update reading/writing accordingly

class SScenarioDOMNode : public SDOMNodeSerializable {
    uint32_t mScenarioNo;
    uint32_t mPowerStarId; // Bitfield, what power stars exist in scenario. TODO: make this a list that is generated on load?
    std::string mPowerStarType;
    std::string mAppearPowerStarObj; // Name of object that spawns the power star associate w/ scenario #
    std::string mComet; // Comet Type
    uint32_t mCometLimitTimer;

    //Unused
    std::string mScenarioName;
    uint32_t mLuigiModeTimer;
    uint32_t mErrorCheck;

    // SMG1 Specific
    uint32_t mIsHidden;

    std::map<std::string, uint32_t> mZoneLayers;

    // Selected node
    std::string mSelectedZone;

public:
    typedef SDOMNodeSerializable Super; 

    void RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected);
    void RenderDetailsUI();

    void AddZone(std::string zoneName) { mZoneLayers[zoneName] = 0; }
    void RemoveZone(std::string zoneName) { mZoneLayers.erase(zoneName); }

    SScenarioDOMNode();
    ~SScenarioDOMNode();

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::Scenario){
            return true;
        }

        return Super::IsNodeType(type);
    }

    void Deserialize(SBcsvIO* bcsv, int entry);
    void Serialize(SBcsvIO* bcsv, int entry);

};