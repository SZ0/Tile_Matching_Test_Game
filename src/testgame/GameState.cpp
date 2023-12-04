#include "GameState.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef TARGET_MSVC
    #include <SDL.h>
#endif
#ifdef TARGET_UNIX
    #include <SDL2/SDL.h>
#endif


const unsigned int GameState::sNUMBER_OF_TILE_COLORS = 5;


GameState::Color GameState::GetRandomColor() {
    int colorInt = 1 + glm::linearRand (0.0f, static_cast<float>(sNUMBER_OF_TILE_COLORS));
    if (colorInt == sNUMBER_OF_TILE_COLORS + 1){
        colorInt -=1;
    }
    return static_cast<GameState::Color>(colorInt);
}


GameState::Color GameState::GetColorAt (int row, int column) const
{
    if (row >= mRows || row < 0) {
        printf ("WARNING: non-existing row %d (out of %d rows) accessed using GetColorAt.\n", row, mRows);
        return NotAColor;
    }
    if (column >= mColumns || column < 0) {
        printf ("WARNING: non-existing column %d (out of %d columns) accessed using GetColorAt.\n", column, mColumns);
        return NotAColor;
    }
    return mGrid[row * mColumns + column];
}		/* -----  end of function GetColorAt  ----- */


GameState::Color GameState::GetColorAt (int index) const
{
    if (index >= mGrid.size() || index < 0) {
        printf ("WARNING: GameState::GetColorAt index %d (size() == %d) out of bounds.\n", index, static_cast<int>(mGrid.size()));
        return NotAColor;
    }
    return mGrid [index];
}


GameState::Color GameState::GetColorAtOrRandom (int row, int column) const
{
    if (row >= mRows || row < 0) {
        return GetRandomColor();

    }
    if (column >= mColumns || column < 0) {
        return GetRandomColor();
    }
    return mGrid[row * mColumns + column];
}		/* -----  end of function GetColorAt  ----- */


std::vector<bool> GameState::GetMatchesOfN (int n) const
{
    std::vector<bool> matches (mRows*mColumns, false);
    // check for horizontal n-in-a-rows
    for (int currentRow = 0; currentRow < mRows; currentRow++) {
        for (int currentColumn = 0; currentColumn < mColumns - n + 1; currentColumn++) {
            GameState::Color currentColor = GetColorAt (currentRow, currentColumn);
            int colorRepetitionCount = 1;
            while (GetColorAt (currentRow, currentColumn+colorRepetitionCount) == currentColor) {
                colorRepetitionCount++;
            }
            if (colorRepetitionCount >= n) {
                for (int i = 0; i<colorRepetitionCount; i++) {
                    matches[currentRow*mColumns + currentColumn + i] = true;
                }
            }
        }
    }
    // check for horizontal n-in-a-rows
    for (int currentColumn = 0; currentColumn < mColumns; currentColumn++) {
        for (int currentRow = 0; currentRow < mRows - n + 1; currentRow++) {
            GameState::Color currentColor = GetColorAt (currentRow, currentColumn);
            int colorRepetitionCount = 1;
            while (GetColorAt (currentRow+colorRepetitionCount, currentColumn) == currentColor) {
                colorRepetitionCount++;
            }
            if (colorRepetitionCount >= n) {
                for (int i = 0; i<colorRepetitionCount; i++) {
                    matches[currentRow*mColumns + currentColumn + i*mColumns] = true;
                }
            }
        }
    }
    return matches;
}


void GameState::ResetGridToRandom (int rows, int columns)
{
    if (rows == 0 || columns == 0) {
        mRows = 0;
        mColumns = 0;
        printf ("GameState::ResetGridToRandom: empty game grid created.\n");
        return;
    }

    mRows = rows;
    mColumns = columns;
    mGrid.resize (rows * columns);
    for (int i = 0; i < rows * columns; i++) {
        mGrid[i] =  GetRandomColor();
    }
    printf ("GameState::ResetGridToRandom: %dx%d game grid created.\n", rows, columns);
}		/* -----  end of function ResetGridToRandom  ----- */


void GameState::ResetGridToRandomNoNMatches (int rows, int columns, int n)
{
    if (n<=1) {
        printf ("ERROR: GameStage::ResetGridToRandomNoNMatches (int n) called with n < 2 (n==%d).\n", n);
        assert (n>1);
    }
    ResetGridToRandom (rows, columns);
    bool hasMatches = true;
    while (hasMatches) {
        hasMatches = false;
        std::vector<bool> matches = GetMatchesOfN  (n);
        for (int index = 0; index<mRows*mColumns; index++) {
            if (matches[index]) {
                hasMatches = true;
                mGrid[index] = GetRandomColor();
            }
        }
    }
}


