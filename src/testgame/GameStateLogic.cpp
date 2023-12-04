#include "GameStateLogic.h"
#include <vector>
#include <stdio.h>
#include <GameStateRenderer.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


const Uint32 GameStateLogic::ANIMATION_DURATION_MILIS = 500;
const Uint32 GameStateLogic::MIN_MATCH_SIZE = 3;

/*!
 * Reacts to an input event.
 */
void GameStateLogic::Input (SDL_Event& e, GameState& gameState) const
{
    if (gameState.GetAnimationState() == GameState::GameOver) {
        return;
    }
    if (mIsToCheckGameGrid) {
        // ignore input while game animations are running
        return;
    }
    if (e.type == SDL_MOUSEMOTION) {
        // only process mouse click if no animation is already running
        if (GameState::Idle == gameState.GetAnimationState()) {
            // ask gameState whether it has an active drag
            if (gameState.IsDragActive()) {
                // ask renderer for mouse up location in grid space based on screen space location
                glm::vec2 gridCoordinates = GameStateRenderer::GetGridCoordinatesFromScreenLocation(e.motion.x, e.motion.y);
                // save the current grid coordinates to gameState.
                gameState.SetDragCurrentLocation (gridCoordinates);
            }
        }
    } else if (e.type == SDL_MOUSEBUTTONDOWN) {
        // only process mouse click if no animation is already running
        if (GameState::Idle == gameState.GetAnimationState()) {
            // ask renderer for mouse up location in grid space based on screen space location
            glm::vec2 gridCoordinates = GameStateRenderer::GetGridCoordinatesFromScreenLocation(e.motion.x, e.motion.y);
            // store grid mousedown location on gameState and set drag motion to active
            gameState.SetDragStartLocation (gridCoordinates);
        }
    } else if (e.type == SDL_MOUSEBUTTONUP) {
        // only process mouse click if no animation is already running
        if (GameState::Idle == gameState.GetAnimationState()) {
            if (gameState.IsDragActive()) {
                glm::vec2 gridCoordinates = GameStateRenderer::GetGridCoordinatesFromScreenLocation(e.motion.x, e.motion.y);
                gameState.SetDragCurrentLocation (gridCoordinates);
                // align with game state whether the drag was such that two tiles should be switched
                glm::vec3 currentTileDisplacement = gameState.GetCurrentDraggedTileDisplacement();
                if (fabs(currentTileDisplacement.x) >= 1.0f/static_cast<float>(gameState.GetColumns())/2 ||
                        fabs(currentTileDisplacement.y) >= 1.0f/static_cast<float>(gameState.GetRows())/2) {
                    bool isSuccessful = gameState.SwapDraggedAndReplacedTiles(ANIMATION_DURATION_MILIS, true, true);

                } else {
                    // check whether it is a click
                    // it is a click if up is on the same tile as down
                    int tileMouseDownRow = gameState.GetDraggedTileRow();
                    int tileMouseDownColumn = gameState.GetDraggedTileColumn();
                    int tileMouseUpRow = gameState.GetRowOfGridCoordinates(gridCoordinates);
                    int tileMouseUpColumn = gameState.GetColumnOfGridCoordinates(gridCoordinates);
                    if (tileMouseUpRow == tileMouseDownRow && tileMouseUpColumn == tileMouseDownColumn) {
                        // check if selected to be tile is next to selected tile
                        int currentlySelectedTileRow = gameState.GetSelectedTileRow();
                        int currentlySelectedTileColumn = gameState.GetSelectedTileColumn();
                        if ((currentlySelectedTileRow == tileMouseUpRow && abs(currentlySelectedTileColumn - tileMouseUpColumn) < 2) ||
 (currentlySelectedTileColumn == tileMouseUpColumn && abs(currentlySelectedTileRow - tileMouseUpRow) < 2)) {
                            gameState.SwapTiles(tileMouseUpRow, tileMouseUpColumn, currentlySelectedTileRow, currentlySelectedTileColumn, ANIMATION_DURATION_MILIS, true);
                        } else {
                            // otherwise set this as selected
                            gameState.SelectTile(tileMouseUpRow, tileMouseUpColumn);
                        }
                    }
                }
                // deactivate drag because of mouse up event
                gameState.DeactivateDrag();
            }
        }
    }
}		/* -----  end of function Input  ----- */


bool GameStateLogic::Update (Uint32 deltaTime, GameState& gameState)
{
    bool isSuccessful = true;

    gameState.Elapse (deltaTime);
    if (gameState.GetAnimationState() == GameState::GameOver) {
        return true;
    }
    while (mIsToCheckGameGrid) {
        mIsToCheckGameGrid = false;
        std::vector<bool> tilesToDestroy = gameState.GetMatchesOfN (MIN_MATCH_SIZE);
        bool isToDestroy = false;
        bool isToCollapse = false;
        int counter = 0;
        // analyse how GameStateLogic should react to the changed GameState's grid
        while (counter < tilesToDestroy.size()) {
            isToDestroy = isToDestroy || tilesToDestroy[counter];
            if (gameState.GetColorAt(counter) == GameState::DestroyedColor) {
                isToCollapse = true;
            };
            counter++;
        }
        if (isToCollapse) {
            int scoreToAdd = 0;
            std::vector<std::vector<int>> columnsToCollapse;
            columnsToCollapse.resize (gameState.GetColumns());
            for (int currentColumn = 0; currentColumn < gameState.GetColumns(); currentColumn++) {
                columnsToCollapse [currentColumn] = std::vector<int>{-1,1};

                for (int currentRow = gameState.GetRows()-1; currentRow > -1; currentRow--) {
                    if (gameState.GetColorAt(currentRow, currentColumn) == GameState::DestroyedColor) {
                        scoreToAdd++;
                        if (columnsToCollapse[currentColumn][0] == -1) {
                            columnsToCollapse[currentColumn][0] = currentRow;
                        } else {
                            columnsToCollapse[currentColumn][1]++;
                        }
                    } else {
                        if (columnsToCollapse[currentColumn][0] != -1) {
                            currentRow = -1;
                        }
                    }
                }

            }
            isSuccessful = isSuccessful && gameState.CollapseColumns (columnsToCollapse, ANIMATION_DURATION_MILIS);
            gameState.AddToScore(scoreToAdd);
        } else if (isToDestroy) {
            gameState.ResetIsSwapBack();
            isSuccessful = isSuccessful && gameState.DestroyTiles (tilesToDestroy , ANIMATION_DURATION_MILIS);
        } else if (gameState.GetIsSwapBack()) {
            gameState.SwapTiles (gameState.GetDraggedTileRow(), gameState.GetDraggedTileColumn(), gameState.GetReplacedTileRow(), gameState.GetReplacedTileColumn(), ANIMATION_DURATION_MILIS, false);
            gameState.ResetIsSwapBack();
        }
    }

    return isSuccessful;
}


void GameStateLogic::NotifyOfGameStateGridChange()
{
    mIsToCheckGameGrid = true;
}
