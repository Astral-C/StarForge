#include "io/BcsvIO.hpp"
#include "GenUtil.hpp"
#include <map>
#include <fmt/core.h>

SBcsvIO::SBcsvIO()
{

}

SBcsvIO::~SBcsvIO()
{

}

bool SBcsvIO::Load(bStream::CMemoryStream* stream)
{
	mEntryCount = stream->readInt32();
	mFieldCount = stream->readInt32();
	mEntryStartOffset = stream->readUInt32();
	mEntrySize = stream->readUInt32();

	if (mEntrySize == 0 || mEntryStartOffset + mEntrySize * mEntryCount > stream->getSize())
		return false;


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

	mData.clear();
	mData.reserve(mEntryCount);

	uint32_t stringTableOffset = mEntryStartOffset + mEntrySize * mEntryCount;

	for(int32_t i = 0; i < mEntryCount; i++)
	{
		SBcsvEntry entry;
		for (const SBcsvFieldInfo& f : mFields)
		{
			SBcsvValue value;

			stream->seek(mEntryStartOffset + (mEntrySize * i) + f.Start);
			switch(f.Type){
				case EJmpFieldType::Integer:
				case EJmpFieldType::Integer2:
					std::get<0>(value) = stream->readUInt32();
					break;
				case EJmpFieldType::Float:
					std::get<1>(value) = stream->readFloat();
					break;
				case EJmpFieldType::String:
					std::get<2>(value) = stream->readString(mStringSize);
					break;
				case EJmpFieldType::StringOffset:
					char c;
					std::string str;

					size_t stringOffset = stream->readUInt32();
					stream->seek(stringTableOffset + stringOffset);
					
					while((c = stream->readUInt8()) != '\0'){
						str.push_back(c);
					}

					std::get<2>(value) = str;
					break;
			}

			entry.insert({f.Hash, value});
		}
		mData.push_back(entry);
	}

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


uint32_t SBcsvIO::GetUnsignedInt(uint32_t entry_index, std::string field_name){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		return (std::get<uint32_t>(mData.at(entry_index).at(field->Hash)) & field->Bitmask) >> field->Shift;
	} else {
		return 0;
	}
}


int32_t SBcsvIO::GetSignedInt(uint32_t entry_index, std::string field_name){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		return (int32_t)std::get<uint32_t>(mData.at(entry_index).at(HashFieldName(field_name)));
	} else {
		return 0;
	}
}

int32_t SBcsvIO::GetSignedInt(uint32_t entry_index, uint32_t field_hash){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_hash);

	if(entry_index < mData.size() && field != nullptr){
		return (int32_t)std::get<uint32_t>(mData.at(entry_index).at(field_hash));
	} else {
		return 0;
	}
}

float SBcsvIO::GetFloat(uint32_t entry_index, std::string field_name){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		return std::get<float>(mData.at(entry_index).at(field->Hash));
	} else {
		return 0.0f;
	}
}

bool SBcsvIO::GetBoolean(uint32_t entry_index, std::string field_name){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		return std::get<uint32_t>(mData.at(entry_index).at(field->Hash)) != 0;
	} else {
		return false;
	}
}

std::string SBcsvIO::GetString(uint32_t entry_index, std::string field_name){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		return std::get<std::string>(mData.at(entry_index).at(field->Hash));
	} else {
		return "";
	}
}



bool SBcsvIO::SetUnsignedInt(uint32_t entry_index, std::string field_name, uint32_t value){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		std::get<uint32_t>(mData.at(entry_index).at(field->Hash)) = (std::get<0>(mData.at(entry_index).at(field->Hash)) & ~field->Bitmask) | (value << field->Shift) & field->Bitmask;
		return true;
	} else {
		return false;
	}
}

bool SBcsvIO::SetSignedInt(uint32_t entry_index, std::string field_name, int32_t value){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		std::get<uint32_t>(mData.at(entry_index).at(field->Hash)) = value;
		return true;
	} else {
		return false;
	}
}

bool SBcsvIO::SetSignedInt(uint32_t entry_index, uint32_t field_hash, int32_t value){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_hash);

	if(entry_index < mData.size() && field != nullptr){
		std::get<uint32_t>(mData.at(entry_index).at(field->Hash)) = value;
		return true;
	} else {
		return false;
	}
}

