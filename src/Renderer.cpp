#include "../headers/Renderer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <utility>

namespace
{
	auto Text(const char* t, const float x, const float y, const float size, const Color c) -> auto
	{
		DrawTextEx(customFont, t, Vector2{.x = x, .y = y}, size, 0, c);
	}

	auto TextW(const char* t, const float size) -> int
	{
		return static_cast<int>(MeasureTextEx(customFont, t, size, 0).x);
	}

	auto DrawDroplet(const Vector2 c, const float r, const Color col, const bool filled) -> void
	{
		const Vector2 tip{.x = c.x, .y = c.y - r * 1.9f};
		const Vector2 baseL{.x = c.x - r * 0.95f, .y = c.y - r * 0.5f};
		const Vector2 baseR{.x = c.x + r * 0.95f, .y = c.y - r * 0.5f};
		if (filled)
		{
			DrawCircleV(c, r, col);
			DrawTriangle(tip, baseL, baseR, col);
		}
		else
		{
			DrawCircleLines(static_cast<int>(c.x), static_cast<int>(c.y), r, col);
			DrawTriangleLines(tip, baseL, baseR, col);
		}
	}

	// Small sprout icon for the score indicator
	auto DrawSproutIcon(const Vector2 c, const float scale, const Color col) -> void
	{
		DrawRectangle(static_cast<int>(c.x - 1.f * scale), static_cast<int>(c.y), static_cast<int>(2.f * scale),
					static_cast<int>(10.f * scale), Color{.r = 90, .g = 70, .b = 40, .a = 255});
		DrawCircleV(Vector2{.x = c.x - 4.f * scale, .y = c.y - 2.f * scale}, 5.f * scale, col);
		DrawCircleV(Vector2{.x = c.x + 4.f * scale, .y = c.y - 4.f * scale}, 4.f * scale, col);
	}
}

auto Renderer::Fade(Color c, const float a) -> Color
{
	c.a = static_cast<unsigned char>(std::min(1.f, std::max(0.f, a)) * 255.f);
	return c;
}

auto Renderer::PlantBodyColor(const Plant& pl) -> Color
{
	if (!pl.revealed) return Color{.r = 90, .g = 90, .b = 90, .a = 255};
	if (pl.done && !pl.dying) return Color{.r = 30, .g = 165, .b = 30, .a = 255};
	switch (pl.kind)
	{
		case PK_NORMAL: return Color{.r = 50, .g = 155, .b = 50, .a = 255};
		case PK_WILTED: return Color{.r = 130, .g = 88, .b = 40, .a = 255};
		case PK_WEED: return Color{.r = 75, .g = 115, .b = 40, .a = 255};
		case PK_BONUS: return Color{.r = 210, .g = 180, .b = 0, .a = 255};
		case PK_SABOTAGE: return Color{.r = 185, .g = 35, .b = 35, .a = 255};
	}
	return GREEN;
}

auto Renderer::DrawBG() -> void
{
	// Sky
	DrawRectangleGradientV(0, 0, SW, SH / 2,
							Color{.r = 118, .g = 190, .b = 228, .a = 255},
							Color{.r = 200, .g = 232, .b = 255, .a = 255});
	// Ground
	DrawRectangleGradientV(0, SH / 2, SW, SH / 2,
							Color{.r = 34, .g = 115, .b = 34, .a = 255}, Color{.r = 18, .g = 70, .b = 18, .a = 255});
	// Grass tuft detail
	for (int x = 0; x < SW; x += 9)
	{
		const int h = 4 + (x * 7 + 3) % 5;
		DrawRectangle(x, SH / 2 - h, 3, h, Color{.r = 50, .g = 165, .b = 50, .a = 255});
	}
	// Soil path
	DrawRectangleRounded({
							.x = 55.f, .y = static_cast<float>(GY) - 28.f, .width = static_cast<float>(SW - 110),
							.height = 95.f,
						},
						0.15f, 8, Color{.r = 140, .g = 106, .b = 68, .a = 200});
	// Path edge highlights
	DrawLineEx({.x = 55.f, .y = static_cast<float>(GY) - 28.f},
				{.x = static_cast<float>(SW - 55), .y = static_cast<float>(GY) - 28.f},
				2.f, Color{.r = 170, .g = 130, .b = 90, .a = 180});
	DrawLineEx({.x = 55.f, .y = static_cast<float>(GY) + 67.f},
				{.x = static_cast<float>(SW - 55), .y = static_cast<float>(GY) + 67.f},
				2.f, Color{.r = 100, .g = 72, .b = 40, .a = 180});
}

