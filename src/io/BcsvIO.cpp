#include "io/BcsvIO.hpp"

SBcsvIO::SBcsvIO()
{

}

SBcsvIO::~SBcsvIO()
{
	if(mData != nullptr){
		delete mData;
	}

	if(mStringTable != nullptr){
		delete mStringTable;
	}
}

bool SBcsvIO::Load(bStream::CMemoryStream* stream)
{
	mEntryCount = stream->readInt32();
	mFieldCount = stream->readInt32();
	mEntryStartOffset = stream->readUInt32();
	mEntrySize = stream->readUInt32();

	if (mEntrySize == 0 || mEntryStartOffset + mEntrySize * mEntryCount > stream->getSize())
		return false;

	mData = new uint8_t[mEntryCount * mEntrySize];

	// Make our own copy of the entry data, since the stream will be
	// destroyed when we're done reading.
	memcpy(mData, stream->getBuffer() + mEntryStartOffset, mEntrySize * mEntryCount);


	mFields.clear();
	mFields.reserve(mFieldCount);

	for (int32_t i = 0; i < mFieldCount; i++)
	{
		SBcsvFieldInfo newField;
		newField.Hash = stream->readUInt32();
		newField.Bitmask = stream->readUInt32();
		newField.Start = stream->readUInt16();
		newField.Shift = stream->readUInt8();
		newField.Type = (EJmpFieldType)stream->readUInt8();

		mFields.push_back(newField);
	}

	// Head to the end of the enstries and read the string table
	uint32_t stringTableSize = (stream->getSize() - (mEntryStartOffset + (mEntrySize * mEntryCount)));
	mStringTable = new uint8_t[stringTableSize];

	memcpy(mStringTable, stream->getBuffer() + mEntryStartOffset + (mEntrySize * mEntryCount), stringTableSize);

	return true;
}

uint32_t SBcsvIO::HashFieldName(std::string name) const
{
	uint32_t hash = 0;

	for (char c : name)
	{
		hash *= 0x1F;
		hash += c;
	}

	return hash;
}

const SBcsvFieldInfo* SBcsvIO::FetchJmpFieldInfo(std::string name)
{
	const SBcsvFieldInfo* field = nullptr;
	uint32_t nameHash = HashFieldName(name);

	for (const SBcsvFieldInfo& f : mFields)
	{
		if (nameHash == f.Hash)
		{
			field = &f;
			break;
		}
	}

	return field;
}

const SBcsvFieldInfo* SBcsvIO::FetchJmpFieldInfo(uint32_t hash)
{
	const SBcsvFieldInfo* field = nullptr;

	for (const SBcsvFieldInfo& f : mFields)
	{
		if (hash == f.Hash)
		{
			field = &f;
			break;
		}
	}

	return field;
}

uint32_t SBcsvIO::PeekU32(uint32_t offset)
{
	if (offset >= mEntryCount * mEntrySize)
		return 0;

	return (
		mData[offset]     << 24 |
		mData[offset + 1] << 16 |
		mData[offset + 2] << 8 |
		mData[offset + 3]
	);
}

int32_t SBcsvIO::PeekS32(uint32_t offset)
{
	return (int32_t)PeekU32(offset);
}

float SBcsvIO::PeekF32(uint32_t offset)
{
	union {
		uint32_t u32;
		float f32;
	} converter;

	converter.u32 = PeekU32(offset);
	return converter.f32;
}

bool SBcsvIO::PokeU32(uint32_t offset, uint32_t value)
{
	if (offset >= mEntryCount * mEntrySize)
		return false;

	mData[offset] = (uint8_t)(value >> 24);
	mData[offset + 1] = (uint8_t)(value >> 16);
	mData[offset + 2] = (uint8_t)(value >> 8);
	mData[offset + 3] = (uint8_t)(value);

	return true;
}

bool SBcsvIO::PokeS32(uint32_t offset, int32_t value)
{
	return PokeU32(offset, (uint32_t)value);
}

bool SBcsvIO::PokeF32(uint32_t offset, float value)
{
	union {
		uint32_t u32;
		float f32;
	} converter;

	converter.f32 = value;
	return PokeU32(offset, converter.u32);
}

uint32_t SBcsvIO::GetUnsignedInt(uint32_t entry_index, std::string field_name)
{
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	// If field is still nullptr, we failed to find a field matching the given name.
	if (field == nullptr)
		return 0;

	uint32_t fieldOffset = entry_index * mEntrySize + field->Start;
	uint32_t rawFieldValue = PeekU32(fieldOffset);

	return (rawFieldValue & field->Bitmask) >> field->Shift;
}

int32_t SBcsvIO::GetSignedInt(uint32_t entry_index, std::string field_name)
{
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	// If field is still nullptr, we failed to find a field matching the given name.
	if (field == nullptr)
		return 0;

	uint32_t fieldOffset = entry_index * mEntrySize + field->Start;

	return PeekS32(fieldOffset);
}

int32_t SBcsvIO::GetSignedInt(uint32_t entry_index, uint32_t field_hash)
{
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_hash);

	// If field is still nullptr, we failed to find a field matching the given name.
	if (field == nullptr)
		return 0;

	uint32_t fieldOffset = entry_index * mEntrySize + field->Start;

	return PeekS32(fieldOffset);
}

