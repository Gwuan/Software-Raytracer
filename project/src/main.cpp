//External includes
#ifdef ENABLE_VLD
#include "vld.h"
#endif
#include "SDL.h"
#include "SDL_surface.h"
#undef main

//Standard includes
#include <iostream>
#include <vector>

//Project includes
#include <memory>

#include "Timer.h"
#include "Renderer.h"
#include "Scene.h"

using namespace dae;


std::shared_ptr<Scene> g_pScene;
uint32_t g_SceneIndex{ 5 };

std::vector<std::shared_ptr<Scene>> scenes; 

void InitScenes()
{
	scenes.push_back(std::make_shared<Scene_W1>());
	scenes.push_back(std::make_shared<Scene_W2>());
	scenes.push_back(std::make_shared<Scene_W3_TestScene>());
	scenes.push_back(std::make_shared<Scene_W3>());
	scenes.push_back(std::make_shared<Scene_W4_TestScene>());
	scenes.push_back(std::make_shared<Scene_W4_ReferenceScene>());
	scenes.push_back(std::make_shared<Scene_W4_BunnyScene>());
}

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

enum FollowingSceneType
{
	Previous,
	Next
};

void ShowFollowingScene(const FollowingSceneType& followingScene)
{
	switch (followingScene)
	{
	case FollowingSceneType::Next:
			g_SceneIndex++;
			g_SceneIndex %= scenes.size();
		break;
	case FollowingSceneType::Previous:
		if(g_SceneIndex == 0)
		{
			g_SceneIndex = scenes.size() -1;
		}
		else
		{
			g_SceneIndex--;
		}
		break;
	}

	g_pScene->Deinitializing();

	g_pScene = scenes[g_SceneIndex];
	g_pScene->Initialize();

	std::cout << "Displaying: " << g_pScene->GetSceneName() << std::endl;
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"RayTracer - Jelle Rubbens",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	InitScenes();

	g_pScene = scenes[g_SceneIndex];

	g_pScene->Initialize();

	//Start loop
	pTimer->Start();

	// Start Benchmark
	// pTimer->StartBenchmark();

	float printTimer = 0.f;
	bool isLooping = true;
	bool takeScreenshot = false;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if (e.key.keysym.scancode == SDL_SCANCODE_X)
					takeScreenshot = true;

				if (e.key.keysym.scancode == SDL_SCANCODE_F2)
					pRenderer->ToggleShadows();

				if (e.key.keysym.scancode == SDL_SCANCODE_F3)
					pRenderer->CycleLightingMode();

				if(e.key.keysym.scancode == SDL_SCANCODE_LEFT)
					ShowFollowingScene(FollowingSceneType::Previous);

				if(e.key.keysym.scancode == SDL_SCANCODE_RIGHT)
					ShowFollowingScene(FollowingSceneType::Next);

				if(e.key.keysym.scancode == SDL_SCANCODE_UP)
				{
					pRenderer->IncreaseMSAA();
					std::cout << "Current samples used for MSAA: " << pRenderer->GetSampleAmount() << std::endl;
				}

				if(e.key.keysym.scancode == SDL_SCANCODE_DOWN)
				{
					pRenderer->DecreaseMSAA();
					std::cout << "Current samples used for MSAA: " << pRenderer->GetSampleAmount() << std::endl;
				}

				break;
			}
		}

		//--------- Update ---------
		g_pScene->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render(g_pScene.get());

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
		}

		//Save screenshot after full render
		if (takeScreenshot)
		{
			if (!pRenderer->SaveBufferToImage())
				std::cout << "Screenshot saved!" << std::endl;
			else
				std::cout << "Something went wrong. Screenshot not saved!" << std::endl;
			takeScreenshot = false;
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	g_pScene.reset();
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}