auto Renderer::DrawRiver(const int side) -> void
{
	const float cx = Game::RiverX(side);
	const float bx = cx - 30.f;
	constexpr float by = static_cast<float>(GY) - 48.f;
	// Well / trough body
	DrawRectangleRounded({.x = bx, .y = by, .width = 60.f, .height = 108.f}, 0.25f, 8,
						Color{.r = 100, .g = 74, .b = 48, .a = 255});
	// Water inside
	DrawRectangleRounded({.x = bx + 5.f, .y = by + 6.f, .width = 50.f, .height = 85.f}, 0.25f, 8,
						Color{.r = 55, .g = 145, .b = 225, .a = 255});
	// Ripple lines
	for (int j = 0; j < 3; j++)
	{
		DrawLineEx({.x = bx + 10.f, .y = by + 25.f + j * 18.f},
					{.x = bx + 50.f, .y = by + 25.f + j * 18.f},
					1.5f, Color{.r = 180, .g = 220, .b = 255, .a = 160});
	}

	// Small animated puddle at the base instead of a text label
	const float phase = static_cast<float>(GetTime()) * 1.6f + static_cast<float>(side) * MY_PI;
	const float pulse = (sinf(phase) + 1.f) * 0.5f; // 0..1
	const Vector2 pc{.x = cx, .y = by + 100.f};
	DrawEllipse(static_cast<int>(pc.x), static_cast<int>(pc.y), 22.f, 7.f,
				Color{.r = 40, .g = 100, .b = 150, .a = 140});
	DrawEllipseLines(static_cast<int>(pc.x), static_cast<int>(pc.y), 14.f + pulse * 6.f, 4.5f + pulse * 2.f,
					Color{.r = 170, .g = 220, .b = 255, .a = static_cast<unsigned char>(150.f - pulse * 90.f)});
	DrawEllipse(static_cast<int>(pc.x - 6.f), static_cast<int>(pc.y - 2.f), 5.f, 2.f,
				Color{.r = 210, .g = 235, .b = 255, .a = 120});
}

