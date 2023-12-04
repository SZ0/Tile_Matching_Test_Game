#pragma once

#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <vector>


#ifdef TARGET_MSVC
    #include <SDL.h>
#endif
#ifdef TARGET_UNIX
    #include <SDL2/SDL.h>
#endif


class IGameStateGridChangeObserver;

/*!
 *  Acts as storage of information about the game and provides an interface to manage its state.
 *  Provides enough information to allow GameStateRenderer to render the game scene or independently to allow GameStateLogic to make decisions on how to process input or react to changes in GameState.
 *  Stores a grid representing the game board with each element representing a tile and its value a tile color.
 *  GameState is not intended to hold information about application (eg. game menus) nor information on how to render. Apart from convenience functions (especially if needed for rendering), it leaves the game mechanics decisions to GameStateLogic.
 */
class GameState
{
    public:
        /*!
         * Colors of tiles in the GameState
         */
        enum Color {
            NotAColor = 0, ///< used for when a color cannot be returned
            Red = 1,
            Green = 2,
            Blue = 3,
            Purple = 4,
            Yellow = 5,
            DestroyedColor = 6 ///< used to denote a tile not to be rendered and to be removed from the game (collapse)
        };

        /*!
         * GameState animations such as switching two tiles, destroying two tiles or collapsing.
         */
        enum AnimationState {
            Idle = 0,            ///< denotes game time is running and no animation is running
            SwappingTiles = 1,   ///< denotes that an animation of swapping two tiles is running
            DestroyingTiles = 2, ///< denotes an animation is running for matched tiles that they are being destroyed
            CollapsingTiles = 3, ///< denotes an animation is running of destroyed tiles being covered by falling tiles from above
            GameOver = 4         ///< denotes the game is over (input and update will be ignored)
        };

        /// how many colors of tiles can be on the board, also used for number of OpenGL texture objects to generate
        static const unsigned int sNUMBER_OF_TILE_COLORS;


        /* ====================  LIFECYCLE     ======================================= */
        explicit GameState (int rows, int columns, int minMatchSize, int maxGameplayTimeSeconds) :  /* constructor */
			mMinMatchSize (minMatchSize),
			mTileDragData(),
			mGameTime (0),
			mGameplayTime (0),
			mTilesBeingDestroyed(),
			mColumnsToCollapse(),
			mGrid(),
			mAnimationState (Idle),
			mGameStateGridChangeObservers(),
			mTimeAnimationStart (0),
			mAnimationDuration (0),
            mMaxGameplayTimeSeconds (maxGameplayTimeSeconds),
            mGameScore (0)
        {
            printf ("GameState::GameState: Creating a new GameState...\n");
            ResetGridToRandomNoNMatches (rows, columns, mMinMatchSize);
        }


        /* ====================  ACCESSORS     ======================================= */
        /*!
         * Retrieves the tile Color at given row and column.
         * @param row The number of the row (zero-indexed).
         * @param column The number of the column (zero-indexed).
         * @return The Color enum describing the color of the tile at given row and columns.
         */
        Color GetColorAt (int row, int column) const;


        /*!
         * Retrieves the tile Color at given row and column or returns a random color if row or column are out of bounds.
         * @param row The number of the row (zero-indexed).
         * @param column The number of the column (zero-indexed).
         * @return The Color enum describing the color of the tile at given row and columns.
         */
        Color GetColorAtOrRandom (int row, int column) const;


        /*!
         * Retrieves the tile Color at index.
         * @param index the index of tile to sample.
         * @return The Color enum describing the color of the tile at given index.
         */
        Color GetColorAt (int index) const;


        /*!
         * Returns an enum of type Color of a linearly chosen random value other than NotAColor or DestroyedColor.
         * @return Enum of type Color that is not NotAColor.
         */
        static Color GetRandomColor();


        /*!
         * Finds all tiles of same color that are in either horizontal or vertical rows of n.
         * Uses brute force to locate these occurances.
         * @param gameState The game state for which the bool table will be generated.
         * @return A vector of booleans with all rows of three marked as true and others marked as false.
         */
        std::vector<bool> GetMatchesOfN (int n) const;


