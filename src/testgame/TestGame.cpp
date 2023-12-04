#include "TestGame.h"
#include <assert.h>
#include <stdio.h>
#include <GameStateRenderer.h>
#include <GameStateLogic.h>
#include <GameState.h>
#include <time.h>

#ifdef TARGET_MSVC
    #include <SDL.h>
#endif
#ifdef TARGET_UNIX
    #include <SDL2/SDL.h>
#endif


TestGame::TestGame () : mGameStateLogic()
{
}


bool TestGame::Input()
{
    SDL_Event e;

    while (SDL_PollEvent (&e) != 0){
        mGameStateLogic.Input (e, *mGameState);
        // quit if user presses Alt+F4 or closes the window
        if (e.type == SDL_QUIT){
            return false;
        }
    }
    return true;
}


void TestGame::Start()
{
    Init();

    bool isLooping = true;
    while (isLooping) {
        isLooping &= Input();
        isLooping &= Update();
        isLooping &= GameStateRenderer::Render (*mGameState);
    }
}		/* -----  end of function Start();  ----- */


TestGame::~TestGame ()
{
	GameStateRenderer::DestroyRenderer();
    SDL_Quit();
}


bool TestGame::Init()
{
    bool isSuccessful = true;
    printf ("TestGame::Init starting...\n");

    srand (time (NULL));

    int sdlInitReturn = SDL_Init (SDL_INIT_VIDEO);
	if (sdlInitReturn < 0) {
        isSuccessful = false;
		printf ("SDL_INIT failed. SDL_ERROR: %s\n", SDL_GetError());
	}
	assert (sdlInitReturn >= 0);

    isSuccessful = isSuccessful && GameStateRenderer::InitRenderer();
    // create a new game

    mGameState.reset (new GameState (8, 8, 3, 60));
    mGameState->AttachGameStateGridChangeObserver (&mGameStateLogic);
    mTimeAtLastFrame = SDL_GetTicks();

    if (isSuccessful) {
        printf ("TestGame::Init DONE.\n");
    } else {
        printf ("TestGame::Init FAILED.\n");
    }
    return isSuccessful;
}


bool TestGame::Update()
{
    bool isSuccessful = true;

    Uint32 timeAtThisFrame = SDL_GetTicks();
    Uint32 deltaTime = timeAtThisFrame - mTimeAtLastFrame;
    mTimeAtLastFrame = timeAtThisFrame;
    isSuccessful = isSuccessful && mGameStateLogic.Update (deltaTime, *mGameState);

    return isSuccessful;
}		/* -----  end of function 'Update'  ----- */
