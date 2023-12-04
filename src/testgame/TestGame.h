#pragma once
#include <GameStateLogic.h>
#include <GameState.h>
#include <iostream>
#include <memory>

#ifdef TARGET_MSVC
    #include <SDL.h>
#endif
#ifdef TARGET_UNIX
    #include <SDL2/SDL.h>
#endif


/*!
 * This is the main application code.
 * It holds the main game loop.
 */
class TestGame
{
    public:
        /* ====================  LIFECYCLE     ======================================= */
        TestGame ();                             /* constructor */
        ~TestGame ();

        /* ====================  ACCESSORS     ======================================= */

        /* ====================  MUTATORS      ======================================= */

        /*!
         * Handle menu level input, pass game input onto GameStateLogic.
         * @return true if the game loop is to continue running.
         */
        bool Input ();


        /*!
         * Handle menu level update, pass game update onto GameStateLogic.
         * @return false if there's a problem or if the application is exited, true otherwise.
         */
        bool Update ();


        /* ====================  OPERATORS     ======================================= */

        /*!
         *  Initializes OpenGL context and opens a window.
         *  @return true if the initialization was successful.
         */
        bool Init ();


        /*!
         * This is the function main.cpp needs to call to run the application.
         * It runs the Init function; then loops though input, update and render functions.
         */
        void Start ();

    protected:
        /* ====================  DATA MEMBERS  ======================================= */

    private:
        /* ====================  DATA MEMBERS  ======================================= */

        GameStateLogic mGameStateLogic;
        std::unique_ptr<GameState> mGameState;
        Uint32 mTimeAtLastFrame;


}; /* -----  end of class TestGame  ----- */