bool SBcsvIO::SetFloat(uint32_t entry_index, std::string field_name, float value){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		std::get<float>(mData.at(entry_index).at(field->Hash)) = value;
		return true;
	} else {
		return false;
	}	
}

bool SBcsvIO::SetBoolean(uint32_t entry_index, std::string field_name, bool value){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		return SetUnsignedInt(entry_index, field_name, (uint32_t)value);
	} else {
		return false;
	}	
}


bool SBcsvIO::SetString(uint32_t entry_index, std::string field_name, std::string value){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		std::get<std::string>(mData.at(entry_index).at(field->Hash)) = SGenUtility::SjisToUtf8(value);
		return true;
	} else {
		return false;
	}	
}

bool SBcsvIO::Save(std::vector<std::shared_ptr<SDOMNodeSerializable>> entities, bStream::CMemoryStream& stream){
	if(entities.size() <= 0){
		return false; // Don't bother trying to write 0 entries
	}

	mEntryCount = entities.size();

	stream.writeInt32((int32_t)entities.size());
	stream.writeInt32(mFieldCount);
	stream.writeUInt32(mFieldCount * sizeof(SBcsvFieldInfo) + JMP_HEADER_SIZE);

	uint32_t newEntrySize = CalculateNewEntrySize();
	stream.writeUInt32(newEntrySize);

	SBcsvEntry entry; //empty entry data

	// Write the field info data
	for (const SBcsvFieldInfo f : mFields)
	{
		stream.writeUInt32(f.Hash);
		stream.writeUInt32(f.Bitmask);
		stream.writeUInt16(f.Start);
		stream.writeUInt8(f.Shift);
		stream.writeUInt8((uint8_t)f.Type);

		entry.insert({f.Hash, SBcsvValue()});
	}

	mData.clear();
	mData.reserve(entities.size());

	for (uint32_t i = 0; i < entities.size(); i++)
	{
		mData.push_back(entry); //Add empty dummy entry
		std::cout << "Serializing node " << entities[i]->GetName() << std::endl;
		entities[i]->Serialize(this, i); // set entry data
	}

	std::map<uint32_t, std::string> StringTable;
	uint32_t StringTableOffset = 0;

	for (uint32_t i = 0; i < entities.size(); i++)
	{
		for (const SBcsvFieldInfo& f : mFields)
		{
			uint32_t offset = -1;
			std::string str = std::get<std::string>(mData.at(i).at(f.Hash));
			stream.seek(mEntryStartOffset + (mEntrySize * i) + f.Start);
			switch(f.Type){
				case EJmpFieldType::Integer:
				case EJmpFieldType::Integer2:
					stream.writeUInt32(std::get<uint32_t>(mData.at(i).at(f.Hash)));
					break;
				case EJmpFieldType::Float:
					std::cout << "set float as " << std::get<1>(mData.at(i).at(f.Hash)) << std::endl;
					stream.writeFloat(std::get<float>(mData.at(i).at(f.Hash)));
					break;
				case EJmpFieldType::StringOffset:
					if(StringTable.empty()){
						offset = 0;
						StringTable.insert({offset, str});
						StringTableOffset += str.length() + 1;
					} else {
						for(auto& [k, v] : StringTable){
							if(v == str){
								//std::cout << "Found " << str << " w/ str table entry " << k << ":" << v << std::endl;
								offset = k;
								break;
							}
						}

						if(offset == -1){
							offset = StringTableOffset;
							StringTable.insert({offset, str});
							StringTableOffset += str.length() + 1;
						}
					}

					stream.writeUInt32(offset);
					break;
				default:
					stream.writeUInt32(0);
					break;
			}
		}
	}

	stream.seek(mEntryStartOffset + (mEntrySize * mEntryCount));

	for(auto& [k, v] : StringTable){
		std::cout << "Writing stringtable entry " << v << " at offset " << k << std::endl;
		stream.writeString(v);
		stream.writeUInt8(0);
	}

	size_t padding_size = SGenUtility::PadToBoundary(stream.tell(), 32) - stream.tell();
	for(int i = 0; i < padding_size; i++) stream.writeUInt8(0);

	return true;
}