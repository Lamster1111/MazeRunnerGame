#include "Scene0.h"
#include <SDL.h>
#include "SDL_image.h"
#include "Randomizer.h"
#include "Physics.h"
#include "GameManager.h"
#include <iostream>

Scene0::Scene0(SDL_Window* sdlWindow_){
	window = sdlWindow_;
	renderer = SDL_CreateRenderer(window, -1,SDL_RENDERER_ACCELERATED);
	nextS = false;
}

Scene0::~Scene0(){// Rember to delete every pointer NO MEMORY LEAKS!!!!!!

	OnDestroy();
}	

bool Scene0::OnCreate(float GOF){
	int w, h;
	float xAxis = 32.0f;
	float yAxis = 18.0f;
	float zAxis = 1.0f;
	SDL_GetWindowSize(window,&w,&h);
	
	Matrix4 ndc = MMath::viewportNDC(w, h);
	Matrix4 ortho = MMath::orthographic(0.0f, xAxis, 0.0f, yAxis, 0.0f, zAxis);
	projectionMatrix = ndc * ortho;

	//temporary border walls
	wallLeft = new Plane(Vec3(1.0f, 0.0f, 0.0f), 0.0f);//1.0f, 0.0f, 0.0f), 0.0f
	wallRight = new Plane(Vec3(-1.0f, 0.0f, 0.0f), xAxis);
	wallTop = new Plane(Vec3(0.0f, -1.0f, 0.0f), yAxis);
	wallBottom = new Plane(Vec3(0.0f, 1.0f, 0.0f), 0.0f);

	IMG_Init(IMG_INIT_PNG); //Make loading PNGs easer so only use PNGs
	//Load the Back ground image and set the texture as well
	surfacePtr = IMG_Load("Art/grassnoflower256.png");
	background = SDL_CreateTextureFromSurface(renderer, surfacePtr);
	
	if (surfacePtr == nullptr) {
		std::cerr << "Imgage does not work" << std::endl;
		return false;
	}
	if (background == nullptr) {
		printf("%s\n", SDL_GetError());
		return false;
	}

	SDL_FreeSurface(surfacePtr);


	//Load the crouton image and set the texture as well
	surfacePtr = IMG_Load("Art/Crouton256.png");
	croutonTexture = SDL_CreateTextureFromSurface(renderer, surfacePtr);

	if (surfacePtr == nullptr) {
		std::cerr << "Imgage does not work" << std::endl;
		return false;
	}
	if (background == nullptr) {
		printf("%s\n", SDL_GetError());
		return false;
	}

	SDL_FreeSurface(surfacePtr);
	

	//Loads in the wall image and set the texture to the walls
	surfacePtr = IMG_Load("Art/Wall02.png");
	texturePtr = SDL_CreateTextureFromSurface(renderer, surfacePtr);

	if (surfacePtr == nullptr) {
		std::cerr << "Imgage does not work" << std::endl;
		return false;
	}
	if (texturePtr == nullptr) {
		printf("%s\n", SDL_GetError());
		return false;
	}

	SDL_FreeSurface(surfacePtr);

	//Making the level
	level = new Level(NUMWALL);
	level->makeLevel(5); 
	level->setWallTextures(texturePtr);

	//load player character
	surfacePtr = IMG_Load("Art/BreadManConcept.png");
	texturePtr = SDL_CreateTextureFromSurface(renderer, surfacePtr);

	if (surfacePtr == nullptr) {
		std::cerr << "Imgage does not work" << std::endl;
		return false;
	}
	if (texturePtr == nullptr) {
		printf("%s\n", SDL_GetError());
		return false;
	}

	SDL_FreeSurface(surfacePtr);

	player = new PlayerCharacter();
	player->setPos(Vec3(5.0f, 5.0f, 0.0f));
	player->setBoundingSphere(Sphere(0.5f));
	player->setTexture(texturePtr);
	player->setHealth(3.0f);

	//character health icons
	surfacePtr = IMG_Load("Art/BreadHealth.png");
	health = SDL_CreateTextureFromSurface(renderer, surfacePtr);

	if (surfacePtr == nullptr) {
		std::cerr << "Imgage does not work" << std::endl;
		return false;
	}
	if (health == nullptr) {
		printf("%s\n", SDL_GetError());
		return false;
	}
	
	SDL_FreeSurface(surfacePtr);


	


	//load collectibles
	//health pickup
	surfacePtr = IMG_Load("Art/BreadHealth.png");
	texturePtr = SDL_CreateTextureFromSurface(renderer, surfacePtr);

	if (surfacePtr == nullptr) {
		std::cerr << "Imgage does not work" << std::endl;
		return false;
	}
	if (texturePtr == nullptr) {
		printf("%s\n", SDL_GetError());
		return false;
	}
	healthPickup = new GameObject();

	SDL_FreeSurface(surfacePtr);

	healthPickup->setPos(Vec3(16.0f, 9.0f, 0.0f));
	healthPickup->setBoundingSphere(Sphere(0.5f));
	healthPickup->setTexture(texturePtr);


	return true;
}

void Scene0::OnDestroy() {
	
	if (player) delete player, player = nullptr;
	if (level) delete level, level = nullptr;
	


	if (wallLeft) delete wallLeft, wallLeft = nullptr;
	if (wallRight) delete wallRight, wallRight = nullptr;
	if (wallTop) delete wallTop, wallTop = nullptr;
	if (wallBottom) delete wallBottom, wallBottom = nullptr;

	SDL_DestroyRenderer(renderer);
}

