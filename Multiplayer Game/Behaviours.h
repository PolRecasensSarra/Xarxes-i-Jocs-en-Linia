#pragma once


enum class BehaviourType : uint8;

struct Behaviour
{
	GameObject *gameObject = nullptr;
	bool isServer = false;
	bool isLocalPlayer = false;

	virtual BehaviourType type() const = 0;

	virtual void start() { }

	virtual void onInput(const InputController &input) { }

	virtual void update() { }

	virtual void destroy() { }

	virtual void onCollisionTriggered(Collider &c1, Collider &c2) { }

	virtual void write(OutputMemoryStream &packet) { }

	virtual void read(const InputMemoryStream &packet) { }

	virtual bool GetIfPowerUp() { return false; }
	virtual void SetIfPowerUp(bool powerup){}

	virtual int GetSpaceshipType() { return 0; }
	virtual void SetSpaceshipType(int type) {}
};


enum class BehaviourType : uint8
{
	None = 0,
	Spaceship = 1,
	Laser = 2,
	Asteroid = 3,
	Battery = 4,
	Shield = 5,
	DoubleBullet = 6,
	SuperSpeed = 7,
};

enum class AudioType : uint32
{
	None = 0,
	Laser = 1, 
	Explosion = 2,
	PowerUp = 3,
	Shield = 4,
	Damage = 5,
	ShieldBreak = 6,
};


struct Laser : public Behaviour
{
	float secondsSinceCreation = 0.0f;
	bool powerUp = false;

	BehaviourType type() const override { return BehaviourType::Laser; }

	void start() override;

	void update() override;

	bool GetIfPowerUp() override;
	void SetIfPowerUp(bool powerup) override;

	void write(OutputMemoryStream& packet) override;

	void read(const InputMemoryStream& packet) override;

};

struct Asteroid : public Behaviour
{

	BehaviourType type() const override { return BehaviourType::Asteroid; }

	void start() override;

	void update() override;

	void destroy() override;

	void onCollisionTriggered(Collider& c1, Collider& c2) override;
};

struct Battery : public Behaviour
{

	BehaviourType type() const override { return BehaviourType::Asteroid; }

	void start() override;

	void update() override;
};

struct DoubleBullet : public Behaviour
{

	BehaviourType type() const override { return BehaviourType::DoubleBullet; }

	void start() override;

	void update() override;
};

struct Shield : public Behaviour
{

	BehaviourType type() const override { return BehaviourType::Shield; }

	void start() override;

	void update() override;
};

struct SuperSpeed : public Behaviour
{
	BehaviourType type() const override { return BehaviourType::SuperSpeed; }

	void start() override;

	void update() override;
};

struct Spaceship : public Behaviour
{
	static const uint8 MAX_HIT_POINTS = 10;
	uint8 hitPoints = MAX_HIT_POINTS;

	GameObject *lifebar = nullptr;
	bool is_invulnerable = false;
	float invulnerable_time = 1.0f;
	float respawn_time = 1.0f;
	float battery_time = 10.0f;
	float doubleBullet_time = 10.0f;
	bool is_respawning = false;
	bool powerUp = false;
	bool doubleBullet = false;
	bool shielded = false;
	bool textureChanging = false;
	bool playSound = false;
	Texture* original_texture = nullptr;
	Texture* shielded_texture = nullptr;

	float advanceSpeed = 200.0f;
	bool is_superspeed = false;
	float superSpeed_time = 10.0f;

	int spaceshipType = -1;

	AudioType audioType = AudioType::None;

	BehaviourType type() const override { return BehaviourType::Spaceship; }

	void start() override;

	void onInput(const InputController &input) override;

	void update() override;

	void destroy() override;

	void onCollisionTriggered(Collider &c1, Collider &c2) override;

	void write(OutputMemoryStream &packet) override;

	void read(const InputMemoryStream &packet) override;

	int GetSpaceshipType() override;
	void SetSpaceshipType(int type) override;

	void Respawn();
};