        /*!
         * Gets the number of rows on the board/grid of the current game state.
         * @return The number of rows on the board/grid.
         */
        int GetRows () const
        {
            return mRows;
        }		/* -----  end of function GetRows  ----- */


        /*!
         * Gets the number of columns on the board/grid of the current game state.
         * @return The number of columns on the board/grid.
         */
        int GetColumns () const
        {
            return mColumns;
        }		/* -----  end of function GetColumns  ----- */


        /*!
         * Returns the current state of animation on GameState.
         * @return The enum of currently running animation (enum's value is Idle if no animation is currently running).
         */
        AnimationState GetAnimationState() const
        {
            return mAnimationState;
        }


        /*!
         * Returns whether a tile is beingn dragged.
         * @return Returns true if GameState currently has a tile being dragged, false otherwise.
         */
        bool IsDragActive() const
        {
            return mTileDragData.mIsActive;
        }


        /*!
         * Gets the start location of a tile drag.
         * @return Returns glm::vec2 with location on grid with last set mouse down event.
         */
        glm::vec2 GetDragStartLocation() const;


        /*!
         * Gets the current location of a tile drag.
         * @return Returns glm::vec2 with location on grid with last set mouse move or up event.
         */
        glm::vec2 GetDragCurrentLocation() const;


        /*!
         * Retrieves dragged tile's row.
         * @return an int index of dragged tile's row.
         */
        int GetDraggedTileRow() const;


        /*!
         * Retrieves dragged tile's column.
         * @return an int index of dragged tile's column.
         */
        int GetDraggedTileColumn() const;


        /*!
         * Retrieves the row of the tile being replaced by current drag.
         * @return an int index of replaced tile's row.
         */
        int GetReplacedTileRow() const;


        /*!
         * Retrieves the column of the tile being replaced by current drag.
         * @return an int index of replaced tile's column.
         */
        int GetReplacedTileColumn() const;


        /*!
         * Retrieves the displacement for dragged tile (equals -displacement for replaced tile).
         * @return glm::vec3 with the x and y values containing the displacement.
         */
        glm::vec3 GetCurrentDraggedTileDisplacement() const;


        /*!
         * Map grid coordinates to corresponding row on the grid.
         * @param gridCoordinates glm::vec2 where x and y are elements of [0,1].
         */
        int GetRowOfGridCoordinates (glm::vec2 gridCoordinates) const;


        /*!
         * Map grid coordinates to corresponding column on the grid.
         * @param gridCoordinates glm::vec2 where x and y are elements of [0,1].
         */
        int GetColumnOfGridCoordinates (glm::vec2 gridCoordinates) const;


        /*!
         * Returns whether swap should be reversed if no matches are found.
         * @return true if swap should be reversed at end of animation when no matches are found.
         */
        bool GetIsSwapBack() const;


        /*!
         * Gets the current animation's progress.
         * @return float from [0,1] with 0 meaning, the animation just started and 1 that it ended.
         */
        float GetAnimationPercentage() const;


        /*!
         * Answers whether a tile at location is being destroyed. Always returns false if no DestroyingTiles animation is running.
         * @param row the row of the tile queried for.
         * @param column the column of the tile queried for.
         * @return true if the tile is being destroyed, false otherwise.
         */
        bool IsTileBeingDestroyed (int row, int column) const;


        /*!
         * Retrieves the info marking columns for collapsing.
         * @return the vector of mColumns vectors, each with 2 integers, one indicating the lowest hole in the column and the other its size.
         */
        std::vector<std::vector<int>> GetColumnsToCollapse() const;


        /*!
         * Retrieves the selected tile's row.
         * @return the row of the selected tile
         */
        int GetSelectedTileRow() const;


        /*!
         * Retrieves the selected tile's column.
         * @return the column of the selected tile
         */
        int GetSelectedTileColumn() const;


        /*!
         * Retrieves the leftover playable time until the game ends.
         * @return the time left in seconds, rounded up.
         */
        int GetGameplayTimeLeft() const;


