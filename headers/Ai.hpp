#pragma once

struct AiAgent
{
	enum class State { SeekPlant, SeekRiver };

	State state = State::SeekRiver;
	int target = -1; // target plant index when SeekPlant
	float thinkCd = 0.f; // seconds until next decision
};