void Scene0::Update(const float time) {
	/// This is the physics in the x and y dimension don't mess with z

	//Player Movement
	Physics::SimpleNewtonMotion(*player, time);

	//Player Hits Edge of Window Walls
	//Potentially Obsolete with working circle rect collision
	if (Physics::PlaneSphereCollision(*player, *wallLeft) == true) {
		player->setPos(Vec3(player->getBoundingSphere().r, player->getPos().y, player->getPos().z));
	}
	if (Physics::PlaneSphereCollision(*player, *wallRight) == true) { //Load the next scene here
		player->setPos(Vec3(-wallRight->d - player->getBoundingSphere().r, player->getPos().y, player->getPos().z));
		nextS = true;
	}
	if (Physics::PlaneSphereCollision(*player, *wallTop) == true) {
		player->setPos(Vec3(player->getPos().x, -wallTop->d - player->getBoundingSphere().r, player->getPos().z));
	}
	if (Physics::PlaneSphereCollision(*player, *wallBottom) == true) {
		player->setPos(Vec3(player->getPos().x, player->getBoundingSphere().r, player->getPos().z));
	}

	//Player Hits Walls
	for (int i = 0; i < level->getWallNum(); ++i) {
		if (Physics::CircleRectCollision(*player, *level->getWall(i)) == true) {
			Physics::SimpleNewtonMotion(*player, -time);
		}
	}

	//Player Hits Collectibles
	if (healthPickup) {
		if (Physics::SphereSphereCollision(*player, *healthPickup) == true) {
			if (player->restoreHealth(1.0f) == true) {
				delete healthPickup;
				healthPickup = nullptr;
			}
		}
	}

	if (weaponPickup) {
		if (Physics::SphereSphereCollision(*player, *weaponPickup) == true) {
			player->setAltWeaponAvailable(true);
			player->setWeaponType(1);
			delete weaponPickup;
			weaponPickup = nullptr;
		}
	}
	
	
}

void Scene0::HandleEvents(const SDL_Event& sdlEvent) { //Make stuff happen here with the clickety clack

	player->HandleEvents(sdlEvent, projectionMatrix);

	
}

void Scene0::Render() {
	SDL_SetRenderDrawColor(renderer, 0,0,0,0);
	SDL_Renderer* renderer = SDL_GetRenderer(window);
	//Clear screen
	SDL_RenderClear(renderer);
	
	//Draws the background
	SDL_Rect bg;
	bg.x = 0;
	bg.y = 0;
	bg.w = 1280;
	bg.h = 720;
	SDL_RenderCopy(renderer, background, nullptr, &bg);

	//Draws all the walls
	SDL_Rect WallRect;
	Vec3 wallScreenCoords;
	int WallW, WallH;

	for (int i = 0; i < NUMWALL; ++i) {
		SDL_QueryTexture(level->getWall(i)->getTexture(), nullptr, nullptr, &WallW, &WallH);
		wallScreenCoords = projectionMatrix * level->getWall(i)->getPos();
		WallRect.x = static_cast<int> (wallScreenCoords.x) - 40;
		WallRect.y = static_cast<int> (wallScreenCoords.y) - 40;
		WallRect.w = 80;
		WallRect.h = 80;
		SDL_RenderCopy(renderer, level->getWall(i)->getTexture(), nullptr, &WallRect);
	}


	
	//Draw player
	SDL_Rect playerRect;
	Vec3 playerScreenCoords;
	int playerW, playerH;

	SDL_QueryTexture(player->getTexture(), nullptr, nullptr, &playerW, &playerH);
	playerScreenCoords = projectionMatrix * player->getPos();
	playerRect.x = static_cast<int>(playerScreenCoords.x) - playerW;
	playerRect.y = static_cast<int>(playerScreenCoords.y) - playerH;
	playerRect.w = playerW * 2;
	playerRect.h = playerH * 2;
	SDL_RenderCopyEx(renderer, player->getTexture(), nullptr, &playerRect, player->getAngle(), nullptr, SDL_FLIP_NONE);

	
	if (player->getHealth() > 0)
	{
		SDL_Rect healthRect;

		healthRect.x = 10;
		healthRect.y = 0;
		healthRect.w = 100;
		healthRect.h = 100;
		SDL_RenderCopy(renderer, health, nullptr, &healthRect);

		if (player->getHealth() > 1)
		{
			SDL_Rect health1Rect;

			health1Rect.x = 40;
			health1Rect.y = 0;
			health1Rect.w = 100;
			health1Rect.h = 100;
			SDL_RenderCopy(renderer, health, nullptr, &health1Rect);

			if (player->getHealth() > 2)
			{
				SDL_Rect health2Rect;

				health2Rect.x = 70;
				health2Rect.y = 0;
				health2Rect.w = 100;
				health2Rect.h = 100;
				SDL_RenderCopy(renderer, health, nullptr, &health2Rect);
			}
		}
	}

	//Draw collectibles
	SDL_Rect collectibleRect;
	Vec3 healthPickupScreenCoords;
	Vec3 weaponPickupScreenCoords;
	int collectibleW, collectibleH;

	if (healthPickup) {
		SDL_QueryTexture(healthPickup->getTexture(), nullptr, nullptr, &collectibleW, &collectibleH);
		healthPickupScreenCoords = projectionMatrix * healthPickup->getPos();
		collectibleRect.x = static_cast<int>(healthPickupScreenCoords.x) - collectibleW / 8;
		collectibleRect.y = static_cast<int>(healthPickupScreenCoords.y) - collectibleH / 8;
		collectibleRect.w = collectibleW / 4;
		collectibleRect.h = collectibleH / 4;
		SDL_RenderCopy(renderer, healthPickup->getTexture(), nullptr, &collectibleRect);
	}


	//Update screen
	SDL_RenderPresent(renderer);



	
}


bool Scene0::getDead() {
	if (player->getHealth() <= 0) return true;
	return false;
}


bool Scene0::nextScene() {
	return nextS;
}