auto Renderer::DrawPlant(const Game& game, const int i) -> void
{
	const Plant& pl = game.Plants()[i];
	const float x = game.PlantX(i);
	constexpr auto y = static_cast<float>(GY);
	const float wb = sinf(pl.wobble * MY_PI * 6.f) * 7.f * pl.wobble;

	// Death anim (weed)
	if (pl.dying)
	{
		const float a = 1.f - pl.dieT;
		const float r = 20.f * (1.f - pl.dieT * 0.4f);
		DrawCircle(static_cast<int>(x), static_cast<int>(y - 22), r,
					Fade(Color{.r = 200, .g = 55, .b = 55, .a = 255}, a));
		Text("X", x - 7, y - 34, 18, Fade(RED, a));
		return;
	}

	// Stem
	Color sc = pl.done ? Color{.r = 22, .g = 130, .b = 22, .a = 255} : Color{.r = 78, .g = 80, .b = 38, .a = 220};
	if (!pl.revealed) sc = Color{.r = 68, .g = 68, .b = 68, .a = 200};
	DrawRectangle(static_cast<int>(x - 3), static_cast<int>(y - 40), 6, 40, sc);

	// Bloom
	const float sc2 = 1.f + (pl.done ? pl.wobble * 0.28f : 0.f);
	const int r = static_cast<int>(22.f * sc2);
	const Color bc = PlantBodyColor(pl);
	DrawCircle(static_cast<int>(x + wb), static_cast<int>(y - 40), static_cast<float>(r), bc);

	// Inner details per kind
	if (pl.revealed)
	{
		switch (pl.kind)
		{
			case PK_BONUS:
				DrawCircle(static_cast<int>(x + wb), static_cast<int>(y - 40), r / 2, YELLOW);
				Text("$", x + wb - 6, y - 50, 16, DARKBROWN);
				break;
			case PK_WEED:
				Text("X", x + wb - 7, y - 50, 18, Color{.r = 220, .g = 50, .b = 50, .a = 255});
				break;
			case PK_SABOTAGE:
				DrawCircle(static_cast<int>(x + wb), static_cast<int>(y - 40), r / 3,
							Color{.r = 230, .g = 50, .b = 50, .a = 220});
				break;
			case PK_WILTED:
				DrawLine(static_cast<int>(x + wb - 9), static_cast<int>(y - 40), static_cast<int>(x + wb + 9),
						static_cast<int>(y - 40),
						Color{.r = 160, .g = 100, .b = 50, .a = 200});
				break;
			default: break;
		}

		// Water-cost badge
		if (!pl.done)
		{
			char buf[8];
			snprintf(buf, sizeof(buf), "%d", pl.need - pl.given);
			const int bw = TextW(buf, 16);
			DrawRectangle(static_cast<int>(x + wb - static_cast<float>(bw) / 2.f - 3), static_cast<int>(y - 72), bw + 6,
						20,
						Color{.r = 0, .g = 0, .b = 0, .a = 180});
			Text(buf, x + wb - static_cast<float>(bw) / 2.f, y - 71, 16, WHITE);
		}
		else
		{
			Text("OK", x - 10, y - 70, 15, Color{.r = 80, .g = 220, .b = 80, .a = 255});
		}
	}
	else
	{
		// Hidden
		Text("?", x - 6, y - 54, 22, WHITE);
	}
}

auto Renderer::DrawCharacter(const Player& p) -> void
{
	const float x = p.vx;
	const float y = p.vy;
	// Shadow
	DrawEllipse(static_cast<int>(x), GY + 10, 18, 5, Color{.r = 0, .g = 0, .b = 0, .a = 65});
	// Body
	DrawRectangleRounded({.x = x - 13.f, .y = y - 46.f, .width = 26.f, .height = 42.f}, 0.4f, 8, p.col);
	// Head
	DrawCircle(static_cast<int>(x), static_cast<int>(y - 57), 17, Color{.r = 255, .g = 218, .b = 178, .a = 255});
	// Hat brim / crown
	DrawRectangle(static_cast<int>(x - 15), static_cast<int>(y - 77), 30, 8, p.col);
	DrawRectangle(static_cast<int>(x - 10), static_cast<int>(y - 90), 20, 14, p.col);
	// Eyes
	DrawCircle(static_cast<int>(x - 5), static_cast<int>(y - 59), 3, BLACK);
	DrawCircle(static_cast<int>(x + 5), static_cast<int>(y - 59), 3, BLACK);
	// Mouth expression
	if (p.watering)
		Text(":D", x - 11, y - 57, 13, Color{.r = 30, .g = 30, .b = 30, .a = 200});
	// Watering-can animation
	if (p.watering)
	{
		const float cx2 = x + (p.side == 0 ? 14.f : -14.f);
		const float cy = y - 36.f + sinf(p.waterT * MY_PI * 3.2f) * 6.f;
		DrawRectangleRounded({.x = cx2 - 2.f, .y = cy, .width = 20.f, .height = 13.f}, 0.3f, 5,
							Color{.r = 160, .g = 110, .b = 55, .a = 255});
		const float dx0 = (p.side == 0) ? cx2 + 18.f : cx2 - 6.f;
		const float step = (p.side == 0) ? 6.f : -6.f;
		for (int j = 0; j < 4; j++)
		{
			DrawCircle(static_cast<int>(dx0 + step * j), static_cast<int>(cy + j * 5), 3,
						Color{.r = 100, .g = 175, .b = 255, .a = 200});
		}
	}
	// Water-level bar above head
	constexpr int bw = 42, bh = 8;
	const float bx = x - bw / 2.f, by = y - 110.f;
	DrawRectangle(static_cast<int>(bx), static_cast<int>(by), bw, bh, Color{.r = 30, .g = 30, .b = 30, .a = 200});
	const float ratio = p.maxWater > 0 ? static_cast<float>(p.water) / static_cast<float>(p.maxWater) : 0.f;
	const Color fc = ratio < 0.3f ? RED : ratio < 0.6f ? ORANGE : SKYBLUE;
	const int fillW = static_cast<int>(bw * std::min(ratio, 1.5f)); // allow overflow for BigCan
	DrawRectangle(static_cast<int>(bx), static_cast<int>(by), fillW, bh, fc);
	DrawRectangleLinesEx({.x = bx, .y = by, .width = static_cast<float>(bw), .height = static_cast<float>(bh)}, 1.f,
						WHITE);
	// Name
	const int tw = TextW(p.name.c_str(), 14);
	Text(p.name.c_str(), x - static_cast<float>(tw) / 2.f, by - 20, 14, WHITE);
}