        /*!
         * Retrieves the current score. It simply equals the number of destroyed tiles.
         * @return current score.
         */
        int GetScore() const
        {
            return mGameScore;
        }


        /* ====================  MUTATORS      ======================================= */

        /*!
         * Adds points to total score.
         * @param points the amount of points to add to total game score.
             */
        void AddToScore (int points);


        /*!
         * Generates a board/grid with rows * colums tiles/elements. Each tile is of a random color.
         * @param rows The number of rows to generate.
         * @param columns The number of columns to generate.
         */
        void ResetGridToRandom (int rows, int columns);


        /*!
         * Resets the board/grid to a size of rows * columns with random tile colors.
         * But also makes sure there aren't already matches the size of n or greater on the board/grid.
         * Uses brute force to identify matches, randomly replaces colors of matches and repeats until matches are gone.
         * @param rows Number of rows in the new grid of tiles.
         * @param columns Number of columns in the new grid of tiles.
         * @param n How many tiles of the same color in a row count as a match. Must be at least 2.
         */
        void ResetGridToRandomNoNMatches (int rows, int columns, int n);


        /*!
         * Sets the start of a tile drag.
         * @param gridMouseDownLocation the location in grid coordinate system (x and y are elements of [0,1]).
         * @return Returns true if location was set without an issue and false otherwise.
         */
        bool SetDragStartLocation (glm::vec2 gridMouseDownLocation);


        /*!
         * Sets the current location of the mouse cursor for drag.
         * @param gridMouseDownLocation the location in grid coordinate system (x and y are elements of [0,1]).
         * @return Returns true if location was set without an issue and false otherwise.
         */
        bool SetDragCurrentLocation (glm::vec2 gridMouseCurrentLocation);


        /*!
         * Pre-calculates values used for rendering or reacting to a drag&drop action.
         * GameStateRenderer and GameStateLogic make use of these values.
         */
        void UpdateDragCache();


        /*!
         * Replaces the tile currently dragged.
         * @param animationDuration time in miliseconds for the swapping animation to take.
         * @param animateFromCurrentPositionOn whether animation should be shortened based on how close to final position the tile already is.
         * @param swapBack if true, the grid at the end of swap should be checked for matches and swap is reversed if none are found.
         * @returns true if the tiles are eligible to be switched, returns false otherwise.
         */
        bool SwapDraggedAndReplacedTiles (Uint32 animationDuration, bool animateFromCurrentPositionOn, bool swapBack);


        /*!
         * Runs animation to swap two given tiles.
         * @param tileARow the row of tile A or first tile.
         * @param tileAColumn the column of tile A or first tile.
         * @param tileBRow the row of tile B or second tile.
         * @param tileBColumn the column of tile B or second tile.
         * @param animationDuration time in miliseconds for the swapping animation to take.
         * @param swapBack if true, the grid at the end of swap should be checked for matches and swap is reversed if none are found.
         */
        void SwapTiles (int tileARow, int tileAColumn, int tileBRow, int tileBColumn, Uint32 animationDuration, bool swapBack);


        /*!
         * Deactivates drag.
         */
        void DeactivateDrag();


        /*!
         * Set swapback to false. Used to indicate swap caused a match.
         */
        void ResetIsSwapBack();


        /*!
         * Changes relevant variables based on mAnimationState.
         * @param deltaTime The amount of time by which to move the animation.
         */
        void Elapse (Uint32 deltaTime);


        /*!
         * Destroy tiles.
         * @param tilesToDestory vector of bool values, mGrid tiles with corresponding true value will be destroyed.
         * @param animationDuration the time the animation should take to destroy the tiles.
         */
        bool DestroyTiles (std::vector<bool> tilesToDestory, Uint32 animationDuration);


        /*!
         * Collapse the given columns.
         * @param columnsToCollapse a vector of a vectors per column, each containing two integers: the first tells the number of row to collapse to, and the second how many tiles should be squished.
         * @param animationDuration how long should the collapse animation take.
         */
        bool CollapseColumns (std::vector<std::vector<int>> columnsToCollapse, Uint32 animationDuration);


