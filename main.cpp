#include "pch.h"
//#undef main

//Standard includes
#include <iostream>

//Project includes
#include "ETimer.h"
#include "ERenderer.h"
#include "ECamera.h"

void ShutDown(SDL_Window* pWindow)
{
	Elite::Camera::CleanUp();
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
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
		"Dual Rasterizer - Vincent Van Denberghe [2GD07E]",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	auto pTimer{ std::make_unique<Elite::Timer>() };
	auto pRenderer{ std::make_unique<Elite::Renderer>(pWindow) };
	Elite::Camera* pCamera = Elite::Camera::GetInstance();
	pCamera->SetAspectRatio(width, height);
	pCamera->SetProjectionMatrix(pRenderer->GetRenderMode());

	std::cout << "Controls:\n\t" <<
				 "LMB: Move Forward & Backwards, Yaw\n\t" <<
				 "RMB: Pitch & Yaw\n\t" <<
				 "LMB + RMB: Move Up & Down\n\t" <<
				 "W, A, S, D: Move\n\t" <<
				 "T: Switch texture filtering technique \n\t" <<
				 "R: Toggle rotation\n\t" <<
				 "C: Change cull mode\n\t" <<
				 "E: Switch render mode\n\t" <<
				 "F: Toggle fire effect\n\t" <<
				 "V: Toggle V-Sync\n\t" <<
				 "Konami Code: ????\n";

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;

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
				if (e.key.keysym.scancode == SDL_SCANCODE_T)
				{
					pRenderer->ChangeTechnique();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_R)
				{
					pRenderer->SetIsRotating();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_C)
				{
					pRenderer->ChangeCullMode();
				}
				if(e.key.keysym.scancode == SDL_SCANCODE_E)
				{
					pRenderer->ChangeRenderMode();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F)
				{
					pRenderer->ChangeRenderFireEffect();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_V)
				{
					pRenderer->ToggleVSync();
				}
				pRenderer->RegisterKeyPress(e.key.keysym.scancode);
				break;
			}
		}
		const float elapsedSec{ pTimer->GetElapsed() };

		//--------- Rotation -------
		pRenderer->UpdateRotation(elapsedSec);

		//--------- Camera ---------
		pCamera->Update(elapsedSec);

		//--------- Render ---------
		switch(pRenderer->GetRenderMode())
		{
		case RenderMode::HARDWARE:
			pRenderer->RenderHardware();
			break;
		case RenderMode::SOFTWARE:
			pRenderer->RenderSoftware();
			break;
		}

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "FPS: " << pTimer->GetFPS() << std::endl;
		}

	}
	pTimer->Stop();

	//Shutdown "framework"
	ShutDown(pWindow);
	return 0;
}