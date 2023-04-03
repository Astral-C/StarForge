#pragma once

#include "UApplication.hpp"

class UStarForgeApplication : public UApplication {
	struct GLFWwindow* mWindow;
	class UStarForgeContext* mContext;

	virtual bool Execute(float deltaTime) override;

public:
	UStarForgeApplication();
	virtual ~UStarForgeApplication() {}

	virtual bool Setup() override;
	virtual bool Teardown() override;
};
