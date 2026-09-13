#pragma once
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
using namespace std;
using namespace sf;


class AnimationController {
private:
	int frameWidth;
	int frameHeight;
	int currentFrame;
	int totalFrames;
	int row;
	float frameDuration;
	float frameTimer;
	bool loop;

public:
	static const bool ANIMATIONS_ENABLED = false;

	AnimationController()
		: frameWidth(64), frameHeight(64), currentFrame(0), totalFrames(1),
		row(0), frameDuration(0.1f), frameTimer(0.0f), loop(true) {
	}

	void configure(int fw, int fh, int frames, int r, float fps, bool shouldLoop = true) {
		frameWidth = (fw > 0) ? fw : 64;
		frameHeight = (fh > 0) ? fh : 64;
		totalFrames = (frames > 0) ? frames : 1;
		row = (r >= 0) ? r : 0;
		frameDuration = (fps > 0.f) ? 1.f / fps : 0.1f;
		loop = shouldLoop;
		currentFrame = 0;
		frameTimer = 0.f;
	}

	void update(float dt, Sprite& sprite) {
		(void)dt;
		(void)sprite;
		currentFrame = 0;
		frameTimer = 0.0f;
	}

	void switchTo(int newRow, int frames, float fps, bool shouldLoop = true) {
		(void)frames;
		(void)fps;
		(void)shouldLoop;
		row = (newRow >= 0) ? newRow : 0;
		currentFrame = 0;
		frameTimer = 0.0f;
	}

	bool isFinished() const { return true; }
	int getCurrentFrame() const { return 0; }
	int getRow() const { return row; }
	int getFrameWidth() const { return frameWidth; }
	int getFrameHeight() const { return frameHeight; }
	void reset() { currentFrame = 0; frameTimer = 0.f; }

	~AnimationController() {}
};


class Environment {
private:
	bool isRaining;
	float rainTimer;
	float rainInterval;
	float rainDuration;

public:
	Environment() : isRaining(false), rainTimer(0.f), rainInterval(45.f), rainDuration(15.f) {}

	bool getIsRaining() const { return isRaining; }
	float getRainTimer() const { return rainTimer; }
	void setRainInterval(float t) { rainInterval = t; }
	void setRainDuration(float d) { rainDuration = d; }

	void update(float dt) {
		rainTimer += dt;

		if (!isRaining) {
			if (rainTimer >= rainInterval) startRain();
		}
		else {
			if (rainTimer >= rainDuration) stopRain();
		}
	}

	void startRain() {
		isRaining = true;
		rainTimer = 0.f;
		cout << "Rain started\n";
	}

	void stopRain() {
		isRaining = false;
		rainTimer = 0.f;
		cout << "Rain stopped\n";
	}

	~Environment() {}
};


class Entity {
protected:
	float posX;
	float posY;
	float width;
	float height;
	bool active;
	Sprite sprite;
	Texture texture;
	AnimationController anim;
public:
	Entity() : posX(0.0f), posY(0.0f), width(0.0f), height(0.0f), active(true) {}

	Entity(float x, float y, float w, float h) : posX(x), posY(y), width(w), height(h), active(true) {}

	float getPosX()   const { return posX; }
	float getPosY()   const { return posY; }
	float getWidth()  const { return width; }
	float getHeight() const { return height; }
	bool  isActive()  const { return active; }
	void setActive(bool a) { active = a; }

	virtual void update(float dt) = 0;
	virtual void render(RenderWindow& window, float camX, float camY) = 0;

	bool loadTexture(const char* path) {
		if (!texture.loadFromFile(path))
		{
			std::cout << "ENTITY ERROR: COULD NOT LOAD: " << path << "\n";
			return false;
		}

		sprite.setTexture(texture);
		return true;
	}

	void setPosition(float x, float y)
	{
		posX = x;
		posY = y;
		sprite.setPosition(x, y);
	}

	void setSize(float w, float h)
	{
		width = w;
		height = h;
	}

	void fitSprite() {
		if (texture.getSize().x > 0 && texture.getSize().y > 0 && width > 0 && height > 0) {
			float texW = (float)texture.getSize().x;
			float texH = (float)texture.getSize().y;
			sprite.setScale(width / texW, height / texH);
		}
	}

	bool overlaps(float ox, float oy, float ow, float oh) const
	{

		bool noOverlapX = (posX + width <= ox) || (ox + ow <= posX);
		bool noOverlapY = (posY + height <= oy) || (oy + oh <= posY);
		return !(noOverlapX || noOverlapY);
	}

	bool containsPointInsideEntity(float px, float py) const
	{
		return (px >= posX && px <= posX + width && py >= posY && py <= posY + height);
	}

	virtual ~Entity() {}
};


class DamagableEntity : public Entity {
public:
	static const int DAMAGE_HEALTHY = 0;
	static const int DAMAGE_INJURED = 1;
	static const int DAMAGE_CRITICAL = 2;
	static const int DAMAGE_DEAD = 3;
	static const float STATE_WINDOW;

protected:
	int hp;
	int maxHP;
	int damageState;
	float stateTimer;

public:
	DamagableEntity() : Entity(), hp(1), maxHP(1), damageState(DAMAGE_HEALTHY), stateTimer(0.0f) {}

	DamagableEntity(float x, float y, float w, float h, int mHP) : Entity(x, y, w, h), hp(mHP), maxHP(mHP), damageState(DAMAGE_HEALTHY), stateTimer(0.f) {}

	int  getHP() const { return hp; }
	int  getMaxHP() const { return maxHP; }
	int  getDamageState() const { return damageState; }

	bool isDead() const { return damageState == DAMAGE_DEAD; }
	bool isInjured() const {
		return (damageState == DAMAGE_INJURED || damageState == DAMAGE_CRITICAL);
	}

	virtual void update(float dt) override {
		if (damageState != DAMAGE_HEALTHY && damageState != DAMAGE_DEAD) {
			stateTimer -= dt;

			if (stateTimer < 0.f) stateTimer = 0.f;
		}
	}

	virtual void render(RenderWindow& window, float camX, float camY) = 0;

	virtual bool takeDamage(int damage) {
		if (damageState == DAMAGE_DEAD) return false;
		hp -= damage;

		if (hp < 0) hp = 0;

		if (damageState == DAMAGE_HEALTHY)
		{
			damageState = DAMAGE_INJURED;
			stateTimer = STATE_WINDOW;
		}
		else if (damageState == DAMAGE_INJURED)
		{
			damageState = DAMAGE_CRITICAL;
			stateTimer = STATE_WINDOW;
		}
		else if (damageState == DAMAGE_CRITICAL)
		{
			damageState = DAMAGE_DEAD;
			stateTimer = 0.0f;
			onDeath();
			return true;
		}

		if (hp <= 0 && damageState != DAMAGE_DEAD)
		{
			damageState = DAMAGE_DEAD;
			stateTimer = 0.0f;
			onDeath();
			return true;
		}
		return false;
	}

	void heal(int amount)
	{
		hp += amount;
		if (hp > maxHP) hp = maxHP;
		if (damageState != DAMAGE_DEAD)
			damageState = DAMAGE_HEALTHY;
	}

	int getDamageOverlayAlpha() const {
		if (damageState == DAMAGE_INJURED)  return 80;
		if (damageState == DAMAGE_CRITICAL) return 160;
		return 0;
	}

	virtual ~DamagableEntity() {}

protected:
	virtual void onDeath() { active = false; }
};
const float DamagableEntity::STATE_WINDOW = 1.0f;


class Block {
public:
	static const int BLOCK_SIZE = 64;
protected:
	float posX;
	float posY;
	bool  isDestructible;
	bool  hasWater;
	int   hp;
	bool  active;
	Sprite sprite;
	Texture texture;
	bool textureLoaded;

public:
	Block() : posX(0.f), posY(0.f), isDestructible(true), hasWater(false), hp(1), active(true), textureLoaded(false) {}

	Block(float x, float y) : posX(x), posY(y), isDestructible(true), hasWater(false), hp(1), active(true), textureLoaded(false) {}

	float getPosX() const { return posX; }
	float getPosY() const { return posY; }
	bool getIsDestructible() const { return isDestructible; }
	bool getHasWater() const { return hasWater; }
	bool isActive() const { return active; }
	int getHP() const { return hp; }

	bool loadTexture(const char* path)
	{
		textureLoaded = false;
		if (!texture.loadFromFile(path))
		{
			std::cout << "BLOCK ERROR loading texture: " << path << "\n";
			return false;
		}
		textureLoaded = true;
		sprite.setTexture(texture);
		sprite.setPosition(posX, posY);
		return true;
	}

	virtual void destroy() = 0;

	virtual void fillWithWater() { hasWater = true; }

	bool inBlastRadius(float centerX, float centerY, float radius) const
	{
		float bx = posX + BLOCK_SIZE * 0.5f;
		float by = posY + BLOCK_SIZE * 0.5f;

		float dx = bx - centerX;
		float dy = by - centerY;

		float distanceSquared = dx * dx + dy * dy;

		return distanceSquared <= (radius * radius);
	}

	void render(sf::RenderWindow& window, float camX, float camY)
	{
		if (!active) return;

		if (!textureLoaded) {
			RectangleShape fallback;
			fallback.setSize(Vector2f((float)BLOCK_SIZE, (float)BLOCK_SIZE));
			if (hasWater) fallback.setFillColor(Color(25, 90, 205, 170));
			else if (!isDestructible) fallback.setFillColor(Color(95, 95, 105, 255));
			else fallback.setFillColor(Color(120, 80, 45, 255));
			fallback.setPosition(posX - camX, posY - camY);
			window.draw(fallback);
			return;
		}

		sprite.setPosition(posX - camX, posY - camY);
		window.draw(sprite);
	}
	virtual ~Block() {}
};


class NormalBlock : public Block {
public:
	NormalBlock() : Block() {}

	NormalBlock(float x, float y, const char* texPath = "Sprites/blocks/grass_block_side.png") : Block(x, y) { loadTexture(texPath); }

	void destroy() override { active = false; }

	~NormalBlock() override {}
};

class IndestructibleBlock : public Block {
public:
	IndestructibleBlock() : Block() { isDestructible = false; }

	IndestructibleBlock(float x, float y, const char* texPath = "Sprites/blocks/stone.png") : Block(x, y) {
		isDestructible = false;
		loadTexture(texPath);
	}

	void destroy() override {}

	~IndestructibleBlock() override {}
};

class WaterBlock : public Block {
public:
	WaterBlock() : Block() { isDestructible = false; hasWater = true; }

	WaterBlock(float x, float y, const char* texPath = "Sprites/blocks/water.png") : Block(x, y) {
		isDestructible = false;
		hasWater = true;
		loadTexture(texPath);
	}

	void destroy() override {}
	void fillWithWater() override { hasWater = true; }

	~WaterBlock() override {}
};

static bool isSolidBlock(Block* b) { return b && b->isActive() && !b->getHasWater(); }
static bool isBreakableBlock(Block* b) { return b && b->isActive() && b->getIsDestructible() && !b->getHasWater(); }


struct Range {
	float minY;
	float maxY;

	Range() : minY(0.0f), maxY(0.0f) {}

	Range(float mn, float mx) : minY(mn), maxY(mx) {}
};

class Biome {
protected:
	Range heightRange;
	Sprite bgSprite;
	Texture bgTexture;

public:
	Biome() : heightRange(0.0f, 0.0f) {}

	Biome(float minY, float maxY) : heightRange(minY, maxY) {}

	bool loadBackground(const char* path) {
		if (!bgTexture.loadFromFile(path)) return false;
		bgSprite.setTexture(bgTexture);
		return true;
	}

	virtual void generateTerrain(Block*** grid, int gridW, int gridH) = 0;

	virtual void render(RenderWindow& window, float camX, float camY) {
		bgSprite.setPosition(0.0f - camX, heightRange.minY - camY);
		window.draw(bgSprite);
	}

	Range getHeightRange() const { return heightRange; }

	bool containsY(float worldY) const { return (worldY >= heightRange.minY && worldY <= heightRange.maxY); }

	virtual ~Biome() {}
};


class AerialBiome : public Biome {
private:
	float peakJaggedLevel;
public:
	AerialBiome() : Biome(), peakJaggedLevel(0.7f) {}

	AerialBiome(float minY, float maxY, float jaggedLevel = 0.7f) : Biome(minY, maxY), peakJaggedLevel(jaggedLevel) {}

	void generateTerrain(Block*** grid, int gridW, int gridH) override {
		int startRow = (int)(heightRange.minY / Block::BLOCK_SIZE);
		int endRow = (int)(heightRange.maxY / Block::BLOCK_SIZE);

		if (endRow >= gridH) endRow = gridH - 1;
		for (int c = 0; c < gridW; c++) {
			float t = (float)c / (float)gridW;
			float sinVal = (float)(sin(t * 12.0 + cos(t * 7.0)) * 0.5 + 0.5);
			float peakFrac = 0.3f + peakJaggedLevel * 0.5f * sinVal;
			int surface = startRow + (int)((endRow - startRow) * (1.0f - peakFrac));

			for (int r = surface; r <= endRow; r++) {
				float bx = (float)(c * Block::BLOCK_SIZE);
				float by = (float)(r * Block::BLOCK_SIZE);

				if ((grid)[r][c] != nullptr) { delete (grid)[r][c]; }

				if (r == gridH - 1) (grid)[r][c] = new IndestructibleBlock(bx, by);
				else (grid)[r][c] = new NormalBlock(bx, by);
			}
		}
	}

	~AerialBiome() override {}
};

class PlainsBiome : public Biome {
private:
	float hillFrequency;

public:
	PlainsBiome() : Biome(), hillFrequency(3.f) {}

	PlainsBiome(float minY, float maxY, float freq = 3.f) : Biome(minY, maxY), hillFrequency(freq) {}

	void generateTerrain(Block*** grid, int gridW, int gridH) override {
		int startRow = (int)(heightRange.minY / Block::BLOCK_SIZE);
		int endRow = (int)(heightRange.maxY / Block::BLOCK_SIZE);
		if (endRow >= gridH) endRow = gridH - 1;

		for (int c = 0; c < gridW; c++) {
			float t = (float)c / (float)gridW;
			float wave = (float)(sin(t * hillFrequency * 3.14159) * 0.5 + 0.5);
			float frac = 0.6f + wave * 0.2f;
			int surface = startRow + (int)((endRow - startRow) * (1.0f - frac));

			for (int r = surface; r <= endRow; r++) {
				float bx = (float)(c * Block::BLOCK_SIZE);
				float by = (float)(r * Block::BLOCK_SIZE);
				if (grid[r][c] != nullptr) delete grid[r][c];
				if (r == gridH - 1) grid[r][c] = new IndestructibleBlock(bx, by);
				else grid[r][c] = new NormalBlock(bx, by);
			}
		}
	}

	~PlainsBiome() override {}
};


class AquaticBiome : public Biome {
private:
	float seaLevel;
	float underwaterDepth;

public:
	AquaticBiome() : Biome(), seaLevel(0.f), underwaterDepth(5.f) {}

	AquaticBiome(float minY, float maxY, float seaLvl, float depth) : Biome(minY, maxY), seaLevel(seaLvl), underwaterDepth(depth) {}

	void generateTerrain(Block*** grid, int gridW, int gridH) override {
		int startRow = (int)(heightRange.minY / Block::BLOCK_SIZE);
		int endRow = (int)(heightRange.maxY / Block::BLOCK_SIZE);
		if (endRow >= gridH) endRow = gridH - 1;
		int seaRow = (int)(seaLevel / Block::BLOCK_SIZE);
		int floorRow = endRow;

		for (int c = 0; c < gridW; c++) {
			for (int r = startRow; r <= floorRow; r++) {
				float bx = (float)(c * Block::BLOCK_SIZE);
				float by = (float)(r * Block::BLOCK_SIZE);
				if (grid[r][c] != nullptr) delete grid[r][c];
				if (r == floorRow) {
					grid[r][c] = new IndestructibleBlock(bx, by);
				}
				else if (r >= seaRow) {
					NormalBlock* b = new NormalBlock(bx, by);
					b->fillWithWater();
					grid[r][c] = b;
				}
			}
		}
	}

	~AquaticBiome() override {}
};


struct SpawnPoint {
	static const int T_REBEL = 0;
	static const int T_SHIELDED = 1;
	static const int T_BAZOOKA = 2;
	static const int T_GRENADE = 3;
	static const int T_ZOMBIE = 4;
	static const int T_MUMMY = 5;
	static const int T_MARTIAN = 6;
	static const int T_PARATROOPER = 7;
	static const int T_FLYING_TARA = 8;
	static const int T_ENEMY_SUB = 9;
	static const int T_BRADLEY = 10;
	static const int T_POW = 11;

	float worldX;
	float worldY;
	int type;
	int batch;
	bool spawned;

	SpawnPoint() : worldX(0), worldY(0), type(0), batch(1), spawned(false) {}

	SpawnPoint(float x, float y, int t, int b) : worldX(x), worldY(y), type(t), batch(b), spawned(false) {}
};


class Level {
protected:
	int width;
	int height;
	float scrollX;
	float scrollY;
	SpawnPoint spawnPoints[200];
	int spawnCount;

	Block*** grid;
	Biome* biomes[3];
	int biomeCount;
	Environment* environment;

	Texture bgTexture;
	Sprite bgSprite;


	void addSpawn(float x, float y, int type, int batch) {
		if (spawnCount >= 200) return;
		spawnPoints[spawnCount] = SpawnPoint(x, y, type, batch);
		spawnCount++;
	}

public:
	Level() : width(110), height(14), scrollX(0.f), scrollY(0.f), grid(nullptr), biomeCount(0), environment(nullptr), spawnCount(0) {
		biomes[0] = biomes[1] = biomes[2] = nullptr;
		allocateGrid();
	}

	Level(int w, int h) : width(w), height(h), scrollX(0.f), scrollY(0.f), grid(nullptr), biomeCount(0), environment(nullptr), spawnCount(0) {
		biomes[0] = biomes[1] = biomes[2] = nullptr;
		allocateGrid();
	}


	int getWidth() const { return width; }
	int getHeight() const { return height; }


	bool loadBackground(const char* path) {
		if (!bgTexture.loadFromFile(path)) {
			cout << "Could not load background: " << path << "\n";
			return false;
		}
		bgSprite.setTexture(bgTexture);
		bgSprite.setScale(
			(float)(width * Block::BLOCK_SIZE) / (float)bgTexture.getSize().x,
			(float)(height * Block::BLOCK_SIZE) / (float)bgTexture.getSize().y
		);
		return true;
	}


	virtual bool isComplete() = 0;

	virtual void update(float dt) { if (environment) environment->update(dt); }

	virtual void render(RenderWindow& window, float camX, float camY) {
		bgSprite.setPosition(-camX, -camY);
		window.draw(bgSprite);

		if (biomeCount >= 3) {
			float zoneW = (float)(width * Block::BLOCK_SIZE) / 3.0f;
			float zoneH = (float)(height * Block::BLOCK_SIZE);
			RectangleShape biomeBand;
			biomeBand.setSize(Vector2f(zoneW, zoneH));

			biomeBand.setFillColor(Color(45, 120, 50, 45));
			biomeBand.setPosition(0.0f - camX, 0.0f - camY);
			window.draw(biomeBand);

			biomeBand.setFillColor(Color(150, 150, 165, 45));
			biomeBand.setPosition(zoneW - camX, 0.0f - camY);
			window.draw(biomeBand);

			biomeBand.setFillColor(Color(40, 90, 170, 55));
			biomeBand.setPosition(zoneW * 2.0f - camX, 0.0f - camY);
			window.draw(biomeBand);
		}

		int colStart = (int)(camX / Block::BLOCK_SIZE) - 1;
		int colEnd = (int)((camX + 1600) / Block::BLOCK_SIZE) + 1;
		int rowStart = (int)(camY / Block::BLOCK_SIZE) - 1;
		int rowEnd = (int)((camY + 900) / Block::BLOCK_SIZE) + 1;

		if (colStart < 0) colStart = 0;
		if (rowStart < 0) rowStart = 0;
		if (colEnd >= width) colEnd = width - 1;
		if (rowEnd >= height) rowEnd = height - 1;

		for (int r = rowStart; r <= rowEnd; r++) {
			for (int c = colStart; c <= colEnd; c++) {
				if (grid[r][c] && grid[r][c]->isActive()) grid[r][c]->render(window, camX, camY);
			}
		}
	}

	void scroll(float dx, float dy) {
		scrollX += dx;
		scrollY += dy;
	}

	Block* getBlockAt(int col, int row) const {
		if (col < 0 || col >= width || row < 0 || row >= height)
			return nullptr;
		return grid[row][col];
	}

	Block* getBlockAtWorld(float wx, float wy) const {
		int col = (int)(wx / Block::BLOCK_SIZE);
		int row = (int)(wy / Block::BLOCK_SIZE);
		return getBlockAt(col, row);
	}

	void setBlockAt(int col, int row, Block* b) {
		if (col < 0 || col >= width || row < 0 || row >= height) return;
		delete grid[row][col];
		grid[row][col] = b;
	}

	void applyBlastRadius(float cx, float cy, float radius) {
		int colMin = (int)((cx - radius) / Block::BLOCK_SIZE) - 1;
		int colMax = (int)((cx + radius) / Block::BLOCK_SIZE) + 1;
		int rowMin = (int)((cy - radius) / Block::BLOCK_SIZE) - 1;
		int rowMax = (int)((cy + radius) / Block::BLOCK_SIZE) + 1;

		if (colMin < 0) colMin = 0;
		if (rowMin < 0) rowMin = 0;
		if (colMax >= width) colMax = width - 1;
		if (rowMax >= height) rowMax = height - 1;

		for (int r = rowMin; r <= rowMax; r++) {
			for (int c = colMin; c <= colMax; c++) {
				Block* b = grid[r][c];
				if (isBreakableBlock(b) && b->inBlastRadius(cx, cy, radius)) b->destroy();
			}
		}
	}

	Biome* getBiome(int idx) const {
		if (idx < 0 || idx >= 3) return nullptr;
		return biomes[idx];
	}

	int getBiomeIndexAtY(float worldY) const {
		for (int i = 0; i < 3; i++) {
			if (biomes[i] && biomes[i]->containsY(worldY)) return i;
		}
		return -1;
	}

	Environment* getEnvironment() const { return environment; }

	const SpawnPoint* getSpawnPoints(int& count) const {
		count = spawnCount;
		return spawnPoints;
	}

	virtual void spawnPlayerVehicles() {}

	~Level()
	{
		freeGrid();
		for (int i = 0; i < 3; i++)
		{
			delete biomes[i];
			biomes[i] = nullptr;
		}
		delete environment;
		environment = nullptr;
	}

protected:
	void allocateGrid() {
		grid = new Block * *[height];
		for (int r = 0; r < height; r++) {
			grid[r] = new Block * [width];
			for (int c = 0; c < width; c++)
				grid[r][c] = nullptr;
		}
	}

	void freeGrid() {
		if (!grid) return;
		for (int r = 0; r < height; r++) {
			for (int c = 0; c < width; c++) {
				delete grid[r][c];
				grid[r][c] = nullptr;
			}
			delete[] grid[r];
		}
		delete[] grid;
		grid = nullptr;
	}

	void fillBedrockRow() {
		int bedrockRow = height - 1;
		for (int c = 0; c < width; c++) {
			float bx = (float)(c * Block::BLOCK_SIZE);
			float by = (float)(bedrockRow * Block::BLOCK_SIZE);
			delete grid[bedrockRow][c];
			grid[bedrockRow][c] = new IndestructibleBlock(bx, by);
		}
	}
};


class GameStats {
public:
	static const int MAX_CHARS = 4;
	static const int STAT_HP = 0;
	static const int STAT_LIVES = 1;
	static const int STAT_AMMO = 2;
	static const int STAT_GRENADES = 3;
	static const int STAT_SATURATION = 4;

	static const int DAMAGE_HEALTHY = 0;
	static const int DAMAGE_INJURED = 1;
	static const int DAMAGE_CRITICAL = 2;
	static const int DAMAGE_DEAD = 3;

private:
	int hp[MAX_CHARS];
	int maxHP[MAX_CHARS];
	int lives[MAX_CHARS];
	int ammo[MAX_CHARS];
	int grenades[MAX_CHARS];
	int saturation[MAX_CHARS];
	bool isFat[MAX_CHARS];
	bool charAlive[MAX_CHARS];
	int weaponType[MAX_CHARS];
	int damageState[MAX_CHARS];
	float damageTimer[MAX_CHARS];
	int score;
	int activeCharIndex;
	bool isImmortal;
	static const float DAMAGE_WINDOW;

public:
	GameStats() {
		score = 0;
		activeCharIndex = 0;
		isImmortal = false;

		for (int i = 0; i < MAX_CHARS; i++) {
			hp[i] = 100;
			maxHP[i] = 100;
			lives[i] = 2;
			ammo[i] = 0;
			grenades[i] = 10;
			saturation[i] = 0;
			isFat[i] = false;
			charAlive[i] = true;
			weaponType[i] = 0;
			damageState[i] = DAMAGE_HEALTHY;
			damageTimer[i] = 0.f;
		}
	}

	void Reset() {
		score = 0;
		activeCharIndex = 0;
		isImmortal = false;

		for (int i = 0; i < MAX_CHARS; i++) {
			hp[i] = maxHP[i] = 100;
			lives[i] = 2;
			ammo[i] = grenades[i] = 0;
			saturation[i] = 0;
			isFat[i] = false;
			charAlive[i] = true;
			weaponType[i] = 0;
			damageState[i] = DAMAGE_HEALTHY;
			damageTimer[i] = 0.f;
		}
		grenades[0] = grenades[1] = grenades[2] = grenades[3] = 10;
	}

	void addStat(int charIdx, int targetStat, int val) {
		if (charIdx < 0 || charIdx >= MAX_CHARS) return;
		if (targetStat == STAT_HP)
		{
			hp[charIdx] += val;
			if (hp[charIdx] > maxHP[charIdx]) hp[charIdx] = maxHP[charIdx];
			if (hp[charIdx] < 0) hp[charIdx] = 0;
		}
		else if (targetStat == STAT_LIVES)
		{
			lives[charIdx] += val;
			if (lives[charIdx] < 0) lives[charIdx] = 0;
		}
		else if (targetStat == STAT_AMMO)
		{
			ammo[charIdx] += val;
			if (ammo[charIdx] < 0) ammo[charIdx] = 0;
		}
		else if (targetStat == STAT_GRENADES)
		{
			grenades[charIdx] += val;
			if (grenades[charIdx] < 0) grenades[charIdx] = 0;
		}
		else if (targetStat == STAT_SATURATION)
		{
			saturation[charIdx] += val;
			if (saturation[charIdx] < 0) saturation[charIdx] = 0;
			if (saturation[charIdx] > 80) isFat[charIdx] = true;
		}
	}

	void subtractStat(int charIdx, int targetStat, int val) { addStat(charIdx, targetStat, -val); }

	int getStat(int charIdx, int targetStat) const {
		if (charIdx < 0 || charIdx >= MAX_CHARS) return 0;
		if (targetStat == STAT_HP) return hp[charIdx];
		if (targetStat == STAT_LIVES) return lives[charIdx];
		if (targetStat == STAT_AMMO) return ammo[charIdx];
		if (targetStat == STAT_GRENADES) return grenades[charIdx];
		if (targetStat == STAT_SATURATION) return saturation[charIdx];
		return 0;
	}

	bool takeDamage(int charIdx) {
		if (isImmortal) return false;
		if (charIdx < 0 || charIdx >= MAX_CHARS) return false;
		if (!charAlive[charIdx]) return false;
		if (damageTimer[charIdx] > 0.0f) return false;

		if (damageState[charIdx] == DAMAGE_HEALTHY)
		{
			damageState[charIdx] = DAMAGE_INJURED;
			damageTimer[charIdx] = DAMAGE_WINDOW;
		}
		else if (damageState[charIdx] == DAMAGE_INJURED)
		{
			damageState[charIdx] = DAMAGE_CRITICAL;
			damageTimer[charIdx] = DAMAGE_WINDOW;
		}
		else if (damageState[charIdx] == DAMAGE_CRITICAL)
		{
			damageState[charIdx] = DAMAGE_DEAD;
			damageTimer[charIdx] = 0.f;
			lives[charIdx]--;

			if (lives[charIdx] > 0)
			{
				damageState[charIdx] = DAMAGE_HEALTHY;
				hp[charIdx] = maxHP[charIdx];
			}
			else
			{
				charAlive[charIdx] = false;
			}
			return true;
		}
		return false;
	}

	void updateDamageTimers(float dt) {
		for (int i = 0; i < MAX_CHARS; i++) {
			if (damageTimer[i] > 0.f) {
				damageTimer[i] -= dt;
				if (damageTimer[i] < 0.f) damageTimer[i] = 0.f;
			}
		}
	}

	void addScore(int val) { score += val; if (score < 0) score = 0; }
	int  getScore() const { return score; }
	int  getActiveCharIndex() const { return activeCharIndex; }
	void setActiveCharIndex(int i) { activeCharIndex = i; }
	bool getIsImmortal()  const { return isImmortal; }
	void setIsImmortal(bool v) { isImmortal = v; }

