#include "ResUtil.hpp"
#include "imgui.h"

SResUtility::SGCResourceManager GCResourceManager;

void SResUtility::SGCResourceManager::Init()
{
	GCerror err;
	if ((err = gcInitContext(&mResManagerContext)) != GC_ERROR_SUCCESS)
	{
		printf("Error initing arc loader context: %s\n", gcGetErrorMessage(err));
	}

	mInitialized = true;
}

bool SResUtility::SGCResourceManager::LoadArchive(const char* path, GCarchive* archive)
{
	if(!mInitialized) return false;
	
	GCerror err;

	FILE* f = fopen(path, "rb");
	if (f == nullptr)
	{
		printf("Error opening file \"%s\"\n", path);
		return false;
	}

	fseek(f, 0L, SEEK_END);
	GCsize size = (GCsize)ftell(f);
	rewind(f);

	void* file = malloc(size);
	if (file == nullptr)
	{
		printf("Error allocating buffer for file \"%s\"\n", path);
		return false;
	}

	fread(file, 1, size, f);
	fclose(f);

	// If the file starts with 'Yay0', it's Yay0 compressed.
	if (*((uint32_t*)file) == 0x30796159)
	{
		GCsize compressedSize = gcDecompressedSize(&mResManagerContext, (GCuint8*)file, 0);

		void* decompBuffer = malloc(compressedSize);
		gcYay0Decompress(&mResManagerContext, (GCuint8*)file, (GCuint8*)decompBuffer, compressedSize, 0);

		free(file);
		size = compressedSize;
		file = decompBuffer;
	}
	// Likewise, if the file starts with 'Yaz0' it's Yaz0 compressed.
	else if (*((uint32_t*)file) == 0x307A6159)
	{
		GCsize compressedSize = gcDecompressedSize(&mResManagerContext, (GCuint8*)file, 0);

		void* decompBuffer = malloc(compressedSize);
		gcYaz0Decompress(&mResManagerContext, (GCuint8*)file, (GCuint8*)decompBuffer, compressedSize, 0);
		free(file);
		size = compressedSize;
		file = decompBuffer;
	}

	gcInitArchive(archive, &mResManagerContext);
	if ((err = gcLoadArchive(archive, file, size)) != GC_ERROR_SUCCESS) {
		printf("Error Loading Archive: %s\n", gcGetErrorMessage(err));
		return false;
	}

	return true;
}

bool SResUtility::SGCResourceManager::ReplaceArchiveFileData(GCarcfile* file, uint8_t* new_data, size_t new_data_size){
	if(!mInitialized) return false;
	
	// free existing file
	gcFreeMem(&mResManagerContext, file->data);

	//allocate size of new file
	file->data = gcAllocMem(&mResManagerContext, new_data_size);
		
	//copy new jmp to file buffer for arc
	memcpy(file->data, new_data, new_data_size);

	//set size properly
	file->size = new_data_size;

	return true;
}

bool SResUtility::SGCResourceManager::SaveArchiveCompressed(const char* path, GCarchive* archive)
{
	if(!mInitialized) return false;

	GCsize outSize = gcSaveArchive(archive, NULL);
	GCuint8* archiveOut = new GCuint8[outSize];
	GCuint8* archiveCmp = new GCuint8[outSize];

	gcSaveArchive(archive, archiveOut);
	GCsize cmpSize = gcYay0Compress(&mResManagerContext, archiveOut, archiveCmp, outSize);
	
	std::ofstream fileStream;
	fileStream.open(path, std::ios::binary | std::ios::out);
	fileStream.write((const char*)archiveCmp, cmpSize);
	fileStream.close();

	delete archiveOut;
	delete archiveCmp;

	return true;
}