float SBcsvIO::GetFloat(uint32_t entry_index, std::string field_name)
{
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	// If field is still nullptr, we failed to find a field matching the given name.
	if (field == nullptr)
		return 0.0f;

	uint32_t fieldOffset = entry_index * mEntrySize + field->Start;

	return PeekF32(fieldOffset);
}

bool SBcsvIO::GetBoolean(uint32_t entry_index, std::string field_name)
{
	return GetUnsignedInt(entry_index, field_name) != 0;
}

std::string SBcsvIO::GetString(uint32_t entry_index, std::string field_name)
{
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	// If field is still nullptr, we failed to find a field matching the given name.
	if (field == nullptr)
		return "";

	uint32_t fieldOffset = entry_index * mEntrySize + field->Start;
	uint32_t strOffset = PeekU32(fieldOffset);

	//char strBuffer[33];
	//memcpy(strBuffer, mData + fieldOffset, mStringSize);
	//strBuffer[32] = 0;
	std::string strBuffer;
	while(mStringTable[strOffset] != '\0'){
		strBuffer.push_back(mStringTable[strOffset]);
		strOffset++;
	}

	return strBuffer;
}

uint32_t SBcsvIO::CalculateNewEntrySize()
{
	uint32_t newSize = 0;

	for (const SBcsvFieldInfo f : mFields)
	{
		uint32_t tempNewSize = f.Start;

		if (f.Type == EJmpFieldType::String)
			tempNewSize += mStringSize;
		else
			tempNewSize += sizeof(uint32_t);

		newSize = std::max(newSize, tempNewSize);
	}

	return newSize;
}

/*
bool SBcsvIO::Save(std::vector<std::shared_ptr<LEntityDOMNode>> entities, bStream::CMemoryStream& stream)
{
	stream.writeInt32((int32_t)entities.size());
	stream.writeInt32(mFieldCount);
	stream.writeUInt32(mFieldCount * sizeof(SBcsvFieldInfo) + JMP_HEADER_SIZE);

	uint32_t newEntrySize = CalculateNewEntrySize();
	stream.writeUInt32(newEntrySize);

	// Write the field info data
	for (const SBcsvFieldInfo f : mFields)
	{
		stream.writeUInt32(f.Hash);
		stream.writeUInt32(f.Bitmask);
		stream.writeUInt16(f.Start);
		stream.writeUInt8(f.Shift);
		stream.writeUInt8((uint8_t)f.Type);
	}

	// Discard old entry data
	if (mData != nullptr)
		delete[] mData;

	uint32_t newDataSize = (entities.size() * newEntrySize + 31) & ~31;

	// Allocate new entry data. Empty braces zero-initialize the memory region!
	mData = new uint8_t[newDataSize] {};
	if (mData == nullptr)
		return false;

	for (uint32_t i = 0; i < entities.size(); i++)
	{
		entities[i]->Serialize(this, i);
	}

	stream.writeBytes((char*)mData, newDataSize);

	return true;
}
*/

bool SBcsvIO::SetUnsignedInt(uint32_t entry_index, std::string field_name, uint32_t value)
{
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	// If field is still nullptr, we failed to find a field matching the given name.
	if (field == nullptr)
		return false;

	uint32_t fieldOffset = entry_index * mEntrySize + field->Start;

	uint32_t curField = PeekU32(fieldOffset);
	uint32_t packedValue = (value << field->Shift) & field->Bitmask;

	return PokeU32(fieldOffset, (curField & ~field->Bitmask) | packedValue);
}

bool SBcsvIO::SetSignedInt(uint32_t entry_index, std::string field_name, int32_t value)
{
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	// If field is still nullptr, we failed to find a field matching the given name.
	if (field == nullptr)
		return false;

	uint32_t fieldOffset = entry_index * mEntrySize + field->Start;

	return PokeS32(fieldOffset, value);
}

bool SBcsvIO::SetSignedInt(uint32_t entry_index, uint32_t field_hash, int32_t value)
{
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_hash);

	// If field is still nullptr, we failed to find a field matching the given name.
	if (field == nullptr)
		return false;

	uint32_t fieldOffset = entry_index * mEntrySize + field->Start;

	return PokeS32(fieldOffset, value);
}

bool SBcsvIO::SetFloat(uint32_t entry_index, std::string field_name, float value)
{
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	// If field is still nullptr, we failed to find a field matching the given name.
	if (field == nullptr)
		return false;

	uint32_t fieldOffset = entry_index * mEntrySize + field->Start;

	return PokeF32(fieldOffset, value);
}

bool SBcsvIO::SetBoolean(uint32_t entry_index, std::string field_name, bool value)
{
	return SetUnsignedInt(entry_index, field_name, (uint32_t)value);
}

bool SBcsvIO::SetString(uint32_t entry_index, std::string field_name, std::string value)
{
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	// If field is still nullptr, we failed to find a field matching the given name.
	if (field == nullptr)
		return false;

	uint32_t fieldOffset = entry_index * mEntrySize + field->Start;
	memcpy(mData + fieldOffset, value.data(), std::min(mStringSize - 1, value.length()));

	return true;
}
