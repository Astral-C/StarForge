#include "glm/fwd.hpp"
#include <io/KclIO.hpp>

SKclIO::SKclIO(){

}

SKclIO::~SKclIO(){

}

void SKclIO::Draw(glm::mat4& transform){

}

bool SKclIO::Load(bStream::CMemoryStream* stream){
    uint32_t positionDataOffs = stream->readUInt32();
    uint32_t normDataOffs = stream->readUInt32();
    uint32_t prismDataOffs = stream->readUInt32();
    uint32_t blockDataOffs = stream->readUInt32();
    float prismThickness = stream->readFloat();

    glm::vec3 areaMinPos = {stream->readFloat(), stream->readFloat(), stream->readFloat()};
    glm::vec3 coordMask = {stream->readUInt32(), stream->readUInt32(), stream->readUInt32()};
    glm::vec3 coordShift = {stream->readUInt32(), stream->readUInt32(), stream->readUInt32()};

    mPositions.reserve(normDataOffs - positionDataOffs);
    mNormals.reserve(prismDataOffs - normDataOffs);

    stream->seek(positionDataOffs);
    for (int i = 0; i < (normDataOffs - positionDataOffs) / 12; i++) {
        mPositions.push_back({stream->readFloat(), stream->readFloat(), stream->readFloat()});
    }

    stream->seek(normDataOffs);
    for (int i = 0; i < (prismDataOffs - normDataOffs) / 12; i++) {
        mNormals.push_back({stream->readFloat(), stream->readFloat(), stream->readFloat()});
    }

    stream->seek(prismDataOffs);
    for (int i = 0; i < (blockDataOffs - prismDataOffs) / 16; i++) {
        KCLPrism prism;
        prism.mLength = stream->readFloat();
        prism.mPositionIdx = stream->readUInt16();
        prism.mDirectionIdx = stream->readUInt16();
        prism.mNormal1Idx = stream->readUInt16();
        prism.mNormal2Idx = stream->readUInt16();
        prism.mNormal3Idx = stream->readUInt16();
        prism.mFlags = stream->readUInt16();
        mPrisms.push_back(prism);
    }

    return true;
}

bool SKclIO::Save(bStream::CMemoryStream& stream){
    return true;
}