	int getHP(int i) const { return hp[i]; }
	int getMaxHP(int i) const { return maxHP[i]; }
	int getLives(int i) const { return lives[i]; }
	int getAmmo(int i) const { return ammo[i]; }
	int getGrenades(int i) const { return grenades[i]; }
	int getSaturation(int i) const { return saturation[i]; }
	bool isCharAlive(int i) const { return charAlive[i]; }
	int getWeaponType(int i) const { return weaponType[i]; }
	int getDamageState(int i) const { return damageState[i]; }

	void setHP(int i, int v) { hp[i] = v; if (hp[i] > maxHP[i]) hp[i] = maxHP[i]; if (hp[i] < 0) hp[i] = 0; }
	void setMaxHP(int i, int v) { maxHP[i] = v; }
	void setLives(int i, int v) { lives[i] = v; }
	void setAmmo(int i, int v) { ammo[i] = v; if (ammo[i] < 0) ammo[i] = 0; }
	void setGrenades(int i, int v) { grenades[i] = v; if (grenades[i] < 0) grenades[i] = 0; }
	void setCharAlive(int i, bool v) { charAlive[i] = v; }
	void setWeaponType(int i, int v) { weaponType[i] = v; }

	~GameStats() {}
};
const float GameStats::DAMAGE_WINDOW = 1.0f;


class InputManager {
private:
	bool keyStates[Keyboard::KeyCount];
	bool prevKeyStates[Keyboard::KeyCount];
	int  mouseX;
	int  mouseY;
	bool mouseLeft;
	bool mouseRight;
	bool prevMouseLeft;

public:
	InputManager() : mouseX(0), mouseY(0), mouseLeft(false), mouseRight(false), prevMouseLeft(false) {
		memset(keyStates, 0, sizeof(keyStates));
		memset(prevKeyStates, 0, sizeof(prevKeyStates));
	}

	int  getMouseX() const { return mouseX; }
	int  getMouseY() const { return mouseY; }
	bool isMouseLeftDown() const { return mouseLeft; }
	bool isMouseRightDown() const { return mouseRight; }

	void update(RenderWindow& window) {
		memcpy(prevKeyStates, keyStates, sizeof(keyStates));
		prevMouseLeft = mouseLeft;
		for (int k = 0; k < Keyboard::KeyCount; k++) {
			keyStates[k] = Keyboard::isKeyPressed(static_cast<Keyboard::Key>(k));
		}

		mouseX = Mouse::getPosition(window).x;
		mouseY = Mouse::getPosition(window).y;
		mouseLeft = Mouse::isButtonPressed(sf::Mouse::Left);
		mouseRight = Mouse::isButtonPressed(sf::Mouse::Right);
	}

	void handleEvent(const Event& ev) { (void)ev; }

	bool isKeyDown(Keyboard::Key k) const {
		return keyStates[(int)k];
	}

	bool isKeyPressed(Keyboard::Key k) const {
		return keyStates[(int)k] && !prevKeyStates[(int)k];
	}

	bool isKeyReleased(sf::Keyboard::Key k) const {
		return !keyStates[(int)k] && prevKeyStates[(int)k];
	}

	bool isMouseLeftPressed() const {
		return mouseLeft && !prevMouseLeft;
	}

	float getAimAngle(float playerScreenX, float playerScreenY) const {
		float dx = (float)mouseX - playerScreenX;
		float dy = playerScreenY - (float)mouseY;
		if (dx == 0.f && dy == 0.f) return 0.f;
		float absDx = (dx >= 0.f) ? dx : -dx;
		float angleRad = atan2f(dy, absDx);
		float angleDeg = angleRad * 180.f / 3.14159265f;
		if (angleDeg < 0.f)  angleDeg = 0.f;
		if (angleDeg > 90.f) angleDeg = 90.f;

		return angleDeg;
	}

	~InputManager() {}
};


class Weapon {
public:
	static const int WEAPON_PISTOL = 0;
	static const int WEAPON_HMG = 1;
	static const int WEAPON_ROCKET = 2;
	static const int WEAPON_FLAME = 3;
	static const int WEAPON_LASER = 4;
	static const int WEAPON_KNIFE = 5;

protected:
	int damage;
	int ammo;
	float fireRate;
	float fireCooldown;
	int weaponType;

public:
	Weapon(int dmg, int startAmmo, float rate, int type) : damage(dmg), ammo(startAmmo), fireRate(rate), fireCooldown(0.f), weaponType(type) {}

	int getDamage() const { return damage; }
	int getAmmo() const { return ammo; }
	float getFireRate() const { return fireRate; }
	int getWeaponType() const { return weaponType; }
	bool isInfiniteAmmo() const { return ammo == -1; }
	bool isEmpty() const { return ammo == 0; }

	void updateCooldown(float dt) {
		if (fireCooldown > 0.0f) {
			fireCooldown -= dt;
			if (fireCooldown < 0.0f) fireCooldown = 0.0f;
		}
	}

	bool canFire() const { return fireCooldown <= 0.0f && (ammo > 0 || ammo == -1); }

	virtual void fire(float originX, float originY, float angleDeg, bool facingRight) {
		if (!canFire()) return;
		if (ammo != -1) ammo--;
		fireCooldown = 1.f / fireRate;
		cout << "Ammo remaining: " << ammo << "\n";
	}

	virtual void addAmmo(int amount) {
		if (ammo == -1) return;
		ammo += amount;
	}

	virtual ~Weapon() {}
};


class Projectile {
protected:
	float posX;
	float posY;
	float velX;
	float velY;
	int damage;
	bool active;
	bool fromPlayer;
	float hitW;
	float hitH;
	Sprite sprite;
	Texture texture;

public:
	Projectile(float x, float y, float vx, float vy, int dmg, bool fromPly)
		: posX(x), posY(y), velX(vx), velY(vy), damage(dmg),
		active(true), fromPlayer(fromPly), hitW(14.0f), hitH(8.0f) {
	}

	float getPosX() const { return posX; }
	float getPosY() const { return posY; }
	int getDamage() const { return damage; }
	bool isActive() const { return active; }
	bool isFromPlayer() const { return fromPlayer; }
	float getHitW() const { return hitW; }
	float getHitH() const { return hitH; }
	void setActive(bool a) { active = a; }
	void setHitBox(float w, float h) { hitW = w; hitH = h; }

	bool overlapsRect(float x, float y, float w, float h) const {
		float left = posX - hitW * 0.5f;
		float top = posY - hitH * 0.5f;
		return !(left + hitW < x || x + w < left || top + hitH < y || y + h < top);
	}

	virtual void update(float dt) = 0;

	virtual void render(RenderWindow& window, float camX, float camY) {
		if (!active) return;
		sprite.setPosition(posX - camX, posY - camY);
		window.draw(sprite);
	}

	virtual void onHit() { active = false; }

	bool loadTexture(const char* path) {
		if (!texture.loadFromFile(path)) return false;
		sprite.setTexture(texture);
		return true;
	}

	void setSpriteScale(float sx, float sy) { sprite.setScale(sx, sy); }

	virtual ~Projectile() {}
};


class Bullet : public Projectile {
private:
	float speed;
	float angleDeg;
	float maxRange;
	float distTravelled;

public:
	Bullet(float x, float y, float spd, float angle, bool facingRight, int dmg, bool fromPly, float range = 1600.0f) : Projectile(x, y, 0.0f, 0.0f, dmg, fromPly), speed(spd), angleDeg(angle), maxRange(range), distTravelled(0.0f) {
		float rad = angleDeg * 3.14159265f / 180.0f;
		float vx = cosf(rad) * speed;
		float vy = -sinf(rad) * speed;

		velX = facingRight ? vx : -vx;
		velY = vy;
		hitW = 24.0f;
		hitH = 12.0f;
	}

	void update(float dt) override {
		if (!active) return;

		posX += velX * dt;
		posY += velY * dt;

		float distThisFrame = speed * dt;
		distTravelled += distThisFrame;
		if (distTravelled >= maxRange) onHit();
	}

	~Bullet() override {}
};


class EnemyAIState;

class Enemy : public DamagableEntity {
public:
	static const int TYPE_REBEL = 0;
	static const int TYPE_SHIELDED = 1;
	static const int TYPE_BAZOOKA = 2;
	static const int TYPE_GRENADE = 3;
	static const int TYPE_ZOMBIE = 4;
	static const int TYPE_MUMMY = 5;
	static const int TYPE_MARTIAN = 6;
	static const int TYPE_PARA = 7;
	static const int TYPE_FLYING_TARA = 8;
	static const int TYPE_ENEMY_SUB = 9;
	static const int TYPE_BRADLEY = 10;

protected:
	int enemyType;
	int scoreValue;
	int spawnBatchSize;
	float velX;
	float velY;
	bool facingRight;
	bool isOnGround;
	EnemyAIState* aiState;
	Level* level;
	const Entity* playerTarget;

	static const float GRAVITY;
	static const float MAX_FALL_SPEED;

public:
	Enemy(float x, float y, float w, float h, int mHP, int type, int score, int batch, Level* lvl) : DamagableEntity(x, y, w, h, mHP), enemyType(type), scoreValue(score), spawnBatchSize(batch), velX(0.0f), velY(0.0f), facingRight(false), isOnGround(false), aiState(nullptr), level(lvl), playerTarget(nullptr) {}

	int getEnemyType() const { return enemyType; }
	int getScoreValue() const { return scoreValue; }
	int getSpawnBatchSize() const { return spawnBatchSize; }
	bool getFacingRight() const { return facingRight; }
	bool  getIsOnGround() const { return isOnGround; }
	float getVelX() const { return velX; }
	float getVelY() const { return velY; }
	Level* getLevel() const { return level; }
	const Entity* getPlayerTarget() const { return playerTarget; }

	void setVelX(float vx) { velX = vx; }
	void setVelY(float vy) { velY = vy; }
	void setFacingRight(bool f) { facingRight = f; }
	void setIsOnGround(bool g) { isOnGround = g; }
	void setPlayerTarget(const Entity* p) { playerTarget = p; }


	void setAIState(EnemyAIState* newState);
	virtual void attack() = 0;
	virtual void updateProjectiles(float dt) {};
	virtual void renderProjectiles(RenderWindow& window, float camX, float camY) {}

	virtual bool receivePlayerHit(float attackX, float attackY, int damage, bool explosive, bool fire, bool meleePierce) {
		(void)attackX; (void)attackY; (void)explosive; (void)fire; (void)meleePierce;
		return takeDamage(damage);
	}

	virtual bool checkProjectileHits(float px, float py, float pw, float ph, int& dmgOut) { dmgOut = 0; return false; }

	virtual void update(float dt) override;

	virtual void render(RenderWindow& window, float camX, float camY) override {
		if (!active) return;

		anim.update(0.0f, sprite);
		float frameW = (float)texture.getSize().x;
		float frameH = (float)texture.getSize().y;
		float sx = (frameW > 0.0f) ? width / frameW : 1.f;
		float sy = (frameH > 0.0f) ? height / frameH : 1.f;

		if (facingRight) {
			sprite.setScale(sx, sy);
			sprite.setOrigin(0.f, 0.f);
		}
		else {
			sprite.setScale(-sx, sy);
			sprite.setOrigin(frameW, 0.f);
		}
		sprite.setPosition(posX - camX, posY - camY);
		window.draw(sprite);
	}

	virtual ~Enemy();

protected:
	void doGroundCollision() {
		if (!level) return;
		float footY = posY + height + 3.0f;
		Block* bL = level->getBlockAtWorld(posX + 8.0f, footY);
		Block* bM = level->getBlockAtWorld(posX + width * 0.5f, footY);
		Block* bR = level->getBlockAtWorld(posX + width - 8.0f, footY);
		bool touching = isSolidBlock(bL) || isSolidBlock(bM) || isSolidBlock(bR);

		if (touching && velY >= 0.0f) {
			int row = (int)(footY / (float)Block::BLOCK_SIZE);
			posY = (float)(row * Block::BLOCK_SIZE) - height;
			velY = 0.0f;
			isOnGround = true;
		}
		else {
			isOnGround = false;
		}
	}


	void doWallCollision() {
		if (!level) return;
		float checkY1 = posY + height * 0.3f;
		float checkY2 = posY + height * 0.7f;

		if (velX < 0.0f) {
			Block* wL1 = level->getBlockAtWorld(posX - 2.0f, checkY1);
			Block* wL2 = level->getBlockAtWorld(posX - 2.0f, checkY2);
			if (isSolidBlock(wL1) || isSolidBlock(wL2)) {
				int col = (int)((posX - 2.0f) / Block::BLOCK_SIZE);
				posX = (float)((col + 1) * Block::BLOCK_SIZE);
				velX = -velX;
				facingRight = !facingRight;
			}
		}
		if (velX > 0.0f) {
			Block* wR1 = level->getBlockAtWorld(posX + width + 2.0f, checkY1);
			Block* wR2 = level->getBlockAtWorld(posX + width + 2.0f, checkY2);
			if (isSolidBlock(wR1) || isSolidBlock(wR2)) {
				int col = (int)((posX + width + 2.0f) / Block::BLOCK_SIZE);
				posX = (float)(col * Block::BLOCK_SIZE) - width;
				velX = -velX;
				facingRight = !facingRight;
			}
		}
	}

};
const float Enemy::GRAVITY = 1800.0f;
const float Enemy::MAX_FALL_SPEED = 900.0f;


class EnemyAIState {
public:
	static const int STATE_PATROL = 0;
	static const int STATE_CHASE = 1;
	static const int STATE_ATTACK = 2;
	static const int STATE_FLEE = 3;
	static const int STATE_IDLE = 4;

	virtual void handle(Enemy* e, float dt) = 0;
	virtual int getStateType() const = 0;
	virtual void onEnter(Enemy* e) { (void)e; }
	virtual void onExit(Enemy* e) { (void)e; }

	virtual ~EnemyAIState() {}
};

void Enemy::setAIState(EnemyAIState* newState) {
	if (aiState) aiState->onExit(this);
	delete aiState;
	aiState = newState;
	if (aiState) aiState->onEnter(this);
}

void Enemy::update(float dt) {
	DamagableEntity::update(dt);
	if (isDead()) return;

	if (!isOnGround) {
		velY += GRAVITY * dt;
		if (velY > MAX_FALL_SPEED) velY = MAX_FALL_SPEED;
	}

	if (aiState) aiState->handle(this, dt);

	posX += velX * dt;
	posY += velY * dt;

	doGroundCollision();
	doWallCollision();

	if (level) {
		float maxX = (float)(level->getWidth() * Block::BLOCK_SIZE) - width;
		if (posX < 0.f) posX = 0.f;
		if (posX > maxX) posX = maxX;
	}

	anim.update(dt, sprite);
}

Enemy::~Enemy() {
	delete aiState;
	aiState = nullptr;
}


class EnemyProjectiles {
public:
	static const int CAP = 6;

private:
	Projectile* shots[CAP];
	int count;

public:
	EnemyProjectiles() : count(0) { for (int i = 0; i < CAP; i++) shots[i] = nullptr; }

	bool add(Projectile* p) {
		if (count >= CAP) { delete p; return false; }
		shots[count++] = p;
		return true;
	}

	void update(float dt) {
		for (int i = 0; i < count; ) {
			shots[i]->update(dt);
			if (!shots[i]->isActive()) {
				delete shots[i];
				shots[i] = shots[--count];
				shots[count] = nullptr;
			}
			else { i++; }
		}
	}

	void render(RenderWindow& window, float camX, float camY) {
		for (int i = 0; i < count; i++) shots[i]->render(window, camX, camY);
	}

	bool checkHit(float px, float py, float pw, float ph, int& dmgOut) {
		for (int i = 0; i < count; i++) {
			Projectile* p = shots[i];
			if (!p->isActive()) continue;
			const float BW = 10.0f, BH = 10.0f;
			bool overX = !((p->getPosX() + BW < px) || (p->getPosX() > px + pw));
			bool overY = !((p->getPosY() + BH < py) || (p->getPosY() > py + ph));
			if (overX && overY) {
				dmgOut = p->getDamage();
				p->onHit();
				return true;
			}
		}
		return false;
	}

	~EnemyProjectiles() { for (int i = 0; i < count; i++) { delete shots[i]; shots[i] = nullptr; } }
};

class BallisticProjectile : public Projectile {
private:
	float blastRadius;
	bool  exploded;
	bool  isFireBomb;
	Level* level;

public:
	BallisticProjectile(float x, float y, float speed, float angleDeg, bool facingRight, int dmg, float blastRad, Level* lvl, bool fromPlayer = false, bool fireBomb = false) : Projectile(x, y, 0.f, 0.f, dmg, fromPlayer), blastRadius(blastRad), exploded(false), isFireBomb(fireBomb), level(lvl) {
		float rad = angleDeg * 3.14159265f / 180.0f;
		float vx = cosf(rad) * speed;
		float vy = -sinf(rad) * speed;
		velX = facingRight ? vx : -vx;
		velY = vy;

		loadTexture("Sprites/projectiles/grenade_ball.png");
	}

	void update(float dt) override {
		if (!active || exploded) return;
		velY += 980.0f * dt;
		posX += velX * dt;
		posY += velY * dt;

		if (level) {
			Block* b = level->getBlockAtWorld(posX, posY);
			if (isSolidBlock(b)) { onHit(); return; }
		}

		if (level && posY > (float)(level->getHeight() * Block::BLOCK_SIZE)) active = false;
	}

	void onHit() override {
		if (exploded) return;
		exploded = true;
		if (level) level->applyBlastRadius(posX, posY, blastRadius);
		active = false;
	}

	bool getIsFireBomb() const { return isFireBomb; }
	float getBlastRadius() const { return blastRadius; }

	~BallisticProjectile() override {}
};

class PatrolState : public EnemyAIState {
	const Entity* target;
	float patrolSpeed;
	float detectionRange;
	float attackRange;
	float dirTimer;

	static const float FLIP_INTERVAL;

public:
	PatrolState(const Entity* player, float speed = 55.f, float detect = 320.f, float atk = 110.f) : target(player), patrolSpeed(speed), detectionRange(detect), attackRange(atk), dirTimer(0.f) {}

	int getStateType() const override { return STATE_PATROL; }

	void onEnter(Enemy* e) override { e->setVelX(e->getFacingRight() ? patrolSpeed : -patrolSpeed); }

	void handle(Enemy* e, float dt) override;
};
const float PatrolState::FLIP_INTERVAL = 3.5f;


class ChaseState : public EnemyAIState {
	const Entity* target;
	float chaseSpeed;
	float attackRange;

public:
	ChaseState(const Entity* player, float speed = 90.f, float atk = 110.f) : target(player), chaseSpeed(speed), attackRange(atk) {}

	int getStateType() const override { return STATE_CHASE; }

	void onEnter(Enemy* e) override { (void)e; }

	void handle(Enemy* e, float dt) override;
};


class AttackState : public EnemyAIState {
	const Entity* target;
	float fireCooldown;
	float fireInterval;
	float chaseRange;

public:
	AttackState(const Entity* player, float interval = 1.2f, float chase = 250.f) : target(player), fireCooldown(0.f), fireInterval(interval), chaseRange(chase) {}

	int getStateType() const override { return STATE_ATTACK; }

	void onEnter(Enemy* e) override { e->setVelX(0.f); }

	void handle(Enemy* e, float dt) override;
};


class IdleState : public EnemyAIState {
	const Entity* target;
	float detectRange;

public:
	IdleState(const Entity* player, float detect = 400.f) : target(player), detectRange(detect) {}

	int getStateType() const override { return STATE_IDLE; }

	void onEnter(Enemy* e) override { e->setVelX(0.f); }

	void handle(Enemy* e, float dt) override;
};

class DescentState : public EnemyAIState {
	const Entity* target;
	float fallSpeed;

public:
	explicit DescentState(const Entity* player, float fall = 80.0f) : target(player), fallSpeed(fall) {}

	int getStateType() const override { return STATE_IDLE; }

	void onEnter(Enemy* e) override { e->setVelX(0.0f); }

	void handle(Enemy* e, float dt) override {
		(void)dt;
		e->setVelX(0.0f);
		e->setVelY(fallSpeed);

		if (e->getIsOnGround()) {
			e->setAIState(new PatrolState(target, 55.0f, 320.0f, 110.0f));
		}
	}
};

void PatrolState::handle(Enemy* e, float dt) {
	dirTimer += dt;
	if (dirTimer >= FLIP_INTERVAL) {
		dirTimer = 0.0f;
		e->setFacingRight(!e->getFacingRight());
	}
	e->setVelX(e->getFacingRight() ? patrolSpeed : -patrolSpeed);

	if (!target) return;
	float dx = target->getPosX() - e->getPosX();
	float dy = target->getPosY() - e->getPosY();
	float dist = sqrtf(dx * dx + dy * dy);

	e->setFacingRight(dx > 0.0f);

	if (dist < attackRange) e->setAIState(new AttackState(target));
	else if (dist < detectionRange) e->setAIState(new ChaseState(target, patrolSpeed * 1.5f, attackRange));
}

void ChaseState::handle(Enemy* e, float dt) {
	(void)dt;
	if (!target) return;
	float dx = target->getPosX() - e->getPosX();
	float dy = target->getPosY() - e->getPosY();
	float dist = sqrtf(dx * dx + dy * dy);

	e->setFacingRight(dx > 0.f);
	e->setVelX(dx > 0.f ? chaseSpeed : -chaseSpeed);

	if (dist < attackRange) e->setAIState(new AttackState(target));
	else if (dist > 550.f) e->setAIState(new PatrolState(target));
}

void AttackState::handle(Enemy* e, float dt) {
	e->setVelX(0.0f);
	fireCooldown -= dt;

	if (!target) return;
	float dx = target->getPosX() - e->getPosX();
	float dist = sqrtf(dx * dx + (target->getPosY() - e->getPosY()) * (target->getPosY() - e->getPosY()));

	e->setFacingRight(dx > 0.0f);

	if (fireCooldown <= 0.0f) {
		e->attack();
		fireCooldown = fireInterval;
	}
	if (dist > chaseRange) e->setAIState(new ChaseState(target));
}

void IdleState::handle(Enemy* e, float dt) {
	(void)dt;
	e->setVelX(0.f);
	if (!target) return;
	float dx = target->getPosX() - e->getPosX();
	float dy = target->getPosY() - e->getPosY();
	float dist = sqrtf(dx * dx + dy * dy);
	if (dist < detectRange) e->setAIState(new ChaseState(target));
}


class PatrolState;
class ChaseState;
class AttackState;
class IdleState;
class DescentState;

class RebelSoldier : public Enemy {
private:
	EnemyProjectiles pool;
	float shootCooldown;
	static const float SHOOT_INTERVAL;
	static const float BULLET_SPEED;

public:
	RebelSoldier(float x, float y, Level* lvl, const Entity* player) : Enemy(x, y, 118.f, 104.f, 2, TYPE_REBEL, 50, 3, lvl), shootCooldown(0.f) {
		playerTarget = player;
		loadTexture("Sprites/enemies/rebel.png");
		anim.configure(48, 48, 4, 0, 8.0f);
		setAIState(new PatrolState(player, 55.0f, 300.0f, 130.0f));
	}

	void attack() override {
		if (shootCooldown > 0.f) return;
		if (!playerTarget) return;

		float ox = facingRight ? posX + width : posX;
		float oy = posY + height * 0.4f;

		float dx = playerTarget->getPosX() - posX;
		float angle = 0.0f;
		float dy = posY - playerTarget->getPosY();
		if (dy > 20.f) angle = 15.f;

		Bullet* b = new Bullet(ox, oy, BULLET_SPEED, angle, facingRight, 3, false);
		b->loadTexture("Sprites/projectiles/bullet.png");
		b->setSpriteScale(0.08f, 0.08f);
		pool.add(b);

		shootCooldown = SHOOT_INTERVAL;
		anim.switchTo(1, 2, 6.f, false);
	}

	void update(float dt) override {
		Enemy::update(dt);
		if (shootCooldown > 0.f) { shootCooldown -= dt; if (shootCooldown < 0.f) shootCooldown = 0.f; }
		if (aiState && aiState->getStateType() != 2) anim.switchTo(0, 4, 8.f);
		pool.update(dt);
	}

	void render(RenderWindow& window, float camX, float camY) override {
		Enemy::render(window, camX, camY);
		pool.render(window, camX, camY);
	}

	void updateProjectiles(float dt) override { pool.update(dt); }
	void renderProjectiles(RenderWindow& w, float cx, float cy) override { pool.render(w, cx, cy); }
	bool checkProjectileHits(float px, float py, float pw, float ph, int& dmgOut) override
	{
		return pool.checkHit(px, py, pw, ph, dmgOut);
	}

	~RebelSoldier() override {}
};
const float RebelSoldier::SHOOT_INTERVAL = 1.4f;
const float RebelSoldier::BULLET_SPEED = 380.f;


class ShieldedSoldier : public Enemy {
private:
	EnemyProjectiles pool;
	float shootCooldown;
	bool shieldActive;

	static const float SHOOT_INTERVAL;
	static const float BULLET_SPEED;

public:
	ShieldedSoldier(float x, float y, Level* lvl, const Entity* player) : Enemy(x, y, 128.f, 112.f, 5, TYPE_SHIELDED, 75, 2, lvl), shootCooldown(0.f), shieldActive(true) {
		playerTarget = player;
		loadTexture("Sprites/enemies/shielded.png");
		anim.configure(48, 52, 4, 0, 7.f);
		setAIState(new PatrolState(player, 45.f, 280.f, 120.f));
	}

	bool takeDamage(int damage) override {
		return Enemy::takeDamage(damage);
	}

	bool receivePlayerHit(float attackX, float attackY, int damage, bool explosive, bool fire, bool meleePierce) override {
		(void)attackY;
		if (explosive || fire || meleePierce) return Enemy::takeDamage(damage);
		if (attackY < posY + height * 0.25f) return Enemy::takeDamage(damage);

		bool attackFromRight = attackX > (posX + width * 0.5f);
		bool shieldFacingAttack = (facingRight && attackFromRight) || (!facingRight && !attackFromRight);
		if (shieldActive && shieldFacingAttack) {
			cout << "[ShieldedSoldier] Shield blocked frontal bullet. Use knife, grenade, rocket, fire, laser, or hit from behind.\n";
			return false;
		}
		return Enemy::takeDamage(damage);
	}

	void attack() override {
		if (shootCooldown > 0.0f || !playerTarget) return;
		float ox = facingRight ? posX + width : posX;
		Bullet* b = new Bullet(ox, posY + height * 0.4f, BULLET_SPEED, 0.0f, facingRight, 3, false);
		b->loadTexture("Sprites/projectiles/bullet.png");
		b->setSpriteScale(0.08f, 0.08f);
		pool.add(b);
		shootCooldown = SHOOT_INTERVAL;
		anim.switchTo(1, 2, 5.0f, false);
	}

	void update(float dt) override {
		Enemy::update(dt);
		if (shootCooldown > 0.0f) { shootCooldown -= dt; if (shootCooldown < 0.0f) shootCooldown = 0.0f; }
		if (aiState && aiState->getStateType() != 2) anim.switchTo(0, 4, 7.0f);
		pool.update(dt);
	}

	void updateProjectiles(float dt) override { pool.update(dt); }
	void renderProjectiles(RenderWindow& w, float cx, float cy) override { pool.render(w, cx, cy); }
	bool checkProjectileHits(float px, float py, float pw, float ph, int& dmgOut) override { return pool.checkHit(px, py, pw, ph, dmgOut); }

	void render(RenderWindow& window, float camX, float camY) override {
		if (!active) return;
		unsigned int texW = texture.getSize().x;
		unsigned int texH = texture.getSize().y;
		float sx = (texW > 0) ? width / (float)texW : 1.f;
		float sy = (texH > 0) ? height / (float)texH : 1.f;
		bool drawRight = !facingRight;
		if (drawRight) { sprite.setScale(sx, sy); sprite.setOrigin(0.f, 0.f); }
		else { sprite.setScale(-sx, sy); sprite.setOrigin((float)texW, 0.f); }
		sprite.setPosition(posX - camX, posY - camY);
		window.draw(sprite);
		pool.render(window, camX, camY);
	}

	~ShieldedSoldier() override {}
};
const float ShieldedSoldier::SHOOT_INTERVAL = 1.8f;
const float ShieldedSoldier::BULLET_SPEED = 340.f;


class BazookaSoldier : public Enemy {
private:
	EnemyProjectiles pool;
	float rocketCooldown;

	static const float ROCKET_INTERVAL;
	static const float ROCKET_SPEED;
	static const float BLAST_RADIUS;

public:
	BazookaSoldier(float x, float y, Level* lvl, const Entity* player) : Enemy(x, y, 132.f, 108.f, 2, TYPE_BAZOOKA, 100, 2, lvl), rocketCooldown(1.5f) {
		playerTarget = player;
		loadTexture("Sprites/enemies/bazooka.png");
		anim.configure(52, 48, 3, 0, 5.f);
		setAIState(new PatrolState(player, 30.f, 400.f, 280.f));
	}

