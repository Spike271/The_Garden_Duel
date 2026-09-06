#include "../headers/Game.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace
{
	constexpr float AI_THINK_INTERVAL[3] = {0.90f, 0.35f, 0.08f};
	constexpr float AI_MOVE_FACTOR[3] = {3.00f, 1.80f, 1.00f};
}

Game::Game() = default;

float Game::PlantX(const int i) const
{
	if (m_N < 2) return (RLX + RRX) * 0.5f;
	constexpr float span = RRX - RLX - 130.f;
	return RLX + 65.f + i * (span / static_cast<float>(m_N - 1));
}

float Game::RiverX(const int side) { return static_cast<float>(side == 0 ? RLX : RRX); }

float Game::GridX(const int gp) const
{
	if (gp == -1) return RiverX(0);
	if (gp == m_N) return RiverX(1);
	return PlantX(gp);
}

bool Game::AllDone() const
{
	for (auto& p : m_Plants) if (!p.done) return false;
	return true;
}

void Game::AddMsg(const std::string& t, const Color c)
{
	m_Msgs.push_back({.text = t, .timer = 2.6f, .col = c});
	if (static_cast<int>(m_Msgs.size()) > 7) m_Msgs.erase(m_Msgs.begin());
}

void Game::Burst(const float x, const float y, const Color c, const int n)
{
	std::uniform_real_distribution aD(0.f, 2.f * MY_PI);
	std::uniform_real_distribution sD(55.f, 190.f);
	std::uniform_real_distribution rD(3.f, 8.f);
	for (int i = 0; i < n; i++)
	{
		const float a = aD(m_Rng);
		const float s = sD(m_Rng);
		Particle p{};
		p.pos = {.x = x, .y = y};
		p.vel = {.x = cosf(a) * s, .y = sinf(a) * s - 70.f};
		p.life = p.maxLife = 0.45f + static_cast<float>(m_Rng() % 100) / 260.f;
		p.radius = rD(m_Rng);
		p.col = c;
		m_FX.push_back(p);
	}
}

void Game::InitGame(const int diff)
{
	// diff 0=easy(8pl,cap10) 1=med(10pl,cap8) 2=hard(12pl,cap6)
	m_Difficulty = diff;
	m_N = 8 + diff * 2;
	const int cap = 10 - diff * 2;

	m_Plants.clear();
	std::uniform_int_distribution td(0, 9);
	std::uniform_int_distribution nd(1, cap);
	for (int i = 0; i < m_N; i++)
	{
		Plant pl;
		if (const int r = td(m_Rng); r < 5) pl.kind = PK_NORMAL;
		else if (r < 7) pl.kind = PK_WILTED;
		else if (r < 8) pl.kind = PK_WEED;
		else if (r < 9) pl.kind = PK_BONUS;
		else pl.kind = PK_SABOTAGE;

		pl.need = nd(m_Rng);
		switch (pl.kind)
		{
			case PK_WILTED: pl.need = std::min(pl.need + cap / 3 + 1, cap);
				break;
			case PK_WEED: pl.need = 1;
				break;
			case PK_BONUS: pl.need = std::max(1, pl.need / 2);
				break;
			case PK_SABOTAGE: pl.need = std::max(cap * 2 / 3, pl.need);
				break;
			default: break;
		}
		m_Plants.push_back(pl);
	}

	auto make = [&](const int side, const char* n, const Color c)
	{
		Player p;
		p.name = n;
		p.col = c;
		p.side = side;
		p.gridPos = side == 0 ? -1 : m_N;
		p.vx = p.tx = GridX(p.gridPos);
		p.vy = static_cast<float>(GY) - 32.f;
		p.water = p.maxWater = cap;
		p.score = 0;
		p.refills = 0;
		p.pu = PU_NONE;
		p.puCharges = 0;
		p.moveCd = p.actCd = 0.f;
		p.watering = false;
		return p;
	};
	m_P1 = make(0, "Player 1", Color{.r = 70, .g = 130, .b = 210, .a = 255});
	m_P2 = make(1, m_Mode == GM_PVC ? "CPU" : "Player 2", Color{.r = 205, .g = 70, .b = 70, .a = 255});

	// Reset AI agent
	m_Ai = AiAgent{};

	m_FX.clear();
	m_Msgs.clear();
	m_Time = 0.f;
	m_EndTimer = -1.f;
	m_Winner = -2;
	m_State = GS_PLAY;

	if (m_Mode == GM_PVC)
	{
		const char* diffName[] = {"Easy", "Medium", "Hard"};
		char buf[64];
		snprintf(buf, sizeof(buf), "Garden ready — GO!  [%s]", diffName[diff]);
		AddMsg(buf, GREEN);
	}
	else
	{
		AddMsg("Garden ready — GO!", GREEN);
	}
}

