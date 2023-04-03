#pragma once

#include "GenUtil.hpp"
#include "archive.h"
#include "compression.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <map>

namespace SResUtility
{
	class SGCResourceManager
	{
		bool mInitialized = false;
		GCcontext mResManagerContext;
		public:
			bool LoadArchive(const char* path, GCarchive* archive);
			bool SaveArchiveCompressed(const char* path, GCarchive* archive);
			bool ReplaceArchiveFileData(GCarcfile* file, uint8_t* new_data, size_t new_data_size);
			void Init();
	};
}

extern SResUtility::SGCResourceManager GCResourceManager;