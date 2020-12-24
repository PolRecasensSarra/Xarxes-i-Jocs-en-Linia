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
};


enum class BehaviourType : uint8
{
	None = 0,
	Spaceship = 1,
	Laser = 2,
	Asteroid = 3,
	Battery = 4,
	Shield = 5,
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

struct Shield : public Behaviour
{

	BehaviourType type() const override { return BehaviourType::Shield; }

	void start() override;

	void update() override;
};

struct Spaceship : public Behaviour
{
	static const uint8 MAX_HIT_POINTS = 5;
	uint8 hitPoints = MAX_HIT_POINTS;

	GameObject *lifebar = nullptr;
	bool powerUp = false;
	bool shielded = false;

	BehaviourType type() const override { return BehaviourType::Spaceship; }

	void start() override;

	void onInput(const InputController &input) override;

	void update() override;

	void destroy() override;

	void onCollisionTriggered(Collider &c1, Collider &c2) override;

	void write(OutputMemoryStream &packet) override;

	void read(const InputMemoryStream &packet) override;


};
