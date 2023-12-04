#pragma once
#include <vector>
#include <GameState.h>

#ifdef TARGET_MSVC
    #include <SDL.h>
#endif
#ifdef TARGET_UNIX
    #include <SDL2/SDL.h>
#endif


/*!
 * Takes the GameState, time data and input and modifies GameState.
 */
class GameStateLogic : public IGameStateGridChangeObserver
{
    public:
        /* ====================  LIFECYCLE     ======================================= */
        GameStateLogic () :mIsToCheckGameGrid(false)
        {
        }                            /* constructor */


        /* ====================  ACCESSORS     ======================================= */

        /*!
         * Reacts to an input event.
         * @param e an sdl event object to react to
         * @param gameState the game state to react upon, the gameState may be modified based on the input.
         */
       void Input (SDL_Event& e, GameState& gameState) const;


        /* ====================  MUTATORS      ======================================= */

        /*!
         * Reacts to amount of time (in miliseconds) past since last call of the function.
         * @param deltaTime uint32 the amount of time since the function was last called.
         * @param gameState the game state to react upon, the gameState may be modified based on the input.
         * @return false if something goes wrong that requires code change, true otherwise.
         */
        bool Update (Uint32 deltaTime, GameState& gameState);


        /*!
         * Overrides notify function; sets a flag for update to check whether grid change in GameState caused any matches.
         */
        void NotifyOfGameStateGridChange();


        /* ====================  OPERATORS     ======================================= */

    protected:
        /* ====================  DATA MEMBERS  ======================================= */

    private:
        /* ====================  DATA MEMBERS  ======================================= */
        static const Uint32 ANIMATION_DURATION_MILIS;
        static const Uint32 MIN_MATCH_SIZE;
        bool mIsToCheckGameGrid;

}; /* -----  end of class GameStateLogic  ----- */

