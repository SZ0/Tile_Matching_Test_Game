#pragma once

#include "GameStateRenderer.h"
#include <GL/glew.h>
#include <GL/glu.h>
#include <assert.h>
#include <stdio.h>
#include <GameState.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#ifdef TARGET_MSVC
    #include <SDL.h>
    #include <SDL_opengl.h>
	#include <SDL_ttf.h>
#endif
#ifdef TARGET_UNIX
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengl.h>
    #include <SDL2/SDL_ttf.h>
#endif


/*!
 * Receives as input the surface to render to and the state of the game to render.
 */
class GameStateRenderer
{
    public:
        /* ====================  ACCESSORS     ======================================= */

        /*!
         * Renders the game state.
         * Calls the appropriate draw functions to represent the input data based on its animation state and grid, drag, time, score variables.
         * @param gameState the gameState to render.
         */
        static bool Render (const GameState& gameState);


        /*!
         * Accepts location in screen space and converts it into game board/grid location.
         * GameState and GameStateLogic should not need to know about the view and therefore the screen resolution.
         * This function is used to convert screen space coordinates (eg. mouse input) into the location on the grid.
         * @param x X-axis location on the screen.
         * @param y Y-axis location on the screen.
         * @return A float 2D vector with elements in range of [0,1] represeting grid location.
         */
        static glm::vec2 GetGridCoordinatesFromScreenLocation (int x, int y);


        /* ====================  MUTATORS      ======================================= */

        /*!
         * Initializes renderer.
         * Required before calling any other functions.
         * It initializes OpenGL context, reads the relevant asset files,
         * links the shader program and uploads buffer objects (vertices, elements, textures), prepares font.
         */
        static bool InitRenderer ();

		static void DestroyRenderer() {
			glDeleteProgram (sHudTexShaderProgram);
            glDeleteTextures (1, &sTextureObjectName_BackGroundJpg);
            glDeleteTextures (1, &sTextureObjectName_backgroundTransparent);
            glDeleteTextures (GameState::sNUMBER_OF_TILE_COLORS, sTextureObjectNames_tileImages);
			// will destroy both the window and its surface
			SDL_DestroyWindow (sSdlWindow);
			// point the pointer to NULL after its previous location became invalid
			sSdlWindow = NULL;
            TTF_CloseFont (sTextFont);
            sTextFont = NULL;
		};

        /* ====================  OPERATORS     ======================================= */

    protected:
        /* ====================  DATA MEMBERS  ======================================= */

    private:
        /* ====================  LIFECYCLE     ======================================= */
        // static class
        GameStateRenderer ();                             /* constructor */

        /* ====================  DATA MEMBERS  ======================================= */
        // window
		static SDL_Window *  sSdlWindow;                   ///< the application's window
		static SDL_GLContext sSdlGlContext;                ///< opengl context
		static const int     SCREEN_WIDTH;                 ///< the width of the application's window
		static const int     SCREEN_HEIGHT;                ///< the height of the application's window
		static const float   VERTICAL_LEFT_GRID_LINE;      ///< the vertical line on the background image where the grid starts
		static const float   VERTICAL_RIGHT_GRID_LINE;     ///< the vertical line on the background image where the grid ends
		static const float   HORIZONTAL_TOP_GRID_LINE;     ///< the horizontal line on the background image where the grid starts
		static const float   HORIZONTAL_BOTTOM_GRID_LINE;  ///< the horizontal line on the background image where the grid ends

        // shader program and attribute data
        static GLuint        sHudTexShaderProgram;          ///< handle for shader program that draws a textured square in HUD
        static const GLchar* sHudTexVertexShaderSource[];   ///< in-code source for vertex shader
        static const GLchar* sHudTexFragmentShaderSource[]; ///< in-code source for fragment shader
        static GLint         sHudTexShaderProgram_position2DLocation;    ///< index of attribute inside shader program
        static GLuint        sHudSquare_VBO;                ///< name of vertex buffer object for drawing a square
        static const GLfloat sHudSquare_vertices[];         ///< vertex position attribute data shaping a square in 2D
        static GLuint        sHudSquare_IBO;                ///< name of index buffer object for drawing a square
        static const GLuint  sHudSquare_indices[];          ///< indices for 2D square

