#pragma once
#include <J3D/J3DLight.hpp>
#include <io/BcsvIO.hpp>
#include <string>
#include <map>

struct SLightingConfig {
    J3DLight Light0, Light1, Light2;
};

void LoadLightConfig(std::string name, SBcsvIO* bcsv);

extern std::map<std::string, SLightingConfig> LightingConfigs;