	void attack() override {
		if (rocketCooldown > 0.f || !playerTarget) return;
		float ox = facingRight ? posX + width : posX;
		float oy = posY + height * 0.2f;
		float arcAng = 55.f;

		BallisticProjectile* rocket = new BallisticProjectile(ox, oy, ROCKET_SPEED, arcAng, facingRight, 5, BLAST_RADIUS, level, false, false);
		rocket->loadTexture("Sprites/projectiles/rocket.png");
		pool.add(rocket);
		rocketCooldown = ROCKET_INTERVAL;
		anim.switchTo(1, 2, 4.f, false);
	}

	void update(float dt) override {
		Enemy::update(dt);
		if (rocketCooldown > 0.0f) { rocketCooldown -= dt; if (rocketCooldown < 0.f) rocketCooldown = 0.0f; }
		if (aiState && aiState->getStateType() != 2) anim.switchTo(0, 3, 5.0f);
		pool.update(dt);
	}

	void render(RenderWindow& window, float camX, float camY) override {
		Enemy::render(window, camX, camY);
		pool.render(window, camX, camY);
	}

	void updateProjectiles(float dt) override { pool.update(dt); }
	void renderProjectiles(RenderWindow& w, float cx, float cy) override { pool.render(w, cx, cy); }
	bool checkProjectileHits(float px, float py, float pw, float ph, int& dmgOut) override { return pool.checkHit(px, py, pw, ph, dmgOut); }

	~BazookaSoldier() override {}
};
const float BazookaSoldier::ROCKET_INTERVAL = 3.5f;
const float BazookaSoldier::ROCKET_SPEED = 260.0f;
const float BazookaSoldier::BLAST_RADIUS = 3.0f * Block::BLOCK_SIZE;


class GrenadeSoldier : public Enemy {
private:
	EnemyProjectiles pool;
	float grenadeCooldown;

	static const float GRENADE_INTERVAL;
	static const float GRENADE_SPEED;
	static const float BLAST_RADIUS;

public:
	GrenadeSoldier(float x, float y, Level* lvl, const Entity* player) : Enemy(x, y, 118.f, 104.f, 2, TYPE_GRENADE, 100, 2, lvl), grenadeCooldown(0.8f) {
		playerTarget = player;
		loadTexture("Sprites/enemies/grenade_soldier.png");
		anim.configure(48, 48, 4, 0, 8.f);
		setAIState(new PatrolState(player, 40.f, 380.f, 200.f));
	}

	void attack() override {
		if (grenadeCooldown > 0.f || !playerTarget) return;
		float dx = playerTarget->getPosX() - posX;
		float dist = fabsf(dx);

		float angle = 65.f - (dist / 8.f);
		if (angle < 35.f) angle = 35.f;
		if (angle > 65.f) angle = 65.f;

		float ox = facingRight ? posX + width : posX;
		BallisticProjectile* gren = new BallisticProjectile(ox, posY + height * 0.2f, GRENADE_SPEED, angle, facingRight, 5, BLAST_RADIUS, level, false, false);
		gren->loadTexture("Sprites/projectiles/grenade_ball.png");
		pool.add(gren);
		grenadeCooldown = GRENADE_INTERVAL;
		anim.switchTo(1, 3, 7.0f, false);
	}

	void update(float dt) override {
		Enemy::update(dt);
		if (grenadeCooldown > 0.0f) { grenadeCooldown -= dt; if (grenadeCooldown < 0.0f) grenadeCooldown = 0.0f; }
		if (aiState && aiState->getStateType() != 2) anim.switchTo(0, 4, 8.f);
		pool.update(dt);
	}

	void render(RenderWindow& window, float camX, float camY) override {
		Enemy::render(window, camX, camY);
		pool.render(window, camX, camY);
	}

	void updateProjectiles(float dt) override { pool.update(dt); }
	void renderProjectiles(RenderWindow& w, float cx, float cy) override { pool.render(w, cx, cy); }
	bool checkProjectileHits(float px, float py, float pw, float ph, int& dmgOut) override { return pool.checkHit(px, py, pw, ph, dmgOut); }

	~GrenadeSoldier() override {}
};
const float GrenadeSoldier::GRENADE_INTERVAL = 2.8f;
const float GrenadeSoldier::GRENADE_SPEED = 220.f;
const float GrenadeSoldier::BLAST_RADIUS = 3.f * Block::BLOCK_SIZE;


class Zombie : public Enemy {
private:
	EnemyProjectiles pool;
	float shootCooldown;
	static const float WALK_SPEED;
	static const float SHOOT_INTERVAL;

public:
	Zombie(float x, float y, Level* lvl, const Entity* player) : Enemy(x, y, 90.0f, 104.0f, 5, TYPE_ZOMBIE, 100, 4, lvl), shootCooldown(2.0f) {
		playerTarget = player;
		loadTexture("Sprites/enemies/zombie.png");
		anim.configure(44, 52, 5, 0, 4.0f);
		setAIState(new ChaseState(player, WALK_SPEED, 60.0f));
	}

	bool transformsOnCollision() const { return true; }
	int getTransformType() const { return 1; }

	void attack() override {
		if (shootCooldown > 0.0f || !playerTarget) return;
		float ox = facingRight ? posX + width : posX;
		Bullet* b = new Bullet(ox, posY + height * 0.3f, 280.0f, 0.0f, facingRight, 3, false);
		b->loadTexture("Sprites/projectiles/bullet.png");
		b->setSpriteScale(0.08f, 0.08f);
		pool.add(b);
		shootCooldown = SHOOT_INTERVAL;
	}

	void update(float dt) override {
		Enemy::update(dt);
		if (shootCooldown > 0.0f) { shootCooldown -= dt; if (shootCooldown < 0.0f) shootCooldown = 0.0f; }
		if (playerTarget) {
			float dx = playerTarget->getPosX() - posX;
			float dy = playerTarget->getPosY() - posY;
			if (sqrtf(dx * dx + dy * dy) < 200.0f) attack();
		}
		pool.update(dt);
	}

	void render(RenderWindow& window, float camX, float camY) override {
		Enemy::render(window, camX, camY);
		pool.render(window, camX, camY);
	}

	void updateProjectiles(float dt) override { pool.update(dt); }
	void renderProjectiles(RenderWindow& w, float cx, float cy) override { pool.render(w, cx, cy); }
	bool checkProjectileHits(float px, float py, float pw, float ph, int& dmgOut) override
	{
		return pool.checkHit(px, py, pw, ph, dmgOut);
	}

	~Zombie() override {}
};
const float Zombie::WALK_SPEED = 38.0f;
const float Zombie::SHOOT_INTERVAL = 3.0f;


class MummyWarrior : public Enemy {
private:
	bool  crumbled;
	float resurrectTimer;
	static const float RESURRECT_TIME;
	static const float WALK_SPEED;

public:
	MummyWarrior(float x, float y, Level* lvl, const Entity* player) : Enemy(x, y, 92.0f, 106.0f, 5, TYPE_MUMMY, 150, 1, lvl), crumbled(false), resurrectTimer(0.f) {
		playerTarget = player;
		loadTexture("Sprites/enemies/mummy.png");
		anim.configure(44, 56, 5, 0, 4.0f);
		setAIState(new ChaseState(player, WALK_SPEED, 55.0f));
	}

	bool transformsOnCollision() const { return true; }
	int getTransformType() const { return 2; }

	bool takeDamage(int damage) override {
		if (crumbled) return false;
		crumbled = true;
		resurrectTimer = RESURRECT_TIME;
		anim.switchTo(1, 2, 3.f, true);
		setAIState(new IdleState(playerTarget, 0.0f));
		return false;
	}

	void takeDamageFromFire() { DamagableEntity::takeDamage(9999); }
	void takeDamageFromExplosion(int damage) { DamagableEntity::takeDamage(damage); }

	bool receivePlayerHit(float attackX, float attackY, int damage, bool explosive, bool fire, bool meleePierce) override {
		(void)attackX; (void)attackY; (void)meleePierce;
		if (fire) { takeDamageFromFire(); return true; }
		if (explosive) { takeDamageFromExplosion(damage); return true; }
		return takeDamage(damage);
	}

	void attack() override {}

	void update(float dt) override {
		if (crumbled) {
			resurrectTimer -= dt;
			if (resurrectTimer <= 0.0f) {
				crumbled = false;
				hp = maxHP;
				damageState = DAMAGE_HEALTHY;
				anim.switchTo(2, 3, 8.0f, false);
				setAIState(new ChaseState(playerTarget, WALK_SPEED, 55.f));
				cout << "Mummy Resurrected!\n";
			}
			DamagableEntity::update(dt);
			anim.update(dt, sprite);
			return;
		}
		Enemy::update(dt);
	}

	~MummyWarrior() override {}
};
const float MummyWarrior::RESURRECT_TIME = 3.0f;
const float MummyWarrior::WALK_SPEED = 30.0f;


class Martian : public Enemy {
private:
	int phase;
	float podHP;
	float hoverTargetX;
	float hoverY;
	float beamCooldown;
	EnemyProjectiles pool;

	static const float BEAM_COOLDOWN;
	static const float HOVER_SPEED;

public:
	Martian(float x, float y, Level* lvl, const Entity* player) : Enemy(x, y, 110.0f, 130.0f, 3, TYPE_MARTIAN, 200, 1, lvl), phase(1), podHP(3.0f), hoverTargetX(x), hoverY(y), beamCooldown(0.0f) {
		playerTarget = player;
		loadTexture("Sprites/enemies/martian_pod.png");
		anim.configure(56, 72, 4, 0, 8.0f);
		isOnGround = false;
	}

	bool takeDamage(int damage) override {
		if (phase == 1) {
			podHP -= damage;
			if (podHP <= 0.f) transitionToPhase2();
			return false;
		}
		return Enemy::takeDamage(damage);
	}

	void attack() override {
		if (phase == 1) fireEnergyBeam();
		else fireFootPistol();
	}

	void update(float dt) override {
		beamCooldown -= dt; if (beamCooldown < 0.f) beamCooldown = 0.f;

		if (phase == 1) {
			updatePodPhase(dt);
		}
		else {
			Enemy::update(dt);
		}
		pool.update(dt);
	}

	void render(RenderWindow& window, float camX, float camY) override {
		Enemy::render(window, camX, camY);
		pool.render(window, camX, camY);
	}

	void updateProjectiles(float dt) override { pool.update(dt); }
	void renderProjectiles(RenderWindow& w, float cx, float cy) override { pool.render(w, cx, cy); }
	bool checkProjectileHits(float px, float py, float pw, float ph, int& dmgOut) override
	{
		return pool.checkHit(px, py, pw, ph, dmgOut);
	}

	~Martian() override {}

private:
	void updatePodPhase(float dt) {
		if (!playerTarget) return;

		float targetX = playerTarget->getPosX() - width * 0.5f;
		float dx = targetX - posX;
		if (fabsf(dx) > 5.f)
			posX += (dx > 0.f ? HOVER_SPEED : -HOVER_SPEED) * dt;

		facingRight = (playerTarget->getPosX() > posX);

		float overlapX = fabsf(posX - playerTarget->getPosX());
		if (overlapX < 40.f && beamCooldown <= 0.f) {
			attack();
		}
		anim.update(dt, sprite);
	}

	void fireEnergyBeam() {
		Bullet* beam = new Bullet(posX + width * 0.5f, posY + height,
			600.f, -90.f,
			true, 3, false, 800.f);
		beam->loadTexture("Sprites/projectiles/energy_beam.png");
		pool.add(beam);
		beamCooldown = BEAM_COOLDOWN;
	}

	void fireFootPistol() {
		if (beamCooldown > 0.f || !playerTarget) return;
		float ox = facingRight ? posX + width : posX;
		Bullet* b = new Bullet(ox, posY + height * 0.4f,
			320.f, 0.f, facingRight, 3, false);
		b->loadTexture("Sprites/projectiles/bullet.png");
		b->setSpriteScale(0.08f, 0.08f);
		pool.add(b);
		beamCooldown = 1.5f;
	}

	void transitionToPhase2() {
		phase = 2;
		cout << "Phase 2: pod destroyed, Martian on foot!" << endl;
		loadTexture("Sprites/enemies/martian_foot.png");
		anim.configure(44, 52, 4, 0, 8.f);
		isOnGround = false;
		setAIState(new PatrolState(playerTarget, 55.f, 300.f, 130.f));
	}
};
const float Martian::BEAM_COOLDOWN = 2.f;
const float Martian::HOVER_SPEED = 80.f;


class FlyingTara : public Enemy {
private:
	EnemyProjectiles pool;
	float attackCooldown;
	float directionTimer;
	static const float MOVE_SPEED;
	static const float ATTACK_INTERVAL;
	static const float GRENADE_SPEED;
	static const float BLAST_RADIUS;

public:
	FlyingTara(float x, float y, Level* lvl, const Entity* player)
		: Enemy(x, y, 230.0f, 105.0f, 5, TYPE_FLYING_TARA, 250, 2, lvl),
		attackCooldown(1.2f), directionTimer(0.0f) {
		playerTarget = player;
		facingRight = false;
		loadTexture("Sprites/vehicles/slug_flyer.png");
		anim.configure(96, 64, 1, 0, 1.0f, false);
	}

	void attack() override {
		if (attackCooldown > 0.0f || !playerTarget) return;
		float ox = posX + width * 0.5f;
		float oy = posY + height * 0.85f;
		BallisticProjectile* bomb = new BallisticProjectile(ox, oy, GRENADE_SPEED, -90.0f, true, 5, BLAST_RADIUS, level, false, false);
		bomb->loadTexture("Sprites/projectiles/grenade_ball.png");
		pool.add(bomb);
		attackCooldown = ATTACK_INTERVAL;
	}

	void update(float dt) override {
		DamagableEntity::update(dt);
		if (isDead()) return;

		directionTimer += dt;
		if (directionTimer > 3.0f) {
			directionTimer = 0.0f;
			facingRight = !facingRight;
		}

		velX = facingRight ? MOVE_SPEED : -MOVE_SPEED;
		posX += velX * dt;

		if (playerTarget) {
			float dx = (playerTarget->getPosX() + playerTarget->getWidth() * 0.5f) - (posX + width * 0.5f);
			if (dx > 0.0f) facingRight = true;
			else if (dx < 0.0f) facingRight = false;
			if (fabsf(dx) < 170.0f) attack();
		}

		if (attackCooldown > 0.0f) {
			attackCooldown -= dt;
			if (attackCooldown < 0.0f) attackCooldown = 0.0f;
		}
		pool.update(dt);

		if (level) {
			float maxX = (float)(level->getWidth() * Block::BLOCK_SIZE) - width;
			if (posX < 0.0f) { posX = 0.0f; facingRight = true; }
			if (posX > maxX) { posX = maxX; facingRight = false; }
		}
		anim.update(0.0f, sprite);
	}

	void render(RenderWindow& window, float camX, float camY) override {
		Enemy::render(window, camX, camY);
		pool.render(window, camX, camY);
	}

	void updateProjectiles(float dt) override { pool.update(dt); }
	void renderProjectiles(RenderWindow& w, float cx, float cy) override { pool.render(w, cx, cy); }
	bool checkProjectileHits(float px, float py, float pw, float ph, int& dmgOut) override { return pool.checkHit(px, py, pw, ph, dmgOut); }
};
const float FlyingTara::MOVE_SPEED = 75.0f;
const float FlyingTara::ATTACK_INTERVAL = 2.2f;
const float FlyingTara::GRENADE_SPEED = 150.0f;
const float FlyingTara::BLAST_RADIUS = 2.5f * Block::BLOCK_SIZE;

class BradleyVehicle : public Enemy {
private:
	EnemyProjectiles pool;
	float rocketCooldown;
	static const float ROCKET_INTERVAL;
	static const float ROCKET_SPEED;
	static const float BLAST_RADIUS;

public:
	BradleyVehicle(float x, float y, Level* lvl, const Entity* player)
		: Enemy(x, y, 250.0f, 145.0f, 7, TYPE_BRADLEY, 350, 1, lvl), rocketCooldown(1.5f) {
		playerTarget = player;
		loadTexture("Sprites/vehicles/metal_slug.png");
		anim.configure(96, 64, 1, 0, 1.0f, false);
		setAIState(new AttackState(player, 2.0f, 900.0f));
	}

	void attack() override {
		if (rocketCooldown > 0.0f || !playerTarget) return;
		float dx = playerTarget->getPosX() - posX;
		facingRight = dx > 0.0f;
		float ox = facingRight ? posX + width - 20.0f : posX + 20.0f;
		float oy = posY + height * 0.25f;
		BallisticProjectile* rocket = new BallisticProjectile(ox, oy, ROCKET_SPEED, 42.0f, facingRight, 5, BLAST_RADIUS, level, false, false);
		rocket->loadTexture("Sprites/projectiles/rocket.png");
		pool.add(rocket);
		rocketCooldown = ROCKET_INTERVAL;
	}

	void update(float dt) override {
		Enemy::update(dt);
		setVelX(0.0f);
		if (rocketCooldown > 0.0f) {
			rocketCooldown -= dt;
			if (rocketCooldown < 0.0f) rocketCooldown = 0.0f;
		}
		pool.update(dt);
	}

	void render(RenderWindow& window, float camX, float camY) override {
		Enemy::render(window, camX, camY);
		pool.render(window, camX, camY);
	}

	void updateProjectiles(float dt) override { pool.update(dt); }
	void renderProjectiles(RenderWindow& w, float cx, float cy) override { pool.render(w, cx, cy); }
	bool checkProjectileHits(float px, float py, float pw, float ph, int& dmgOut) override { return pool.checkHit(px, py, pw, ph, dmgOut); }
};
const float BradleyVehicle::ROCKET_INTERVAL = 2.8f;
const float BradleyVehicle::ROCKET_SPEED = 270.0f;
const float BradleyVehicle::BLAST_RADIUS = 3.0f * Block::BLOCK_SIZE;

class EnemySub : public Enemy {
private:
	EnemyProjectiles pool;
	float missileCooldown;
	static const float MISSILE_INTERVAL;
	static const float MISSILE_SPEED;

public:
	EnemySub(float x, float y, Level* lvl, const Entity* player)
		: Enemy(x, y, 245.0f, 115.0f, 7, TYPE_ENEMY_SUB, 350, 1, lvl), missileCooldown(1.0f) {
		playerTarget = player;
		facingRight = false;
		loadTexture("Sprites/vehicles/slug_flyer.png");
		anim.configure(96, 64, 1, 0, 1.0f, false);
	}

	void attack() override {
		if (missileCooldown > 0.0f || !playerTarget) return;
		float dx = playerTarget->getPosX() - posX;
		facingRight = dx > 0.0f;
		float ox = facingRight ? posX + width - 10.0f : posX + 10.0f;
		float oy = posY + height * 0.45f;
		Bullet* missile = new Bullet(ox, oy, MISSILE_SPEED, 0.0f, facingRight, 4, false);
		missile->loadTexture("Sprites/projectiles/rocket.png");
		missile->setSpriteScale(0.18f, 0.18f);
		missile->setHitBox(42.0f, 24.0f);
		pool.add(missile);
		missileCooldown = MISSILE_INTERVAL;
	}

	void update(float dt) override {
		DamagableEntity::update(dt);
		if (isDead()) return;
		if (playerTarget) attack();
		if (missileCooldown > 0.0f) {
			missileCooldown -= dt;
			if (missileCooldown < 0.0f) missileCooldown = 0.0f;
		}
		pool.update(dt);
		anim.update(0.0f, sprite);
	}

	void render(RenderWindow& window, float camX, float camY) override {
		Enemy::render(window, camX, camY);
		pool.render(window, camX, camY);
	}

	void updateProjectiles(float dt) override { pool.update(dt); }
	void renderProjectiles(RenderWindow& w, float cx, float cy) override { pool.render(w, cx, cy); }
	bool checkProjectileHits(float px, float py, float pw, float ph, int& dmgOut) override { return pool.checkHit(px, py, pw, ph, dmgOut); }
};
const float EnemySub::MISSILE_INTERVAL = 2.4f;
const float EnemySub::MISSILE_SPEED = 320.0f;

class Paratrooper : public Enemy {
	int innerType;
	Enemy* innerEnemy;
	bool landed;
	EnemyProjectiles pool;

	Texture parachuteTexture;
	Sprite parachuteSprite;
	bool parachuteLoaded;

	static const float DESCENT_SPEED;

public:
	Paratrooper(float x, float y, Level* lvl, const Entity* player, int type = 0) : Enemy(x, y, 86.f, 86.f, 2, TYPE_PARA, (type == 0 ? 75 : type == 1 ? 100 : type == 2 ? 125 : 125), 3, lvl), innerType(type), innerEnemy(nullptr), landed(false), parachuteLoaded(false) {
		playerTarget = player;
		loadTexture("Sprites/enemies/paratrooper.png");
		anim.configure(48, 48, 4, 0, 8.f);

		parachuteLoaded = parachuteTexture.loadFromFile("Sprites/enemies/parachute.png");
		if (parachuteLoaded) {
			parachuteSprite.setTexture(parachuteTexture);
			parachuteSprite.setOrigin(parachuteTexture.getSize().x * 0.5f, 0.f);
		}

		isOnGround = false;
		setAIState(new DescentState(player, DESCENT_SPEED));
	}

	void attack() override {
		if (innerEnemy) innerEnemy->attack();
	}

	void update(float dt) override {
		Enemy::update(dt);

		if (!landed && isOnGround) {
			landed = true;
			createInnerEnemy();
			cout << "Paratrooper Landed! Becoming infantry type " << innerType << "\n";
		}

		if (landed && innerEnemy) {
			innerEnemy->setPosition(posX, posY);
			innerEnemy->update(dt);
		}
	}

	void render(RenderWindow& window, float camX, float camY) override {
		if (!landed && parachuteLoaded) {
			parachuteSprite.setPosition(posX + width * 0.5f - camX, posY - 40.f - camY);
			window.draw(parachuteSprite);
		}
		Enemy::render(window, camX, camY);

		if (landed && innerEnemy)
			innerEnemy->renderProjectiles(window, camX, camY);
	}

	void updateProjectiles(float dt) override {
		if (landed && innerEnemy) innerEnemy->updateProjectiles(dt);
	}
	void renderProjectiles(RenderWindow& w, float cx, float cy) override {
		if (landed && innerEnemy) innerEnemy->renderProjectiles(w, cx, cy);
	}
	bool checkProjectileHits(float px, float py, float pw, float ph, int& dmgOut) override {
		if (landed && innerEnemy) return innerEnemy->checkProjectileHits(px, py, pw, ph, dmgOut);
		return false;
	}

	~Paratrooper() override {
		delete innerEnemy;
		innerEnemy = nullptr;
	}

private:
	void createInnerEnemy() {
		switch (innerType) {
		case 0: innerEnemy = new RebelSoldier(posX, posY, level, playerTarget); break;
		case 1: innerEnemy = new ShieldedSoldier(posX, posY, level, playerTarget); break;
		case 2: innerEnemy = new BazookaSoldier(posX, posY, level, playerTarget); break;
		case 3: innerEnemy = new GrenadeSoldier(posX, posY, level, playerTarget); break;
		default: innerEnemy = new RebelSoldier(posX, posY, level, playerTarget); break;
		}
	}
};
const float Paratrooper::DESCENT_SPEED = 240.f;


class EnemyManager {
private:
	static const int MAX_ENEMIES = 512;
	Enemy* enemies[MAX_ENEMIES];
	int count;
	int bonusScoreBuffer;

	void addKillBonuses(Enemy* e, bool explosive, bool meleeAttack, int killsInSameBlast = 0) {
		if (!e) return;
		if (meleeAttack) bonusScoreBuffer += 50;
		if (!e->getIsOnGround()) bonusScoreBuffer += 100;
		if (explosive && killsInSameBlast == 2) bonusScoreBuffer += 200;
		else if (explosive && killsInSameBlast >= 3) bonusScoreBuffer += 300 + (killsInSameBlast - 3) * 50;
	}

public:
	EnemyManager() : count(0), bonusScoreBuffer(0) {
		for (int i = 0; i < MAX_ENEMIES; i++) enemies[i] = nullptr;
	}

	void add(Enemy* e) {
		if (count >= MAX_ENEMIES) { delete e; return; }
		enemies[count++] = e;
	}

	void spawnFromLevel(Level* lvl, const Entity* player) {
		clear();
		int spawnCount = 0;
		const SpawnPoint* pts = lvl->getSpawnPoints(spawnCount);

		for (int i = 0; i < spawnCount; i++) {
			const SpawnPoint& sp = pts[i];
			for (int b = 0; b < sp.batch && count < MAX_ENEMIES; b++) {
				float sx = sp.worldX + b * (Block::BLOCK_SIZE * 0.6f);
				float sy = sp.worldY;
				Enemy* e = createEnemy(sp.type, sx, sy, lvl, player);
				if (e) add(e);
			}
		}
		cout << "EnemyManager Spawned " << count << " enemies.\n";
	}

	void update(float dt, int& scoreOut) {
		scoreOut = 0;
		for (int i = 0; i < count; ) {
			enemies[i]->update(dt);

			if (!enemies[i]->isActive()) {
				scoreOut += enemies[i]->getScoreValue();
				delete enemies[i];
				enemies[i] = enemies[--count];
				enemies[count] = nullptr;
			}
			else { i++; }
		}
	}

	void update(float dt) { int dummy = 0; update(dt, dummy); }

	void render(RenderWindow& window, float camX, float camY) {
		for (int i = 0; i < count; i++) {
			enemies[i]->render(window, camX, camY);
			enemies[i]->renderProjectiles(window, camX, camY);
		}
	}

	bool checkHitsOnPlayer(float px, float py, float pw, float ph, int& dmgOut) {
		for (int i = 0; i < count; i++) {
			if (enemies[i]->checkProjectileHits(px, py, pw, ph, dmgOut))
				return true;
		}
		dmgOut = 0;
		return false;
	}

	void checkBodyCollisionsOnPlayer(Entity* player, GameStats* stats, int charIdx);

	Enemy* getEnemyAt(int idx) const {
		if (idx < 0 || idx >= count) return nullptr;
		return enemies[idx];
	}

	bool damageEnemyAtPoint(float x, float y, int damage) {
		for (int i = 0; i < count; i++) {
			Enemy* e = enemies[i];
			if (!e || !e->isActive()) continue;
			if (e->containsPointInsideEntity(x, y)) {
				bool wasActive = e->isActive();
				e->receivePlayerHit(x, y, damage, false, false, false);
				if (wasActive && !e->isActive()) addKillBonuses(e, false, false);
				return true;
			}
		}
		return false;
	}

	bool damageEnemyRectEx(float x, float y, float w, float h, int damage, bool explosive, bool fire, bool meleePierce, bool meleeAttack = false) {
		const float HIT_MARGIN = 18.0f;
		float hx = x - HIT_MARGIN;
		float hy = y - HIT_MARGIN;
		float hw = w + HIT_MARGIN * 2.0f;
		float hh = h + HIT_MARGIN * 2.0f;
		float attackX = x + w * 0.5f;
		float attackY = y + h * 0.5f;
		for (int i = 0; i < count; i++) {
			Enemy* e = enemies[i];
			if (!e || !e->isActive()) continue;
			if (e->overlaps(hx, hy, hw, hh)) {
				bool wasActive = e->isActive();
				e->receivePlayerHit(attackX, attackY, damage, explosive, fire, meleePierce);
				if (wasActive && !e->isActive()) addKillBonuses(e, explosive, meleeAttack);
				return true;
			}
		}
		return false;
	}

	bool damageEnemyRect(float x, float y, float w, float h, int damage) {
		return damageEnemyRectEx(x, y, w, h, damage, false, false, false);
	}

	void applyBlast(float cx, float cy, float radius, int damage) {
		Enemy* killed[32];
		int killCount = 0;
		for (int i = 0; i < count; i++) {
			Enemy* e = enemies[i];
			if (!e->isActive()) continue;
			float dx = (e->getPosX() + e->getWidth() * 0.5f) - cx;
			float dy = (e->getPosY() + e->getHeight() * 0.5f) - cy;
			float dist = sqrtf(dx * dx + dy * dy);
			if (dist <= radius) {
				bool wasActive = e->isActive();
				e->receivePlayerHit(cx, cy, damage, true, false, false);
				if (wasActive && !e->isActive() && killCount < 32) killed[killCount++] = e;
			}
		}
		for (int k = 0; k < killCount; k++) addKillBonuses(killed[k], true, false, killCount);
	}

	void applyFireDamage(float cx, float cy, float range, int dmgPerSec, float dt) {
		(void)dt;
		for (int i = 0; i < count; i++) {
			Enemy* e = enemies[i];
			if (!e->isActive()) continue;
			float dx = (e->getPosX() + e->getWidth() * 0.5f) - cx;
			float dy = (e->getPosY() + e->getHeight() * 0.5f) - cy;
			if (sqrtf(dx * dx + dy * dy) > range) continue;
			bool wasActive = e->isActive();
			e->receivePlayerHit(cx, cy, dmgPerSec, false, true, false);
			if (wasActive && !e->isActive()) addKillBonuses(e, false, false);
		}
	}

