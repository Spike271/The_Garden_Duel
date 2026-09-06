#pragma once
#include <memory>
#include <string>

#include "raylib.h"

#include "Game.hpp"
#include "Renderer.hpp"

struct ApplicationSpecification
{
	int Width = 1280;
	int Height = 720;
	std::string Title = "Watering Plants: The Garden Duel";
};

class Application
{
public:
	explicit Application(ApplicationSpecification spec = ApplicationSpecification());

	~Application();

	void Run() const;

	static void SetIcon();

private:
	void OnUpdate() const;

	void OnRender() const;

private:
	ApplicationSpecification m_Specification;
	Renderer m_Renderer;
	std::unique_ptr<Game> m_Game;
	RenderTexture2D m_Target{};
};
