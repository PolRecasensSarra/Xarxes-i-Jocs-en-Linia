#pragma once

#define USE_TASK_MANAGER

struct Texture;

class ModuleResources : public Module
{
public:

	Texture *background = nullptr;
	Texture *space = nullptr;
	Texture *asteroid1 = nullptr;
	Texture *asteroid2 = nullptr;
	Texture *spacecraft1 = nullptr;
	Texture *spacecraft2 = nullptr;
	Texture *spacecraft3 = nullptr;
	Texture *spacecraft4 = nullptr;
	Texture *spacecraft5 = nullptr;
	Texture *spacecraft6 = nullptr;
	Texture* spacecraft1Shield = nullptr;
	Texture* spacecraft2Shield = nullptr;
	Texture* spacecraft3Shield = nullptr;
	Texture* spacecraft4Shield = nullptr;
	Texture* spacecraft5Shield = nullptr;
	Texture* spacecraft6Shield = nullptr;
	Texture *laser = nullptr;
	Texture *laser2 = nullptr;
	Texture *explosion1 = nullptr;
	Texture *explosion2 = nullptr;
	Texture *battery = nullptr;
	Texture *shield = nullptr;
	Texture *doubleBullet = nullptr;

	AnimationClip *explosionClip = nullptr;

	AudioClip *audioClipLaser = nullptr;
	AudioClip *audioClipExplosion = nullptr;
	AudioClip *audioPowerUp = nullptr;
	AudioClip *audioShield = nullptr;
	AudioClip *audioDamage = nullptr;
	AudioClip *audioShieldBreak = nullptr;

	bool finishedLoading = false;

private:

	bool init() override;

#if defined(USE_TASK_MANAGER)
	
	class TaskLoadTexture : public Task
	{
	public:

		const char *filename = nullptr;
		Texture **texture = nullptr;
		int id;

		void execute() override;
	};

	static const int MAX_RESOURCES = 32;
	TaskLoadTexture tasks[MAX_RESOURCES] = {};
	uint32 taskCount = 0;
	uint32 finishedTaskCount = 0;

	void onTaskFinished(Task *task) override;

	void loadTextureAsync(const char *filename, Texture **texturePtrAddress, int id);

#endif

};

