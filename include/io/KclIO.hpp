#pragma once

#include "../lib/bStream/bstream.h"
#include "DOM/DOMNodeSerializable.hpp"
#include "GenUtil.hpp"
#include "glm/fwd.hpp"
#include <cstdint>
#include <memory>
#include <vector>
#include <map>


struct KCLPrism {
    float mLength;
    uint16_t mPositionIdx;
    uint16_t mDirectionIdx;
    uint16_t mNormal1Idx;
    uint16_t mNormal2Idx;
    uint16_t mNormal3Idx;
    uint16_t mFlags;

};

class SKclIO
{
    std::vector<glm::vec3> mPositions, mNormals;
    std::vector<KCLPrism> mPrisms;
public:
	SKclIO();
	~SKclIO();

	void Draw(glm::mat4& transform);

	bool Load(bStream::CMemoryStream* stream);
	bool Save(bStream::CMemoryStream& stream);

};
