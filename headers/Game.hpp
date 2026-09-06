#pragma once
#include <random>
#include <string>
#include <vector>

#include "raylib.h"
#include "Ai.hpp"
#include "Common.hpp"
#include "Message.hpp"
#include "Particle.hpp"
#include "Plant.hpp"
#include "Player.hpp"

class Game
{
public:
	Game();

	void OnUpdate(float dt);

	[[nodiscard]] GameState State() const { return m_State; }
	[[nodiscard]] GameMode Mode() const { return m_Mode; }
	[[nodiscard]] int Difficulty() const { return m_Difficulty; }
	[[nodiscard]] const std::vector<Plant>& Plants() const { return m_Plants; }
	[[nodiscard]] int PlantCount() const { return m_N; }
	[[nodiscard]] const Player& P1() const { return m_P1; }
	[[nodiscard]] const Player& P2() const { return m_P2; }
	[[nodiscard]] const std::vector<Particle>& FX() const { return m_FX; }
	[[nodiscard]] const std::vector<Message>& Messages() const { return m_Msgs; }
	[[nodiscard]] float Time() const { return m_Time; }
	[[nodiscard]] float EndTimer() const { return m_EndTimer; }
	[[nodiscard]] int Winner() const { return m_Winner; }
	[[nodiscard]] int MenuSel() const { return m_MenuSel; }
	[[nodiscard]] float BgOff() const { return m_BgOff; }
	[[nodiscard]] bool QuitRequested() const { return m_QuitRequested; }

	[[nodiscard]] float PlantX(int i) const;

	static float RiverX(int side);

private:
	void InitGame(int diff);

	// Actions
	void DoMove(Player& p, int dir);

	void DoAction(Player& p, Player& opp);

	void DoPowerUp(Player& p, const Player& opp);

	void UpdateMenu(float dt);

	void UpdatePlay(float dt);

	void UpdateOver(float dt);

	static void UpdatePlayer(Player& p, float dt);

	void ThinkAI();

	void UpdateAI(float dt);

	[[nodiscard]] int AiCalcNextTarget() const;

	[[nodiscard]] int AiCalcWaterCost(int target) const;

	void AiMove(Player& p, int dir);

	// Helpers
	[[nodiscard]] float GridX(int gp) const;

	[[nodiscard]] bool AllDone() const;

	void AddMsg(const std::string& t, Color c);

	void Burst(float x, float y, Color c, int n = 14);

private:
	GameState m_State = GS_MENU;
	GameMode m_Mode = GM_PVP;
	std::vector<Plant> m_Plants;
	int m_N = 0;
	int m_Difficulty = 1; // 0=easy 1=medium 2=hard — also drives AI tuning
	Player m_P1, m_P2;
	AiAgent m_Ai;
	std::vector<Particle> m_FX;
	std::vector<Message> m_Msgs;
	float m_Time = 0.f;
	float m_EndTimer = -1.f;
	int m_Winner = -2; // -2=pending, -1=tie
	int m_MenuSel = 1;
	float m_BgOff = 0.f;
	std::mt19937 m_Rng{std::random_device{}()};
	bool m_QuitRequested = false;
};