bool GameState::SetDragStartLocation (glm::vec2 gridMouseDownLocation)
{
    if (Idle != mAnimationState) {
        printf ("ERROR in GameState::SetDragStartLocation: cannot start a drag motion, animation state not Idle.\n");
        return false;
    }
    if (mTileDragData.mIsActive) {
        printf ("ERROR in GameState::SetDragStartLocation: cannot start a drag motion, a drag motion already taking place.\n");
        return false;
    }
    if (mTileDragData.mIsActive) {
        printf ("ERROR in GameState::SetDragStartLocation: cannot start a drag motion, drag started outside of the grid.\n");
        return false;
    }
    mTileDragData.mStartLocation = gridMouseDownLocation;
    mTileDragData.mIsActive = true;
    SetDragCurrentLocation (gridMouseDownLocation);
    printf ("GameState::SetDragStartLocation (gridMouseDownLocation - x:%f y:%f).\n", gridMouseDownLocation.x, gridMouseDownLocation.y);
    return true;
}


bool GameState::SetDragCurrentLocation (glm::vec2 gridMouseCurrentLocation)
{
    if (Idle != mAnimationState) {
        printf ("ERROR in GameState::SetDragCurrentLocation: pointless call, animation state not Idle.\n");
        return false;
    }
    if (!mTileDragData.mIsActive) {
        printf ("ERROR in GameState::SetDragCurrentLocation: pointless call, no drag motion taking place.\n");
        return false;
    }
    mTileDragData.mCurrentLocation = gridMouseCurrentLocation;
    UpdateDragCache();
    return true;
}

void GameState::UpdateDragCache() {
    if (Idle != mAnimationState) {
        printf ("ERROR in GameState::SetDragCurrentLocation: pointless call, animation state not Idle.\n");
        return;
    }
    if (!mTileDragData.mIsActive) {
        printf ("ERROR in GameState::SetDragCurrentLocation: pointless call, no drag motion taking place.\n");
        return;
    }
    // if there is a dragged tile, identify it
    // identify the row and column
    mTileDragData.mDraggedTileColumn = GetColumnOfGridCoordinates (mTileDragData.mStartLocation);
    mTileDragData.mDraggedTileRow = GetRowOfGridCoordinates (mTileDragData.mStartLocation);

    glm::vec2 dragDiff = mTileDragData.mCurrentLocation - mTileDragData.mStartLocation;
    // adjust dragDiff if it is an edge case
    if (mTileDragData.mDraggedTileColumn == 0 && dragDiff.x < 0.0f) {
        dragDiff.x = 0.0f;
    }
    if (mTileDragData.mDraggedTileColumn == mColumns-1 && dragDiff.x > 0.0f) {
        dragDiff.x = 0.0f;
    }
    if (mTileDragData.mDraggedTileRow == 0 && dragDiff.y < 0.0f) {
        dragDiff.y = 0.0f;
    }
    if (mTileDragData.mDraggedTileRow == mRows-1 && dragDiff.y > 0.0f) {
        dragDiff.y = 0.0f;
    }
    mTileDragData.mCurrentTileDisplacement = glm::vec3 (0.0f, 0.0f, 0.0f);
    // find out whether the dragged tile is moved more horizontally or vertically
    if (fabs (dragDiff.x) > fabs (dragDiff.y)) {
        mTileDragData.mReplacedTileRow = mTileDragData.mDraggedTileRow;
        if (dragDiff.x > 0) {
            mTileDragData.mReplacedTileColumn = mTileDragData.mDraggedTileColumn + 1;
            mTileDragData.mCurrentTileDisplacement.x = fmin (dragDiff.x, 1.0f/static_cast<float>(mColumns));
        } else {
            mTileDragData.mReplacedTileColumn = mTileDragData.mDraggedTileColumn - 1;
            mTileDragData.mCurrentTileDisplacement.x = fmax (dragDiff.x, -1.0f/static_cast<float>(mColumns));
        }
    } else if (dragDiff.x == 0.0 && dragDiff.y == 0.0f){
            mTileDragData.mReplacedTileColumn = -1;
            mTileDragData.mReplacedTileRow = -1;
    } else {
        mTileDragData.mReplacedTileColumn = mTileDragData.mDraggedTileColumn;
        if (dragDiff.y > 0) {
            mTileDragData.mReplacedTileRow = mTileDragData.mDraggedTileRow + 1;
            mTileDragData.mCurrentTileDisplacement.y = fmin (dragDiff.y, 1.0f/static_cast<float>(mRows));
        } else {
            mTileDragData.mReplacedTileRow = mTileDragData.mDraggedTileRow - 1;
            mTileDragData.mCurrentTileDisplacement.y = fmax (dragDiff.y, -1.0f/static_cast<float>(mRows));
        }
    }
}


