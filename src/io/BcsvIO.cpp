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
				case EJmpFieldType::Byte:
					std::get<uint32_t>(value) = (stream->readUInt8() & f.Bitmask) >> f.Shift;
					break;
				case EJmpFieldType::Short:
					std::get<uint32_t>(value) = (stream->readUInt16() & f.Bitmask) >> f.Shift;
					break;

				case EJmpFieldType::Integer:
				case EJmpFieldType::Integer2:
					std::get<uint32_t>(value) = (stream->readUInt32() & f.Bitmask) >> f.Shift;
					break;
				case EJmpFieldType::Float:
					std::get<float>(value) = stream->readFloat();
					break;
				case EJmpFieldType::String:
					std::get<std::string>(value) = stream->readString(mStringSize);
					break;
				case EJmpFieldType::StringOffset:
					char c;
					std::string str;

					size_t stringOffset = stream->readUInt32();
					stream->seek(stringTableOffset + stringOffset);
					
					while((c = stream->readUInt8()) != '\0'){
						str.push_back(c);
					}

					std::get<std::string>(value) = str;
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

	return hash & 0xFFFFFFFF;
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

	return (newSize + 3) & ~3; //just make 100% sure its aligned to 4
}


uint32_t SBcsvIO::GetUnsignedInt(uint32_t entry_index, std::string field_name){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		return std::get<uint32_t>(mData.at(entry_index).at(field->Hash));
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
		return SGenUtility::SjisToUtf8(std::get<std::string>(mData.at(entry_index).at(field->Hash)));
	} else {
		return "";
	}
}

uint16_t SBcsvIO::GetShort(uint32_t entry_index, std::string field_name){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		return std::get<uint32_t>(mData.at(entry_index).at(field->Hash));
	} else {
		return 0;
	}
}

uint8_t SBcsvIO::GetChar(uint32_t entry_index, std::string field_name){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		return std::get<uint32_t>(mData.at(entry_index).at(field->Hash));
	} else {
		return 0;
	}
}


bool SBcsvIO::SetUnsignedInt(uint32_t entry_index, std::string field_name, uint32_t value){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		std::get<uint32_t>(mData.at(entry_index).at(field->Hash)) = value;
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

bool SBcsvIO::SetShort(uint32_t entry_index, std::string field_name, uint16_t value){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		std::get<uint32_t>(mData.at(entry_index).at(field->Hash)) = value;
		return true;
	} else {
		return false;
	}
}

bool SBcsvIO::SetChar(uint32_t entry_index, std::string field_name, uint8_t value){
	const SBcsvFieldInfo* field = FetchJmpFieldInfo(field_name);

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
		std::get<std::string>(mData.at(entry_index).at(field->Hash)) = SGenUtility::Utf8ToSjis(value);
		return true;
	} else {
		return false;
	}	
}

void SBcsvIO::AddField(std::string name, EJmpFieldType type){
	SBcsvFieldInfo fieldInfo;
	
	fieldInfo.Hash = HashFieldName(name);
	fieldInfo.Bitmask = 0;
	fieldInfo.Shift = 0;
	fieldInfo.Type = type;
	fieldInfo.Start = (uint16_t)CalculateNewEntrySize();

	mFields.push_back(fieldInfo);
}

bool SBcsvIO::Save(std::vector<std::shared_ptr<SDOMNodeSerializable>> entities, bStream::CMemoryStream& stream, std::function<void(SBcsvIO*, int, std::shared_ptr<SDOMNodeSerializable> node)> Serializer){
	if(entities.size() <= 0){
		return false; // Don't bother trying to write 0 entries
	}

	mEntryCount = entities.size();
	mEntrySize = CalculateNewEntrySize();
	mEntryStartOffset = mFieldCount * sizeof(SBcsvFieldInfo) + JMP_HEADER_SIZE;

	stream.writeInt32((int32_t)entities.size());
	stream.writeInt32(mFieldCount);
	stream.writeUInt32(mFieldCount * sizeof(SBcsvFieldInfo) + JMP_HEADER_SIZE);


	stream.writeUInt32(mEntrySize);

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
		if(!Serializer){
			entities[i]->Serialize(this, i); // set entry data
		} else {
			Serializer(this, i, entities[i]);
		}
	}

	std::map<uint32_t, std::string> StringTable;
	uint32_t StringTableOffset = 0;

	uint8_t* tempBuffer = new uint8_t[mEntryCount * mEntrySize]{};

	bStream::CMemoryStream ReadStream(tempBuffer, mEntryCount * mEntrySize, bStream::Endianess::Big, bStream::OpenMode::In);
	bStream::CMemoryStream WriteStream(tempBuffer, mEntryCount * mEntrySize, bStream::Endianess::Big, bStream::OpenMode::Out);

	for (uint32_t i = 0; i < entities.size(); i++)
	{
		for (const SBcsvFieldInfo& f : mFields)
		{
			uint32_t offset = -1;
			std::string str = std::get<std::string>(mData.at(i).at(f.Hash));
			ReadStream.seek((mEntrySize * i) + f.Start);
			WriteStream.seek((mEntrySize * i) + f.Start);
			switch(f.Type){
				case EJmpFieldType::Byte:
					{
						uint8_t value = ReadStream.readUInt8(); // Value already in place
						value = (value & ~f.Bitmask) | (((uint8_t)std::get<uint32_t>(mData.at(i).at(f.Hash))) << f.Shift) & f.Bitmask;
						WriteStream.writeUInt8(value);
					}
					break;
				case EJmpFieldType::Short:
					{
						uint16_t value = ReadStream.readUInt16(); // Value already in place
						value = (value & ~f.Bitmask) | (((uint16_t)std::get<uint32_t>(mData.at(i).at(f.Hash))) << f.Shift) & f.Bitmask;
						WriteStream.writeUInt16(value);
					}
					break;
				case EJmpFieldType::Integer:
				case EJmpFieldType::Integer2:
					{
						uint32_t value = ReadStream.readUInt32(); // Value already in place
						value = (value & ~f.Bitmask) | (std::get<uint32_t>(mData.at(i).at(f.Hash)) << f.Shift) & f.Bitmask;
						WriteStream.writeUInt32(value);
					}
					break;
				case EJmpFieldType::Float:
					WriteStream.writeFloat(std::get<float>(mData.at(i).at(f.Hash)));
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

					WriteStream.writeUInt32(offset);
					break;
			}
		}
	}

	stream.seek(mEntryStartOffset);
	stream.writeBytes((char*)tempBuffer, mEntryCount * mEntrySize);

	stream.seek(mEntryStartOffset + (mEntryCount * mEntrySize));

	for(auto& [k, v] : StringTable){
		stream.writeString(v);
		stream.writeUInt8(0);
	}

	size_t padded_size = SGenUtility::PadToBoundary(stream.tell(), 4);
	std::cout << "Padding to " << padded_size << " bytes" << std::endl;
	for(int i = 0; stream.tell() < padded_size; i++) stream.writeUInt8(0x40);

	delete[] tempBuffer;

	return true;
}