void Game::DoMove(Player& p, const int dir)
{
	if (p.moveCd > 0.f) return;
	const int np = std::max(-1, std::min(m_N, p.gridPos + dir));
	if (np == p.gridPos) return;
	p.gridPos = np;
	p.tx = GridX(np);
	p.moveCd = MOVE_CD;
	if (np >= 0 && np < m_N) m_Plants[np].revealed = true;
}

void Game::DoAction(Player& p, Player& opp)
{
	if (p.actCd > 0.f) return;

	if (const int gp = p.gridPos; (p.side == 0 && gp == -1) || (p.side == 1 && gp == m_N))
	{
		if (p.water >= p.maxWater && !(p.pu == PU_BIGCAN && p.puCharges > 0))
		{
			AddMsg(p.name + ": Can is full!", SKYBLUE);
			return;
		}
		if (p.pu == PU_BIGCAN && p.puCharges > 0)
		{
			p.water = p.maxWater * 2; // overcharge
			if (--p.puCharges <= 0) p.pu = PU_NONE;
			AddMsg(p.name + ": BIG CAN — 2x water!", SKYBLUE);
		}
		else
		{
			p.water = p.maxWater;
		}
		p.refills++;
		p.score -= 5;
		Burst(RiverX(p.side), static_cast<float>(GY), BLUE, 22);
		char buf[80];
		snprintf(buf, sizeof(buf), "%s refilled! (refills: %d, -5pts)", p.name.c_str(), p.refills);
		AddMsg(buf, p.col);
	}
	else if (gp >= 0 && gp < m_N)
	{
		Plant& pl = m_Plants[gp];
		if (!pl.revealed) pl.revealed = true;
		if (pl.done)
		{
			AddMsg(p.name + ": Already handled!", Color{.r = 130, .g = 130, .b = 130, .a = 255});
			return;
		}

		if (pl.kind == PK_WEED)
		{
			char buf[80];
			snprintf(buf, sizeof(buf), "%s: WEED! Lost %d water!", p.name.c_str(), p.water);
			AddMsg(buf, Color{.r = 220, .g = 60, .b = 60, .a = 255});
			Burst(PlantX(gp), static_cast<float>(GY), Color{.r = 220, .g = 60, .b = 60, .a = 255}, 18);
			p.score -= 5;
			p.water = 0;
			pl.done = true;
			pl.dying = true;
			p.actCd = ACT_CD;
			return;
		}

		const int cost = pl.need - pl.given;
		if (p.water < cost)
		{
			char buf[80];
			snprintf(buf, sizeof(buf), "%s: Need %d water — go refill!", p.name.c_str(), cost);
			AddMsg(buf, Color{.r = 255, .g = 160, .b = 40, .a = 255});
			return;
		}

		p.water -= cost;
		pl.given = pl.need;
		pl.done = true;
		pl.wobble = 1.0f;
		Burst(PlantX(gp), static_cast<float>(GY), Color{.r = 100, .g = 200, .b = 255, .a = 255}, 16);

		int pts = 10;
		if (pl.kind == PK_WILTED) pts = 20;
		if (pl.kind == PK_BONUS) pts = 30;
		if (pl.kind == PK_SABOTAGE) pts = 15;
		p.score += pts;
		p.watering = true;
		p.waterT = 0.f;

		char buf[80];
		snprintf(buf, sizeof(buf), "%s watered! +%d pts", p.name.c_str(), pts);
		AddMsg(buf, p.col);

		// Bonus plant grants random power-up
		if (pl.kind == PK_BONUS)
		{
			std::uniform_int_distribution pd(1, 3);
			p.pu = static_cast<PUKind>(pd(m_Rng));
			p.puCharges = 1;
			char b2[80];
			snprintf(b2, sizeof(b2), "%s earned: %s!", p.name.c_str(), PUName(p.pu));
			AddMsg(b2, GOLD);
		}
	}
	p.actCd = ACT_CD;
}