	void setPlayerTarget(const Entity* player) {
		for (int i = 0; i < count; i++)
			enemies[i]->setPlayerTarget(player);
	}

	int getCount() const { return count; }

	int consumeBonusScore() {
		int s = bonusScoreBuffer;
		bonusScoreBuffer = 0;
		return s;
	}

	void clear() {
		for (int i = 0; i < count; i++) { delete enemies[i]; enemies[i] = nullptr; }
		count = 0;
	}

	~EnemyManager() { clear(); }

private:
	Enemy* createEnemy(int type, float x, float y, Level* lvl, const Entity* player) {
		switch (type) {
		case SpawnPoint::T_REBEL: return new RebelSoldier(x, y, lvl, player);
		case SpawnPoint::T_SHIELDED: return new ShieldedSoldier(x, y, lvl, player);
		case SpawnPoint::T_BAZOOKA: return new BazookaSoldier(x, y, lvl, player);
		case SpawnPoint::T_GRENADE: return new GrenadeSoldier(x, y, lvl, player);
		case SpawnPoint::T_ZOMBIE: return new Zombie(x, y, lvl, player);
		case SpawnPoint::T_MUMMY: return new MummyWarrior(x, y, lvl, player);
		case SpawnPoint::T_MARTIAN: return new Martian(x, y, lvl, player);
		case SpawnPoint::T_FLYING_TARA: return new FlyingTara(x, y, lvl, player);
		case SpawnPoint::T_BRADLEY: return new BradleyVehicle(x, y, lvl, player);
		case SpawnPoint::T_ENEMY_SUB: return new EnemySub(x, y, lvl, player);
		case SpawnPoint::T_PARATROOPER: {
			int inner = (int)(x * 1.3f + y * 0.7f) % 4;
			return new Paratrooper(x, -420.f, lvl, player, inner);
		}
		default: return nullptr;
		}
	}
};


class Vehicle : public Entity {
public:
	static const int VDMG_HEALTHY = 0;
	static const int VDMG_INJURED = 1;
	static const int VDMG_CRITICAL = 2;
	static const int VDMG_DEAD = 3;
	static const int SHOT_CAP = 96;

protected:
	int hp;
	int maxHP;
	int damageState;
	float stateTimer;
	float velX;
	float velY;
	bool isOccupied;
	Level* level;
	Projectile* vehicleShots[SHOT_CAP];

	static const float STATE_WINDOW;
	static const float GRAVITY;
	static const float MAX_FALL_SPEED;

public:
	Vehicle(float x, float y, float w, float h, int mHP, Level* lvl)
		: Entity(x, y, w, h), hp(mHP), maxHP(mHP), damageState(VDMG_HEALTHY),
		stateTimer(0.f), velX(0.f), velY(0.f), isOccupied(false), level(lvl) {
		for (int i = 0; i < SHOT_CAP; i++) vehicleShots[i] = nullptr;
	}

	int getHP() const { return hp; }
	int getDamageState() const { return damageState; }
	bool isDead() const { return damageState == VDMG_DEAD; }
	bool getIsOccupied() const { return isOccupied; }
	void setOccupied(bool o) { isOccupied = o; }

	virtual bool takeDamage(int damage) {
		if (damageState == VDMG_DEAD) return false;

		if (damage <= 0) damage = 1;
		hp -= damage;
		if (hp < 0) hp = 0;

		if (damageState == VDMG_HEALTHY) {
			damageState = VDMG_INJURED;
			stateTimer = STATE_WINDOW;
		}
		else if (damageState == VDMG_INJURED) {
			damageState = VDMG_CRITICAL;
			stateTimer = STATE_WINDOW;
		}
		else if (damageState == VDMG_CRITICAL) {
			damageState = VDMG_DEAD;
			stateTimer = 0.f;
			onDestroyed();
			return true;
		}

		if (hp <= 0 && damageState != VDMG_DEAD) {
			damageState = VDMG_DEAD;
			stateTimer = 0.f;
			onDestroyed();
			return true;
		}
		return false;
	}

	int getDamageOverlayAlpha() const {
		if (damageState == VDMG_INJURED) return 70;
		if (damageState == VDMG_CRITICAL) return 140;
		return 0;
	}

	bool addVehicleShot(Projectile* p) {
		if (!p) return false;
		for (int i = 0; i < SHOT_CAP; i++) {
			if (vehicleShots[i] == nullptr || !vehicleShots[i]->isActive()) {
				delete vehicleShots[i];
				vehicleShots[i] = p;
				return true;
			}
		}
		delete p;
		return false;
	}

	void fireVehicleBullet(float x, float y, bool facingRight, int damage, float speed) {
		Bullet* b = new Bullet(x, y, speed, 0.0f, facingRight, damage, true);
		b->loadTexture("Sprites/projectiles/bullet.png");
		b->setSpriteScale(0.08f, 0.08f);
		b->setHitBox(38.0f, 20.0f);
		addVehicleShot(b);
	}

	void updateVehicleShots(float dt) {
		for (int i = 0; i < SHOT_CAP; i++) {
			Projectile* p = vehicleShots[i];
			if (!p) continue;
			if (!p->isActive()) { delete vehicleShots[i]; vehicleShots[i] = nullptr; continue; }

			p->update(dt);

			if (level && p->isActive()) {
				Block* hit = level->getBlockAtWorld(p->getPosX(), p->getPosY());
				if (isSolidBlock(hit)) p->onHit();
			}

			if (!p->isActive()) {
				delete vehicleShots[i];
				vehicleShots[i] = nullptr;
			}
		}
	}

	void damageProjectiles(EnemyManager* enemyMgr) {
		if (!enemyMgr) return;
		for (int i = 0; i < SHOT_CAP; i++) {
			Projectile* p = vehicleShots[i];
			if (!p || !p->isActive()) continue;
			float x = p->getPosX() - p->getHitW() * 0.5f;
			float y = p->getPosY() - p->getHitH() * 0.5f;
			if (enemyMgr->damageEnemyRect(x, y, p->getHitW(), p->getHitH(), p->getDamage())) {
				p->onHit();
				delete vehicleShots[i];
				vehicleShots[i] = nullptr;
			}
		}
	}

	virtual void update(float dt) override {
		if (damageState != VDMG_HEALTHY && damageState != VDMG_DEAD) {
			stateTimer -= dt;
			if (stateTimer < 0.f) {
				stateTimer = 0.f;
				damageState = VDMG_HEALTHY;
			}
		}
		anim.update(0.0f, sprite);
		updateVehicleShots(dt);
	}

	virtual void render(RenderWindow& window, float camX, float camY) override {
		if (!active) return;
		sprite.setPosition(posX - camX, posY - camY);
		window.draw(sprite);

		int alpha = getDamageOverlayAlpha();
		if (alpha > 0) {
			RectangleShape flash;
			flash.setSize(Vector2f(width, height));
			flash.setFillColor(Color(255, 0, 0, alpha));
			flash.setPosition(posX - camX, posY - camY);
			window.draw(flash);
		}

		for (int i = 0; i < SHOT_CAP; i++) {
			if (vehicleShots[i] && vehicleShots[i]->isActive())
				vehicleShots[i]->render(window, camX, camY);
		}
	}

	virtual void renderVehicleShots(RenderWindow& window, float camX, float camY) {
		for (int i = 0; i < SHOT_CAP; i++) {
			if (vehicleShots[i] && vehicleShots[i]->isActive())
				vehicleShots[i]->render(window, camX, camY);
		}
	}

	virtual void fireWeapon(float angleDeg) = 0;

	virtual ~Vehicle() {
		for (int i = 0; i < SHOT_CAP; i++) {
			delete vehicleShots[i];
			vehicleShots[i] = nullptr;
		}
	}

protected:
	virtual void onDestroyed() { active = false; }
};
const float Vehicle::STATE_WINDOW = 1.0f;
const float Vehicle::GRAVITY = 1800.0f;
const float Vehicle::MAX_FALL_SPEED = 900.0f;


class MetalSlug : public Vehicle {
private:
	InputManager* inputMgr;
	float tiltAngle;
	bool facingRight;
	bool isOnGround;
	float fireCooldown;

	static const float FIRE_RATE;
	static const float BULLET_SPEED;
	static const float MOVE_SPEED;
	static const float JUMP_SPEED;

public:
	MetalSlug(float x, float y, Level* lvl) : Vehicle(x, y, 280.f, 170.f, 8, lvl), inputMgr(nullptr), tiltAngle(0.f), facingRight(true), isOnGround(false), fireCooldown(0.f) {
		loadTexture("Sprites/vehicles/metal_slug.png");
		anim.configure(96, 64, 4, 0, 8.0f, true);
	}

	void setInputManager(InputManager* im) { inputMgr = im; }
	float getTiltAngle()  const { return tiltAngle; }
	bool getFacingRight()const { return facingRight; }

	void fireWeapon(float angleDeg) override {
		(void)angleDeg;
		if (fireCooldown > 0.0f) return;
		float bx = facingRight ? posX + width - 12.0f : posX + 12.0f;
		float by = posY + height * 0.42f;
		fireVehicleBullet(bx, by, facingRight, 3, BULLET_SPEED);
		fireCooldown = 1.0f / FIRE_RATE;
	}

	void update(float dt) override {
		Vehicle::update(dt);
		if (isDead()) return;

		if (fireCooldown > 0.f) {
			fireCooldown -= dt;
			if (fireCooldown < 0.f) fireCooldown = 0.f;
		}


		if (!isOccupied) {
			if (!isOnGround) {
				velY += GRAVITY * dt;
				if (velY > MAX_FALL_SPEED) velY = MAX_FALL_SPEED;
			}
			posY += velY * dt;
			if (level) groundCollision();
			return;
		}

		if (inputMgr) {
			if (inputMgr->isKeyDown(Keyboard::Right)) {
				velX = MOVE_SPEED;
				facingRight = true;
				anim.switchTo(1, 6, 12.f);
			}
			else if (inputMgr->isKeyDown(Keyboard::Left)) {
				velX = -MOVE_SPEED;
				facingRight = false;
				anim.switchTo(1, 6, 12.f);
			}
			else {
				velX = 0.f;
				anim.switchTo(0, 4, 8.f);
			}

			if (inputMgr->isKeyPressed(Keyboard::Space) && isOnGround) {
				velY = JUMP_SPEED;
				isOnGround = false;
			}

			if (inputMgr->isKeyDown(Keyboard::Up)) {
				tiltAngle += 90.f * dt;
				if (tiltAngle > 90.f) tiltAngle = 90.f;
			}

			if (inputMgr->isKeyDown(Keyboard::Down)) {
				tiltAngle -= 90.f * dt;
				if (tiltAngle < 0.f) tiltAngle = 0.f;
			}

			bool wantFire = inputMgr->isMouseLeftDown()
				|| inputMgr->isKeyDown(Keyboard::X);
			if (wantFire) fireWeapon(tiltAngle);
		}

		if (!isOnGround) {
			velY += GRAVITY * dt;
			if (velY > MAX_FALL_SPEED) velY = MAX_FALL_SPEED;
		}

		posX += velX * dt;
		posY += velY * dt;

		if (level) {
			groundCollision();
			wallCollision();
		}

		if (posX < 0.f) posX = 0.f;
		if (level) {
			float rightEdge = (float)(level->getWidth() * Block::BLOCK_SIZE) - width;
			if (posX > rightEdge) posX = rightEdge;
		}
	}

	void render(RenderWindow& window, float camX, float camY) override {
		if (!active) return;
		anim.update(0.0f, sprite);
		float frameW = (float)texture.getSize().x;
		float frameH = (float)texture.getSize().y;
		float sx = (frameW > 0.0f) ? width / frameW : 1.0f;
		float sy = (frameH > 0.0f) ? height / frameH : 1.0f;

		if (facingRight) {
			sprite.setScale(sx, sy);
			sprite.setOrigin(0.0f, 0.0f);
		}
		else {
			sprite.setScale(-sx, sy);
			sprite.setOrigin(frameW, 0.0f);
		}

		sprite.setPosition(posX - camX, posY - camY);
		window.draw(sprite);

		int alpha = getDamageOverlayAlpha();
		if (alpha > 0) {
			RectangleShape flash;
			flash.setSize(Vector2f(width, height));
			flash.setFillColor(Color(255, 0, 0, alpha));
			flash.setPosition(posX - camX, posY - camY);
			window.draw(flash);
		}

		renderVehicleShots(window, camX, camY);
	}

	~MetalSlug() override {}

private:
	void groundCollision() {
		float checkY = posY + height + 3.0f;
		Block* bLeft = level->getBlockAtWorld(posX + 10.0f, checkY);
		Block* bMid = level->getBlockAtWorld(posX + width * 0.5f, checkY);
		Block* bRight = level->getBlockAtWorld(posX + width - 10.0f, checkY);
		bool touching = isSolidBlock(bLeft) || isSolidBlock(bMid) || isSolidBlock(bRight);

		if (touching && velY >= 0.f) {
			int row = (int)(checkY / (float)Block::BLOCK_SIZE);
			posY = (float)(row * Block::BLOCK_SIZE) - height;
			velY = 0.f;
			isOnGround = true;
		}
		else {
			isOnGround = false;
		}
	}

	void wallCollision() {
		float checkY1 = posY + height * 0.3f;
		float checkY2 = posY + height * 0.7f;

		if (velX < 0.f) {
			Block* wL1 = level->getBlockAtWorld(posX - 2.0f, checkY1);
			Block* wL2 = level->getBlockAtWorld(posX - 2.0f, checkY2);
			if (isSolidBlock(wL1) || isSolidBlock(wL2)) {
				int col = (int)((posX - 2.f) / (float)Block::BLOCK_SIZE);
				posX = (float)((col + 1) * Block::BLOCK_SIZE);
				velX = 0.0f;
			}
		}
		if (velX > 0.f) {
			Block* wR1 = level->getBlockAtWorld(posX + width + 2.0f, checkY1);
			Block* wR2 = level->getBlockAtWorld(posX + width + 2.0f, checkY2);
			if (isSolidBlock(wR1) || isSolidBlock(wR2)) {
				int col = (int)((posX + width + 2.0f) / (float)Block::BLOCK_SIZE);
				posX = (float)(col * Block::BLOCK_SIZE) - width;
				velX = 0.0f;
			}
		}
	}
};
const float MetalSlug::FIRE_RATE = 8.f;
const float MetalSlug::BULLET_SPEED = 900.f;
const float MetalSlug::MOVE_SPEED = 180.f;
const float MetalSlug::JUMP_SPEED = -550.f;


class SlugFlyer : public Vehicle {
	InputManager* inputMgr;
	bool facingRight;
	float fireCooldown;
	int missileCount;
	float missileCooldown;

	static const float BULLET_SPEED;
	static const float FIRE_RATE;
	static const float FLY_SPEED;
	static const int MAX_MISSILES;

public:
	SlugFlyer(float x, float y, Level* lvl) : Vehicle(x, y, 280.f, 160.f, 6, lvl), inputMgr(nullptr), facingRight(true), fireCooldown(0.f), missileCount(4), missileCooldown(0.f) {
		loadTexture("Sprites/vehicles/slug_flyer.png");
		anim.configure(96, 48, 4, 0, 12.0f, true);
	}

	void setInputManager(InputManager* im) { inputMgr = im; }
	int getMissileCount() const { return missileCount; }

	void fireWeapon(float angleDeg) override {
		(void)angleDeg;
		if (fireCooldown > 0.f) return;
		float bx = facingRight ? posX + width - 12.0f : posX + 12.0f;
		float by = posY + height * 0.48f;
		fireVehicleBullet(bx, by, facingRight, 3, BULLET_SPEED);
		fireCooldown = 1.f / FIRE_RATE;
	}

	void fireMissile() {
		if (missileCount <= 0 || missileCooldown > 0.f) return;
		missileCount--;
		float bx = facingRight ? posX + width - 10.0f : posX + 10.0f;
		float by = posY + height * 0.52f;
		Bullet* m = new Bullet(bx, by, 620.0f, 0.0f, facingRight, 5, true, 1200.0f);
		m->loadTexture("Sprites/projectiles/missile.png");
		m->setSpriteScale(0.22f, 0.22f);
		m->setHitBox(46.0f, 22.0f);
		addVehicleShot(m);
		missileCooldown = 1.5f;
	}

	void update(float dt) override {
		Vehicle::update(dt);
		if (isDead()) return;

		if (fireCooldown > 0.f) { fireCooldown -= dt; if (fireCooldown < 0.f) fireCooldown = 0.f; }
		if (missileCooldown > 0.f) { missileCooldown -= dt; if (missileCooldown < 0.f) missileCooldown = 0.f; }

		if (!isOccupied) {
			velX = 0.f;
			velY = 0.f;
			return;
		}

		velX = 0.f;
		velY = 0.f;

		if (inputMgr) {
			if (inputMgr->isKeyDown(Keyboard::Right)) { velX = FLY_SPEED; facingRight = true;  anim.switchTo(1, 4, 10.f); }
			if (inputMgr->isKeyDown(Keyboard::Left)) { velX = -FLY_SPEED; facingRight = false; anim.switchTo(1, 4, 10.f); }
			if (inputMgr->isKeyDown(Keyboard::Up)) { velY = -FLY_SPEED; }
			if (inputMgr->isKeyDown(Keyboard::Down)) { velY = FLY_SPEED; }

			if (!inputMgr->isKeyDown(Keyboard::Left) && !inputMgr->isKeyDown(Keyboard::Right))
				anim.switchTo(0, 4, 12.f);

			if (inputMgr->isMouseLeftDown() || inputMgr->isKeyDown(Keyboard::X)) fireWeapon(0.f);

			if (inputMgr->isMouseRightDown()) fireMissile();
		}

		posX += velX * dt;
		posY += velY * dt;

		if (posY < 0.f) posY = 0.f;
		if (level) {
			float maxY = (float)(level->getHeight() * Block::BLOCK_SIZE) - height;
			if (posY > maxY) posY = maxY;
			if (posX < 0.f) posX = 0.f;
			float maxX = (float)(level->getWidth() * Block::BLOCK_SIZE) - width;
			if (posX > maxX) posX = maxX;
		}
	}

	void render(RenderWindow& window, float camX, float camY) override {
		if (!active) return;
		anim.update(0.0f, sprite);
		float frameW = (float)texture.getSize().x;
		float frameH = (float)texture.getSize().y;
		float sx = (frameW > 0.0f) ? width / frameW : 1.0f;
		float sy = (frameH > 0.0f) ? height / frameH : 1.0f;

		if (facingRight) {
			sprite.setScale(sx, sy);
			sprite.setOrigin(0.0f, 0.0f);
		}
		else {
			sprite.setScale(-sx, sy);
			sprite.setOrigin(frameW, 0.0f);
		}

		sprite.setPosition(posX - camX, posY - camY);
		window.draw(sprite);

		int alpha = getDamageOverlayAlpha();
		if (alpha > 0) {
			RectangleShape flash;
			flash.setSize(Vector2f(width, height));
			flash.setFillColor(Color(255, 0, 0, alpha));
			flash.setPosition(posX - camX, posY - camY);
			window.draw(flash);
		}

		renderVehicleShots(window, camX, camY);
	}

	~SlugFlyer() override {}
};
const float SlugFlyer::BULLET_SPEED = 450.f;
const float SlugFlyer::FIRE_RATE = 4.f;
const float SlugFlyer::FLY_SPEED = 220.f;
const int SlugFlyer::MAX_MISSILES = 4;


class SlugMariner : public Vehicle {
	InputManager* inputMgr;
	bool facingRight;
	float fireCooldown;
	int horizontalMissiles;
	int verticalMissiles;
	int surfaceMissiles;
	static const float MOVE_SPEED;
public:
	SlugMariner(float x, float y, Level* lvl) : Vehicle(x, y, 280.f, 150.f, 7, lvl), inputMgr(nullptr), facingRight(true), fireCooldown(0.f), horizontalMissiles(3), verticalMissiles(3), surfaceMissiles(3) {
		loadTexture("Sprites/vehicles/slug_mariner.png");
		anim.configure(96, 48, 1, 0, 1.0f, false);
	}
	void setInputManager(InputManager* im) { inputMgr = im; }
	void fireWeapon(float angleDeg) override {
		(void)angleDeg;
		if (fireCooldown > 0.0f || horizontalMissiles <= 0) return;
		horizontalMissiles--;
		float bx = facingRight ? posX + width - 8.0f : posX + 8.0f;
		float by = posY + height * 0.5f;
		Bullet* m = new Bullet(bx, by, 620.0f, 0.0f, facingRight, 5, true, 1200.0f);
		m->loadTexture("Sprites/projectiles/missile.png");
		m->setSpriteScale(0.22f, 0.22f);
		m->setHitBox(44.0f, 22.0f);
		addVehicleShot(m);
		fireCooldown = 0.8f;
	}
	void fireVerticalMissile() {
		if (fireCooldown > 0.0f || verticalMissiles <= 0) return;
		verticalMissiles--;
		Bullet* m = new Bullet(posX + width * 0.5f, posY + 8.0f, 580.0f, 90.0f, true, 5, true, 800.0f);
		m->loadTexture("Sprites/projectiles/missile.png"); m->setSpriteScale(0.22f, 0.22f); m->setHitBox(24.0f, 42.0f);
		addVehicleShot(m); fireCooldown = 0.9f;
	}
	void fireSurfaceMissile() {
		if (fireCooldown > 0.0f || surfaceMissiles <= 0) return;
		surfaceMissiles--;
		float bx = facingRight ? posX + width - 8.0f : posX + 8.0f;
		BallisticProjectile* m = new BallisticProjectile(bx, posY + height * 0.35f, 520.0f, 62.0f, facingRight, 5, 2.5f * Block::BLOCK_SIZE, level, true, false);
		m->loadTexture("Sprites/projectiles/missile.png"); m->setSpriteScale(0.22f, 0.22f); m->setHitBox(36.0f, 22.0f);
		addVehicleShot(m); fireCooldown = 1.0f;
	}
	void update(float dt) override {
		Vehicle::update(dt);
		if (isDead()) return;
		if (fireCooldown > 0.0f) { fireCooldown -= dt; if (fireCooldown < 0.0f) fireCooldown = 0.0f; }
		if (!isOccupied) { velX = 0.0f; velY = 0.0f; return; }
		velX = velY = 0.0f;
		if (inputMgr) {
			if (inputMgr->isKeyDown(Keyboard::Right)) { velX = MOVE_SPEED; facingRight = true; }
			if (inputMgr->isKeyDown(Keyboard::Left)) { velX = -MOVE_SPEED; facingRight = false; }
			if (inputMgr->isKeyDown(Keyboard::Up)) velY = -MOVE_SPEED;
			if (inputMgr->isKeyDown(Keyboard::Down)) velY = MOVE_SPEED;
			if (inputMgr->isKeyDown(Keyboard::X) || inputMgr->isMouseLeftDown()) fireWeapon(0.0f);
			if (inputMgr->isMouseRightDown()) fireVerticalMissile();
			if (inputMgr->isKeyPressed(Keyboard::C)) fireSurfaceMissile();
		}
		posX += velX * dt; posY += velY * dt;
		if (level) {
			if (posX < 0.f) posX = 0.f;
			float maxX = (float)(level->getWidth() * Block::BLOCK_SIZE) - width;
			float maxY = (float)(level->getHeight() * Block::BLOCK_SIZE) - height;
			if (posX > maxX) posX = maxX; if (posY < 0.f) posY = 0.f; if (posY > maxY) posY = maxY;
		}
	}
	void render(RenderWindow& window, float camX, float camY) override {
		if (!active) return;
		float fw = (float)texture.getSize().x, fh = (float)texture.getSize().y;
		float sx = fw > 0.f ? width / fw : 1.f, sy = fh > 0.f ? height / fh : 1.f;
		if (facingRight) { sprite.setScale(sx, sy); sprite.setOrigin(0.f, 0.f); }
		else { sprite.setScale(-sx, sy); sprite.setOrigin(fw, 0.f); }
		sprite.setPosition(posX - camX, posY - camY); window.draw(sprite);
		renderVehicleShots(window, camX, camY);
	}
};
const float SlugMariner::MOVE_SPEED = 230.f;

class AmphibiousSlug : public Vehicle {
	InputManager* inputMgr;
	bool facingRight;
	bool isOnGround;
	float fireCooldown;
	int mode;
	static const float MOVE_SPEED;
public:
	AmphibiousSlug(float x, float y, Level* lvl) : Vehicle(x, y, 300.f, 165.f, 10, lvl), inputMgr(nullptr), facingRight(true), isOnGround(false), fireCooldown(0.f), mode(0) {
		loadTexture("Sprites/vehicles/amphibious_slug.png");
		anim.configure(96, 64, 1, 0, 1.0f, false);
	}
	void setInputManager(InputManager* im) { inputMgr = im; }
	int getMode() const { return mode; }
	void fireWeapon(float angleDeg) override {
		(void)angleDeg;
		if (fireCooldown > 0.0f) return;
		float bx = facingRight ? posX + width - 10.f : posX + 10.f;
		float by = posY + height * 0.48f;
		if (mode == 2) {
			Bullet* m = new Bullet(bx, by, 620.0f, 0.0f, facingRight, 5, true, 1200.0f);
			m->loadTexture("Sprites/projectiles/missile.png");
			m->setSpriteScale(0.22f, 0.22f);
			m->setHitBox(44.f, 22.f);
			addVehicleShot(m);
			fireCooldown = 0.9f;
		}
		else {
			fireVehicleBullet(bx, by, facingRight, 3, mode == 1 ? 450.0f : 880.0f);
			fireCooldown = mode == 1 ? 0.22f : 0.13f;
		}
	}
	void updateMode() {
		mode = 1;
		if (level) {
			Block* center = level->getBlockAtWorld(posX + width * 0.5f, posY + height * 0.6f);
			Block* foot = level->getBlockAtWorld(posX + width * 0.5f, posY + height + 3.f);
			if (center && center->getHasWater()) mode = 2;
			else if (isSolidBlock(foot)) mode = 0;
		}
	}
	void update(float dt) override {
		Vehicle::update(dt);
		if (isDead()) return;
		if (fireCooldown > 0.f) { fireCooldown -= dt; if (fireCooldown < 0.f) fireCooldown = 0.f; }
		updateMode();
		if (!isOccupied) { velX = 0.f; velY = 0.f; return; }
		velX = velY = 0.f;
		if (inputMgr) {
			if (inputMgr->isKeyDown(Keyboard::Right)) { velX = MOVE_SPEED; facingRight = true; }
			if (inputMgr->isKeyDown(Keyboard::Left)) { velX = -MOVE_SPEED; facingRight = false; }
			if (mode != 0) { if (inputMgr->isKeyDown(Keyboard::Up)) velY = -MOVE_SPEED; if (inputMgr->isKeyDown(Keyboard::Down)) velY = MOVE_SPEED; }
			else { if (inputMgr->isKeyPressed(Keyboard::Space)) velY = -520.f; }
			if (inputMgr->isKeyDown(Keyboard::X) || inputMgr->isMouseLeftDown()) fireWeapon(0.f);
		}
		if (mode == 0 && velY == 0.f) velY += GRAVITY * dt;
		posX += velX * dt; posY += velY * dt;
		if (level) {
			if (posX < 0.f) posX = 0.f;
			float maxX = (float)(level->getWidth() * Block::BLOCK_SIZE) - width;
			float maxY = (float)(level->getHeight() * Block::BLOCK_SIZE) - height;
			if (posX > maxX) posX = maxX; if (posY < 0.f) posY = 0.f; if (posY > maxY) posY = maxY;
		}
	}
	void render(RenderWindow& window, float camX, float camY) override {
		if (!active) return;
		float fw = (float)texture.getSize().x, fh = (float)texture.getSize().y;
		float sx = fw > 0.f ? width / fw : 1.f, sy = fh > 0.f ? height / fh : 1.f;
		if (facingRight) { sprite.setScale(sx, sy); sprite.setOrigin(0.f, 0.f); }
		else { sprite.setScale(-sx, sy); sprite.setOrigin(fw, 0.f); }
		sprite.setPosition(posX - camX, posY - camY); window.draw(sprite);
		renderVehicleShots(window, camX, camY);
	}
};
const float AmphibiousSlug::MOVE_SPEED = 220.f;


class PlayerSoldier;

class Collectible : public Entity {
protected:
	bool collected;

public:
	Collectible(float x, float y, float w, float h) : Entity(x, y, w, h), collected(false) {}

	bool isCollected() const { return collected; }
	virtual void pickup(PlayerSoldier* player) = 0;

	void update(float dt) override { (void)dt; }

	void render(RenderWindow& window, float camX, float camY) override {
		if (!active || collected) return;
		sprite.setPosition(posX - camX, posY - camY);
		window.draw(sprite);
	}

	virtual ~Collectible() {}
};


class HUD {
private:
	GameStats* stats;
	Font font;
	bool fontLoaded;

	Texture overlayTexture;
	Sprite  overlaySprite;

