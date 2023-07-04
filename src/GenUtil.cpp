#include "GenUtil.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <iconv.h>

std::string SGenUtility::Utf8ToSjis(const std::string& value) {
    iconv_t conv = iconv_open("SHIFT-JIS", "UTF-8");
    if (conv == (iconv_t)(-1)) {
        throw std::runtime_error("Error opening iconv for UTF-8 to Shift-JIS conversion");
    }

    size_t inbytesleft = value.size();
    char* inbuf = const_cast<char*>(value.data());

    size_t outbytesleft = value.size() * 2;
    std::string sjis(outbytesleft, '\0');
    char* outbuf = &sjis[0];

    if (iconv(conv, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == (size_t)(-1)) {
        throw std::runtime_error("Error converting from UTF-8 to Shift-JIS");
    }

    sjis.resize(sjis.size() - outbytesleft);
    iconv_close(conv);
    return sjis;
}

std::string SGenUtility::SjisToUtf8(const std::string& value) {
    iconv_t conv = iconv_open("UTF-8", "SHIFT-JIS");
    if (conv == (iconv_t)(-1)) {
        throw std::runtime_error("Error opening iconv for Shift-JIS to UTF-8 conversion");
    }

    size_t inbytesleft = value.size();
    char* inbuf = const_cast<char*>(value.data());

    size_t outbytesleft = value.size() * 3;
    std::string utf8(outbytesleft, '\0');
    char* outbuf = &utf8[0];

    if (iconv(conv, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == (size_t)(-1)) {
        throw std::runtime_error("Error converting from Shift-JIS to UTF-8");
    }

    utf8.resize(utf8.size() - outbytesleft);
    iconv_close(conv);
    return utf8;
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