void Game::DoPowerUp(Player& p, const Player& opp)
{
	if (p.pu == PU_NONE || p.puCharges <= 0 || p.actCd > 0.f) return;
	const int gp = p.gridPos;

	if (p.pu == PU_SKIP)
	{
		if (gp >= 0 && gp < m_N && !m_Plants[gp].done)
		{
			m_Plants[gp].done = true;
			m_Plants[gp].revealed = true;
			Burst(PlantX(gp), static_cast<float>(GY), YELLOW, 12);
			p.score += 5;
			AddMsg(p.name + ": SKIP used! +5 pts", YELLOW);
			if (--p.puCharges <= 0) p.pu = PU_NONE;
			p.actCd = ACT_CD;
		}
	}
	else if (p.pu == PU_SABOTAGE)
	{
		bool found = false;
		// P1 sabotages rightmost plant (P2's approach); P2 sabotages leftmost
		if (p.side == 0)
		{
			for (int i = m_N - 1; i >= 0 && !found; i--)
			{
				if (!m_Plants[i].done && m_Plants[i].kind != PK_SABOTAGE)
				{
					m_Plants[i].kind = PK_SABOTAGE;
					m_Plants[i].need = std::min(std::max(m_Plants[i].need, opp.maxWater * 2 / 3 + 1), opp.maxWater);
					m_Plants[i].given = 0;
					m_Plants[i].revealed = false;
					Burst(PlantX(i), static_cast<float>(GY), Color{.r = 220, .g = 60, .b = 60, .a = 255}, 14);
					AddMsg(p.name + ": SABOTAGE deployed!", Color{.r = 220, .g = 60, .b = 60, .a = 255});
					found = true;
				}
			}
		}
		else
		{
			for (int i = 0; i < m_N && !found; i++)
			{
				if (!m_Plants[i].done && m_Plants[i].kind != PK_SABOTAGE)
				{
					m_Plants[i].kind = PK_SABOTAGE;
					m_Plants[i].need = std::min(std::max(m_Plants[i].need, opp.maxWater * 2 / 3 + 1), opp.maxWater);
					m_Plants[i].given = 0;
					m_Plants[i].revealed = false;
					Burst(PlantX(i), static_cast<float>(GY), Color{.r = 220, .g = 60, .b = 60, .a = 255}, 14);
					AddMsg(p.name + ": SABOTAGE deployed!", Color{.r = 220, .g = 60, .b = 60, .a = 255});
					found = true;
				}
			}
		}
		if (found && --p.puCharges <= 0) p.pu = PU_NONE;
		p.actCd = ACT_CD;
	}
}

int Game::AiCalcNextTarget() const
{
	for (int i = m_N - 1; i >= 0; --i)
		if (!m_Plants[i].done) return i;
	return -1; // all done
}

int Game::AiCalcWaterCost(const int target) const
{
	int total = 0;
	const int lo = std::min(m_P2.gridPos, target);
	const int hi = std::max(m_P2.gridPos, target);
	for (int i = lo; i <= hi; ++i)
	{
		if (i >= 0 && i < m_N && !m_Plants[i].done)
		{
			total += m_Plants[i].need - m_Plants[i].given;
		}
	}
	return total;
}

void Game::ThinkAI()
{
	m_Ai.thinkCd = AI_THINK_INTERVAL[m_Difficulty];

	const int nextPlant = AiCalcNextTarget();
	if (nextPlant == -1) return; // all done

	if (const int waterNeeded = AiCalcWaterCost(nextPlant); m_P2.water < waterNeeded)
	{
		// Can't reach + water the target with what's in the tank
		m_Ai.state = AiAgent::State::SeekRiver;
	}
	else
	{
		m_Ai.state = AiAgent::State::SeekPlant;
		m_Ai.target = nextPlant;
	}
}

void Game::AiMove(Player& p, const int dir)
{
	const float prev = p.moveCd;
	DoMove(p, dir);
	// Scale the cooldown DoMove just started by the difficulty's speed factor.
	if (p.moveCd > prev) p.moveCd *= AI_MOVE_FACTOR[m_Difficulty];
}

void Game::UpdateAI(const float dt)
{
	m_Ai.thinkCd -= dt;
	if (m_Ai.thinkCd <= 0.f) ThinkAI();

	if (m_P2.pu == PU_SABOTAGE && m_P2.puCharges > 0) DoPowerUp(m_P2, m_P1);

	// Don't act or move until the sprite has visually arrived at the grid cell.
	const float arriveThresh = 6.f;
	const bool arrived = std::abs(m_P2.vx - m_P2.tx) < arriveThresh;

	if (m_Ai.state == AiAgent::State::SeekRiver)
	{
		if (arrived && m_P2.gridPos >= 0 && m_P2.gridPos < m_N)
		{
			if (const Plant& pl = m_Plants[m_P2.gridPos]; !pl.done && (pl.need - pl.given) <= m_P2.water)
			{
				DoAction(m_P2, m_P1);
				m_Ai.thinkCd = 0.f;
				return;
			}
		}

		if (m_P2.gridPos < m_N)
		{
			if (arrived) AiMove(m_P2, +1); // walk back toward the right river
		}
		else
		{
			if (arrived && m_P2.water < m_P2.maxWater) DoAction(m_P2, m_P1); // refill
			m_Ai.thinkCd = 0.f; // re-plan next frame
		}
	}
	else // AiAgent::State::SeekPlant
	{
		if (m_Ai.target < 0 || m_Ai.target >= m_N || m_Plants[m_Ai.target].done)
		{
			m_Ai.thinkCd = 0.f; // target invalidated — re-plan
			return;
		}

		if (m_P2.gridPos < m_Ai.target) { if (arrived) AiMove(m_P2, +1); }
		else if (m_P2.gridPos > m_Ai.target) { if (arrived) AiMove(m_P2, -1); }
		else if (arrived)
		{
			if (const Plant& pl = m_Plants[m_Ai.target]; !pl.done && m_P2.water < pl.need - pl.given && m_P2.pu == PU_SKIP && m_P2.puCharges > 0)
				DoPowerUp(m_P2, m_P1);
			else
				DoAction(m_P2, m_P1);
		}
	}
}

