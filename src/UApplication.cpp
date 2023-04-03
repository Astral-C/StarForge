#include "UApplication.hpp"
#include "UTime.hpp"

void UApplication::Run() {
	Clock::time_point lastFrameTime, thisFrameTime;

	while (true) {
		lastFrameTime = thisFrameTime;
		thisFrameTime = UUtil::GetTime();

		if (!Execute(UUtil::GetDeltaTime(lastFrameTime, thisFrameTime)))
			break;
	}
}