auto Renderer::DrawHUD(const Game& game) -> void
{
	const Player& p1 = game.P1();
	const Player& p2 = game.P2();
	const bool vsCpu = game.Mode() == GM_PVC;

	constexpr float PANEL_W = 228.f;
	constexpr float PANEL_H = 124.f;
	constexpr float panelY = static_cast<float>(SH) - 134.f;

	auto drawPanel = [&](const Player& p, const float px)
	{
		const float tx = px + 10.f;
		DrawRectangleRounded({.x = px, .y = panelY, .width = PANEL_W, .height = PANEL_H}, 0.14f, 8,
							Color{.r = 0, .g = 0, .b = 0, .a = 175});
		DrawRectangleLinesEx({.x = px, .y = panelY, .width = PANEL_W, .height = PANEL_H}, 1.5f, p.col);

		Text(p.name.c_str(), tx, panelY + 9.f, 18, p.col);

		// Score — sprout icon + number instead of "Score: N"
		char buf[80];
		snprintf(buf, sizeof(buf), "%d", p.score);
		DrawSproutIcon(Vector2{.x = tx + 8.f, .y = panelY + 42.f}, 1.f, Color{.r = 210, .g = 180, .b = 0, .a = 255});
		Text(buf, tx + 22.f, panelY + 32.f, 20, WHITE);

		// row of droplets
		const bool overcharged = p.water > p.maxWater;
		const Color fullCol = overcharged
								? Color{.r = 255, .g = 210, .b = 60, .a = 255}
								: Color{.r = 90, .g = 180, .b = 255, .a = 255};
		for (int i = 0; i < p.maxWater; i++)
		{
			const Vector2 dc{.x = tx + 8.f + static_cast<float>(i) * 17.f, .y = panelY + 68.f};
			const bool filled = i < p.water;
			DrawDroplet(dc, 7.f, filled ? fullCol : Color{.r = 110, .g = 110, .b = 110, .a = 150}, filled);
		}

		snprintf(buf, sizeof(buf), "Refills: %d  (-%d pts)", p.refills, p.refills * 5);
		Text(buf, tx, panelY + 90.f, 14, ORANGE);

		if (p.pu != PU_NONE)
			snprintf(buf, sizeof(buf), "PU: %s x%d", PUName(p.pu), p.puCharges);
		else
			snprintf(buf, sizeof(buf), "PU: none");
		Text(buf, tx, panelY + 107.f, 13, PUColor(p.pu));
	};

	drawPanel(p1, 8.f);
	drawPanel(p2, static_cast<float>(SW) - 236.f);

	if (!vsCpu)
	{
		const float limit = TIME_LIMIT[game.Difficulty()];
		const float remaining = std::max(0.f, limit - game.Time());
		const bool urgent = remaining <= limit * 0.3f && remaining > 0.f;
		const float pulse = urgent ? (sinf(game.Time() * 9.f) + 1.f) * 0.5f : 0.f;
		const Color timerBg = urgent
								? Color{.r = static_cast<unsigned char>(90.f + pulse * 60.f), .g = 20, .b = 20, .a = 190}
								: Color{.r = 0, .g = 0, .b = 0, .a = 175};
		DrawRectangleRounded({.x = static_cast<float>(SW / 2 - 62), .y = 8.f, .width = 124.f, .height = 44.f}, 0.3f,
							8, timerBg);
		char buf[80];
		const int m = static_cast<int>(remaining) / 60, s = static_cast<int>(remaining) % 60;
		snprintf(buf, sizeof(buf), "%02d:%02d", m, s);
		const Color timerCol = urgent ? Color{.r = 255, .g = 90, .b = 90, .a = 255} : WHITE;
		Text(buf, static_cast<float>(SW / 2 - TextW(buf, 28) / 2), 14, 28, timerCol);
	}

	constexpr float BANNER_SHOW = 6.f;
	if (const float t = game.Time(); t < BANNER_SHOW)
	{
		constexpr float BANNER_FADE = 1.5f;
		const float remain = BANNER_SHOW - t;
		const float a = remain < BANNER_FADE ? remain / BANNER_FADE : 1.f;

		char l1[96];
		snprintf(l1, sizeof(l1), "P1 (Blue):  A/D = Move   S = Water   Q = Power-up");
		char l2[96];
		if (vsCpu)
		{
			const char* diffName[] = {"Easy", "Medium", "Hard"};
			snprintf(l2, sizeof(l2), "AI opponent - %s difficulty", diffName[game.Difficulty()]);
		}
		else
		{
			snprintf(l2, sizeof(l2), "P2 (Red):  </> = Move   Down = Water   / = Power-up");
		}

		constexpr float bannerSize = 15.f;
		const int w1 = TextW(l1, bannerSize);
		const int w2 = TextW(l2, bannerSize);
		const float boxW = static_cast<float>(std::max(w1, w2)) + 32.f;
		constexpr float boxY = 58.f;
		constexpr float boxH = 54.f;
		const float boxX = static_cast<float>(SW) / 2.f - boxW / 2.f;

		DrawRectangleRounded({.x = boxX, .y = boxY, .width = boxW, .height = boxH}, 0.25f, 8,
							Fade(Color{.r = 0, .g = 0, .b = 0, .a = 190}, a));
		Text(l1, static_cast<float>(SW) / 2.f - static_cast<float>(w1) / 2.f, boxY + 8.f, bannerSize,
			Fade(Color{.r = 130, .g = 200, .b = 255, .a = 255}, a));
		Text(l2, static_cast<float>(SW) / 2.f - static_cast<float>(w2) / 2.f, boxY + 30.f, bannerSize,
			Fade(vsCpu
					? Color{.r = 200, .g = 200, .b = 200, .a = 255}
					: Color{.r = 255, .g = 140, .b = 140, .a = 255}, a));
	}

	constexpr auto lx = static_cast<float>(SW - 190);
	constexpr float ly = 10.f;

	DrawRectangleRounded({.x = lx - 4.f, .y = ly, .width = 186.f, .height = 112.f}, 0.15f, 6,
						Color{.r = 0, .g = 0, .b = 0, .a = 160});
	const struct
	{
		const char* label;
		Color col;
	} legend[] = {
		{.label = "Normal", .col = Color{.r = 50, .g = 155, .b = 50, .a = 255}},
		{.label = "Wilted 2x", .col = Color{.r = 130, .g = 88, .b = 40, .a = 255}},
		{.label = "Weed drain", .col = Color{.r = 75, .g = 115, .b = 40, .a = 255}},
		{.label = "Bonus+PU", .col = Color{.r = 210, .g = 180, .b = 0, .a = 255}},
		{.label = "Sabotage", .col = Color{.r = 185, .g = 35, .b = 35, .a = 255}},
	};
	for (int i = 0; i < 5; i++)
	{
		DrawCircle(static_cast<int>(lx + 8), static_cast<int>(ly + 12 + i * 19), 7, legend[i].col);
		Text(legend[i].label, lx + 20, ly + 5 + i * 19, 13, WHITE);
	}
}