	static const int SCREEN_W = 1600;
	static const int SCREEN_H = 900;

public:
	HUD(GameStats* gs) : stats(gs), fontLoaded(false) {
		fontLoaded = font.loadFromFile("Sprites/font.ttf");
		if (!fontLoaded) cout << "font.ttf not found\n";
		Image img;
		img.create(1, 1, Color(220, 0, 0, 80));
		overlayTexture.loadFromImage(img);
		overlaySprite.setTexture(overlayTexture);
		overlaySprite.setScale((float)SCREEN_W, (float)SCREEN_H);
		overlaySprite.setPosition(0.0f, 0.0f);
	}

	void render(RenderWindow& window, int activeCharIdx) {
		drawHealthBar(window, activeCharIdx);
		drawAmmoAndGrenades(window, activeCharIdx);
		drawScore(window);
		drawLives(window, activeCharIdx);
		drawDamageOverlay(window, activeCharIdx);
	}

public:
	void drawHealthBar(RenderWindow& window, int idx) {
		if (!fontLoaded) return;

		int hp = stats->getHP(idx);
		int maxHP = stats->getMaxHP(idx);

		Text label;
		label.setFont(font);
		label.setCharacterSize(20);
		label.setFillColor(Color::White);
		label.setString("HP: " + to_string(hp) + "/" + to_string(maxHP));
		label.setPosition(20.0f, 20.0f);
		window.draw(label);
	}

	void drawAmmoAndGrenades(RenderWindow& window, int idx) {
		if (!fontLoaded) return;

		Text ammoText, grenText;
		ammoText.setFont(font); ammoText.setCharacterSize(20);
		ammoText.setFillColor(Color::Yellow);
		int ammo = stats->getAmmo(idx);
		ammoText.setString(ammo < 0 ? "AMMO: INF" : "AMMO: " + to_string(ammo));
		ammoText.setPosition(20.f, 50.f);
		window.draw(ammoText);

		grenText.setFont(font); grenText.setCharacterSize(20);
		grenText.setFillColor(Color(255, 165, 0));
		grenText.setString("BOMB: " + to_string(stats->getGrenades(idx)));
		grenText.setPosition(20.f, 80.f);
		window.draw(grenText);
	}

	void drawScore(RenderWindow& window) {
		if (!fontLoaded) return;
		Text scoreText;
		scoreText.setFont(font);
		scoreText.setCharacterSize(24);
		scoreText.setFillColor(Color::White);
		scoreText.setString("SCORE: " + to_string(stats->getScore()));
		scoreText.setPosition((float)(SCREEN_W - 220), 20.0f);
		window.draw(scoreText);
	}

	void drawLives(RenderWindow& window, int idx) {
		if (!fontLoaded) return;
		Text livesText;
		livesText.setFont(font);
		livesText.setCharacterSize(20);
		livesText.setFillColor(Color::White);
		livesText.setString("LIVES: " + to_string(stats->getLives(idx)));
		livesText.setPosition(20.0f, 110.0f);
		window.draw(livesText);
	}

	void drawDamageOverlay(RenderWindow& window, int idx) {
		int dmgState = stats->getDamageState(idx);
		if (dmgState == GameStats::DAMAGE_HEALTHY) return;

		unsigned char alpha = 0;
		if (dmgState == GameStats::DAMAGE_INJURED)  alpha = 80;
		if (dmgState == GameStats::DAMAGE_CRITICAL) alpha = 160;

		overlaySprite.setColor(Color(255, 0, 0, alpha));
		window.draw(overlaySprite);
	}
};


class Camera {
private:
	float viewX;
	float viewY;
	int viewW;
	int viewH;
	float worldW;
	float worldH;

	const Entity* target;
	float smoothFactor;

public:
	Camera() : viewX(0.0f), viewY(0.0f), viewW(1600), viewH(900), worldW(7040.0f), worldH(896.0f), target(nullptr), smoothFactor(0.1f) {}

	Camera(int vw, int vh, float ww, float wh) : viewX(0.f), viewY(0.f), viewW(vw), viewH(vh), worldW(ww), worldH(wh), target(nullptr), smoothFactor(0.1f) {}

	float getViewX() const { return viewX; }
	float getViewY() const { return viewY; }
	int getViewW() const { return viewW; }
	int getViewH() const { return viewH; }
	void setWorldBounds(float ww, float wh) { worldW = ww; worldH = wh; }
	void setSmoothFactor(float s) { smoothFactor = s; }
	void setTarget(const Entity* e) { target = e; }

	void update(float dt) {
		if (target == nullptr) return;
		float PlayerCenterScreenX = target->getPosX() - (float)viewW * 0.5f;
		float PlayerCenterScreenY = target->getPosY() - (float)viewH * 0.5f;

		viewX += (PlayerCenterScreenX - viewX) * smoothFactor;
		viewY += (PlayerCenterScreenY - viewY) * smoothFactor;

		if (viewX < 0.f) viewX = 0.f;
		if (viewY < 0.f) viewY = 0.f;
		if (viewX > worldW - viewW) viewX = worldW - (float)viewW;
		if (viewY > worldH - viewH) viewY = worldH - (float)viewH;

		(void)dt;
	}

	void snapToTarget() {
		if (target == nullptr) return;
		viewX = target->getPosX() - (float)viewW * 0.5f;
		viewY = target->getPosY() - (float)viewH * 0.5f;

		if (viewX < 0.f) viewX = 0.f;
		if (viewY < 0.f) viewY = 0.f;
		if (viewX > worldW - viewW) viewX = worldW - (float)viewW;
		if (viewY > worldH - viewH) viewY = worldH - (float)viewH;
	}

	float worldToScreenX(float wx) const { return wx - viewX; }
	float worldToScreenY(float wy) const { return wy - viewY; }
	float screenToWorldX(float sx) const { return sx + viewX; }
	float screenToWorldY(float sy) const { return sy + viewY; }

	bool isInView(float wx, float wy, float ww, float wh) const {
		return !(wx + ww < viewX || wx > viewX + viewW || wy + wh < viewY || wy > viewY + viewH);
	}

	~Camera() {}
};

class TransformationState {
public:
	static const int STATE_NORMAL = 0;
	static const int STATE_UNDEAD = 1;
	static const int STATE_MUMMY = 2;
	static const float DURATION;

private:
	int currentState;
	float stateTimer;
	int savedWeaponType;

public:
	TransformationState() : currentState(STATE_NORMAL), stateTimer(0.0f), savedWeaponType(0) {}

	int   getState() const { return currentState; }
	float getTimer() const { return stateTimer; }
	int   getSavedWeapon() const { return savedWeaponType; }
	bool  isTransformed() const { return currentState != STATE_NORMAL; }
	bool  isUndead() const { return currentState == STATE_UNDEAD; }
	bool  isMummy() const { return currentState == STATE_MUMMY; }

	void transformTo(int newState, int currentWeapon = 0) {
		if (newState == STATE_NORMAL) return;
		currentState = newState;
		stateTimer = DURATION;

		if (newState == STATE_MUMMY) { savedWeaponType = currentWeapon; }
	}

	void update(float dt) {
		if (currentState == STATE_NORMAL) return;
		stateTimer -= dt;

		if (stateTimer <= 0.0f) {
			stateTimer = 0.0f;
			currentState = STATE_NORMAL;
		}
	}

	void revertToNormal() {
		currentState = STATE_NORMAL;
		stateTimer = 0.0f;
	}

	~TransformationState() {}
};
const float TransformationState::DURATION = 10.0f;

class Soldier : public DamagableEntity {
protected:
	float velX;
	float velY;
	bool facingRight;
	bool isOnGround;
	int grenadeCount;
	int weaponType;

	TransformationState* transformState;
	static const float GRAVITY;
	static const float MAX_FALL_SPEED;

public:
	Soldier() : DamagableEntity(), velX(0.f), velY(0.f), facingRight(true), isOnGround(false), grenadeCount(10), weaponType(0) {
		transformState = new TransformationState();
	}

	Soldier(float x, float y, float w, float h, int mHP) : DamagableEntity(x, y, w, h, mHP), velX(0.f), velY(0.f), facingRight(true), isOnGround(false), grenadeCount(10), weaponType(0) {
		transformState = new TransformationState();
	}

	float getVelX() const { return velX; }
	float getVelY() const { return velY; }
	bool getFacingRight() const { return facingRight; }
	bool getIsOnGround() const { return isOnGround; }
	int getGrenadeCount() const { return grenadeCount; }
	int getWeaponType() const { return weaponType; }
	TransformationState* getTransformState() { return transformState; }

	void setGrenadeCount(int n) { grenadeCount = n; }
	void setWeaponType(int t) { weaponType = t; }
	void setFacingRight(bool f) { facingRight = f; }

	virtual void update(float dt) override {
		DamagableEntity::update(dt);
		transformState->update(dt);
	}

	virtual void meleeAttack() = 0;
	virtual void PowerUp() = 0;
	virtual void throwGrenade() {
		if (grenadeCount <= 0) return;
		grenadeCount--;
	}

	virtual ~Soldier() {
		delete transformState;
		transformState = nullptr;
	}
};
const float Soldier::GRAVITY = 1800.f;
const float Soldier::MAX_FALL_SPEED = 900.f;


class PlayerProjectilePool {
public:
	static const int CAP = 160;
	static const int BLAST_CAP = 48;
	static const int FIRE_CAP = 32;
	static const int LASER_CAP = 8;

private:
	Projectile* shots[CAP];

	float pendingBlastX[BLAST_CAP];
	float pendingBlastY[BLAST_CAP];
	float pendingBlastRadius[BLAST_CAP];
	int pendingBlastDamage[BLAST_CAP];
	bool pendingBlastFire[BLAST_CAP];
	int pendingBlastCount;

	float fireX[FIRE_CAP];
	float fireY[FIRE_CAP];
	float fireRadius[FIRE_CAP];
	float fireTimer[FIRE_CAP];
	float fireTick[FIRE_CAP];
	int fireDamage[FIRE_CAP];

	float laserX[LASER_CAP];
	float laserY[LASER_CAP];
	float laserW[LASER_CAP];
	float laserH[LASER_CAP];
	float laserTimer[LASER_CAP];

	void rememberBlast(float x, float y, float radius, int damage, bool fireBomb) {
		if (pendingBlastCount >= BLAST_CAP) return;
		pendingBlastX[pendingBlastCount] = x;
		pendingBlastY[pendingBlastCount] = y;
		pendingBlastRadius[pendingBlastCount] = radius;
		pendingBlastDamage[pendingBlastCount] = damage;
		pendingBlastFire[pendingBlastCount] = fireBomb;
		pendingBlastCount++;
	}

	void addFirePool(float x, float y, float radius, float seconds, int dmgPerPulse) {
		for (int i = 0; i < FIRE_CAP; i++) {
			if (fireTimer[i] <= 0.0f) {
				fireX[i] = x; fireY[i] = y; fireRadius[i] = radius;
				fireTimer[i] = seconds; fireTick[i] = 0.0f; fireDamage[i] = dmgPerPulse;
				return;
			}
		}
	}

	void addLaserBeam(float x, float y, float w, float h) {
		for (int i = 0; i < LASER_CAP; i++) {
			if (laserTimer[i] <= 0.0f) {
				laserX[i] = x; laserY[i] = y; laserW[i] = w; laserH[i] = h; laserTimer[i] = 0.12f;
				return;
			}
		}
	}

public:
	PlayerProjectilePool() : pendingBlastCount(0) {
		for (int i = 0; i < CAP; i++) shots[i] = nullptr;
		for (int i = 0; i < FIRE_CAP; i++) { fireTimer[i] = 0.0f; fireTick[i] = 0.0f; }
		for (int i = 0; i < LASER_CAP; i++) laserTimer[i] = 0.0f;
	}

	bool add(Projectile* p) {
		if (!p) return false;
		for (int i = 0; i < CAP; i++) {
			if (shots[i] == nullptr || !shots[i]->isActive()) {
				delete shots[i];
				shots[i] = p;
				return true;
			}
		}
		delete p;
		return false;
	}

	void fireBullet(float x, float y, bool facingRight, int damage, float speed = 800.0f) {
		Bullet* b = new Bullet(x, y, speed, 0.0f, facingRight, damage, true);
		b->loadTexture("Sprites/projectiles/bullet.png");
		b->setSpriteScale(0.08f, 0.08f);
		b->setHitBox(34.0f, 18.0f);
		add(b);
	}

	void throwGrenade(float x, float y, bool facingRight, Level* level, int damage = 20, float speed = 430.0f, float angle = 45.0f, float radius = 3.0f * Block::BLOCK_SIZE) {
		BallisticProjectile* g = new BallisticProjectile(x, y, speed, angle, facingRight, damage, radius, level, true, false);
		g->loadTexture("Sprites/projectiles/grenade_ball.png");
		g->setSpriteScale(0.22f, 0.22f);
		g->setHitBox(22.0f, 22.0f);
		add(g);
	}

	void throwFireBomb(float x, float y, bool facingRight, Level* level, int damage = 20, float speed = 430.0f, float angle = 45.0f, float radius = 3.0f * Block::BLOCK_SIZE) {
		BallisticProjectile* g = new BallisticProjectile(x, y, speed, angle, facingRight, damage, radius, level, true, true);
		g->loadTexture("Sprites/projectiles/fire_bomb.png");
		g->setSpriteScale(0.25f, 0.25f);
		g->setHitBox(24.0f, 24.0f);
		add(g);
	}

	void fireRocket(float x, float y, bool facingRight, Level* level) {
		BallisticProjectile* r = new BallisticProjectile(x, y, 680.0f, 8.0f, facingRight, 5, 3.0f * Block::BLOCK_SIZE, level, true, false);
		r->loadTexture("Sprites/projectiles/rocket.png");
		r->setSpriteScale(0.20f, 0.20f);
		r->setHitBox(34.0f, 20.0f);
		add(r);
	}

	void fireFlameStream(float x, float y, bool facingRight) {
		float range = 5.0f * Block::BLOCK_SIZE;
		float cx = facingRight ? x + range * 0.5f : x - range * 0.5f;
		addFirePool(cx, y, range * 0.55f, 0.18f, 2);
	}

	void fireLaser(float x, float y, bool facingRight, float screenRange = 1800.0f) {
		float lx = facingRight ? x : x - screenRange;
		addLaserBeam(lx, y - 8.0f, screenRange, 16.0f);
	}

	void update(float dt, Level* level) {
		for (int i = 0; i < FIRE_CAP; i++) {
			if (fireTimer[i] > 0.0f) {
				fireTimer[i] -= dt;
				fireTick[i] -= dt;
				if (fireTimer[i] < 0.0f) fireTimer[i] = 0.0f;
			}
		}
		for (int i = 0; i < LASER_CAP; i++) {
			if (laserTimer[i] > 0.0f) { laserTimer[i] -= dt; if (laserTimer[i] < 0.0f) laserTimer[i] = 0.0f; }
		}

		for (int i = 0; i < CAP; i++) {
			Projectile* p = shots[i];
			if (!p) continue;

			if (!p->isActive()) {
				BallisticProjectile* bp = dynamic_cast<BallisticProjectile*>(p);
				if (bp) rememberBlast(p->getPosX(), p->getPosY(), bp->getBlastRadius(), p->getDamage(), bp->getIsFireBomb());
				delete shots[i]; shots[i] = nullptr; continue;
			}

			p->update(dt);

			if (!p->isActive()) {
				BallisticProjectile* bp = dynamic_cast<BallisticProjectile*>(p);
				if (bp) rememberBlast(p->getPosX(), p->getPosY(), bp->getBlastRadius(), p->getDamage(), bp->getIsFireBomb());
				delete shots[i]; shots[i] = nullptr; continue;
			}

			if (level) {
				Block* hit = level->getBlockAtWorld(p->getPosX(), p->getPosY());
				if (isSolidBlock(hit)) {
					BallisticProjectile* bp = dynamic_cast<BallisticProjectile*>(p);
					if (bp) rememberBlast(p->getPosX(), p->getPosY(), bp->getBlastRadius(), p->getDamage(), bp->getIsFireBomb());
					p->onHit(); delete shots[i]; shots[i] = nullptr;
				}
			}
		}
	}

	void damageEnemies(EnemyManager* enemyMgr) {
		if (!enemyMgr) return;

		for (int b = 0; b < pendingBlastCount; b++) {
			enemyMgr->applyBlast(pendingBlastX[b], pendingBlastY[b], pendingBlastRadius[b], pendingBlastDamage[b]);
			if (pendingBlastFire[b]) addFirePool(pendingBlastX[b], pendingBlastY[b], pendingBlastRadius[b], 10.0f, 2);
		}
		pendingBlastCount = 0;

		for (int i = 0; i < FIRE_CAP; i++) {
			if (fireTimer[i] <= 0.0f) continue;
			if (fireTick[i] <= 0.0f) {
				enemyMgr->applyFireDamage(fireX[i], fireY[i], fireRadius[i], fireDamage[i], 1.0f);
				fireTick[i] = 0.5f;
			}
		}

		for (int i = 0; i < LASER_CAP; i++) {
			if (laserTimer[i] <= 0.0f) continue;
			enemyMgr->damageEnemyRectEx(laserX[i], laserY[i], laserW[i], laserH[i], 999, true, false, false);
		}

		for (int i = 0; i < CAP; i++) {
			Projectile* p = shots[i];
			if (!p || !p->isActive()) continue;

			float x = p->getPosX() - p->getHitW() * 0.5f;
			float y = p->getPosY() - p->getHitH() * 0.5f;
			BallisticProjectile* bp = dynamic_cast<BallisticProjectile*>(p);
			bool explosive = (bp != nullptr);
			bool fire = (bp != nullptr && bp->getIsFireBomb());

			if (enemyMgr->damageEnemyRectEx(x, y, p->getHitW(), p->getHitH(), p->getDamage(), explosive, fire, false)) {
				if (bp) enemyMgr->applyBlast(p->getPosX(), p->getPosY(), bp->getBlastRadius(), p->getDamage());
				if (bp && bp->getIsFireBomb()) addFirePool(p->getPosX(), p->getPosY(), bp->getBlastRadius(), 10.0f, 2);
				p->onHit(); delete shots[i]; shots[i] = nullptr;
			}
		}
	}

	void render(RenderWindow& window, float camX, float camY) {
		(void)camX;
		(void)camY;
		for (int i = 0; i < LASER_CAP; i++) {
			if (laserTimer[i] <= 0.0f) continue;
			RectangleShape beam;
			beam.setSize(Vector2f(laserW[i], laserH[i]));
			beam.setFillColor(Color(80, 240, 255, 180));
			beam.setPosition(laserX[i] - camX, laserY[i] - camY);
			window.draw(beam);
		}
		for (int i = 0; i < CAP; i++)
			if (shots[i] && shots[i]->isActive()) shots[i]->render(window, camX, camY);
	}

	~PlayerProjectilePool() {
		for (int i = 0; i < CAP; i++) { delete shots[i]; shots[i] = nullptr; }
	}
};

class PlayerSoldier : public Soldier {
protected:
	Level* level;
	InputManager* inputMgr;
	float moveSpeed;
	float jumpSpeed;
	float spriteScale;
	GameStats* gameStats;
	int charIndex;
	PlayerProjectilePool playerProjectiles;
	float baseFireCooldown;
	int weaponAmmo;
	bool meleeQueued;
	bool meleePierceQueued;
	int meleeQueuedDamage;

	bool powerUpActive;
	float powerUpTimer;
	static const float POWER_UP_DURATION;

public:
	PlayerSoldier() : Soldier(), level(nullptr), inputMgr(nullptr), moveSpeed(200.0f), jumpSpeed(-650.0f), spriteScale(0.2f), gameStats(nullptr), charIndex(0), baseFireCooldown(0.f), weaponAmmo(-1), meleeQueued(false), meleePierceQueued(false), meleeQueuedDamage(2), powerUpActive(false), powerUpTimer(0.0f) {
		width = 72.f;
		height = 92.f;
		anim.configure(48, 64, 1, 0, 1.f);

	}

	PlayerSoldier(float x, float y, int maxHitPoints = 100) : Soldier(x, y, 132.f, 102.f, maxHitPoints), level(nullptr), inputMgr(nullptr), moveSpeed(200.f), jumpSpeed(-650.f), spriteScale(0.2f), gameStats(nullptr), charIndex(0), baseFireCooldown(0.f), weaponAmmo(-1), meleeQueued(false), meleePierceQueued(false), meleeQueuedDamage(2), powerUpActive(false), powerUpTimer(0.f) {
		width = 132.f;
		height = 102.f;
		anim.configure(48, 64, 1, 0, 1.f);
	}


	~PlayerSoldier() override {}

	bool isPowerUpActive() const { return powerUpActive; }
	float getPowerUpTimer() const { return powerUpTimer; }
	float getMoveSpeed() const { return moveSpeed; }
	float getJumpSpeed() const { return jumpSpeed; }

	void setMoveSpeed(float s) { moveSpeed = s; }
	void setJumpSpeed(float s) { jumpSpeed = s; }
	void setLevel(Level* lvl) { level = lvl; }
	void setInputManager(InputManager* im) { inputMgr = im; }

	bool loadSprite(const char* path) {
		bool ok = loadTexture(path);
		if (ok) fitSprite();
		return ok;
	}

	void update(float dt) override {
		if (isDead()) return;
		if (!active) return;
		Soldier::update(dt);

		if (powerUpActive) {
			powerUpTimer -= dt;
			if (powerUpTimer <= 0.f) {
				powerUpTimer = 0.f;
				powerUpActive = false;
				onPowerUpEnd();
			}
		}

		if (inputMgr != nullptr) {
			float currentSpeed = moveSpeed;
			if (transformState->isUndead()) currentSpeed *= 0.5f;

			if (transformState->isMummy()) weaponType = 5;

			if (inputMgr->isKeyDown(Keyboard::Right)) {
				velX = currentSpeed;
				facingRight = true;
			}
			else if (inputMgr->isKeyDown(Keyboard::Left)) {
				velX = -currentSpeed;
				facingRight = false;
			}
			else velX = 0.f;

			if (inputMgr->isKeyPressed(Keyboard::Space) && isOnGround) {
				velY = jumpSpeed;
				isOnGround = false;
			}

			if (inputMgr->isKeyPressed(sf::Keyboard::G)) throwGrenade();
			if (inputMgr->isKeyPressed(sf::Keyboard::F)) meleeAttack();
		}

		if (!isOnGround) {
			velY += GRAVITY * dt;

			if (velY > MAX_FALL_SPEED) velY = MAX_FALL_SPEED;
		}

		posX += velX * dt;
		posY += velY * dt;

		if (level != nullptr)
		{
			groundCollision();
			wallCollision();
			ceilingCollision();
		}

		if (posX < 0.f) posX = 0.f;

		if (level != nullptr) {
			float worldRight = (float)(level->getWidth() * Block::BLOCK_SIZE) - width;
			if (posX > worldRight) posX = worldRight;
		}

		if (level != nullptr) {
			float worldBottom = (float)(level->getHeight() * Block::BLOCK_SIZE);
			if (posY > worldBottom) takeDamage(999);
		}

		if (baseFireCooldown > 0.0f) {
			baseFireCooldown -= dt;
			if (baseFireCooldown < 0.0f) baseFireCooldown = 0.0f;
		}
		playerProjectiles.update(dt, level);
	}

	void render(RenderWindow& window, float camX, float camY) override {
		if (!active) return;

		anim.update(0.0f, sprite);
		float frameW = (float)texture.getSize().x;
		float frameH = (float)texture.getSize().y;

		if (frameW > 0.0f && frameH > 0.0f) {
			float sx = width / frameW;
			float sy = height / frameH;

			if (facingRight) {
				sprite.setScale(sx, sy);
				sprite.setOrigin(0.f, 0.f);
			}
			else {
				sprite.setScale(-sx, sy);
				sprite.setOrigin(frameW, 0.f);
			}
		}

		sprite.setPosition(posX - camX, posY - camY);
		window.draw(sprite);
		playerProjectiles.render(window, camX, camY);
	}

	void queueMelee(bool pierceShield, int damage) {
		meleeQueued = true;
		meleePierceQueued = pierceShield;
		meleeQueuedDamage = damage;
	}

	bool consumeMelee(float& x, float& y, float& w, float& h, int& damage, bool& pierceShield) {
		if (!meleeQueued) return false;
		meleeQueued = false;
		float range = 70.0f;
		w = range; h = height * 0.75f;
		x = facingRight ? posX + width - 6.0f : posX - range + 6.0f;
		y = posY + height * 0.15f;
		damage = meleeQueuedDamage;
		pierceShield = meleePierceQueued;
		return true;
	}

	void meleeAttack() override {
		queueMelee(false, 2);
		cout << "[Player] Knife slash queued (2HP).\n";
	}

	void PowerUp() override {
		powerUpActive = true;
		powerUpTimer = POWER_UP_DURATION;
		cout << "[Player] PowerUp activated for " << POWER_UP_DURATION << "s\n";
	}

	void setGameStats(GameStats* gs, int idx) { gameStats = gs; charIndex = idx; }
	int getCharIndex() const { return charIndex; }
	GameStats* getGameStats() const { return gameStats; }

	void addSaturation(int amount) {
		if (gameStats) gameStats->addStat(charIndex, GameStats::STAT_SATURATION, amount);
	}

	void addGrenades(int amount) {
		if (gameStats) gameStats->addStat(charIndex, GameStats::STAT_GRENADES, amount);
	}

	void addAmmo(int amount) {
		if (gameStats) gameStats->addStat(charIndex, GameStats::STAT_AMMO, amount);
	}

	virtual void giveWeapon(int type, int ammo) {
		weaponType = type;
		weaponAmmo = ammo;
		if (gameStats) { gameStats->setWeaponType(charIndex, type); gameStats->setAmmo(charIndex, ammo < 0 ? 0 : ammo); }
	}

	int getEquippedWeaponType() const { return weaponType; }
	int getEquippedWeaponAmmo() const { return weaponAmmo; }

	void revertToPistolIfEmpty() {
		if (weaponAmmo == 0 && weaponType != Weapon::WEAPON_PISTOL && weaponType != Weapon::WEAPON_KNIFE) {
			weaponType = Weapon::WEAPON_PISTOL;
			weaponAmmo = -1;
			if (gameStats) { gameStats->setWeaponType(charIndex, Weapon::WEAPON_PISTOL); gameStats->setAmmo(charIndex, 0); }
		}
	}

	virtual void fireEquippedWeapon(float fireRateMultiplier = 1.0f, bool dualDirections = false) {
		if (baseFireCooldown > 0.0f) return;
		if (transformState->isMummy()) { meleeAttack(); return; }

		float ox = facingRight ? posX + width - 8.0f : posX + 8.0f;
		float oy = posY + height * 0.45f;
		float rate = 4.0f * fireRateMultiplier;
		if (rate < 0.2f) rate = 0.2f;

		if (weaponType == Weapon::WEAPON_HMG) {
			playerProjectiles.fireBullet(ox, oy, facingRight, 3, 900.0f);
			if (dualDirections) playerProjectiles.fireBullet(posX + width * 0.5f, oy, !facingRight, 3, 900.0f);
			baseFireCooldown = 1.0f / (8.0f * fireRateMultiplier);
			if (weaponAmmo > 0) weaponAmmo--;
		}
		else if (weaponType == Weapon::WEAPON_ROCKET) {
			playerProjectiles.fireRocket(ox, oy, facingRight, level);
			baseFireCooldown = 2.0f / fireRateMultiplier;
			if (weaponAmmo > 0) weaponAmmo--;
		}
		else if (weaponType == Weapon::WEAPON_FLAME) {
			playerProjectiles.fireFlameStream(ox, oy, facingRight);
			baseFireCooldown = 0.12f / fireRateMultiplier;
			if (weaponAmmo > 0) weaponAmmo--;
		}
		else if (weaponType == Weapon::WEAPON_LASER) {
			playerProjectiles.fireLaser(ox, oy, facingRight);
			baseFireCooldown = 1.0f / fireRateMultiplier;
			if (weaponAmmo > 0) weaponAmmo--;
		}
		else {
			playerProjectiles.fireBullet(ox, oy, facingRight, 3, 820.0f);
			if (dualDirections) playerProjectiles.fireBullet(posX + width * 0.5f, oy, !facingRight, 3, 820.0f);
			baseFireCooldown = 1.0f / rate;
		}
		revertToPistolIfEmpty();
		if (gameStats) gameStats->setAmmo(charIndex, weaponAmmo < 0 ? 0 : weaponAmmo);
	}

	virtual void fireBasicBullet(int damage = 3, float cooldown = 0.25f) {
		if (baseFireCooldown > 0.0f) return;
		float bx = facingRight ? posX + width - 8.0f : posX + 8.0f;
		float by = posY + height * 0.45f;
		playerProjectiles.fireBullet(bx, by, facingRight, damage, 820.0f);
		baseFireCooldown = cooldown;
	}

	void throwGrenade() override {
		int available = gameStats ? gameStats->getGrenades(charIndex) : grenadeCount;
		if (available <= 0) return;
		if (gameStats) gameStats->subtractStat(charIndex, GameStats::STAT_GRENADES, 1);
		else grenadeCount--;
		float gx = facingRight ? posX + width - 10.0f : posX + 10.0f;
		float gy = posY + height * 0.35f;
		playerProjectiles.throwGrenade(gx, gy, facingRight, level, 20, 430.0f, 45.0f);
	}