int GameState::GetReplacedTileRow() const {
    if (mTileDragData.mIsActive || mAnimationState == SwappingTiles || mTileDragData.mIsSwapBack) {
        return mTileDragData.mReplacedTileRow;
    } else {
        return -1;
    }
}


int GameState::GetReplacedTileColumn() const {
    if (mTileDragData.mIsActive || mAnimationState == SwappingTiles || mTileDragData.mIsSwapBack) {
        return mTileDragData.mReplacedTileColumn;
    } else {
        return -1;
    }
}


int GameState::GetDraggedTileRow() const {
    if (mTileDragData.mIsActive || mAnimationState == SwappingTiles || mTileDragData.mIsSwapBack) {
        return mTileDragData.mDraggedTileRow;
    } else {
        return -1;
    }
}


int GameState::GetDraggedTileColumn() const {
    if (mTileDragData.mIsActive || mAnimationState == SwappingTiles || mTileDragData.mIsSwapBack) {
        return mTileDragData.mDraggedTileColumn;
    } else {
        return -1;
    }
}


glm::vec3 GameState::GetCurrentDraggedTileDisplacement() const {
    if (mTileDragData.mIsActive || mAnimationState == SwappingTiles) {
        return mTileDragData.mCurrentTileDisplacement;
    } else {
        return glm::vec3 (0.0f, 0.0f, 0.0f);
    }
}


glm::vec2 GameState::GetDragStartLocation () const
{
    if (!mTileDragData.mIsActive) {
        printf ("WARNING: GameState::GetDragStartLocation: asked for drag start location while no tile drag is active.\n");
    }
    return mTileDragData.mStartLocation;
}


glm::vec2 GameState::GetDragCurrentLocation () const
{
    if (!mTileDragData.mIsActive) {
        printf ("WARNING: GameState::GetDragCurrentLocation: asked for drag current location while no tile drag is active.\n");
    }
    return mTileDragData.mCurrentLocation;
}


void GameState::SwapTiles (int tileARow, int tileAColumn, int tileBRow, int tileBColumn, Uint32 animationDuration, bool swapBack)
{
    if (Idle != mAnimationState) {
        printf ("WARNING: GameState::SwapTiles called while mAnimationState is not Idle. Swap not run.\n");
        return;
    }
    mTileDragData  = TileDragData();
    mTileDragData.mDraggedTileRow = tileARow;
    mTileDragData.mDraggedTileColumn = tileAColumn;
    mTileDragData.mReplacedTileRow = tileBRow;
    mTileDragData.mReplacedTileColumn = tileBColumn;
    mTileDragData.mIsSwapBack = swapBack;
    mAnimationState = SwappingTiles;
    mAnimationDuration = animationDuration;
    mTimeAnimationStart = mGameTime;
}


bool GameState::SwapDraggedAndReplacedTiles (Uint32 animationDuration, bool animateFromCurrentPositionOn, bool swapBack)
{
    if (!mTileDragData.mIsActive) {
        printf ("WARNING: GameState::SwapDraggedAndReplacedTiles called with no tile drag active.\n");
        return false;
    }
    if (Idle != mAnimationState) {
        printf ("WARNING: GameState::SwapDraggedAndReplacedTiles called while mAnimationState is not Idle.\n");
        return false;
    }
    mTileDragData.mIsSwapBack = swapBack;
    mAnimationState = SwappingTiles;
    mAnimationDuration = animationDuration;
    mTileDragData.mIsActive = false;

    if (false == animateFromCurrentPositionOn) {
        mTimeAnimationStart = mGameTime;
        mTileDragData.mAnimationStartingTileDisplacement = mTileDragData.mCurrentTileDisplacement;
    } else {
        // find out what percentage of animation is already done
        float displacementMaxX = 1.0f/static_cast<float>(mColumns);
        float displacementMaxY = 1.0f/static_cast<float>(mRows);
        float animationDone = fmax (fabs (mTileDragData.mCurrentTileDisplacement.x)/displacementMaxX, fabs (mTileDragData.mCurrentTileDisplacement.y)/displacementMaxY);
        // move starting time so far back that current displacement would be reached with current time and animation duration
        Uint32 elapsedAnimationTime = static_cast<Uint32>(static_cast<float>(animationDuration) * animationDone);
        mTimeAnimationStart = mGameTime - elapsedAnimationTime;
        mTileDragData.mAnimationStartingTileDisplacement = glm::vec3();
    }
    return true;
}


