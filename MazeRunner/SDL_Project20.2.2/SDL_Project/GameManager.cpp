#include "GameManager.h"
#include "Window.h"
#include "Timer.h"
#include "Scene0.h"
#include "SceneD.h"
#include "SceneMenu.h"
#include "SceneWin.h"

#include <iostream>

GameManager::GameManager() {
	windowPtr = nullptr;
	timer = nullptr;
	isRunning = true;
	currentScene = nullptr;
	playerHealth = 3.0f;

	//This will keep track of wich scene it is -1 will be the menu and -2 will be death -3 will be the win screen
	sceneNum = 0;
}

float GameManager::health() {


	return 3;
}


/// In this OnCreate() method, fuction, subroutine, whatever the word, 
bool GameManager::OnCreate() {
	const int SCREEN_WIDTH = 1280;
	const int SCREEN_HEIGHT = 720;
	windowPtr = new Window(SCREEN_WIDTH, SCREEN_HEIGHT);
	if (windowPtr == nullptr) {
		OnDestroy();
		return false;
	}
	if (windowPtr->OnCreate() == false) {
		OnDestroy();
		return false;
	}

	timer = new Timer();
	if (timer == nullptr) {
		OnDestroy();
		return false;
	}
	if (currentScene == nullptr) {
		currentScene = new Scene0(windowPtr->GetSDL_Window());
	}

	if (currentScene == nullptr) {
		OnDestroy();
		return false;
	}

	if (currentScene->OnCreate(playerHealth) == false) {
		OnDestroy();
		return false;
	}

	return true;
}



/// Here's the whole game
void GameManager::Run() {
	SDL_Event sdlEvent;
	timer->Start();
	while (isRunning) {
		
		while (SDL_PollEvent(&sdlEvent)) {
			currentScene->HandleEvents(sdlEvent);
			if (sdlEvent.type == SDL_QUIT) {
				isRunning = false;

			}
			else if (sdlEvent.type == SDL_KEYDOWN) {
				switch (sdlEvent.key.keysym.scancode) {
				case SDL_SCANCODE_ESCAPE:
					currentScene->OnDestroy();
					delete currentScene;
					currentScene = new SceneMenu(windowPtr->GetSDL_Window());
					currentScene->OnCreate(playerHealth);
					sceneNum = -1;
					break;
				//case SDL_SCANCODE_F1:
				//	currentScene->OnDestroy();
				//	delete currentScene;
				//	currentScene = new Scene1(windowPtr->GetSDL_Window());
				//	currentScene->OnCreate(playerHealth);
				//	sceneNum = 1;
				//	break;
				//	// If we need more scenes later I don't wanna rewrite this 
				//case SDL_SCANCODE_P:
				//	currentScene->OnDestroy();
				//	delete currentScene;
				//	currentScene = new SceneD(windowPtr->GetSDL_Window());
				//	currentScene->OnCreate(playerHealth);
				//	break;
				//case SDL_SCANCODE_F3:
				//	playerHealth = currentScene->getHealth();
				//	currentScene->OnDestroy();
				//	delete currentScene;
				//	currentScene = new Scene3(windowPtr->GetSDL_Window());
				//	currentScene->OnCreate(playerHealth);
				//	sceneNum = 3;
				//	break;
				//case SDL_SCANCODE_F4:
				//	currentScene->OnDestroy();
				//	delete currentScene;
				//	currentScene = new Scene4(windowPtr->GetSDL_Window());
				//	currentScene->OnCreate(playerHealth);
				//	sceneNum = 3;
				//	break;
				//case SDL_SCANCODE_F5:
				//	playerHealth = currentScene->getHealth();
				//	currentScene->OnDestroy();
				//	delete currentScene;
				//	currentScene = new Scene5(windowPtr->GetSDL_Window());
				//	currentScene->OnCreate(playerHealth);
				//	sceneNum = 5;
				//	break;
				//case SDL_SCANCODE_F10:
				//	playerHealth = currentScene->getHealth();
				//	currentScene->OnDestroy();
				//	delete currentScene;
				//	currentScene = new Scene0(windowPtr->GetSDL_Window());
				//	currentScene->OnCreate(playerHealth);
				//	sceneNum = 0;
				//	break;


				case SDL_SCANCODE_RETURN:
					ToggleFullscreen(windowPtr->GetSDL_Window());
					break;
				default:
					
					break;

				}

			}
		}
		//Check to see what happens when you die. The code decides your fate, worship the code, THE CODE!!!!
		if (currentScene->getDead() && sceneNum >= 0) {
			currentScene->OnDestroy();
			delete currentScene;
			currentScene = new SceneD(windowPtr->GetSDL_Window());
			currentScene->OnCreate(playerHealth);
			sceneNum = -2;
		}
		else if (currentScene->getDead() && sceneNum == -1) {
			//Get the player health here from scene
			currentScene->OnDestroy();
			delete currentScene;
			currentScene = new Scene0(windowPtr->GetSDL_Window());
			currentScene->OnCreate(playerHealth);
			//Set the player health here
			sceneNum = 0;
		}
		else if (currentScene->getDead() && sceneNum <= -2) {
			currentScene->OnDestroy();
			delete currentScene;
			currentScene = new SceneMenu(windowPtr->GetSDL_Window());
			currentScene->OnCreate(playerHealth);
			sceneNum = -1;
		}
		


		else if (currentScene->nextScene() && sceneNum == 5) {//Loads the win screen
			currentScene->OnDestroy();
			delete currentScene;
			currentScene = new SceneWin(windowPtr->GetSDL_Window());
			currentScene->OnCreate(playerHealth);
			sceneNum = -3;
		}
		 
		currentScene->Update(timer->GetDeltaTime());
		currentScene->Render();
		
		/// Keeep the event loop running at a proper rate
		SDL_Delay(timer->GetSleepTime(60)); ///60 frames per sec
		timer->UpdateFrameTicks();
		
	}

}

void GameManager::ToggleFullscreen(SDL_Window* Window) {
	Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN;
	bool IsFullscreen = SDL_GetWindowFlags(Window) & FullscreenFlag;
	SDL_SetWindowFullscreen(Window, IsFullscreen ? 0 : FullscreenFlag);
	SDL_ShowCursor(IsFullscreen);
}

GameManager::~GameManager() {
	OnDestroy();
}

void GameManager::OnDestroy(){
	if (windowPtr) delete windowPtr;
	if (timer) delete timer;
	if (currentScene) delete currentScene;
}