	virtual void damageProjectiles(EnemyManager* enemyMgr) {
		playerProjectiles.damageEnemies(enemyMgr);
	}


protected:
	virtual void onPowerUpEnd() {}
private:
	void groundCollision() {
		float checkY = posY + height + 3.f;
		float checkLeft = posX + 8.f;
		float checkMid = posX + width * 0.5f;
		float checkRight = posX + width - 8.f;

		Block* bLeft = level->getBlockAtWorld(checkLeft, checkY);
		Block* bMid = level->getBlockAtWorld(checkMid, checkY);
		Block* bRight = level->getBlockAtWorld(checkRight, checkY);

		bool onBlock = isSolidBlock(bLeft) || isSolidBlock(bMid) || isSolidBlock(bRight);
		if (onBlock && velY >= 0.0f) {
			int blockRow = (int)(checkY / (float)Block::BLOCK_SIZE);
			posY = (float)(blockRow * Block::BLOCK_SIZE) - height;
			velY = 0.0f;
			isOnGround = true;
		}
		else {
			isOnGround = false;
		}
	}

	void wallCollision() {
		float checkY1 = posY + height * 0.3f;
		float checkY2 = posY + height * 0.7f;

		if (velX < 0.0f) {
			Block* wL1 = level->getBlockAtWorld(posX - 2.f, checkY1);
			Block* wL2 = level->getBlockAtWorld(posX - 2.f, checkY2);
			if (isSolidBlock(wL1) || isSolidBlock(wL2)) {
				int col = (int)((posX - 2.0f) / (float)Block::BLOCK_SIZE);
				posX = (float)((col + 1) * Block::BLOCK_SIZE);
				velX = 0.0f;
			}
		}

		if (velX > 0.f) {
			Block* wR1 = level->getBlockAtWorld(posX + width + 2.0f, checkY1);
			Block* wR2 = level->getBlockAtWorld(posX + width + 2.0f, checkY2);
			if (isSolidBlock(wR1) || isSolidBlock(wR2)) {
				int col = (int)((posX + width + 2.0f) / (float)Block::BLOCK_SIZE);
				posX = (float)(col * Block::BLOCK_SIZE) - width;
				velX = 0.00f;
			}
		}
	}

	void ceilingCollision() {
		if (velY >= 0.0f) return;

		float centerX = posX + width * 0.5f;
		float topY = posY - 2.0f;

		Block* ceiling = level->getBlockAtWorld(centerX, topY);
		if (isSolidBlock(ceiling)) {
			int row = (int)(topY / (float)Block::BLOCK_SIZE);
			posY = (float)((row + 1) * Block::BLOCK_SIZE);
			velY = 0.0f;
		}
	}
};
const float PlayerSoldier::POWER_UP_DURATION = 10.f;

class Turkey : public Collectible {
private:
	static const int SATURATION_AMOUNT = 3;
	float bobTimer;
	float baseY;

public:
	Turkey(float x, float y) : Collectible(x, y, 32.f, 32.f), bobTimer(0.f), baseY(y) {
		loadTexture("Sprites/collectibles/turkey.png");
		anim.configure(32, 32, 4, 0, 6.f, true);
	}

	void pickup(PlayerSoldier* player) override {
		if (collected) return;
		collected = true;
		active = false;

		player->addSaturation(SATURATION_AMOUNT);
	}

	void update(float dt) override {
		(void)dt;
		if (collected || !active) return;
		posY = baseY;
		anim.update(0.0f, sprite);
	}

	~Turkey() override {}
};


class Fruit : public Collectible {
private:
	static const int SATURATION_AMOUNT = 2;
	float bobTimer;
	float baseY;

public:
	Fruit(float x, float y) : Collectible(x, y, 32.f, 32.f), bobTimer(0.f), baseY(y) {
		loadTexture("Sprites/collectibles/fruit.png");
		anim.configure(32, 32, 4, 0, 6.f, true);
	}

	void pickup(PlayerSoldier* player) override {
		if (collected) return;
		collected = true;
		active = false;
		player->addSaturation(SATURATION_AMOUNT);
	}

	void update(float dt) override {
		(void)dt;
		if (collected || !active) return;
		posY = baseY;
		anim.update(0.0f, sprite);
	}

	~Fruit() override {}
};


class SupplyCrate : public Collectible {
private:
	int grenadesInside;
	int ammoInside;
	int weaponInside;

public:
	SupplyCrate(float x, float y) : Collectible(x, y, 32.f, 32.f)
	{
		loadTexture("Sprites/collectibles/crate.png");
		anim.configure(32, 32, 1, 0, 1.f, false);

		grenadesInside = 3 + (int)(((float)rand() / RAND_MAX) * 3.f);
		ammoInside = 20 + (int)(((float)rand() / RAND_MAX) * 30.f);
		int roll = rand() % 10;
		if (roll == 0) weaponInside = Weapon::WEAPON_LASER;
		else if (roll < 4) weaponInside = Weapon::WEAPON_HMG;
		else if (roll < 7) weaponInside = Weapon::WEAPON_ROCKET;
		else weaponInside = Weapon::WEAPON_FLAME;
	}

	void pickup(PlayerSoldier* player) override {
		if (collected) return;
		collected = true;
		active = false;

		player->addAmmo(ammoInside);
		player->addGrenades(grenadesInside);
		player->giveWeapon(weaponInside, ammoInside);
	}

	void update(float dt) override { (void)dt; }

	~SupplyCrate() override {}
};


class POWPrisoner : public Collectible {
private:
	bool droppedCrate;
	SupplyCrate* crate;
	float animTimer;

	static const float PROXIMITY_RANGE;

public:
	POWPrisoner(float x, float y) : Collectible(x, y, 32.f, 48.f), droppedCrate(false), crate(nullptr), animTimer(0.f) {
		loadTexture("Sprites/collectibles/pow.png");
		anim.configure(32, 48, 2, 0, 2.f, true);
	}

	bool isPlayerNear(float playerX, float playerY) const {
		float dx = playerX - posX;
		float dy = playerY - posY;
		return (dx * dx + dy * dy) < (PROXIMITY_RANGE * PROXIMITY_RANGE);
	}

	void pickup(PlayerSoldier* player) override {
		if (collected) return;
		collected = true;
		active = false;
		droppedCrate = true;

		crate = new SupplyCrate(posX, posY - 32.f);
		(void)player;
	}

	bool hasDroppedCrate() const { return droppedCrate; }
	SupplyCrate* getCrate() {
		droppedCrate = false;
		return crate;
	}

	void update(float dt) override {
		if (collected || !active) return;
		animTimer += dt;
		anim.update(dt, sprite);
	}

	~POWPrisoner() override {
		if (droppedCrate && crate) { delete crate; crate = nullptr; }
	}
};
const float POWPrisoner::PROXIMITY_RANGE = 80.f;


class Pistol : public Weapon
{
protected:
	bool infiniteAmmo;

public:
	Pistol() : Weapon(3, -1, 4.0f, WEAPON_PISTOL)
	{
		infiniteAmmo = true;
	}

	Pistol(int dmg, float fireRatePerSec) : Weapon(dmg, -1, fireRatePerSec, WEAPON_PISTOL)
	{
		infiniteAmmo = true;
	}

	void fire(float originX, float originY, float angleDeg, bool facingRight) override
	{
		if (!canFire()) return;

		fireCooldown = 1.0f / fireRate;

		cout << "[Pistol] BANG! origin=(" << originX << "," << originY
			<< ") angle=" << angleDeg
			<< " facing=" << (facingRight ? "right" : "left") << "\n";
	}

	~Pistol() override {}
};

class Knife : public Weapon
{
protected:
	bool piercesShield;
	float meleeCooldown;

public:
	Knife() : Weapon(2, -1, 2.0f, WEAPON_KNIFE), piercesShield(false), meleeCooldown(0.0f)
	{
	}

	Knife(bool canPierceShield) : Weapon(2, -1, 2.0f, WEAPON_KNIFE),
		piercesShield(canPierceShield),
		meleeCooldown(0.0f)
	{
	}

	bool getPiercesShield() const { return piercesShield; }

	void updateMeleeCooldown(float dt)
	{
		if (meleeCooldown > 0.0f)
		{
			meleeCooldown -= dt;
			if (meleeCooldown < 0.0f) meleeCooldown = 0.0f;
		}
	}

	bool canMeleeAttack() const { return meleeCooldown <= 0.0f; }

	void fire(float originX, float originY, float angleDeg, bool facingRight) override
	{
		if (!canFire()) return;

		fireCooldown = 1.0f / fireRate;
		meleeCooldown = 0.5f;

		cout << "[Knife] SLASH! at (" << originX << "," << originY << ")"
			<< (piercesShield ? " [SHIELD PIERCE]" : "") << "\n";
	}

	~Knife() override {}
};


class HeavyMachineGun : public Weapon {
public:
	HeavyMachineGun(int startAmmo = 120) : Weapon(3, startAmmo, 8.0f, WEAPON_HMG) {}
	~HeavyMachineGun() override {}
};

class RocketLauncher : public Weapon {
public:
	RocketLauncher(int startAmmo = 12) : Weapon(5, startAmmo, 0.5f, WEAPON_ROCKET) {}
	~RocketLauncher() override {}
};

class FlameShotWeapon : public Weapon {
public:
	FlameShotWeapon(int startAmmo = 80) : Weapon(2, startAmmo, 10.0f, WEAPON_FLAME) {}
	~FlameShotWeapon() override {}
};

class LaserGun : public Weapon {
public:
	LaserGun(int startAmmo = 20) : Weapon(999, startAmmo, 1.0f, WEAPON_LASER) {}
	~LaserGun() override {}
};

class MarcoRossi : public PlayerSoldier
{
private:
	Pistol* marcoPistol;
	Knife* marcoKnife;

	bool dualFireActive;
	float dualFireTimer;

	static const float DUAL_FIRE_DURATION;

	static const int MAX_BULLETS = 200;

	Bullet* bullets[MAX_BULLETS];

	int bulletCount;

	static const float MELEE_RANGE;

	bool wasFiring;

public:
	MarcoRossi() : PlayerSoldier()
	{
		initMarco();
	}

	MarcoRossi(float startX, float startY, int maxHP = 100)
		: PlayerSoldier(startX, startY, maxHP)
	{
		initMarco();
	}

	bool isDualFireActive() const { return dualFireActive; }

	float getDualFireTimer() const { return dualFireTimer; }

	int getBulletCount() const { return bulletCount; }

	Bullet* getBullet(int idx) const
	{
		if (idx < 0 || idx >= bulletCount) return nullptr;

		return bullets[idx];
	}

	void PowerUp() override
	{
		dualFireActive = true;
		dualFireTimer = DUAL_FIRE_DURATION;
		powerUpActive = true;
		powerUpTimer = DUAL_FIRE_DURATION;

		cout << "[Marco] POWER-UP ACTIVATED! Dual fire for "
			<< DUAL_FIRE_DURATION << " seconds!\n";
	}

	void meleeAttack() override
	{
		if (!marcoKnife->canMeleeAttack()) return;

		marcoKnife->fire(posX, posY, 0.0f, facingRight);
		queueMelee(true, 2);

		cout << "[Marco] Knife slash! (pierces shields)\n";
	}

	void fireWeapon(float angleDeg)
	{
		(void)angleDeg;
		if (marcoPistol == nullptr) return;
		if (!marcoPistol->canFire()) return;

		fireEquippedWeapon(1.25f, dualFireActive);
		marcoPistol->fire(posX, posY, 0.0f, facingRight);
	}

	void update(float dt) override
	{
		PlayerSoldier::update(dt);

		if (isDead()) return;

		if (dualFireActive)
		{
			dualFireTimer -= dt;

			if (dualFireTimer <= 0.0f)
			{
				dualFireTimer = 0.0f;
				dualFireActive = false;

				cout << "[Marco] Dual fire expired.\n";
			}
		}

		if (marcoKnife != nullptr)
			marcoKnife->updateMeleeCooldown(dt);

		if (marcoPistol != nullptr)
			marcoPistol->updateCooldown(dt);

		updateBullets(dt);

		if (inputMgr != nullptr)
		{
			float screenCX = posX - 0.0f;
			float screenCY = posY - 0.0f;

			float aimAngle = 0.0f;

			bool xKeyDown = inputMgr->isKeyDown(Keyboard::X);

			if (xKeyDown)
			{
				fireWeapon(aimAngle);
			}

			if (inputMgr->isKeyPressed(Keyboard::P))
			{
				PowerUp();
			}

			if (inputMgr->isKeyPressed(Keyboard::F))
			{
				meleeAttack();
			}
		}
	}

	void render(RenderWindow& window, float camX, float camY) override
	{
		if (!active) return;

		PlayerSoldier::render(window, camX, camY);

		for (int i = 0; i < bulletCount; i++)
		{
			if (bullets[i] != nullptr && bullets[i]->isActive())
			{
				bullets[i]->render(window, camX, camY);
			}
		}

	}

	~MarcoRossi() override
	{
		delete marcoPistol;
		marcoPistol = nullptr;

		delete marcoKnife;
		marcoKnife = nullptr;

		for (int i = 0; i < MAX_BULLETS; i++)
		{
			delete bullets[i];
			bullets[i] = nullptr;
		}

		bulletCount = 0;
	}

private:
	void initMarco()
	{
		marcoPistol = new Pistol(3, 5.0f);

		marcoKnife = new Knife(true);

		grenadeCount = 8;

		weaponType = Weapon::WEAPON_PISTOL;

		dualFireActive = false;

		dualFireTimer = 0.0f;

		bulletCount = 0;

		for (int i = 0; i < MAX_BULLETS; i++)
			bullets[i] = nullptr;

		wasFiring = false;

		spriteScale = 0.2f;

		width = 132.0f;

		height = 102.0f;

		anim.configure(119, 94, 1, 0, 10.0f);
	}

	void spawnBullet(float angleDeg, bool direction)
	{
		angleDeg = 0.0f;
		if (bulletCount >= MAX_BULLETS) return;

		float speed = 800.0f;

		int slot = -1;

		for (int i = 0; i < MAX_BULLETS; i++)
		{
			if (bullets[i] == nullptr || !bullets[i]->isActive())
			{
				slot = i;
				break;
			}
		}

		if (slot == -1) return;

		float bx = posX + width * 0.5f;

		float by = posY + height * 0.5f;

		Bullet* b = new Bullet(bx, by, speed, angleDeg, direction, marcoPistol->getDamage(), true);

		b->loadTexture("Sprites/projectiles/bullet.png");
		b->setSpriteScale(0.08f, 0.08f);
		b->setHitBox(34.0f, 18.0f);
		delete bullets[slot];
		bullets[slot] = b;

		if (slot >= bulletCount)
			bulletCount = slot + 1;
	}

	void updateBullets(float dt)
	{
		for (int i = 0; i < bulletCount; i++)
		{
			if (bullets[i] == nullptr) continue;

			if (!bullets[i]->isActive()) continue;

			bullets[i]->update(dt);

			if (level != nullptr && bullets[i]->isActive())
			{
				Block* hitBlock = level->getBlockAtWorld(
					bullets[i]->getPosX(),
					bullets[i]->getPosY()
				);

				if (isSolidBlock(hitBlock))
				{
					bullets[i]->onHit();
				}
			}
		}
	}
};

const float MarcoRossi::DUAL_FIRE_DURATION = 10.0f;

const float MarcoRossi::MELEE_RANGE = 64.0f;

class EriKasamoto : public PlayerSoldier
{
private:
	static const float ERI_FIRE_RATE_MULTIPLIER;
	static const float ERI_BLAST_RADIUS_MULTIPLIER;
	static const float DOUBLE_GRENADE_DURATION;

	bool doubleGrenadeActive;
	float doubleGrenadeTimer;

	float shootCooldown;

	float grenadePosX;
	float grenadePosY;
	float grenadeVelX;
	float grenadeVelY;
	bool grenadeInFlight;

	float grenadeExtraPosX;
	float grenadeExtraPosY;
	float grenadeExtraVelX;
	float grenadeExtraVelY;
	bool grenadeExtraInFlight;

	static const float GRENADE_GRAVITY;
	static const float GRENADE_SPEED;
	static const float EXTRA_GRENADE_OFFSET;

public:
	EriKasamoto() : PlayerSoldier()
	{
		grenadeCount = 20;

		doubleGrenadeActive = false;
		doubleGrenadeTimer = 0.f;
		shootCooldown = 0.f;

		grenadePosX = 0.f;
		grenadePosY = 0.f;
		grenadeVelX = 0.f;
		grenadeVelY = 0.f;
		grenadeInFlight = false;

		grenadeExtraPosX = 0.f;
		grenadeExtraPosY = 0.f;
		grenadeExtraVelX = 0.f;
		grenadeExtraVelY = 0.f;
		grenadeExtraInFlight = false;
	}

	EriKasamoto(float x, float y) : PlayerSoldier(x, y, 100)
	{
		grenadeCount = 20;

		doubleGrenadeActive = false;
		doubleGrenadeTimer = 0.f;
		shootCooldown = 0.f;

		grenadePosX = 0.f;
		grenadePosY = 0.f;
		grenadeVelX = 0.f;
		grenadeVelY = 0.f;
		grenadeInFlight = false;

		grenadeExtraPosX = 0.f;
		grenadeExtraPosY = 0.f;
		grenadeExtraVelX = 0.f;
		grenadeExtraVelY = 0.f;
		grenadeExtraInFlight = false;
	}

	void PowerUp() override
	{
		doubleGrenadeActive = true;
		doubleGrenadeTimer = DOUBLE_GRENADE_DURATION;
		powerUpActive = true;
		powerUpTimer = DOUBLE_GRENADE_DURATION;
	}

	void meleeAttack() override
	{
		queueMelee(false, 2);
	}

	float getBlastRadius(float baseRadius)
	{
		return baseRadius * ERI_BLAST_RADIUS_MULTIPLIER;
	}

	bool isDoubleGrenadeActive() const
	{
		return doubleGrenadeActive;
	}

	float getDoubleGrenadeTimer() const
	{
		return doubleGrenadeTimer;
	}

	bool isGrenadeInFlight() const
	{
		return grenadeInFlight;
	}

	bool isExtraGrenadeInFlight() const
	{
		return grenadeExtraInFlight;
	}

	float getGrenadePosX() const { return grenadePosX; }
	float getGrenadePosY() const { return grenadePosY; }
	float getGrenadeExtraPosX() const { return grenadeExtraPosX; }
	float getGrenadeExtraPosY() const { return grenadeExtraPosY; }

	void throwGrenade() override
	{
		int available = gameStats ? gameStats->getGrenades(charIndex) : grenadeCount;
		if (available <= 0) return;
		if (gameStats) gameStats->subtractStat(charIndex, GameStats::STAT_GRENADES, 1);
		else grenadeCount--;

		float gx = facingRight ? posX + width - 10.0f : posX + 10.0f;
		float gy = posY + height * 0.35f;
		playerProjectiles.throwFireBomb(gx, gy, facingRight, level, 20, 460.0f, 43.0f, getBlastRadius(3.0f * Block::BLOCK_SIZE));

		if (doubleGrenadeActive) {
			playerProjectiles.throwFireBomb(gx, gy - 12.0f, facingRight, level, 20, 500.0f, 38.0f, getBlastRadius(3.0f * Block::BLOCK_SIZE));
		}

		grenadeInFlight = false;
		grenadeExtraInFlight = false;
	}

	void shoot(float angleDeg)
	{
		(void)angleDeg;
		if (shootCooldown > 0.f) return;

		if (weaponType != Weapon::WEAPON_PISTOL && weaponType != Weapon::WEAPON_KNIFE) {
			fireEquippedWeapon(ERI_FIRE_RATE_MULTIPLIER, false);
			shootCooldown = 0.12f;
			return;
		}
		int available = gameStats ? gameStats->getGrenades(charIndex) : grenadeCount;
		if (available > 0) {
			throwGrenade();
			shootCooldown = 0.55f;
		}
		else {
			float eriCooldown = 1.f / (4.0f * ERI_FIRE_RATE_MULTIPLIER);
			fireEquippedWeapon(ERI_FIRE_RATE_MULTIPLIER, false);
			shootCooldown = eriCooldown;
		}
	}

	void update(float dt) override
	{
		PlayerSoldier::update(dt);

		if (shootCooldown > 0.f)
		{
			shootCooldown -= dt;
			if (shootCooldown < 0.f)
			{
				shootCooldown = 0.f;
			}
		}

		if (doubleGrenadeActive)
		{
			doubleGrenadeTimer -= dt;
			if (doubleGrenadeTimer <= 0.f)
			{
				doubleGrenadeTimer = 0.f;
				doubleGrenadeActive = false;
			}
		}

		if (grenadeInFlight)
		{
			grenadeVelY += GRENADE_GRAVITY * dt;
			grenadePosX += grenadeVelX * dt;
			grenadePosY += grenadeVelY * dt;

			if (level != nullptr)
			{
				float worldBottom = (float)(level->getHeight() * Block::BLOCK_SIZE);
				if (grenadePosY >= worldBottom)
				{
					grenadeInFlight = false;
				}

				Block* hitBlock = level->getBlockAtWorld(grenadePosX, grenadePosY);
				if (isSolidBlock(hitBlock))
				{
					grenadeInFlight = false;
				}
			}
		}

		if (grenadeExtraInFlight)
		{
			grenadeExtraVelY += GRENADE_GRAVITY * dt;
			grenadeExtraPosX += grenadeExtraVelX * dt;
			grenadeExtraPosY += grenadeExtraVelY * dt;

			if (level != nullptr)
			{
				float worldBottom = (float)(level->getHeight() * Block::BLOCK_SIZE);
				if (grenadeExtraPosY >= worldBottom)
				{
					grenadeExtraInFlight = false;
				}

				Block* hitBlock = level->getBlockAtWorld(grenadeExtraPosX, grenadeExtraPosY);
				if (isSolidBlock(hitBlock))
				{
					grenadeExtraInFlight = false;
				}
			}
		}

		if (inputMgr != nullptr)
		{
			if (inputMgr->isMouseLeftDown() || inputMgr->isKeyDown(Keyboard::X))
			{
				float angle = inputMgr->getAimAngle(posX, posY);
				shoot(angle);
			}

			if (inputMgr->isKeyPressed(Keyboard::P))
			{
				PowerUp();
			}
		}
	}

	void render(RenderWindow& window, float camX, float camY) override
	{
		PlayerSoldier::render(window, camX, camY);

		if (grenadeInFlight)
		{
		}

		if (grenadeExtraInFlight)
		{
		}
	}

	~EriKasamoto() override
	{
	}

protected:
	void onPowerUpEnd() override
	{
		doubleGrenadeActive = false;
		doubleGrenadeTimer = 0.f;
	}
};

const float EriKasamoto::ERI_FIRE_RATE_MULTIPLIER = 0.8f;
const float EriKasamoto::ERI_BLAST_RADIUS_MULTIPLIER = 1.5f;
const float EriKasamoto::DOUBLE_GRENADE_DURATION = 10.0f;
const float EriKasamoto::GRENADE_GRAVITY = 1800.0f;
const float EriKasamoto::GRENADE_SPEED = 300.0f;
const float EriKasamoto::EXTRA_GRENADE_OFFSET = 128.0f;


class Tarma : public PlayerSoldier
{
private:
	static const float TARMA_SPEED_MULTIPLIER;
	static const float TARMA_VEHICLE_FIRE_RATE_MULTIPLIER;
	static const float TARMA_VEHICLE_DURABILITY_MULTIPLIER;
	static const float TARMA_POWERUP_DURATION;

	bool immunityActive;
	float immunityTimer;

	bool isInVehicle;
	Vehicle* currentVehicle;

	float shootCooldown;

public:
	Tarma() : PlayerSoldier()
	{
		hp = 80;
		maxHP = 80;

		moveSpeed = 200.0f * TARMA_SPEED_MULTIPLIER;

		immunityActive = false;
		immunityTimer = 0.f;

		isInVehicle = false;
		currentVehicle = nullptr;

		shootCooldown = 0.f;
	}

	Tarma(float x, float y) : PlayerSoldier(x, y, 80)
	{
		moveSpeed = 200.0f * TARMA_SPEED_MULTIPLIER;

		immunityActive = false;
		immunityTimer = 0.f;

		isInVehicle = false;
		currentVehicle = nullptr;

		shootCooldown = 0.f;
	}

	void PowerUp() override
	{
		immunityActive = true;
		immunityTimer = TARMA_POWERUP_DURATION;
		powerUpActive = true;
		powerUpTimer = TARMA_POWERUP_DURATION;
	}

	void meleeAttack() override
	{
		queueMelee(false, 2);
	}

	bool takeDamage(int damage) override
	{
		if (immunityActive)
		{
			return false;
		}

		return PlayerSoldier::takeDamage(damage);
	}

	void enterVehicle(Vehicle* v)
	{
		if (v == nullptr) return;
		isInVehicle = true;
		currentVehicle = v;
		currentVehicle->setOccupied(true);
	}

	void exitVehicle()
	{
		if (currentVehicle != nullptr)
		{
			currentVehicle->setOccupied(false);
		}
		isInVehicle = false;
		currentVehicle = nullptr;
	}

	bool getIsInVehicle() const
	{
		return isInVehicle;
	}

	Vehicle* getCurrentVehicle() const
	{
		return currentVehicle;
	}

	bool isImmune() const
	{
		return immunityActive;
	}

	float getImmunityTimer() const
	{
		return immunityTimer;
	}

	float getVehicleFireRate(float baseFireRate) const
	{
		return baseFireRate * TARMA_VEHICLE_FIRE_RATE_MULTIPLIER;
	}

	int getVehicleDurability(int baseHP) const
	{
		return (int)(baseHP * TARMA_VEHICLE_DURABILITY_MULTIPLIER);
	}

	void onVehicleDestroyed()
	{
		if (isInVehicle)
		{
			isInVehicle = false;

			if (currentVehicle != nullptr)
			{
				posX = currentVehicle->getPosX();
				posY = currentVehicle->getPosY();
			}

			currentVehicle = nullptr;

			hp = maxHP;
			damageState = DAMAGE_HEALTHY;
			stateTimer = 0.f;
			active = true;
		}
	}

	void shoot(float angleDeg)
	{
		(void)angleDeg;
		float baseCooldown = 1.f / 4.0f;

		if (shootCooldown > 0.f) return;
		fireEquippedWeapon(1.0f, false);
		shootCooldown = baseCooldown;
	}

	void update(float dt) override
	{
		if (immunityActive && isInVehicle && currentVehicle != nullptr)
		{
			currentVehicle->takeDamage(0);
		}

		if (isInVehicle && currentVehicle != nullptr)
		{
			if (currentVehicle->isDead())
			{
				onVehicleDestroyed();
			}
		}

		PlayerSoldier::update(dt);

		if (shootCooldown > 0.f)
		{
			shootCooldown -= dt;
			if (shootCooldown < 0.f)
			{
				shootCooldown = 0.f;
			}
		}

		if (immunityActive)
		{
			immunityTimer -= dt;
			if (immunityTimer <= 0.f)
			{
				immunityTimer = 0.f;
				immunityActive = false;
			}
		}

		if (inputMgr != nullptr)
		{
			if (inputMgr->isMouseLeftDown() || inputMgr->isKeyDown(Keyboard::X))
			{
				float angle = inputMgr->getAimAngle(posX, posY);
				shoot(angle);
			}

			if (inputMgr->isKeyPressed(Keyboard::P))
			{
				PowerUp();
			}

			if (inputMgr->isKeyPressed(Keyboard::V))
			{
				if (isInVehicle)
				{
					exitVehicle();
				}
			}
		}
	}

	void render(RenderWindow& window, float camX, float camY) override
	{
		if (isInVehicle) return;

		PlayerSoldier::render(window, camX, camY);

		if (immunityActive)
		{
		}
	}

	~Tarma() override
	{
		currentVehicle = nullptr;
	}

protected:
	void onPowerUpEnd() override
	{
		immunityActive = false;
		immunityTimer = 0.f;
	}
};

const float Tarma::TARMA_SPEED_MULTIPLIER = 0.8f;
const float Tarma::TARMA_VEHICLE_FIRE_RATE_MULTIPLIER = 1.25f;
const float Tarma::TARMA_VEHICLE_DURABILITY_MULTIPLIER = 1.2f;
const float Tarma::TARMA_POWERUP_DURATION = 20.0f;


class FiolinaGermi : public PlayerSoldier
{
private:
	static const float FIO_FIRE_RATE_MULTIPLIER;
	static const float FIO_SUPERCHARGE_MULTIPLIER;
	static const float FIO_POWERUP_DURATION;
	static const float FIO_AMMO_BONUS_MULTIPLIER;
	static const float FIO_MELEE_DAMAGE_MULTIPLIER;

	bool superchargeActive;
	float superchargeTimer;

	float shootCooldown;

	int currentWeaponAmmo;
	int currentWeaponDamage;
	int currentWeaponType;

public:
	FiolinaGermi() : PlayerSoldier()
	{
		grenadeCount = 8;

		superchargeActive = false;
		superchargeTimer = 0.f;
		shootCooldown = 0.f;

		currentWeaponAmmo = -1;
		currentWeaponDamage = 3;
		currentWeaponType = Weapon::WEAPON_PISTOL;
	}

