#ifdef __linux__
#include <unicode/unistr.h>
#include <unicode/ustring.h>
#include <unicode/ucnv.h>
#endif

#ifdef _WIN32
#include <icu.h>
#endif

#include "GenUtil.hpp"
#include <glm/gtc/type_ptr.hpp>

// The following was copied from https://gist.github.com/kilfu0701/e279e35372066ae1832850c438d5611e
std::string SGenUtility::Utf8ToSjis(const std::string& value)
{
#ifdef __linux__
    icu::UnicodeString src(value.c_str(), "utf8");
    int length = src.extract(0, src.length(), NULL, "shift_jis");

    std::vector<char> result(length + 1);
    src.extract(0, src.length(), &result[0], "shift_jis");

    return std::string(result.begin(), result.end() - 1);
#endif

#ifdef _WIN32
    return ""; //currently doesnt work on windows! oops.
#endif
}

std::string SGenUtility::SjisToUtf8(const std::string& value)
{
#ifdef __linux__
    icu::UnicodeString src(value.c_str(), "shift_jis");
    int length = src.extract(0, src.length(), NULL, "utf8");

    std::vector<char> result(length + 1);
    src.extract(0, src.length(), &result[0], "utf8");

    return std::string(result.begin(), result.end() - 1);
#endif


#ifdef _WIN32
    if (value.empty())
        return "";

    UErrorCode errorCode;
    UConverter* conv = ucnv_open("shift_jis", &errorCode);

    std::vector<UChar> uCharBuff;
    const char* a = value.c_str();

    // Convert the input from SHIFT-JIS to UTF-16.
    // TODO: Is there a better way to do this?
    UChar target;
    errorCode = UErrorCode::U_ZERO_ERROR;
    while (errorCode == UErrorCode::U_ZERO_ERROR)
    {
        errorCode = UErrorCode::U_ZERO_ERROR;
        target = ucnv_getNextUChar(conv, &a, (&value.back()) + 1, &errorCode);

        if (errorCode == UErrorCode::U_ZERO_ERROR)
            uCharBuff.push_back(target);
    }

    // Ensure there's a null terminator
    uCharBuff.push_back(0);

    ucnv_close(conv);

    int32_t outputSize;
    std::string output;

    // Figure out how much space we need to convert from UTF-16 to UTF-8.
    errorCode = UErrorCode::U_ZERO_ERROR;
    u_strToUTF8(nullptr, 0, &outputSize, &uCharBuff.front(), uCharBuff.size(), &errorCode);
    
    // Make the string the string big enough to receive the converted data.
    output.resize(static_cast<size_t>(outputSize));

    // Actually convert the data.
    errorCode = UErrorCode::U_ZERO_ERROR;
    u_strToUTF8(output.data(), outputSize, &outputSize, &uCharBuff[0], uCharBuff.size(), &errorCode);

    return output;
#endif
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