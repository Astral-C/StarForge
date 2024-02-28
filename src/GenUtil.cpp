#include "GenUtil.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <unicode/unistr.h>
#include <unicode/ustring.h>
#include <unicode/ucnv.h>

std::string SGenUtility::Utf8ToSjis(const std::string& value)
{
    icu::UnicodeString src(value.c_str(), "utf8");
    int length = src.extract(0, src.length(), NULL, "shift_jis");

    std::vector<char> result(length + 1);
    src.extract(0, src.length(), &result[0], "shift_jis");

    return std::string(result.begin(), result.end() - 1);
}

std::string SGenUtility::SjisToUtf8(const std::string& value)
{
    icu::UnicodeString src(value.c_str(), "shift_jis");
    int length = src.extract(0, src.length(), NULL, "utf8");

    std::vector<char> result(length + 1);
    src.extract(0, src.length(), &result[0], "utf8");

    return std::string(result.begin(), result.end() - 1);
}

glm::mat4 SGenUtility::CreateMTX(glm::vec3 scale, glm::vec3 dir, glm::vec3 pos){
	float out[16];
	float sinX = sin(glm::radians(dir.x)), cosX = cos(glm::radians(dir.x));
	float sinY = sin(glm::radians(dir.y)), cosY = cos(glm::radians(dir.y));
	float sinZ = sin(glm::radians(dir.z)), cosZ = cos(glm::radians(dir.z));

	out[0] = scale.x * (cosY * cosZ);
	out[1] = scale.x * (sinZ * cosY);
	out[2] = scale.x * (-sinY);
	out[3] = 0.0f;

	out[4] = scale.y * (sinX * cosZ * sinY - cosX * sinZ);
	out[5] = scale.y * (sinX * sinZ * sinY + cosX * cosZ);
	out[6] = scale.y * (sinX * cosY);
	out[7] = 0.0f;

	out[8] = scale.z * (cosX * cosZ * sinY + sinX * sinZ);
	out[9] = scale.z * (cosX * sinZ * sinY - sinX * cosZ);
	out[10] = scale.z * (cosY * cosX);
	out[11] = 0.0f;

	out[12] = pos.x;
	out[13] = pos.y;
	out[14] = pos.z;
	out[15] = 1.0f;

	return glm::make_mat4(out);
}