	FiolinaGermi(float x, float y) : PlayerSoldier(x, y, 100)
	{
		grenadeCount = 8;

		superchargeActive = false;
		superchargeTimer = 0.f;
		shootCooldown = 0.f;

		currentWeaponAmmo = -1;
		currentWeaponDamage = 3;
		currentWeaponType = Weapon::WEAPON_PISTOL;
	}

	void PowerUp() override
	{
		superchargeActive = true;
		superchargeTimer = FIO_POWERUP_DURATION;
		powerUpActive = true;
		powerUpTimer = FIO_POWERUP_DURATION;
	}

	void meleeAttack() override
	{
		int meleeDamage = 2;
		int fioMeleeDamage = (int)(meleeDamage * FIO_MELEE_DAMAGE_MULTIPLIER);
		if (fioMeleeDamage < 1) fioMeleeDamage = 1;
		queueMelee(false, fioMeleeDamage);
	}

	int getMeleeDamage()
	{
		int baseMeleeDamage = 2;
		return (int)(baseMeleeDamage * FIO_MELEE_DAMAGE_MULTIPLIER);
	}

	void giveWeapon(int type, int ammo) override {
		int finalAmmo = (ammo < 0) ? -1 : getPickupAmmo(ammo);
		currentWeaponType = type;
		currentWeaponAmmo = finalAmmo;
		if (type == Weapon::WEAPON_ROCKET) currentWeaponDamage = 5;
		else if (type == Weapon::WEAPON_LASER) currentWeaponDamage = 999;
		else currentWeaponDamage = 3;
		PlayerSoldier::giveWeapon(type, finalAmmo);
	}

	void pickupWeapon(int weaponType, int baseAmmo, int weaponDamage)
	{
		int fioAmmo = (int)(baseAmmo * FIO_AMMO_BONUS_MULTIPLIER);

		currentWeaponType = weaponType;
		currentWeaponAmmo = fioAmmo;
		currentWeaponDamage = weaponDamage;
		giveWeapon(currentWeaponType, currentWeaponAmmo);
	}

	int getPickupAmmo(int baseAmmo)
	{
		return (int)(baseAmmo * FIO_AMMO_BONUS_MULTIPLIER);
	}

	float getCurrentFireRate()
	{
		float baseFireRate = 4.0f * FIO_FIRE_RATE_MULTIPLIER;

		if (superchargeActive)
		{
			return baseFireRate * FIO_SUPERCHARGE_MULTIPLIER;
		}

		return baseFireRate;
	}

	float getShootCooldown()
	{
		return 1.f / getCurrentFireRate();
	}

	bool isSuperchargeActive() const
	{
		return superchargeActive;
	}

	float getSuperchargeTimer() const
	{
		return superchargeTimer;
	}

	int getCurrentWeaponAmmo() const
	{
		return currentWeaponAmmo;
	}

	int getCurrentWeaponType() const
	{
		return currentWeaponType;
	}

	void shoot(float angleDeg)
	{
		(void)angleDeg;
		if (shootCooldown > 0.f) return;

		shootCooldown = getShootCooldown();
		fireEquippedWeapon(getCurrentFireRate() / 4.0f, false);
		currentWeaponType = weaponType;
		currentWeaponAmmo = weaponAmmo;
		if (currentWeaponAmmo == 0) {
			currentWeaponType = Weapon::WEAPON_PISTOL;
			currentWeaponAmmo = -1;
			currentWeaponDamage = 3;
		}
	}

	void update(float dt) override
	{
		PlayerSoldier::update(dt);

		if (shootCooldown > 0.f)
		{
			shootCooldown -= dt;
			if (shootCooldown < 0.f)
			{
				shootCooldown = 0.f;
			}
		}

		if (superchargeActive)
		{
			superchargeTimer -= dt;
			if (superchargeTimer <= 0.f)
			{
				superchargeTimer = 0.f;
				superchargeActive = false;
			}
		}

		if (inputMgr != nullptr)
		{
			if (inputMgr->isMouseLeftDown() || inputMgr->isKeyDown(Keyboard::X))
			{
				float angle = inputMgr->getAimAngle(posX, posY);
				shoot(angle);
			}

			if (inputMgr->isKeyPressed(Keyboard::P))
			{
				PowerUp();
			}

			if (inputMgr->isKeyPressed(Keyboard::F))
			{
				meleeAttack();
			}
		}
	}

	void render(RenderWindow& window, float camX, float camY) override
	{
		PlayerSoldier::render(window, camX, camY);

		if (superchargeActive)
		{
		}
	}

	~FiolinaGermi() override
	{
	}

protected:
	void onPowerUpEnd() override
	{
		superchargeActive = false;
		superchargeTimer = 0.f;
	}
};

const float FiolinaGermi::FIO_FIRE_RATE_MULTIPLIER = 1.1f;
const float FiolinaGermi::FIO_SUPERCHARGE_MULTIPLIER = 2.0f;
const float FiolinaGermi::FIO_POWERUP_DURATION = 10.0f;
const float FiolinaGermi::FIO_AMMO_BONUS_MULTIPLIER = 1.5f;
const float FiolinaGermi::FIO_MELEE_DAMAGE_MULTIPLIER = 0.75f;


class SurvivalLevel : public Level {
protected:
	int levelNumber;
	float difficulty;

public:
	SurvivalLevel() : Level(200, 50), levelNumber(1), difficulty(1.0f) {}

	SurvivalLevel(int w, int h) : Level(w, h), levelNumber(1), difficulty(1.0f) {}

	int getLevelNumber() const { return levelNumber; }
	float getDifficulty()  const { return difficulty; }

	void setLevelNumber(int n) { levelNumber = n; }
	void setDifficulty(float d) { difficulty = d; }

	virtual void spawnPredefinedEnemies() = 0;
	virtual void spawnPOWPrisoners() {}
	bool allEnemiesKilled() { return false; }

	virtual ~SurvivalLevel() {}
};


class Level1 : public SurvivalLevel {
public:
	Level1() : SurvivalLevel(150, 22) {
		levelNumber = 1; difficulty = 0.5f;
		registerThreeBiomes();
		buildThreeBiomeVisibleTerrain();
		spawnPredefinedEnemies();
		loadBackground("Sprites/background/sky.png");
	}

	bool isComplete() override { return false; }

	void spawnPredefinedEnemies() override {
		addInfantryBatches(2, 3, 2, 1, 2);

		addAirBatch(62, SpawnPoint::T_PARATROOPER, 2);
		addAirBatch(84, SpawnPoint::T_MARTIAN, 1);

		addGroundBatch(26, SpawnPoint::T_MUMMY, 1);
		addGroundBatch(38, SpawnPoint::T_MUMMY, 1);
		addGroundBatch(18, SpawnPoint::T_ZOMBIE, 3);
		addGroundBatch(44, SpawnPoint::T_ZOMBIE, 3);

		addGroundBatch(112, SpawnPoint::T_ZOMBIE, 3);
		addGroundBatch(136, SpawnPoint::T_ZOMBIE, 3);

		addAirBatch(70, SpawnPoint::T_FLYING_TARA, 2);
		addAirBatch(96, SpawnPoint::T_FLYING_TARA, 2);
		addAquaticVehicle(126, SpawnPoint::T_ENEMY_SUB, 1);
	}

private:
	void registerThreeBiomes() {
		biomes[0] = new PlainsBiome(0.f, 22.f * 64.f);
		biomes[1] = new AerialBiome(0.f, 22.f * 64.f);
		biomes[2] = new AquaticBiome(0.f, 22.f * 64.f, 17.f * 64.f, 4.f);
		biomeCount = 3;
	}

	void buildThreeBiomeVisibleTerrain() {
		fillBedrockRow();
		int aquaticStart = (2 * width) / 3;
		int aerialStart = width / 3;
		int aerialEnd = aquaticStart - 1;
		const int plainsBaseSurface = 14;
		const int seaRow = 14;
		const int seaFloor = 19;

		for (int c = 0; c < width; c++) {
			bool inAerial = (c >= aerialStart && c <= aerialEnd);
			bool inAquatic = (c >= aquaticStart);

			if (inAquatic) {
				for (int r = seaRow; r < seaFloor; r++) {
					delete grid[r][c];
					grid[r][c] = new WaterBlock((float)(c * Block::BLOCK_SIZE), (float)(r * Block::BLOCK_SIZE));
				}
				for (int r = seaFloor; r <= height - 2; r++) {
					delete grid[r][c];
					grid[r][c] = new NormalBlock((float)(c * Block::BLOCK_SIZE), (float)(r * Block::BLOCK_SIZE), "Sprites/blocks/sand.png");
				}
			}
			else if (inAerial) {
				int local = c - aerialStart;
				int wave = local % 18;
				if (wave > 9) wave = 18 - wave;
				int aerialSurface = 9 + (wave / 3);
				for (int r = aerialSurface; r <= height - 2; r++) {
					delete grid[r][c];
					grid[r][c] = new IndestructibleBlock((float)(c * Block::BLOCK_SIZE), (float)(r * Block::BLOCK_SIZE), "Sprites/blocks/stone.png");
				}
			}
			else {
				int plainsSurface = plainsBaseSurface + (((c / 10) % 2 == 0) ? 0 : 1);
				for (int r = plainsSurface; r <= height - 2; r++) {
					delete grid[r][c];
					grid[r][c] = new NormalBlock((float)(c * Block::BLOCK_SIZE), (float)(r * Block::BLOCK_SIZE), r == plainsSurface ? "Sprites/blocks/grass_block_side.png" : "Sprites/blocks/dirt.png");
				}
			}
		}
	}

	int surfaceRowForCol(int col) const {
		if (col >= (2 * width) / 3) return 19;
		if (col >= width / 3) {
			int local = col - width / 3;
			int wave = local % 18;
			if (wave > 9) wave = 18 - wave;
			return 9 + (wave / 3);
		}
		return 14 + (((col / 10) % 2 == 0) ? 0 : 1);
	}
	void addGroundBatch(int col, int type, int batch) { addSpawn(col * 64.f, surfaceRowForCol(col) * 64.f - 132.f, type, batch); }
	void addAirBatch(int col, int type, int batch) { addSpawn(col * 64.f, 4.f * 64.f, type, batch); }
	void addAquaticVehicle(int col, int type, int batch) { addSpawn(col * 64.f, 15.f * 64.f, type, batch); }

	void addInfantryBatches(int batchesPerType, int rebelBatch, int shieldBatch, int bazookaBatch, int grenadeBatch) {
		const int types[4] = { SpawnPoint::T_REBEL, SpawnPoint::T_SHIELDED, SpawnPoint::T_BAZOOKA, SpawnPoint::T_GRENADE };
		const int sizes[4] = { rebelBatch, shieldBatch, bazookaBatch, grenadeBatch };
		int cols[8] = { 10, 58, 108, 32, 74, 128, 46, 94 };
		int k = 0;
		for (int t = 0; t < 4; t++) {
			for (int b = 0; b < batchesPerType; b++) {
				addGroundBatch(cols[k++ % 8], types[t], sizes[t]);
			}
		}
	}
};

class Level2 : public SurvivalLevel {
public:
	Level2() : SurvivalLevel(180, 22) {
		levelNumber = 2; difficulty = 0.8f;
		registerThreeBiomes();
		buildThreeBiomeVisibleTerrain();
		spawnPredefinedEnemies();
		loadBackground("Sprites/background/sky.png");
	}

	bool isComplete() override { return false; }

	void spawnPredefinedEnemies() override {
		addInfantryBatches(3, 3, 2, 2, 2);

		addAirBatch(72, SpawnPoint::T_PARATROOPER, 2);
		addAirBatch(96, SpawnPoint::T_PARATROOPER, 2);
		addAirBatch(82, SpawnPoint::T_MARTIAN, 1);
		addAirBatch(108, SpawnPoint::T_MARTIAN, 1);

		addGroundBatch(18, SpawnPoint::T_MUMMY, 1);
		addGroundBatch(34, SpawnPoint::T_MUMMY, 1);
		addGroundBatch(50, SpawnPoint::T_MUMMY, 1);
		addGroundBatch(24, SpawnPoint::T_ZOMBIE, 4);
		addGroundBatch(42, SpawnPoint::T_ZOMBIE, 4);
		addGroundBatch(56, SpawnPoint::T_ZOMBIE, 4);

		addGroundBatch(126, SpawnPoint::T_ZOMBIE, 4);
		addGroundBatch(150, SpawnPoint::T_ZOMBIE, 4);
		addGroundBatch(170, SpawnPoint::T_ZOMBIE, 4);

		addAirBatch(76, SpawnPoint::T_FLYING_TARA, 2);
		addAirBatch(104, SpawnPoint::T_FLYING_TARA, 2);
		addAirBatch(118, SpawnPoint::T_FLYING_TARA, 2);
		addGroundBatch(30, SpawnPoint::T_BRADLEY, 1);
		addGroundBatch(52, SpawnPoint::T_BRADLEY, 1);
		addAquaticVehicle(146, SpawnPoint::T_ENEMY_SUB, 1);
	}

private:
	void registerThreeBiomes() {
		biomes[0] = new PlainsBiome(0.f, 22.f * 64.f);
		biomes[1] = new AerialBiome(0.f, 22.f * 64.f);
		biomes[2] = new AquaticBiome(0.f, 22.f * 64.f, 17.f * 64.f, 4.f);
		biomeCount = 3;
	}

	void buildThreeBiomeVisibleTerrain() {
		fillBedrockRow();
		int aquaticStart = (2 * width) / 3;
		int aerialStart = width / 3;
		int aerialEnd = aquaticStart - 1;
		const int plainsBaseSurface = 14;
		const int seaRow = 14;
		const int seaFloor = 19;

		for (int c = 0; c < width; c++) {
			bool inAerial = (c >= aerialStart && c <= aerialEnd);
			bool inAquatic = (c >= aquaticStart);

			if (inAquatic) {
				for (int r = seaRow; r < seaFloor; r++) {
					delete grid[r][c];
					grid[r][c] = new WaterBlock((float)(c * Block::BLOCK_SIZE), (float)(r * Block::BLOCK_SIZE));
				}
				for (int r = seaFloor; r <= height - 2; r++) {
					delete grid[r][c];
					grid[r][c] = new NormalBlock((float)(c * Block::BLOCK_SIZE), (float)(r * Block::BLOCK_SIZE), "Sprites/blocks/sand.png");
				}
			}
			else if (inAerial) {
				int local = c - aerialStart;
				int wave = local % 18;
				if (wave > 9) wave = 18 - wave;
				int aerialSurface = 9 + (wave / 3);
				for (int r = aerialSurface; r <= height - 2; r++) {
					delete grid[r][c];
					grid[r][c] = new IndestructibleBlock((float)(c * Block::BLOCK_SIZE), (float)(r * Block::BLOCK_SIZE), "Sprites/blocks/stone.png");
				}
			}
			else {
				int plainsSurface = plainsBaseSurface + (((c / 10) % 2 == 0) ? 0 : 1);
				for (int r = plainsSurface; r <= height - 2; r++) {
					delete grid[r][c];
					grid[r][c] = new NormalBlock((float)(c * Block::BLOCK_SIZE), (float)(r * Block::BLOCK_SIZE), r == plainsSurface ? "Sprites/blocks/grass_block_side.png" : "Sprites/blocks/dirt.png");
				}
			}
		}
	}

	int surfaceRowForCol(int col) const {
		if (col >= (2 * width) / 3) return 19;
		if (col >= width / 3) {
			int local = col - width / 3;
			int wave = local % 18;
			if (wave > 9) wave = 18 - wave;
			return 9 + (wave / 3);
		}
		return 14 + (((col / 10) % 2 == 0) ? 0 : 1);
	}
	void addGroundBatch(int col, int type, int batch) { addSpawn(col * 64.f, surfaceRowForCol(col) * 64.f - 132.f, type, batch); }
	void addAirBatch(int col, int type, int batch) { addSpawn(col * 64.f, 4.f * 64.f, type, batch); }
	void addAquaticVehicle(int col, int type, int batch) { addSpawn(col * 64.f, 15.f * 64.f, type, batch); }

	void addInfantryBatches(int batchesPerType, int rebelBatch, int shieldBatch, int bazookaBatch, int grenadeBatch) {
		const int types[4] = { SpawnPoint::T_REBEL, SpawnPoint::T_SHIELDED, SpawnPoint::T_BAZOOKA, SpawnPoint::T_GRENADE };
		const int sizes[4] = { rebelBatch, shieldBatch, bazookaBatch, grenadeBatch };
		int cols[12] = { 12, 74, 130, 28, 88, 148, 44, 104, 164, 56, 116, 172 };
		int k = 0;
		for (int t = 0; t < 4; t++) {
			for (int b = 0; b < batchesPerType; b++) {
				addGroundBatch(cols[k++ % 12], types[t], sizes[t]);
			}
		}
	}
};

class Level3 : public SurvivalLevel {
public:
	Level3() : SurvivalLevel(200, 22) {
		levelNumber = 3; difficulty = 1.0f;
		registerThreeBiomes();
		buildThreeBiomeVisibleTerrain();
		spawnPredefinedEnemies();
		loadBackground("Sprites/background/sky.png");
	}

	bool isComplete() override { return false; }

	void spawnPredefinedEnemies() override {
		addInfantryBatches(3, 4, 2, 2, 2);

		addAirBatch(78, SpawnPoint::T_PARATROOPER, 2);
		addAirBatch(104, SpawnPoint::T_PARATROOPER, 2);
		addAirBatch(128, SpawnPoint::T_PARATROOPER, 2);
		addAirBatch(88, SpawnPoint::T_MARTIAN, 1);
		addAirBatch(116, SpawnPoint::T_MARTIAN, 1);
		addAirBatch(132, SpawnPoint::T_MARTIAN, 1);

		addGroundBatch(16, SpawnPoint::T_MUMMY, 1);
		addGroundBatch(28, SpawnPoint::T_MUMMY, 1);
		addGroundBatch(40, SpawnPoint::T_MUMMY, 1);
		addGroundBatch(56, SpawnPoint::T_MUMMY, 1);
		addGroundBatch(20, SpawnPoint::T_ZOMBIE, 5);
		addGroundBatch(34, SpawnPoint::T_ZOMBIE, 5);
		addGroundBatch(48, SpawnPoint::T_ZOMBIE, 5);
		addGroundBatch(62, SpawnPoint::T_ZOMBIE, 5);

		addGroundBatch(144, SpawnPoint::T_ZOMBIE, 5);
		addGroundBatch(162, SpawnPoint::T_ZOMBIE, 5);
		addGroundBatch(180, SpawnPoint::T_ZOMBIE, 5);
		addGroundBatch(194, SpawnPoint::T_ZOMBIE, 5);

		addAirBatch(84, SpawnPoint::T_FLYING_TARA, 2);
		addAirBatch(112, SpawnPoint::T_FLYING_TARA, 2);
		addAirBatch(136, SpawnPoint::T_FLYING_TARA, 2);
		addGroundBatch(30, SpawnPoint::T_BRADLEY, 1);
		addGroundBatch(54, SpawnPoint::T_BRADLEY, 1);
		addAquaticVehicle(154, SpawnPoint::T_ENEMY_SUB, 1);
		addAquaticVehicle(184, SpawnPoint::T_ENEMY_SUB, 1);
	}

private:
	void registerThreeBiomes() {
		biomes[0] = new PlainsBiome(0.f, 22.f * 64.f);
		biomes[1] = new AerialBiome(0.f, 22.f * 64.f);
		biomes[2] = new AquaticBiome(0.f, 22.f * 64.f, 17.f * 64.f, 4.f);
		biomeCount = 3;
	}

	void buildThreeBiomeVisibleTerrain() {
		fillBedrockRow();
		int aquaticStart = (2 * width) / 3;
		int aerialStart = width / 3;
		int aerialEnd = aquaticStart - 1;
		const int plainsBaseSurface = 14;
		const int seaRow = 14;
		const int seaFloor = 19;

		for (int c = 0; c < width; c++) {
			bool inAerial = (c >= aerialStart && c <= aerialEnd);
			bool inAquatic = (c >= aquaticStart);

			if (inAquatic) {
				for (int r = seaRow; r < seaFloor; r++) {
					delete grid[r][c];
					grid[r][c] = new WaterBlock((float)(c * Block::BLOCK_SIZE), (float)(r * Block::BLOCK_SIZE));
				}
				for (int r = seaFloor; r <= height - 2; r++) {
					delete grid[r][c];
					grid[r][c] = new NormalBlock((float)(c * Block::BLOCK_SIZE), (float)(r * Block::BLOCK_SIZE), "Sprites/blocks/sand.png");
				}
			}
			else if (inAerial) {
				int local = c - aerialStart;
				int wave = local % 18;
				if (wave > 9) wave = 18 - wave;
				int aerialSurface = 9 + (wave / 3);
				for (int r = aerialSurface; r <= height - 2; r++) {
					delete grid[r][c];
					grid[r][c] = new IndestructibleBlock((float)(c * Block::BLOCK_SIZE), (float)(r * Block::BLOCK_SIZE), "Sprites/blocks/stone.png");
				}
			}
			else {
				int plainsSurface = plainsBaseSurface + (((c / 10) % 2 == 0) ? 0 : 1);
				for (int r = plainsSurface; r <= height - 2; r++) {
					delete grid[r][c];
					grid[r][c] = new NormalBlock((float)(c * Block::BLOCK_SIZE), (float)(r * Block::BLOCK_SIZE), r == plainsSurface ? "Sprites/blocks/grass_block_side.png" : "Sprites/blocks/dirt.png");
				}
			}
		}
	}

	int surfaceRowForCol(int col) const {
		if (col >= (2 * width) / 3) return 19;
		if (col >= width / 3) {
			int local = col - width / 3;
			int wave = local % 18;
			if (wave > 9) wave = 18 - wave;
			return 9 + (wave / 3);
		}
		return 14 + (((col / 10) % 2 == 0) ? 0 : 1);
	}
	void addGroundBatch(int col, int type, int batch) { addSpawn(col * 64.f, surfaceRowForCol(col) * 64.f - 132.f, type, batch); }
	void addAirBatch(int col, int type, int batch) { addSpawn(col * 64.f, 4.f * 64.f, type, batch); }
	void addAquaticVehicle(int col, int type, int batch) { addSpawn(col * 64.f, 15.f * 64.f, type, batch); }

	void addInfantryBatches(int batchesPerType, int rebelBatch, int shieldBatch, int bazookaBatch, int grenadeBatch) {
		const int types[4] = { SpawnPoint::T_REBEL, SpawnPoint::T_SHIELDED, SpawnPoint::T_BAZOOKA, SpawnPoint::T_GRENADE };
		const int sizes[4] = { rebelBatch, shieldBatch, bazookaBatch, grenadeBatch };
		int cols[12] = { 12, 78, 146, 28, 94, 162, 44, 112, 180, 60, 128, 194 };
		int k = 0;
		for (int t = 0; t < 4; t++) {
			for (int b = 0; b < batchesPerType; b++) {
				addGroundBatch(cols[k++ % 12], types[t], sizes[t]);
			}
		}
	}
};

class GameModeMenu {
public:
	static const int MODE_SURVIVAL = 0;
	static const int MODE_CAMPAIGN = 1;

private:
	Font font;
	bool fontLoaded;
	Texture bgTexture;
	Sprite bgSprite;

	int selectedOption;
	bool confirmed;

	static const int SCREEN_W = 1600;
	static const int SCREEN_H = 900;

public:
	GameModeMenu() : selectedOption(0), confirmed(false), fontLoaded(false) {
		fontLoaded = font.loadFromFile("Sprites/font.ttf");

		if (!bgTexture.loadFromFile("Sprites/background/menu_bg.png")) {
			cout << "Failed to load menu" << endl;
		}

		bgSprite.setTexture(bgTexture);

		bgSprite.setScale((float)SCREEN_W / (float)bgTexture.getSize().x, (float)SCREEN_H / (float)bgTexture.getSize().y);
	}

	void update(InputManager& input) {
		if (confirmed) return;

		if (input.isKeyPressed(Keyboard::Up) || input.isKeyPressed(Keyboard::Down)) selectedOption = 1 - selectedOption;
		if (input.isKeyPressed(Keyboard::Return)) confirmed = true;
	}


	void drawOption(RenderWindow& window, const char* label, int optIdx, float y) {
		bool sel = (optIdx == selectedOption);
		Text t;
		t.setFont(font);
		t.setCharacterSize(52);
		t.setFillColor(sel ? Color::Yellow : Color(100, 100, 100));
		string str = sel
			? string("> ") + label + " <"
			: string("   ") + label + "   ";
		t.setString(str);
		t.setOrigin(t.getGlobalBounds().width / 2.0f, 0.0f);
		t.setPosition((float)(SCREEN_W / 2), y);
		window.draw(t);
	}


	void render(RenderWindow& window) {
		window.draw(bgSprite);
		if (!fontLoaded) return;

		Text title;
		title.setFont(font);
		title.setCharacterSize(80);
		title.setStyle(Text::Bold);
		title.setFillColor(Color(255, 100, 0));
		title.setString("METAL SLUG");

		title.setOrigin(title.getGlobalBounds().width / 2.f, 0.f);
		title.setPosition((float)(SCREEN_W / 2), 120.f);
		window.draw(title);

		Text sub;
		sub.setFont(font); sub.setCharacterSize(28);
		sub.setFillColor(Color(200, 200, 200));
		sub.setString("SELECT GAME MODE");
		sub.setOrigin(sub.getGlobalBounds().width / 2.f, 0.f);
		sub.setPosition((float)(SCREEN_W / 2), 250.f);
		window.draw(sub);

		drawOption(window, "SURVIVAL MODE", 0, 370.f);
		drawOption(window, "CAMPAIGN MODE", 1, 480.f);

		const char* desc = (selectedOption == 0) ? "3 Levels  |  Pre-built biomes  |  Boss gauntlet" : "Infinite world  |  Fractal terrain  |  Dynamic spawns";
		Text descT;
		descT.setFont(font); descT.setCharacterSize(24);
		descT.setFillColor(Color(170, 200, 255));
		descT.setString(desc);
		descT.setOrigin(descT.getGlobalBounds().width / 2.f, 0.f);
		descT.setPosition((float)(SCREEN_W / 2), 600.f);
		window.draw(descT);

		Text hint;
		hint.setFont(font); hint.setCharacterSize(22);
		hint.setFillColor(Color(120, 120, 120));
		hint.setString("[ UP / DOWN ]  select       [ ENTER ]  confirm");
		hint.setOrigin(hint.getGlobalBounds().width / 2.f, 0.f);
		hint.setPosition((float)(SCREEN_W / 2), 800.f);
		window.draw(hint);
	}

	bool isConfirmed()     const { return confirmed; }

	int  getSelectedMode() const { return selectedOption; }

	void reset() { confirmed = false; selectedOption = 0; }

	~GameModeMenu() {}
};


void EnemyManager::checkBodyCollisionsOnPlayer(Entity* player, GameStats* stats, int charIdx) {
	float px = player->getPosX();
	float py = player->getPosY();
	float pw = player->getWidth();
	float ph = player->getHeight();

	for (int i = 0; i < count; i++) {
		Enemy* e = enemies[i];
		if (!e->isActive()) continue;
		if (!e->overlaps(px, py, pw, ph)) continue;

		if (e->getEnemyType() == Enemy::TYPE_ZOMBIE) {
			Zombie* z = static_cast<Zombie*>(e);
			if (z->transformsOnCollision()) {
				PlayerSoldier* ps = static_cast<PlayerSoldier*>(player);
				ps->getTransformState()->transformTo(z->getTransformType());
				cout << "Player transformed to UNDEAD by Zombie" << endl;
			}
		}
		if (e->getEnemyType() == Enemy::TYPE_MUMMY) {
			MummyWarrior* m = static_cast<MummyWarrior*>(e);
			PlayerSoldier* ps = static_cast<PlayerSoldier*>(player);
			ps->getTransformState()->transformTo(m->getTransformType(), ps->getWeaponType());
		}

		if (stats && (e->getEnemyType() == Enemy::TYPE_ZOMBIE || e->getEnemyType() == Enemy::TYPE_MUMMY)) stats->takeDamage(charIdx);
		float push = (player->getPosX() < e->getPosX()) ? -48.0f : 48.0f;
		player->setPosition(player->getPosX() + push, player->getPosY());
	}
}


class Game {
private:
	RenderWindow window;
	InputManager* inputMgr;
	GameStats* gameStats;
	Camera* camera;
	HUD* hud;
	EnemyManager* enemyMgr;
	Clock clock;
	bool isRunning;

	bool inMenu;
	int selectedMode;
	GameModeMenu* menu;

	Level* survivalLevels[3];
	int currentLevelIndex;
	Level* level;

	PlayerSoldier* characters[4];
	int activeCharIndex;
	float spawnX, spawnY;

	MetalSlug* metalSlug;
	SlugFlyer* slugFlyer;
	SlugMariner* slugMariner;
	AmphibiousSlug* amphibiousSlug;
	bool playerInSlug;
	bool playerInFlyer;
	bool playerInMariner;
	bool playerInAmphib;

