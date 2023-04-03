#pragma once
#include "DOM/GalaxyDOMNode.hpp"

class UApplication {

	virtual bool Execute(float deltaTime) = 0;
public:
	UApplication() {}
	virtual ~UApplication() {}

	void Run();

	virtual bool Setup() = 0;
	virtual bool Teardown() = 0;
};