auto Renderer::DrawMessages(const Game& game) -> void
{
	const auto& msgs = game.Messages();
	constexpr float sy = static_cast<float>(SH) / 2.f - 130.f;
	for (int i = 0; std::cmp_less(i, msgs.size()) && i < 7; i++)
	{
		const auto& [text, timer, col] = msgs[i];
		const float a = timer < 0.45f ? timer / 0.45f : 1.f;
		const Color c = Fade(col, a * 0.92f);
		const int tw = TextW(text.c_str(), 16);
		DrawRectangle(SW / 2 - tw / 2 - 6, static_cast<int>(sy + i * 22), tw + 12, 20,
					Color{.r = 0, .g = 0, .b = 0, .a = static_cast<unsigned char>(a * 140)});
		Text(text.c_str(), static_cast<float>(SW / 2 - tw / 2), sy + i * 22, 16, c);
	}
}

auto Renderer::DrawFX(const Game& game) -> void
{
	for (auto& f : game.FX())
	{
		const float a = f.life / f.maxLife;
		DrawCircle(static_cast<int>(f.pos.x), static_cast<int>(f.pos.y), f.radius * a, Fade(f.col, a));
	}
}

void Renderer::RenderMenu(const Game& game)
{
	DrawRectangleGradientV(0, 0, SW, SH,
							Color{.r = 12, .g = 55, .b = 12, .a = 255}, Color{.r = 4, .g = 32, .b = 4, .a = 255});
	// Scrolling garden silhouette — subtle background texture, not meant
	// to compete with foreground text/buttons for attention.
	for (int x = -static_cast<int>(std::fmod(game.BgOff(), 90.f)); x < SW + 90; x += 90)
	{
		DrawCircle(x, SH / 2 + 55, 22, Color{.r = 22, .g = 110, .b = 22, .a = 90});
		DrawCircle(x + 45, SH / 2 + 50, 18, Color{.r = 28, .g = 130, .b = 28, .a = 70});
		DrawRectangle(x - 3, SH / 2 + 55, 6, 55, Color{.r = 55, .g = 88, .b = 38, .a = 90});
		DrawRectangle(x + 45 - 3, SH / 2 + 50, 6, 50, Color{.r = 55, .g = 88, .b = 38, .a = 70});
	}

	// Title
	const char* T = "Watering Plants II";
	const char* S = "The Garden Duel";
	Text(T, static_cast<float>(SW / 2 - TextW(T, 58) / 2), 90, 58, Color{.r = 80, .g = 230, .b = 80, .a = 255});
	Text(S, static_cast<float>(SW / 2 - TextW(S, 24) / 2), 158, 24, Color{.r = 160, .g = 255, .b = 160, .a = 200});

	// Mode toggle
	const bool vsCpu = game.Mode() == GM_PVC;
	const std::string modeLine = std::string("MODE:  ") + (vsCpu ? "SINGLE PLAYER" : "MULTI PLAYER (LOCAL)") +
		"   [TAB to switch]";
	Text(modeLine.c_str(), static_cast<float>(SW / 2 - TextW(modeLine.c_str(), 20) / 2), 192, 20,
		vsCpu ? Color{.r = 255, .g = 170, .b = 60, .a = 255} : SKYBLUE);

	// Difficulty buttons
	constexpr Color oc[] = {
		Color{.r = 60, .g = 200, .b = 60, .a = 255}, YELLOW, Color{.r = 220, .g = 70, .b = 70, .a = 255},
	};

	Text("Select Difficulty:",
		static_cast<float>(SW / 2 - TextW("Select Difficulty:", 22) / 2), 228, 22, WHITE);

	for (int i = 0; i < 3; i++)
	{
		const char* opts[] = {
			"Easy    (8 plants   |  can = 10)",
			"Medium  (10 plants  |  can = 8 )",
			"Hard    (12 plants  |  can = 6 )",
		};
		const float oy = 265.f + i * 66.f;
		const bool sel = (game.MenuSel() == i);
		DrawRectangleRounded({.x = static_cast<float>(SW / 2 - 210), .y = oy, .width = 420.f, .height = 52.f}, 0.3f, 8,
							sel
								? Color{.r = 35, .g = 95, .b = 35, .a = 250}
								: Color{.r = 14, .g = 44, .b = 14, .a = 235});
		if (sel)
		{
			DrawRectangleLinesEx({.x = static_cast<float>(SW / 2 - 210), .y = oy, .width = 420.f, .height = 52.f}, 2.f,
								oc[i]);
		}
		Text(opts[i],
			static_cast<float>(SW / 2 - TextW(opts[i], 18) / 2), oy + 16, 18,
			sel ? oc[i] : Color{.r = 180, .g = 180, .b = 180, .a = 255});
	}

	Text("ENTER / SPACE  to Start",
		static_cast<float>(SW / 2 - TextW("ENTER / SPACE  to Start", 20) / 2),
		474, 20, Color{.r = 200, .g = 200, .b = 200, .a = 210});

	constexpr auto bx = static_cast<float>(SW / 2 - 310);
	// Controls box
	constexpr float by = 510.f;
	DrawRectangleRounded({.x = bx, .y = by, .width = 620.f, .height = 200.f}, 0.18f, 8,
						Color{.r = 0, .g = 0, .b = 0, .a = 215});

	auto row = [&](const char* t, const Color c, const float y2) -> auto
	{
		Text(t, static_cast<float>(SW / 2 - TextW(t, 20) / 2), y2, 20, c);
	};
	row("P1 (Blue): A / D = Move    S = Water / Refill    Q = Use Power-up",
		Color{.r = 100, .g = 180, .b = 255, .a = 255}, by + 24);
	if (vsCpu)
	{
		row("CPU (Red): Automated opponent!", Color{.r = 255, .g = 140, .b = 140, .a = 255}, by + 60);
	}
	else
	{
		row("P2 (Red):  Left/Right      Down = Water/Refill   / = Use Power-up",
			Color{.r = 255, .g = 120, .b = 120, .a = 255}, by + 60);
	}
	DrawLineEx(Vector2{.x = bx + 30.f, .y = by + 96.f}, Vector2{.x = bx + 590.f, .y = by + 96.f}, 2.f,
				Color{.r = 80, .g = 80, .b = 80, .a = 200});
	row("Bonus plants give power-ups:  Big Can  |  Skip  |  Sabotage", GOLD, by + 108);
	row("Wilted = 2x water needed  |  Weed = drains ALL your water!", ORANGE, by + 136);
	row("Sabotage = super-thirsty!  |  Refill costs -5 points", Color{.r = 255, .g = 100, .b = 100, .a = 255}, by + 164);
}

