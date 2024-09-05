#pragma once

#include "GenUtil.hpp"
#include <Archive.hpp>
#include <Compression.hpp>


#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <map>

#include <J3D/Animation/J3DJointAnimationInstance.hpp>
#include <J3D/Animation/J3DTexMatrixAnimationInstance.hpp>
#include <J3D/Animation/J3DColorAnimationInstance.hpp>

namespace SResUtility
{
	class SGCResourceManager
	{
		bool mInitialized = false;
		public:

			void CacheModel(std::string modelName);

			std::shared_ptr<J3DAnimation::J3DColorAnimationInstance> LoadColorAnimation(std::string modelName, std::string animName);
			std::shared_ptr<J3DAnimation::J3DJointAnimationInstance> LoadJointAnimation(std::string modelName, std::string animName);
			std::shared_ptr<J3DAnimation::J3DTexMatrixAnimationInstance> LoadTextureAnimation(std::string modelName, std::string animName);

			void Init();
	};

	class SOptions
	{
		bool mSelectRootDialogOpen;
		std::string mObjectDBUrl;

		public:
			bool mRenderBillboardBounds { false };
			std::filesystem::path mRootPath;
			std::filesystem::path mProjectsPath { "projects" }; //defaults to "projects" in cwd

			void RenderOptionMenu();
			void LoadOptions();

			void UpdateObjectDB();
	};
}

extern SResUtility::SGCResourceManager GCResourceManager;
extern SResUtility::SOptions Options;