        /*!
         * Adds an object implementing IGameStateGridChangeObserver interface to be notified whenever the grid is changed. (Eg. when two tiles are swapped.)
         * @param gameStateGridChangeObserver object implementing IGameStateGridChangeObserver interface.
         */
        void AttachGameStateGridChangeObserver (IGameStateGridChangeObserver* gameStateGridChangeObserver);


        /*!
         * Sets tile at given row and column as selected.
         */
        void SelectTile (int row, int column);


        /*!
         * Resets tile selected to -1, -1.
         */
        void DeselectTile();

        /* ====================  OPERATORS     ======================================= */

    protected:
        /* ====================  DATA MEMBERS  ======================================= */

    private:
        /*!
         * Copy constructor is not supported for GameState.
         */
        GameState (const GameState& gameState)
        {
            printf ("ERROR: GameState copy constructor used despite being hidden.");
            assert (0);
            // warning fix
            (void)gameState;
        }

        /*!
         * Copy assignment is not supported for GameState.
         */
        GameState& operator= (const GameState& gameState)
        {
            printf ("ERROR: GameState copy assignment used despite being hidden.");
            assert (0);
            // warning fix
            (void)gameState;
            // warning fix, this function should never actually be used.
            return *const_cast<GameState*>(&gameState);
        }


        /*!
         * Notifies all mGameStateGridChangeObservers.
         */
        void NotifyGameStateGridChangeObservers();


        // data for drag&drop and swap a tile
        struct TileDragData {
            TileDragData() {
                mIsActive = false;
                mStartLocation = glm::vec2 (0.0f, 0.0f);
                mCurrentLocation = glm::vec2 (0.0f, 0.0f);
                // inferred data
                mDraggedTileRow = -1;
                mDraggedTileColumn = -1;
                mReplacedTileRow = -1;
                mReplacedTileColumn = -1;
                mSelectedTileRow = -1;
                mSelectedTileColumn = -1;
                mCurrentTileDisplacement = glm::vec3 (0.0f, 0.0f, 0.0f);
                // additional data for swap animation
                mAnimationStartingTileDisplacement = glm::vec3 (0.0f, 0.0f, 0.0f);
				mIsSwapBack = false;
            }
            // input data
            bool mIsActive;
            glm::vec2 mStartLocation;
            glm::vec2 mCurrentLocation;
            // cache data inferred via call to UpdateDragCache
            glm::vec3 mCurrentTileDisplacement;
            int mDraggedTileRow, mDraggedTileColumn;
            int mReplacedTileRow, mReplacedTileColumn;
            int mSelectedTileRow, mSelectedTileColumn;
            // additional data for swap animation
            glm::vec3 mAnimationStartingTileDisplacement;
            bool mIsSwapBack;   ///< when true, GameStateLogic should run swapback on lack of matches after swap
        } mTileDragData;

        // data for destroying tiles animation
        std::vector<bool> mTilesBeingDestroyed;

        // data for collapsing tiles animation
        std::vector<std::vector<int>> mColumnsToCollapse;

        /* ====================  DATA MEMBERS  ======================================= */
        std::vector<Color> mGrid;           ///< the board
        int mRows, mColumns, mMinMatchSize; ///< determines the number of tiles on the board
        AnimationState mAnimationState;     ///< current state of the GameState animation
        Uint32 mGameTime;                   ///< time elapsed playing this game (used for measuring animation progress)
        Uint32 mGameplayTime;               ///< time elapsed playing this game - time spent on animations (used for measuring time left to play before game ends)
        std::vector<class IGameStateGridChangeObserver*> mGameStateGridChangeObservers;
        // animation data
        Uint32 mTimeAnimationStart;
        Uint32 mAnimationDuration;
        int mMaxGameplayTimeSeconds;
        int mGameScore;

}; /* -----  end of class GameState  ----- */


class IGameStateGridChangeObserver {
    public:
        virtual void NotifyOfGameStateGridChange() = 0;
        virtual ~IGameStateGridChangeObserver() {}
};