auto Renderer::RenderPlay(const Game& game) -> void
{
	DrawBG();
	DrawRiver(0);
	DrawRiver(1);
	for (int i = 0; i < game.PlantCount(); i++) DrawPlant(game, i);
	DrawFX(game);
	DrawCharacter(game.P1());
	DrawCharacter(game.P2());
	DrawHUD(game);
	DrawMessages(game);

	// Fade-out overlay when game ends
	if (game.EndTimer() >= 0.f)
	{
		const float a = std::min(game.EndTimer() / 2.2f, 1.f);
		DrawRectangle(0, 0, SW, SH, Color{.r = 0, .g = 0, .b = 0, .a = static_cast<unsigned char>(a * 170)});
		const bool timedOut = game.Mode() == GM_PVC ? false : game.Time() >= TIME_LIMIT[game.Difficulty()];
		const char* tt = timedOut ? "Time's Up!" : "Garden Complete!";
		Text(tt, static_cast<float>(SW / 2 - TextW(tt, 44) / 2), SH / 2 - 22, 44, Fade(GREEN, a));
	}
}

auto Renderer::RenderGameOver(const Game& game) -> void
{
	DrawBG();
	for (int i = 0; i < game.PlantCount(); i++) DrawPlant(game, i);
	DrawFX(game);
	DrawCharacter(game.P1());
	DrawCharacter(game.P2());

	DrawRectangle(0, 0, SW, SH, Color{.r = 0, .g = 0, .b = 0, .a = 170});
	DrawRectangleRounded({
							.x = static_cast<float>(SW / 2 - 270), .y = static_cast<float>(SH / 2 - 220),
							.width = 540.f, .height = 440.f,
						},
						0.1f, 16, Color{.r = 18, .g = 48, .b = 18, .a = 248});
	DrawRectangleLinesEx({
							.x = static_cast<float>(SW / 2 - 270), .y = static_cast<float>(SH / 2 - 220),
							.width = 540.f, .height = 440.f,
						},
						2.f, GREEN);

	// Title
	const bool timedOut = game.Mode() == GM_PVC ? false : game.Time() >= TIME_LIMIT[game.Difficulty()];
	const char* T = timedOut ? "Time's Up!" : "Garden Complete!";
	Text(T, static_cast<float>(SW / 2 - TextW(T, 38) / 2), SH / 2 - 205, 38, GREEN);

	const Player& p1 = game.P1();
	const Player& p2 = game.P2();

	// Winner
	std::string wt;
	Color wc;
	if (game.Winner() == -1)
	{
		wt = "TIE GAME!";
		wc = YELLOW;
	}
	else if (game.Winner() == 0)
	{
		wt = p1.name + " WINS!";
		wc = p1.col;
	}
	else
	{
		wt = p2.name + " WINS!";
		wc = p2.col;
	}
	Text(wt.c_str(), static_cast<float>(SW / 2 - TextW(wt.c_str(), 34) / 2), SH / 2 - 158, 34, wc);

	// Score table
	DrawLine(SW / 2, SH / 2 - 108, SW / 2, SH / 2 - 20, Color{.r = 80, .g = 80, .b = 80, .a = 200});
	char buf[80];

	// P1
	Text(p1.name.c_str(), SW / 2 - 180, SH / 2 - 108, 20, p1.col);
	snprintf(buf, sizeof(buf), "Score:  %d", p1.score);
	Text(buf, SW / 2 - 180, SH / 2 - 80, 18, WHITE);
	snprintf(buf, sizeof(buf), "Refills: %d", p1.refills);
	Text(buf, SW / 2 - 180, SH / 2 - 55, 18, ORANGE);

	// P2
	Text(p2.name.c_str(), SW / 2 + 80, SH / 2 - 108, 20, p2.col);
	snprintf(buf, sizeof(buf), "Score:  %d", p2.score);
	Text(buf, SW / 2 + 80, SH / 2 - 80, 18, WHITE);
	snprintf(buf, sizeof(buf), "Refills: %d", p2.refills);
	Text(buf, SW / 2 + 80, SH / 2 - 55, 18, ORANGE);

	int done = 0;
	for (auto& pl : game.Plants()) if (pl.done) done++;
	snprintf(buf, sizeof(buf), "Plants handled: %d / %d", done, game.PlantCount());
	Text(buf, static_cast<float>(SW / 2 - TextW(buf, 18) / 2), SH / 2, 18, GREEN);

	// Scoring breakdown reminder
	const char* scoring = "Scoring: Normal+10  Wilted+20  Bonus+30  Sabotage+15  Refill-5";
	Text(scoring, static_cast<float>(SW / 2 - TextW(scoring, 18) / 2), SH / 2 + 30, 18,
		Color{.r = 160, .g = 160, .b = 160, .a = 255});

#ifndef PLATFORM_WEB
	const char* prompt = "ENTER = Go to the Main Menu    ESC = Quit";
#else
	const char* prompt = "ENTER = Go to the Main Menu";
#endif
	Text(prompt, static_cast<float>(SW / 2 - TextW(prompt, 18) / 2), SH / 2 + 62, 18, LIGHTGRAY);
}

auto Renderer::Render(const Game& game) -> void
{
	switch (game.State())
	{
		case GS_MENU: RenderMenu(game);
			break;
		case GS_PLAY: RenderPlay(game);
			break;
		case GS_OVER: RenderGameOver(game);
			break;
	}

#ifndef PLATFORM_WEB
	Text(TextFormat("FPS : %d", GetFPS()), 20.f, 20.f, 24, WHITE);
#endif
}