void GameState::DeactivateDrag() {
    mTileDragData.mIsActive = false;
}

void GameState::Elapse (Uint32 deltaTime)
{
    mGameTime+=deltaTime;
    mGameplayTime+=deltaTime;
    if (Idle != mAnimationState) {
        // correct mGameplayTIme
        int animationLeftover = mTimeAnimationStart + mAnimationDuration - mGameTime;
        mGameplayTime -= deltaTime;
        if (animationLeftover < 0) {
            mGameplayTime += abs (animationLeftover);
        }

        DeselectTile();
        float elapsedAnimation = mGameTime - mTimeAnimationStart;
        float animationElapsedPercentage = fmin (1.0f, static_cast<float>(elapsedAnimation) / static_cast<float>(mAnimationDuration));
        if (GameState::SwappingTiles == mAnimationState) {
            glm::vec3 finalTileDisplacement = glm::vec3 (0.0f, 0.0f, 0.0f);
            finalTileDisplacement.x = (mTileDragData.mReplacedTileColumn - mTileDragData.mDraggedTileColumn) * (1.0f / static_cast<float>(mColumns));
            finalTileDisplacement.y = (mTileDragData.mReplacedTileRow - mTileDragData.mDraggedTileRow) * (1.0f / static_cast<float>(mRows));
            mTileDragData.mCurrentTileDisplacement = mTileDragData.mAnimationStartingTileDisplacement + animationElapsedPercentage *
                (finalTileDisplacement - mTileDragData.mAnimationStartingTileDisplacement);

            if (animationElapsedPercentage >= 1.0f) {
                Color draggedTileColor = mGrid[mTileDragData.mDraggedTileRow*mColumns+mTileDragData.mDraggedTileColumn];
                Color replacedTileColor = mGrid[mTileDragData.mReplacedTileRow*mColumns+mTileDragData.mReplacedTileColumn];
                mGrid [mTileDragData.mDraggedTileRow*mColumns+mTileDragData.mDraggedTileColumn] = replacedTileColor ;
                mGrid [mTileDragData.mReplacedTileRow*mColumns+mTileDragData.mReplacedTileColumn] = draggedTileColor;
                printf ("GameState::Elapse: SwappingTiles animation finished.\n");
                mAnimationState = Idle;
                NotifyGameStateGridChangeObservers ();
            }
        } else if (DestroyingTiles == mAnimationState) {
            if (animationElapsedPercentage >= 1.0f) {
                for (int counter = 0; counter < mGrid.size(); counter++) {
                    if (mTilesBeingDestroyed [counter]) {
                        mGrid [counter] = DestroyedColor;
                    }
                }
                printf ("GameState::Elapse: DestroyingTiles animation finished.\n");
                mAnimationState = Idle;
                NotifyGameStateGridChangeObservers ();
            }
        } else if (CollapsingTiles == mAnimationState) {
            if (animationElapsedPercentage >= 1.0f) {
                printf ("GameState::Elapse: CollapsingTiles animation finished.\n");
                mAnimationState = Idle;
                NotifyGameStateGridChangeObservers ();
            }
        }
    }
    if (GetGameplayTimeLeft() == 0) {
        mAnimationState = GameOver;
    }
}


void GameState::AttachGameStateGridChangeObserver (IGameStateGridChangeObserver* gameStateGridChangeObserver)
{
    mGameStateGridChangeObservers.push_back (gameStateGridChangeObserver);
}


void GameState::NotifyGameStateGridChangeObservers ()
{
    for (std::vector<IGameStateGridChangeObserver*>::iterator iterator = mGameStateGridChangeObservers.begin();
            iterator != mGameStateGridChangeObservers.end(); iterator++) {
        (*iterator)->NotifyOfGameStateGridChange();
    }
}