        // textures
        static GLint         sHudTexShaderProgram_texPosition2DLocation;///< index of attribute inside shader program
        static GLint         sHudTexShaderProgram_samplerLocation;      ///< index of GLSL sampler uniform in the shader program
        static GLuint        sHudSquare_textureCoordinatesBo;           ///< buffer object name for texture coordinates
        static const GLfloat sHudSquare_textureCoordinates[];           ///< texture coordinates data

        static const char*   sFilePath_BackGroundJpg;                   ///< file path to the background image
        static const char*   sFilePath_backgroundTransparent;           ///< file path to the background image with a transparent cave
        static const char*   sFilePath_SelectionPng;                    ///< file path to the image for the selection box
        static GLuint        sTextureObjectName_BackGroundJpg;
        static GLuint        sTextureObjectName_backgroundTransparent;
        static GLuint        sTextureObjectName_Selection;
        static const char*   sFilePath_tileImages[];                    ///< GameState::sNUMBER_OF_TILE_COLORS file paths for the tile images
        static GLuint        sTextureObjectNames_tileImages[];          ///< an object name for each tile color

        // 2D position variables
        static GLint         sHudTexShaderProgram_mvMatrixUniformLocation; ///< model view matrix uniform location in textured head-up display square shader program
        static glm::mat4     sHudMvMatrix_background;           ///< transform from the square of sHudSquare_vertices to the entire screen
        static glm::mat4     sHudMvMatrix_squareToGridPosition; ///< transform from the square of sHudSquare_vertices to the rectangle of the cave on the 'BackGround' image, AKA the grid location.

        // tile destruction animation
        static GLint         sHudTexShaderProgram_destroyUniLoc; ///< float uniform location showing destruction percentage for removing tiles animation

        // text rendering
        static TTF_Font*     sTextFont;    ///< TrueType font object for generating a texture with text
        static SDL_Surface*  sTextSurface; ///< surface temporarely holding rendered text to be uploaded as a texture object
        static SDL_Color     sSdlColorWhite;
        static GLuint        sTextureObjectName_text;
        static glm::mat4     sHudMvMatrix_squareToTextPosition; ///< model view matrix to transfer text to the middle left unoccupied spot on the screen


        /*!
         *  Accepts as input a pointer address to an uninitalized GLuint and
         *  the shader's source code. It modifies the GLuint's value so it indicates one created,
         *  compiles it and returns whether the operation was successful.
         *  @param shader variable whose value will be changed to the newly created shader object's name.
         *  @param shaderType supported shader types are GL_VERTEX_SHADER and GL_FRAGMENT_SHADER
         *  @param codeString the code to compile into a shader object
         *  @return true if compilation was successful
         */
        static GLint CompileShader (GLuint* shader, GLenum shaderType, const GLchar** codeString);


        /*!
         * Draws the background.
         * @param transparent if true the background texture with transparent cave will be used
         */
        static void DrawBackground (bool transparent);


        /*!
         *  Draws a square based on the given texture object name and
         *  transformation matrix.
         *  @param transformMatrix mat4 uniform for positioning the square.
         *  @param textureObject the texture to render.
         */
        static void DrawHudSquare (const glm::mat4& transformMatrix, const GLuint& textureObject);


        /*!
         *  Draws a square based on the given texture object name and
         *  transformation matrix. Also renders tile being destroyed animation.
         *  @param transformMatrix mat4 uniform for positioning the square.
         *  @param textureObject the texture to render.
         *  @param destructionPercentage The amount by which the tile is destroyed from 0 (not destroyed) to 1 (completely destroyed).
         */
        static void DrawHudSquareDestroyed (const glm::mat4& transformMatrix, const GLuint& textureObject, float destructionPercentage);


        /*!
         *  Description:  Draws the current grid of the game state.
         *  @param gameState the GameState to render.
         */
        static void DrawGameState (const GameState& gameState);


        /*!
         *  Description:  Draws the current grid of the game state with GameState::CollapsingTiles animation
         *  @param gameState the GameState to render.
         */
        static void DrawGameStateCollaping (const GameState& gameState);


        /*!
         * Renders the text to the screen.
         */
        static void RenderText (glm::mat4 mvMatrix, const char* text, int textLength);


}; /* -----  end of class GameStateRenderer  ----- */