void Game::UpdatePlayer(Player& p, const float dt)
{
	if (p.moveCd > 0.f) p.moveCd -= dt;
	if (p.actCd > 0.f) p.actCd -= dt;
	p.vx += (p.tx - p.vx) * dt * 14.f;
	if (p.watering)
	{
		p.waterT += dt;
		if (p.waterT > 0.55f) p.watering = false;
	}
}

void Game::UpdateMenu(const float dt)
{
	m_BgOff += dt * 28.f;
	if (IsKeyPressed(KEY_TAB)) m_Mode = (m_Mode == GM_PVP) ? GM_PVC : GM_PVP;
	if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) m_MenuSel = (m_MenuSel + 2) % 3;
	if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) m_MenuSel = (m_MenuSel + 1) % 3;
	if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) InitGame(m_MenuSel);
}

void Game::UpdatePlay(const float dt)
{
	m_Time += dt;

	// Freeze all player/AI input once the match has ended.
	if (m_EndTimer < 0.f)
	{
		// P1: WASD + Q
		if (IsKeyDown(KEY_A)) DoMove(m_P1, -1);
		if (IsKeyDown(KEY_D)) DoMove(m_P1, +1);
		if (IsKeyPressed(KEY_S)) DoAction(m_P1, m_P2);
		if (IsKeyPressed(KEY_Q)) DoPowerUp(m_P1, m_P2);

		// P2: Arrows + / — or the AI, in single-player mode
		if (m_Mode == GM_PVC)
		{
			UpdateAI(dt);
		}
		else
		{
			if (IsKeyDown(KEY_LEFT)) DoMove(m_P2, -1);
			if (IsKeyDown(KEY_RIGHT)) DoMove(m_P2, +1);
			if (IsKeyPressed(KEY_DOWN)) DoAction(m_P2, m_P1);
			if (IsKeyPressed(KEY_SLASH) || IsKeyPressed(KEY_KP_DIVIDE)) DoPowerUp(m_P2, m_P1);
		}

		UpdatePlayer(m_P1, dt);
		UpdatePlayer(m_P2, dt);
	}

	// Plants
	for (auto& pl : m_Plants)
	{
		if (pl.wobble > 0.f) pl.wobble = std::max(0.f, pl.wobble - dt * 2.4f);
		if (pl.dying) pl.dieT = std::min(1.f, pl.dieT + dt * 1.8f);
	}

	// Particles
	for (auto& f : m_FX)
	{
		f.pos.x += f.vel.x * dt;
		f.pos.y += f.vel.y * dt;
		f.vel.y += 260.f * dt;
		f.life -= dt;
	}
	m_FX.erase(std::ranges::remove_if(m_FX,
									[](const Particle& f) { return f.life <= 0.f; }).begin(), m_FX.end());

	// Messages
	for (auto& m : m_Msgs) m.timer -= dt;
	m_Msgs.erase(std::ranges::remove_if(m_Msgs,
										[](const Message& m) { return m.timer <= 0.f; }).begin(), m_Msgs.end());

	if (AllDone() || (m_Mode != GM_PVC && m_Time >= TIME_LIMIT[m_Difficulty]))
	{
		m_EndTimer = std::max(m_EndTimer, 0.f);
		m_EndTimer += dt;
		if (m_Winner == -2)
		{
			if (m_P1.score > m_P2.score) m_Winner = 0;
			else if (m_P2.score > m_P1.score) m_Winner = 1;
			else m_Winner = -1;
		}
		if (m_EndTimer > 2.2f) m_State = GS_OVER;
	}
}

void Game::UpdateOver(const float dt)
{
	(void)dt;

	if (IsKeyPressed(KEY_ENTER))
	{
		m_State = GS_MENU;
		m_Winner = -2;
	}
#if !defined(PLATFORM_WEB)
	if (IsKeyPressed(KEY_ESCAPE)) m_QuitRequested = true;
#endif
}

void Game::OnUpdate(const float dt)
{
	switch (m_State)
	{
		case GS_MENU: UpdateMenu(dt);
			break;
		case GS_PLAY: UpdatePlay(dt);
			break;
		case GS_OVER: UpdateOver(dt);
			break;
	}
}
