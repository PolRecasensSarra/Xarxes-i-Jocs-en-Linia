#include "Networks.h"


#if defined(USE_TASK_MANAGER)

void ModuleResources::TaskLoadTexture::execute()
{
	*texture = App->modTextures->loadTexture(filename);
	(*texture)->id = id;
	
}

#endif


bool ModuleResources::init()
{
	background = App->modTextures->loadTexture("background.jpg");
	int id = 0;

#if !defined(USE_TASK_MANAGER)
	space = App->modTextures->loadTexture("space_background.jpg");
	asteroid1 = App->modTextures->loadTexture("asteroid1.png");
	asteroid2 = App->modTextures->loadTexture("asteroid2.png");
	spacecraft1 = App->modTextures->loadTexture("spacecraft1.png");
	spacecraft2 = App->modTextures->loadTexture("spacecraft2.png");
	spacecraft3 = App->modTextures->loadTexture("spacecraft3.png");
	loadingFinished = true;
	completionRatio = 1.0f;
#else
	loadTextureAsync("space_background.jpg", &space, id);
	loadTextureAsync("asteroid1.png",        &asteroid1, ++id);
	loadTextureAsync("asteroid2.png",        &asteroid2, ++id);
	loadTextureAsync("spacecraft1.png",      &spacecraft1, ++id);
	loadTextureAsync("spacecraft2.png",      &spacecraft2, ++id);
	loadTextureAsync("spacecraft3.png",      &spacecraft3, ++id);
	loadTextureAsync("spacecraft4.png",      &spacecraft4, ++id);
	loadTextureAsync("spacecraft5.png",      &spacecraft5, ++id);
	loadTextureAsync("spacecraft6.png",      &spacecraft6, ++id);
	loadTextureAsync("spacecraft1Shield.png", &spacecraft1Shield, ++id);
	loadTextureAsync("spacecraft2Shield.png", &spacecraft2Shield, ++id);
	loadTextureAsync("spacecraft3Shield.png", &spacecraft3Shield, ++id);
	loadTextureAsync("spacecraft4Shield.png", &spacecraft4Shield, ++id);
	loadTextureAsync("spacecraft5Shield.png", &spacecraft5Shield, ++id);
	loadTextureAsync("spacecraft6Shield.png", &spacecraft6Shield, ++id);
	loadTextureAsync("laser.png",            &laser, ++id);
	loadTextureAsync("laser2.png",            &laser2, ++id);
	loadTextureAsync("explosion1.png",       &explosion1, ++id);
	loadTextureAsync("explosion2.png",       &explosion2, ++id);
	loadTextureAsync("powerUp.png",       &battery, ++id);
	loadTextureAsync("shield.png",       &shield, ++id);
	loadTextureAsync("doubleBullet.png",       &doubleBullet, ++id);
	loadTextureAsync("SuperSpeed.png",       &superSpeed, ++id);
#endif

	audioClipLaser = App->modSound->loadAudioClip("laser.wav");
	audioClipExplosion = App->modSound->loadAudioClip("explosion.wav");
	audioPowerUp = App->modSound->loadAudioClip("powerup.wav");
	audioShield = App->modSound->loadAudioClip("shield.wav");
	audioDamage = App->modSound->loadAudioClip("damage.wav");
	audioShieldBreak = App->modSound->loadAudioClip("shieldBreak.wav");


	return true;
}

#if defined(USE_TASK_MANAGER)

void ModuleResources::loadTextureAsync(const char * filename, Texture **texturePtrAddress, int id)
{
	ASSERT(taskCount < MAX_RESOURCES);
	
	TaskLoadTexture *task = &tasks[taskCount++];
	task->id = id;
	task->owner = this;
	task->filename = filename;
	task->texture = texturePtrAddress;

	App->modTaskManager->scheduleTask(task, this);
}

void ModuleResources::onTaskFinished(Task * task)
{
	ASSERT(task != nullptr);

	TaskLoadTexture *taskLoadTexture = dynamic_cast<TaskLoadTexture*>(task);

	for (uint32 i = 0; i < taskCount; ++i)
	{
		if (task == &tasks[i])
		{
			finishedTaskCount++;
			task = nullptr;
			break;
		}
	}

	ASSERT(task == nullptr);

	if (finishedTaskCount == taskCount)
	{
		finishedLoading = true;

		// Create the explosion animation clip
		explosionClip = App->modRender->addAnimationClip();
		explosionClip->frameTime = 0.1f;
		explosionClip->loop = false;
		for (int i = 0; i < 16; ++i)
		{
			float x = (i % 4) / 4.0f;
			float y = (i / 4) / 4.0f;
			float w = 1.0f / 4.0f;
			float h = 1.0f / 4.0f;
			explosionClip->addFrameRect(vec4{ x, y, w, h });
		}
	}
}

#endif
