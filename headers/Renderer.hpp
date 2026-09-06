#pragma once
#include "raylib.h"
#include "Game.hpp"
#include "Plant.hpp"
#include "Player.hpp"

class Renderer
{
public:
	static void Render(const Game& game);

private:
	static void RenderMenu(const Game& game);
	static void RenderPlay(const Game& game);
	static void RenderGameOver(const Game& game);

	static void DrawBG();
	static void DrawRiver(int side);
	static void DrawPlant(const Game& game, int i);
	static void DrawCharacter(const Player& p);
	static void DrawHUD(const Game& game);
	static void DrawMessages(const Game& game);
	static void DrawFX(const Game& game);

	static Color Fade(Color c, float a);
	static Color PlantBodyColor(const Plant& pl);
};
