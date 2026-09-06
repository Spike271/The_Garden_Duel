#include <algorithm>
#include <utility>

#include "raylib.h"

#include "../headers/Application.hpp"
#include "../headers/Lock.hpp"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

namespace
{
	constexpr int FONT_ATLAS_SCALE = 4;
}

Application::Application(ApplicationSpecification spec)
	: m_Specification(std::move(spec))
{
	if (acquire_lock() == -1)
	{
		fprintf(stderr, "Another instance is already running.");
		return;
	}

	SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
	InitWindow(m_Specification.Width, m_Specification.Height, m_Specification.Title.c_str());
	SetWindowMinSize(SW / 2, SH / 2);
	SetExitKey(0);

#if !defined(PLATFORM_WEB)
	SetTargetFPS(GetMonitorRefreshRate(0));
	SetIcon();
#endif

	const char* exeDir = GetApplicationDirectory();
	customFont = LoadFontEx(TextFormat("%sres/GoogleSans.ttf", exeDir), 70 * FONT_ATLAS_SCALE, nullptr, 95);
	if (customFont.texture.id == 0)
	{
		TraceLog(LOG_WARNING, "Failed to load res/GoogleSans.ttf next to the exe — "
				"falling back to raylib's default font. Check that res/ "
				"was copied alongside the built executable.");
	}
	else
	{
		SetTextureFilter(customFont.texture, TEXTURE_FILTER_BILINEAR);
	}

	m_Game = std::make_unique<Game>();
}

Application::~Application()
{
	release_lock();
	CloseWindow();
}

void Application::OnUpdate() const
{
	float dt = GetFrameTime();
	dt = std::min(dt, 0.1f); // clamp spike when unfocused
	m_Game->OnUpdate(dt);
}

void Application::OnRender() const
{
	if (IsKeyPressed(KEY_F11))
	{
		ToggleFullscreen();
	}

	const auto screenW = static_cast<float>(GetScreenWidth());
	const auto screenH = static_cast<float>(GetScreenHeight());
	const float scale = std::min(screenW / static_cast<float>(SW), screenH / static_cast<float>(SH));

	const Camera2D camera{
		.offset = Vector2{
			.x = (screenW - static_cast<float>(SW) * scale) * 0.5f,
			.y = (screenH - static_cast<float>(SH) * scale) * 0.5f,
		},
		.target = Vector2{.x = 0.0f, .y = 0.0f},
		.rotation = 0.0f,
		.zoom = scale,
	};

	BeginDrawing();
	ClearBackground(BLACK); // letterbox bars outside the scaled play area
	BeginMode2D(camera);
	Renderer::Render(*m_Game);
	EndMode2D();
	EndDrawing();
}

void Application::Run() const
{
	while (!WindowShouldClose() && !m_Game->QuitRequested())
	{
		OnUpdate();
		OnRender();
	}
}

void Application::SetIcon()
{
	if (const Image img = LoadImage(TextFormat("%sres/logo.png", GetApplicationDirectory()));
		img.data != nullptr)
	{
		SetWindowIcon(img);
		UnloadImage(img);
	}
}