	static const int MAX_COLLECTIBLES = 60;
	Collectible* worldCollectibles[MAX_COLLECTIBLES];
	int collectibleCount;

	Texture skyTexture;
	Sprite skySprite;

	Font uiFont;
	bool uiFontLoaded;
	float levelBannerTimer;
	bool completionScreen;
	int highScore;

	static const int SCREEN_W = 1600;
	static const int SCREEN_H = 900;

public:
	Game()
		: window(VideoMode(SCREEN_W, SCREEN_H), "Metal Slug", Style::Close), inputMgr(nullptr), gameStats(nullptr), camera(nullptr), hud(nullptr), enemyMgr(nullptr), isRunning(false), inMenu(true), selectedMode(0), menu(nullptr), currentLevelIndex(0), level(nullptr), activeCharIndex(0), spawnX(200.f), spawnY(300.f), metalSlug(nullptr), slugFlyer(nullptr), slugMariner(nullptr), amphibiousSlug(nullptr), playerInSlug(false), playerInFlyer(false), playerInMariner(false), playerInAmphib(false), collectibleCount(0), uiFontLoaded(false), levelBannerTimer(0.f), completionScreen(false), highScore(0) {
		window.setVerticalSyncEnabled(true);
		window.setFramerateLimit(60);

		for (int i = 0; i < 4; i++) characters[i] = nullptr;
		for (int i = 0; i < 3; i++) survivalLevels[i] = nullptr;
		for (int i = 0; i < MAX_COLLECTIBLES; i++) worldCollectibles[i] = nullptr;

		initCore();
	}

	~Game() {
		cleanupGameplay();
		delete hud; hud = nullptr;
		delete gameStats; gameStats = nullptr;
		delete inputMgr; inputMgr = nullptr;
		delete menu; menu = nullptr;
	}

	void run() {
		isRunning = true;
		clock.restart();

		while (isRunning && window.isOpen()) {
			float dt = clock.restart().asSeconds();
			if (dt > 0.05f) dt = 0.05f;

			Event ev;
			while (window.pollEvent(ev)) {
				if (ev.type == Event::Closed) { isRunning = false; window.close(); }
				inputMgr->handleEvent(ev);
			}
			inputMgr->update(window);

			if (inMenu) runMenu();
			else if (completionScreen) runCompletionScreen();
			else runGame(dt);
		}
	}

private:
	void initCore() {
		gameStats = new GameStats();
		inputMgr = new InputManager();
		hud = new HUD(gameStats);
		menu = new GameModeMenu();
		uiFontLoaded = uiFont.loadFromFile("Sprites/font.ttf");
		if (!uiFontLoaded) uiFontLoaded = uiFont.loadFromFile("C:/Windows/Fonts/arial.ttf");

		loadHighScore();

		if (skyTexture.loadFromFile("Sprites/background/sky.png")) {
			skySprite.setTexture(skyTexture);
			skySprite.setScale((float)SCREEN_W / (float)skyTexture.getSize().x, (float)SCREEN_H / (float)skyTexture.getSize().y);
			skySprite.setPosition(0.f, 0.f);
		}
	}

	void loadHighScore() {
		highScore = 0;
		ifstream in("highscore.txt");
		if (in.good()) in >> highScore;
	}

	void saveHighScore() {
		if (!gameStats) return;
		if (gameStats->getScore() > highScore) highScore = gameStats->getScore();
		ofstream out("highscore.txt");
		if (out.good()) out << highScore;
	}

	void drawHighScoreOnMenu() {
		if (!uiFontLoaded) return;
		Text hs;
		hs.setFont(uiFont);
		hs.setCharacterSize(30);
		hs.setFillColor(Color(255, 230, 80));
		hs.setString("HIGH SCORE: " + to_string(highScore));
		hs.setPosition(40.0f, 820.0f);
		window.draw(hs);
	}


	void initGameplay() {
		survivalLevels[0] = new Level1();
		survivalLevels[1] = new Level2();
		survivalLevels[2] = new Level3();
		currentLevelIndex = 0;
		level = survivalLevels[0];
		completionScreen = false;
		levelBannerTimer = 2.5f;

		spawnX = 3 * 64.f;
		spawnY = 12 * 64.f;

		characters[0] = new MarcoRossi(spawnX, spawnY, 100);
		characters[1] = new EriKasamoto(spawnX, spawnY);
		characters[2] = new Tarma(spawnX, spawnY);
		characters[3] = new FiolinaGermi(spawnX, spawnY);

		for (int i = 0; i < 4; i++) {
			if (i == 0) characters[i]->loadSprite("Sprites/marco.png");
			else if (i == 1) characters[i]->loadSprite("Sprites/eri.png");
			else if (i == 2) characters[i]->loadSprite("Sprites/tarma.png");
			else characters[i]->loadSprite("Sprites/fiolina.png");
			characters[i]->setLevel(level);
			characters[i]->setInputManager(inputMgr);
			characters[i]->setGameStats(gameStats, i);
			characters[i]->setActive(i == 0);
		}

		gameStats->setGrenades(0, 8);
		gameStats->setGrenades(1, 20);
		gameStats->setGrenades(2, 10);
		gameStats->setGrenades(3, 8);
		gameStats->setMaxHP(2, 80);
		gameStats->setHP(2, 80);

		activeCharIndex = 0;
		gameStats->setActiveCharIndex(0);

		float worldW = (float)(level->getWidth() * Block::BLOCK_SIZE);
		float worldH = (float)(level->getHeight() * Block::BLOCK_SIZE);
		camera = new Camera(SCREEN_W, SCREEN_H, worldW, worldH);
		camera->setTarget(characters[0]);
		camera->snapToTarget();

		enemyMgr = new EnemyManager();
		enemyMgr->spawnFromLevel(level, characters[activeCharIndex]);

		playerInSlug = false;
		playerInFlyer = false;
		playerInMariner = false;
		playerInAmphib = false;
		metalSlug = new MetalSlug(6 * 64.f, 14 * 64.f - 170.f, level);
		slugFlyer = new SlugFlyer(12 * 64.f, 10 * 64.f, level);
		slugMariner = new SlugMariner(18 * 64.f, 14 * 64.f - 150.f, level);
		amphibiousSlug = new AmphibiousSlug(24 * 64.f, 14 * 64.f - 165.f, level);

		collectibleCount = 0;
		spawnLevelCollectibles();
	}

	void addC(Collectible* c) {
		if (collectibleCount < MAX_COLLECTIBLES)
			worldCollectibles[collectibleCount++] = c;
		else delete c;
	};


	void spawnLevelCollectibles() {
		for (int i = 0; i < MAX_COLLECTIBLES; i++) {
			delete worldCollectibles[i];
			worldCollectibles[i] = nullptr;
		}
		collectibleCount = 0;

		addC(new Turkey(10 * 64.f, 12 * 64.f));
		addC(new SupplyCrate(22 * 64.f, 12 * 64.f));
		addC(new Fruit(38 * 64.f, 12 * 64.f));
		addC(new SupplyCrate(58 * 64.f, 12 * 64.f));

		if (currentLevelIndex == 0) {
			addC(new POWPrisoner(32 * 64.f, 12 * 64.f));
			addC(new POWPrisoner(76 * 64.f, 12 * 64.f));
			addC(new POWPrisoner(124 * 64.f, 15 * 64.f));
		}
		else if (currentLevelIndex == 1) {
			addC(new POWPrisoner(30 * 64.f, 12 * 64.f));
			addC(new POWPrisoner(52 * 64.f, 12 * 64.f));
			addC(new POWPrisoner(94 * 64.f, 12 * 64.f));
			addC(new POWPrisoner(152 * 64.f, 15 * 64.f));
		}
		else {
			addC(new POWPrisoner(26 * 64.f, 12 * 64.f));
			addC(new POWPrisoner(54 * 64.f, 12 * 64.f));
			addC(new POWPrisoner(88 * 64.f, 12 * 64.f));
			addC(new POWPrisoner(124 * 64.f, 12 * 64.f));
			addC(new POWPrisoner(154 * 64.f, 15 * 64.f));
			addC(new POWPrisoner(188 * 64.f, 15 * 64.f));
		}
	}


	void cleanupGameplay() {
		delete metalSlug;
		metalSlug = nullptr;
		delete slugFlyer;
		slugFlyer = nullptr;
		delete slugMariner;
		slugMariner = nullptr;
		delete amphibiousSlug;
		amphibiousSlug = nullptr;
		delete enemyMgr;
		enemyMgr = nullptr;
		delete camera;
		camera = nullptr;

		for (int i = 0; i < 4; i++) { delete characters[i]; characters[i] = nullptr; }
		for (int i = 0; i < 3; i++) { delete survivalLevels[i]; survivalLevels[i] = nullptr; }
		level = nullptr;

		for (int i = 0; i < MAX_COLLECTIBLES; i++) {
			delete worldCollectibles[i];
			worldCollectibles[i] = nullptr;
		}
		collectibleCount = 0;

		playerInSlug = false;
		playerInFlyer = false;
		playerInMariner = false;
		playerInAmphib = false;
	}


	void loadLevel(int idx) {
		if (idx < 0 || idx > 2) return;
		currentLevelIndex = idx;
		level = survivalLevels[idx];

		spawnX = 3 * 64.f;
		spawnY = 12 * 64.f;

		for (int i = 0; i < 4; i++) {
			if (characters[i]) {
				characters[i]->setLevel(level);
				characters[i]->setPosition(spawnX, spawnY);
			}
		}

		if (metalSlug) { delete metalSlug; metalSlug = nullptr; }
		if (slugFlyer) { delete slugFlyer;  slugFlyer = nullptr; }
		if (slugMariner) { delete slugMariner; slugMariner = nullptr; }
		if (amphibiousSlug) { delete amphibiousSlug; amphibiousSlug = nullptr; }

		metalSlug = new MetalSlug(6 * 64.f, 14 * 64.f - 170.f, level);
		slugFlyer = new SlugFlyer(12 * 64.f, 10 * 64.f, level);
		slugMariner = new SlugMariner(18 * 64.f, 14 * 64.f - 150.f, level);
		amphibiousSlug = new AmphibiousSlug(24 * 64.f, 14 * 64.f - 165.f, level);

		playerInSlug = false;
		playerInFlyer = false;
		playerInMariner = false;
		playerInAmphib = false;

		delete enemyMgr;
		enemyMgr = new EnemyManager();
		enemyMgr->spawnFromLevel(level, characters[activeCharIndex]);
		levelBannerTimer = 2.5f;

		spawnLevelCollectibles();

		float worldW = (float)(level->getWidth() * Block::BLOCK_SIZE);
		float worldH = (float)(level->getHeight() * Block::BLOCK_SIZE);
		camera->setWorldBounds(worldW, worldH);
		camera->snapToTarget();
	}


	PlayerSoldier* activePlayer() const { return characters[activeCharIndex]; }

	void activateDeveloperMode() {
		if (!gameStats) return;
		gameStats->setIsImmortal(true);
		for (int i = 0; i < 4; i++) {
			if (!characters[i]) continue;
			gameStats->setCharAlive(i, true);
			gameStats->setHP(i, gameStats->getMaxHP(i));
			gameStats->setGrenades(i, 50);
		}
		if (characters[0]) characters[0]->giveWeapon(Weapon::WEAPON_HMG, 200);
		if (characters[1]) characters[1]->giveWeapon(Weapon::WEAPON_FLAME, 150);
		if (characters[2]) characters[2]->giveWeapon(Weapon::WEAPON_ROCKET, 40);
		if (characters[3]) characters[3]->giveWeapon(Weapon::WEAPON_LASER, 40);
		cout << "[Developer Mode] Immortal + all required weapons assigned.\n";
	}


	bool anyCharacterAlive() const {
		for (int i = 0; i < 4; i++)
			if (characters[i] && gameStats->isCharAlive(i)) return true;
		return false;
	}


	void switchCharacter() {
		int start = activeCharIndex;
		for (int tries = 0; tries < 4; tries++) {
			int next = (activeCharIndex + 1 + tries) % 4;
			if (next == start) continue;
			if (!gameStats->isCharAlive(next)) continue;

			characters[activeCharIndex]->setActive(false);

			activeCharIndex = next;
			gameStats->setActiveCharIndex(next);

			if (!anyVehicleOccupied())
				camera->setTarget(characters[activeCharIndex]);

			characters[activeCharIndex]->setActive(true);

			characters[activeCharIndex]->setPosition(
				characters[(activeCharIndex + 3) % 4]->getPosX(),
				characters[(activeCharIndex + 3) % 4]->getPosY());

			enemyMgr->setPlayerTarget(characters[activeCharIndex]);
			return;
		}
	}


	void respawnActiveCharacter() {
		PlayerSoldier* p = activePlayer();
		if (!p) return;
		p->setPosition(spawnX, spawnY);
		p->setActive(true);
	}


	bool anyVehicleOccupied() const { return playerInSlug || playerInFlyer || playerInMariner || playerInAmphib; }

	void leaveVehicle(Vehicle* v) {
		PlayerSoldier* p = activePlayer();
		if (!p || !v) return;
		playerInSlug = playerInFlyer = playerInMariner = playerInAmphib = false;
		p->setActive(true);
		p->setPosition(v->getPosX() + 90.f, v->getPosY());
		if (metalSlug) { metalSlug->setOccupied(false); metalSlug->setInputManager(nullptr); }
		if (slugFlyer) { slugFlyer->setOccupied(false); slugFlyer->setInputManager(nullptr); }
		if (slugMariner) { slugMariner->setOccupied(false); slugMariner->setInputManager(nullptr); }
		if (amphibiousSlug) { amphibiousSlug->setOccupied(false); amphibiousSlug->setInputManager(nullptr); }
		camera->setTarget(p);
	}

	bool tryEnterVehicle(Vehicle* v, bool& flag, float px, float py) {
		PlayerSoldier* p = activePlayer();
		if (!p || !v || !v->isActive() || anyVehicleOccupied()) return false;
		float dx = px - (v->getPosX() + v->getWidth() * 0.5f);
		float dy = py - (v->getPosY() + v->getHeight() * 0.5f);
		bool near = (dx * dx + dy * dy) < 260.f * 260.f;
		if (p->isActive() && near && inputMgr->isKeyPressed(Keyboard::E)) {
			flag = true;
			p->setActive(false);
			v->setOccupied(true);
			if (MetalSlug* ms = dynamic_cast<MetalSlug*>(v)) ms->setInputManager(inputMgr);
			if (SlugFlyer* sf = dynamic_cast<SlugFlyer*>(v)) sf->setInputManager(inputMgr);
			if (SlugMariner* sm = dynamic_cast<SlugMariner*>(v)) sm->setInputManager(inputMgr);
			if (AmphibiousSlug* as = dynamic_cast<AmphibiousSlug*>(v)) as->setInputManager(inputMgr);
			camera->setTarget(v);
			return true;
		}
		return false;
	}

	void handleVehicleInput() {
		PlayerSoldier* p = activePlayer();
		if (!p) return;

		if (anyVehicleOccupied() && (inputMgr->isKeyPressed(Keyboard::E) || inputMgr->isKeyPressed(Keyboard::V))) {
			if (playerInSlug) leaveVehicle(metalSlug);
			else if (playerInFlyer) leaveVehicle(slugFlyer);
			else if (playerInMariner) leaveVehicle(slugMariner);
			else if (playerInAmphib) leaveVehicle(amphibiousSlug);
			return;
		}

		float px = p->getPosX() + p->getWidth() * 0.5f;
		float py = p->getPosY() + p->getHeight() * 0.5f;
		if (tryEnterVehicle(metalSlug, playerInSlug, px, py)) return;
		if (tryEnterVehicle(slugFlyer, playerInFlyer, px, py)) return;
		if (tryEnterVehicle(slugMariner, playerInMariner, px, py)) return;
		if (tryEnterVehicle(amphibiousSlug, playerInAmphib, px, py)) return;
	}


	void handleCollectibles(float dt) {
		PlayerSoldier* p = activePlayer();
		if (!p) return;

		float px = p->getPosX() + p->getWidth() * 0.5f;
		float py = p->getPosY() + p->getHeight() * 0.5f;

		for (int i = 0; i < collectibleCount; i++) {
			Collectible* c = worldCollectibles[i];
			if (!c || !c->isActive() || c->isCollected()) continue;

			c->update(dt);

			float cx = c->getPosX() + c->getWidth() * 0.5f;
			float cy = c->getPosY() + c->getHeight() * 0.5f;
			float dx = px - cx, dy = py - cy;

			if (dx * dx + dy * dy < 48.f * 48.f) {
				POWPrisoner* pow = dynamic_cast<POWPrisoner*>(c);
				if (pow) {
					if (inputMgr->isKeyPressed(Keyboard::F)) {
						c->pickup(p);
						if (pow->hasDroppedCrate() && collectibleCount < MAX_COLLECTIBLES) {
							worldCollectibles[collectibleCount++] = pow->getCrate();
						}
					}
				}
				else {
					c->pickup(p);
				}
			}
		}
	}


	void ejectActivePlayerFromDestroyedVehicle(Vehicle* vehicle) {
		PlayerSoldier* p = activePlayer();
		if (!p || !vehicle) return;
		bool wasOccupied = (vehicle == metalSlug && playerInSlug) || (vehicle == slugFlyer && playerInFlyer)
			|| (vehicle == slugMariner && playerInMariner) || (vehicle == amphibiousSlug && playerInAmphib);
		if (!wasOccupied) return;
		leaveVehicle(vehicle);
	}

	void handleVehicleDamageFromEnemies() {
		if (!enemyMgr) return;

		if (metalSlug && metalSlug->isActive()) {
			int dmg = 0;
			if (enemyMgr->checkHitsOnPlayer(metalSlug->getPosX(), metalSlug->getPosY(),
				metalSlug->getWidth(), metalSlug->getHeight(), dmg)) {
				metalSlug->takeDamage(dmg > 0 ? dmg : 1);
				if (!metalSlug->isActive()) ejectActivePlayerFromDestroyedVehicle(metalSlug);
			}
		}

		if (slugFlyer && slugFlyer->isActive()) {
			int dmg = 0;
			if (enemyMgr->checkHitsOnPlayer(slugFlyer->getPosX(), slugFlyer->getPosY(),
				slugFlyer->getWidth(), slugFlyer->getHeight(), dmg)) {
				slugFlyer->takeDamage(dmg > 0 ? dmg : 1);
				if (!slugFlyer->isActive()) ejectActivePlayerFromDestroyedVehicle(slugFlyer);
			}
		}
		if (slugMariner && slugMariner->isActive()) {
			int dmg = 0;
			if (enemyMgr->checkHitsOnPlayer(slugMariner->getPosX(), slugMariner->getPosY(), slugMariner->getWidth(), slugMariner->getHeight(), dmg)) {
				slugMariner->takeDamage(dmg > 0 ? dmg : 1);
				if (!slugMariner->isActive()) ejectActivePlayerFromDestroyedVehicle(slugMariner);
			}
		}
		if (amphibiousSlug && amphibiousSlug->isActive()) {
			int dmg = 0;
			if (enemyMgr->checkHitsOnPlayer(amphibiousSlug->getPosX(), amphibiousSlug->getPosY(), amphibiousSlug->getWidth(), amphibiousSlug->getHeight(), dmg)) {
				amphibiousSlug->takeDamage(dmg > 0 ? dmg : 1);
				if (!amphibiousSlug->isActive()) ejectActivePlayerFromDestroyedVehicle(amphibiousSlug);
			}
		}
	}


	void handlePlayerProjectiles() {
		if (!enemyMgr) return;
		PlayerSoldier* p = activePlayer();
		if (!p) return;

		p->damageProjectiles(enemyMgr);

		float mx, my, mw, mh; int mdmg; bool mpierce;
		if (p->consumeMelee(mx, my, mw, mh, mdmg, mpierce)) {
			enemyMgr->damageEnemyRectEx(mx, my, mw, mh, mdmg, false, false, mpierce, true);
			if (mpierce) gameStats->addScore(50);
		}

		if (playerInSlug && metalSlug) metalSlug->damageProjectiles(enemyMgr);
		if (playerInFlyer && slugFlyer) slugFlyer->damageProjectiles(enemyMgr);
		if (playerInMariner && slugMariner) slugMariner->damageProjectiles(enemyMgr);
		if (playerInAmphib && amphibiousSlug) amphibiousSlug->damageProjectiles(enemyMgr);

		MarcoRossi* marco = dynamic_cast<MarcoRossi*>(p);
		if (marco) {
			for (int i = 0; i < marco->getBulletCount(); i++) {
				Bullet* b = marco->getBullet(i);
				if (!b || !b->isActive()) continue;
				float bx = b->getPosX() - b->getHitW() * 0.5f;
				float by = b->getPosY() - b->getHitH() * 0.5f;
				if (enemyMgr->damageEnemyRect(bx, by, b->getHitW(), b->getHitH(), b->getDamage())) {
					b->onHit();
				}
			}
		}
	}


	void handlePlayerDeath() {
		int dmgState = gameStats->getDamageState(activeCharIndex);
		if (dmgState == GameStats::DAMAGE_DEAD) {
			int lives = gameStats->getLives(activeCharIndex);
			if (lives > 0) {
				respawnActiveCharacter();
			}
			else {
				gameStats->setCharAlive(activeCharIndex, false);
				characters[activeCharIndex]->setActive(false);

				if (anyCharacterAlive()) {
					switchCharacter();
				}
				else {
					saveHighScore();
					cleanupGameplay();
					inMenu = true;
					menu->reset();
				}
			}
		}
	}


	bool checkLevelComplete() {
		if (!level || !enemyMgr) return false;

		float progressX = 0.0f;
		PlayerSoldier* p = activePlayer();
		if (p) progressX = p->getPosX();
		if (playerInSlug && metalSlug) progressX = metalSlug->getPosX();
		if (playerInFlyer && slugFlyer) progressX = slugFlyer->getPosX();
		if (playerInMariner && slugMariner) progressX = slugMariner->getPosX();
		if (playerInAmphib && amphibiousSlug) progressX = amphibiousSlug->getPosX();

		bool killedAllEnemies = (enemyMgr->getCount() == 0);
		bool reachedEndOfLevel = progressX >= (float)(level->getWidth() * Block::BLOCK_SIZE) - 420.0f;

		if (!killedAllEnemies || !reachedEndOfLevel) return false;

		gameStats->addScore(1000);

		if (currentLevelIndex < 2) {
			currentLevelIndex++;
			loadLevel(currentLevelIndex);
			return true;
		}

		cout << "LEVEL 3 COMPLETED - SURVIVAL COMPLETE.\n";
		saveHighScore();
		cleanupGameplay();
		completionScreen = true;
		inMenu = false;
		return true;
	}


	void drawCenteredText(const char* message, unsigned int size, float y, Color color) {
		if (!uiFontLoaded) return;
		Text text;
		text.setFont(uiFont);
		text.setCharacterSize(size);
		text.setStyle(Text::Bold);
		text.setFillColor(color);
		text.setString(message);
		text.setOrigin(text.getGlobalBounds().width / 2.0f, 0.0f);
		text.setPosition((float)SCREEN_W / 2.0f, y);
		window.draw(text);
	}

	void drawLevelBannerAndBiomeLabels(float camX, float camY) {
		if (!uiFontLoaded || !level) return;

		float zoneW = (float)(level->getWidth() * Block::BLOCK_SIZE) / 3.0f;
		const char* names[3] = { "PLAINS BIOME", "AERIAL BIOME", "AQUATIC BIOME - WATER" };
		Color colors[3] = { Color(160, 255, 160), Color(220, 220, 230), Color(120, 190, 255) };
		for (int i = 0; i < 3; i++) {
			float screenX = zoneW * (i + 0.5f) - camX;
			if (screenX < -300.f || screenX > SCREEN_W + 300.f) continue;
			Text label;
			label.setFont(uiFont);
			label.setCharacterSize(26);
			label.setStyle(Text::Bold);
			label.setFillColor(colors[i]);
			label.setString(names[i]);
			label.setOrigin(label.getGlobalBounds().width / 2.0f, 0.0f);
			label.setPosition(screenX, 82.0f);
			window.draw(label);
		}

		if (levelBannerTimer > 0.0f) {
			char title[32];
			snprintf(title, sizeof(title), "LEVEL %d", currentLevelIndex + 1);
			drawCenteredText(title, 90, 320.0f, Color(255, 230, 80));
		}
	}

	void runCompletionScreen() {
		if (inputMgr->isKeyPressed(Keyboard::Escape)) { isRunning = false; window.close(); return; }
		if (inputMgr->isKeyPressed(Keyboard::Return) || inputMgr->isKeyPressed(Keyboard::M)) {
			saveHighScore();
			completionScreen = false;
			inMenu = true;
			menu->reset();
			return;
		}

		window.clear(Color(5, 10, 25));
		drawCenteredText("LEVEL 3 COMPLETED", 82, 250.0f, Color(255, 230, 80));
		drawCenteredText("SURVIVAL MODE COMPLETE", 44, 365.0f, Color(190, 230, 255));
		drawCenteredText("Press ENTER / M to return to Main Menu", 30, 520.0f, Color(230, 230, 230));
		drawCenteredText("Press ESC to close the game", 28, 570.0f, Color(200, 200, 200));
		window.display();
	}


	void runMenu() {
		if (inputMgr->isKeyDown(Keyboard::Escape)) {
			isRunning = false; window.close(); return;
		}
		menu->update(*inputMgr);
		if (menu->isConfirmed()) {
			selectedMode = menu->getSelectedMode();
			inMenu = false;
			initGameplay();
		}
		window.clear(Color(10, 10, 40));
		menu->render(window);
		drawHighScoreOnMenu();
		window.display();
	}


	void runGame(float dt) {
		if (inputMgr->isKeyPressed(Keyboard::Escape)) {
			saveHighScore();
			cleanupGameplay();
			inMenu = true;
			menu->reset();
			return;
		}

		if (inputMgr->isKeyPressed(Keyboard::D)) {
			activateDeveloperMode();
		}


		if (inputMgr->isKeyPressed(Keyboard::Z)) {
			if (!playerInSlug && !playerInFlyer && !playerInMariner && !playerInAmphib) switchCharacter();
		}

		if (inputMgr->isKeyPressed(Keyboard::Num1)) loadLevel(0);
		if (inputMgr->isKeyPressed(Keyboard::Num2)) loadLevel(1);
		if (inputMgr->isKeyPressed(Keyboard::Num3)) loadLevel(2);

		if (levelBannerTimer > 0.0f) {
			levelBannerTimer -= dt;
			if (levelBannerTimer < 0.0f) levelBannerTimer = 0.0f;
		}

		handleVehicleInput();

		gameStats->updateDamageTimers(dt);

		PlayerSoldier* p = activePlayer();
		if (p && !anyVehicleOccupied()) p->update(dt);

		if (metalSlug && metalSlug->isActive()) metalSlug->update(dt);
		if (slugFlyer && slugFlyer->isActive())  slugFlyer->update(dt);
		if (slugMariner && slugMariner->isActive()) slugMariner->update(dt);
		if (amphibiousSlug && amphibiousSlug->isActive()) amphibiousSlug->update(dt);

		if (level) level->update(dt);

		if (camera) camera->update(dt);

		if (enemyMgr) {
			int scoreGained = 0;
			enemyMgr->update(dt, scoreGained);
			if (scoreGained > 0) gameStats->addScore(scoreGained);

			handlePlayerProjectiles();
			if (enemyMgr) gameStats->addScore(enemyMgr->consumeBonusScore());
			handleVehicleDamageFromEnemies();

			if (p && p->isActive()) {
				int dmg = 0;
				if (enemyMgr->checkHitsOnPlayer(
					p->getPosX(), p->getPosY(),
					p->getWidth(), p->getHeight(), dmg)) {
					gameStats->takeDamage(activeCharIndex);
				}
				enemyMgr->checkBodyCollisionsOnPlayer(p, gameStats, activeCharIndex);
			}
		}

		handleCollectibles(dt);

		handlePlayerDeath();

		if (checkLevelComplete()) {
			return;
		}

		window.clear(Color(100, 149, 237));

		float camX = camera ? camera->getViewX() : 0.f;
		float camY = camera ? camera->getViewY() : 0.f;

		window.draw(skySprite);

		if (level) level->render(window, camX, camY);
		drawLevelBannerAndBiomeLabels(camX, camY);

		if (metalSlug && metalSlug->isActive()) metalSlug->render(window, camX, camY);
		if (slugFlyer && slugFlyer->isActive())  slugFlyer->render(window, camX, camY);
		if (slugMariner && slugMariner->isActive()) slugMariner->render(window, camX, camY);
		if (amphibiousSlug && amphibiousSlug->isActive()) amphibiousSlug->render(window, camX, camY);

		if (p) p->render(window, camX, camY);

		for (int i = 0; i < collectibleCount; i++)
			if (worldCollectibles[i] && worldCollectibles[i]->isActive())
				worldCollectibles[i]->render(window, camX, camY);

		if (enemyMgr) enemyMgr->render(window, camX, camY);

		hud->render(window, activeCharIndex);

		window.display();
	}
};
