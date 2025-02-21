#include "LightConfigs.hpp"
#include <format>

std::map<std::string, SLightingConfig> LightingConfigs;

void LoadLightConfig(std::string name, SBcsvIO* bcsv){
    SLightingConfig config;
    
    config.Light0.AngleAtten = glm::vec4(1.0, 0.0, 0.0, 1);
    config.Light0.DistAtten = glm::vec4(1.0, 0.0, 0.0, 1);
    config.Light0.Direction = glm::vec4(1.0, 0.0, 0.0, 1);

    config.Light1.AngleAtten = glm::vec4(1.0, 0.0, 0.0, 1);
    config.Light1.DistAtten = glm::vec4(1.0, 0.0, 0.0, 1);
    config.Light1.Direction = glm::vec4(1.0, 0.0, 0.0, 1);

    config.Light2.Position = glm::vec4(0, 0, 0, 0);
    config.Light2.AngleAtten = glm::vec4(1.0, 0, 0, 1);
    config.Light2.DistAtten = glm::vec4(1.0, 0.0, 0.0, 1);
    config.Light2.Direction = glm::vec4(0, -1, 0, 1);
    config.Light2.Color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);

    config.Light0.Position = glm::vec4(bcsv->GetFloat(0, std::format("{0}Light0PosX", name)), bcsv->GetFloat(0, std::format("{0}Light0PosY", name)), bcsv->GetFloat(0, std::format("{0}Light0PosZ", name)), 1.0f);
    config.Light1.Position = glm::vec4(bcsv->GetFloat(0, std::format("{0}Light1PosX", name)), bcsv->GetFloat(0, std::format("{0}Light1PosY", name)), bcsv->GetFloat(0, std::format("{0}Light1PosZ", name)), 1.0f);
    std::cout << "Read " << name << "Light0 Pos as " << config.Light0.Position.x << " " << config.Light0.Position.y << " " << config.Light0.Position.z << " " << std::endl;
    std::cout << "Read " << name << "Light1 Pos as " << config.Light1.Position.x << " " << config.Light1.Position.y << " " << config.Light1.Position.z << " " << std::endl;

    config.Light0.Position.w = bcsv->GetUnsignedInt(0, std::format("{0}Light0FollowCamera", name));
    std::cout << "Read " << name << "Light0Followcamera as " << config.Light0.Position.w << std::endl;
    config.Light1.Position.w = bcsv->GetUnsignedInt(0, std::format("{0}Light1FollowCamera", name));
    std::cout << "Read " << name << "Light1Followcamera as " << config.Light1.Position.w << std::endl;

    config.Light2.Color.a = bcsv->GetUnsignedInt(0, std::format("{0}Alpha2", name)) / 255.0f;
    std::cout << "Read " << name << "Alpha2 as " << config.Light2.Color.a << " " <<  bcsv->GetUnsignedInt(0, std::format("{0}Alpha2", name))/255.0f << std::endl;

    config.Light0.Color = glm::vec4(
        bcsv->GetUnsignedInt(0, std::format("{0}Light0ColorR", name)) / 255.0f,
        bcsv->GetUnsignedInt(0, std::format("{0}Light0ColorG", name)) / 255.0f,
        bcsv->GetUnsignedInt(0, std::format("{0}Light0ColorB", name)) / 255.0f,
        bcsv->GetUnsignedInt(0, std::format("{0}Light0ColorA", name)) / 255.0f
    );

    config.Light1.Color = glm::vec4(
        bcsv->GetUnsignedInt(0, std::format("{0}Light1ColorR", name)) / 255.0f,
        bcsv->GetUnsignedInt(0, std::format("{0}Light1ColorG", name)) / 255.0f,
        bcsv->GetUnsignedInt(0, std::format("{0}Light1ColorB", name)) / 255.0f,
        bcsv->GetUnsignedInt(0, std::format("{0}Light1ColorA", name)) / 255.0f
    );

    LightingConfigs.insert({name, config});
}