bool GameState::DestroyTiles (std::vector<bool> tilesToDestroy, Uint32 animationDuration)
{
    if (tilesToDestroy.size() != mGrid.size()) {
        printf ("ERROR: GameState::DestroyTiles called with grid size different from that of GameState.\n");
        return false;
    }
    if (Idle != mAnimationState) {
        printf ("ERROR: GameState::DestroyTiles called while mAnimation state is not Idle.\n");
        return false;
    }
    if (mTileDragData.mIsActive) {
        printf ("WARNING: GameState::DestroyTiles called while tile drag is active. Tile drag is reset.\n");
        mTileDragData = TileDragData();
    }
    mAnimationState = DestroyingTiles;
    mTimeAnimationStart = mGameTime;
    mAnimationDuration = animationDuration;
    mTilesBeingDestroyed = tilesToDestroy;
    return true;
}


float GameState::GetAnimationPercentage() const
{
    return static_cast<float>(mGameTime - mTimeAnimationStart) / mAnimationDuration;
}


bool GameState::IsTileBeingDestroyed (int row, int column) const
{
    if (DestroyingTiles != mAnimationState) {
        return false;
    }
    return mTilesBeingDestroyed [row * mColumns + column];
}


bool GameState::CollapseColumns (std::vector<std::vector<int>> columnsToCollapse, Uint32 animationDuration)
{
    if (columnsToCollapse.size() != mColumns) {
        printf ("ERROR: GameState::CollapseColumns called with number of columns mismatch.\n");
        return false;
    }
    if (Idle != mAnimationState) {
        printf ("ERROR: GameState::CollapseColumns called while mAnimation state is not Idle.\n");
        return false;
    }
    if (mTileDragData.mIsActive) {
        printf ("WARNING: GameState::CollapseColumns called while tile drag is active. Tile drag is reset.\n");
        mTileDragData = TileDragData();
    }
    mAnimationState = CollapsingTiles;
    for (int currentColumn = 0; currentColumn < mColumns; currentColumn++) {
        for (int currentRow = columnsToCollapse[currentColumn][0]; currentRow > -1; currentRow--) {
            mGrid [currentRow * mColumns + currentColumn] = GetColorAtOrRandom (currentRow - columnsToCollapse[currentColumn][1], currentColumn);
        }
    }
    mTimeAnimationStart = mGameTime;
    mAnimationDuration = animationDuration;
    mColumnsToCollapse = columnsToCollapse;
    return true;
}


std::vector<std::vector<int>> GameState::GetColumnsToCollapse() const
{
    if (CollapsingTiles != mAnimationState) {
        printf ("ERROR: GameState::GetColumnsToCollapse called while mAnimation state is not CollapsingTiles.\n");
    }
    return mColumnsToCollapse;
}


bool GameState::GetIsSwapBack() const
{
    return mTileDragData.mIsSwapBack;
}


void GameState::ResetIsSwapBack()
{
    mTileDragData.mIsSwapBack = false;
}



int GameState::GetRowOfGridCoordinates (glm::vec2 gridCoordinates) const
{
    int row = gridCoordinates.y * mRows;
    if (row == mRows) {
        row--;
    }
    return row;
}


int GameState::GetColumnOfGridCoordinates (glm::vec2 gridCoordinates) const
{
    int column = gridCoordinates.x * mColumns;
    if (column == mColumns) {
        column--;
    }
    return column;
}


void GameState::SelectTile (int row, int column)
{
    mTileDragData.mSelectedTileRow = row;
    mTileDragData.mSelectedTileColumn = column;
}


int GameState::GetSelectedTileColumn() const
{
    return mTileDragData.mSelectedTileColumn;
}


int GameState::GetSelectedTileRow() const
{
    return mTileDragData.mSelectedTileRow;
}


void GameState::DeselectTile()
{
    mTileDragData.mSelectedTileRow = -1;
    mTileDragData.mSelectedTileColumn = -1;
}


int GameState::GetGameplayTimeLeft() const
{
    if (mMaxGameplayTimeSeconds * 1000 < mGameplayTime) {
        return 0;
    }
    int secondsLeft = ceil (static_cast<float>(mMaxGameplayTimeSeconds * 1000 - mGameplayTime)/1000.0f);
    return secondsLeft;
}


void GameState::AddToScore (int points)
{
    mGameScore+=points;
}
