#include "GameStateRenderer.h"
#include <GL/glew.h>
#include <GL/glu.h>
#include <assert.h>
#include <stdio.h>
#include <GameState.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#ifdef TARGET_MSVC
    #include <SDL.h>
    #include <SDL_opengl.h>
	#include <SDL_image.h>
	#include <SDL_ttf.h>
#endif
#ifdef TARGET_UNIX
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengl.h>
    #include <SDL2/SDL_image.h>
    #include <SDL2/SDL_ttf.h>
#endif


// initializing static members
const int GameStateRenderer::SCREEN_WIDTH = 755;
const int GameStateRenderer::SCREEN_HEIGHT = 600;
const float   GameStateRenderer::VERTICAL_LEFT_GRID_LINE = 0.42;
const float   GameStateRenderer::VERTICAL_RIGHT_GRID_LINE = 0.9;
const float   GameStateRenderer::HORIZONTAL_TOP_GRID_LINE = 0.15;
const float   GameStateRenderer::HORIZONTAL_BOTTOM_GRID_LINE = 0.755;

SDL_Window * GameStateRenderer::sSdlWindow = NULL;
SDL_GLContext GameStateRenderer::sSdlGlContext = NULL;

GLuint GameStateRenderer::sHudTexShaderProgram = 0;

const GLchar* GameStateRenderer::sHudTexVertexShaderSource[] = {
    "#version 140\n\nin vec2 position2D;\nin vec2 vertTexCoordinates;\nout vec2 fragTexCoordinates;\nuniform mat4 mvMatrix;\nvoid main() { gl_Position = mvMatrix * vec4 (position2D.x, position2D.y, 0, 1);\nfragTexCoordinates = vertTexCoordinates;}"
};

const GLchar* GameStateRenderer::sHudTexFragmentShaderSource[] = {
    "#version 140\nout vec4 LFragment;\nin vec2 fragTexCoordinates;\nuniform sampler2D texSampler;\nuniform float destructionPercentage;\nvoid main() { LFragment = texture2D (texSampler, fragTexCoordinates) * (1 - destructionPercentage); }"
};

GLuint GameStateRenderer::sHudSquare_VBO = 0;
GLuint GameStateRenderer::sHudSquare_textureCoordinatesBo = 0;
GLuint GameStateRenderer::sHudSquare_IBO = 0;

GLint GameStateRenderer::sHudTexShaderProgram_position2DLocation  = -1;
GLint GameStateRenderer::sHudTexShaderProgram_texPosition2DLocation  = -1;
GLint GameStateRenderer::sHudTexShaderProgram_samplerLocation  = -1;
GLint GameStateRenderer::sHudTexShaderProgram_mvMatrixUniformLocation  = -1;
GLint GameStateRenderer::sHudTexShaderProgram_destroyUniLoc  = -1;

const GLfloat GameStateRenderer::sHudSquare_vertices[] = {
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
};

const GLfloat GameStateRenderer::sHudSquare_textureCoordinates[] = {
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
};

const GLuint GameStateRenderer::sHudSquare_indices[] = {
	0,1,3,1,2,3
};

// textures
const char* GameStateRenderer::sFilePath_BackGroundJpg = "./assets/BackGround.jpg";
const char* GameStateRenderer::sFilePath_backgroundTransparent = "./assets/backgroundTransparentDoors.png";
const char* GameStateRenderer::sFilePath_SelectionPng = "./assets/Selection.png";
GLuint      GameStateRenderer::sTextureObjectName_BackGroundJpg = 0;
GLuint      GameStateRenderer::sTextureObjectName_backgroundTransparent = 0;
GLuint      GameStateRenderer::sTextureObjectName_Selection = 0;
const char* GameStateRenderer::sFilePath_tileImages[] = {
    "./assets/Red.png", "./assets/Green.png", "./assets/Blue.png", "./assets/Purple.png", "./assets/Yellow.png"
};
GLuint GameStateRenderer::sTextureObjectNames_tileImages[] = { 0, 0, 0, 0, 0 };

// matrices
glm::mat4 GameStateRenderer::sHudMvMatrix_background = glm::mat4 (1);
glm::mat4 GameStateRenderer::sHudMvMatrix_squareToGridPosition = glm::mat4 (1);


// text rendering
TTF_Font* GameStateRenderer::sTextFont = 0;
SDL_Surface* GameStateRenderer::sTextSurface = 0;
SDL_Color GameStateRenderer::sSdlColorWhite = {255,255,255};
GLuint GameStateRenderer::sTextureObjectName_text = 0;
glm::mat4 GameStateRenderer::sHudMvMatrix_squareToTextPosition = glm::mat4 (1);

// defining functions
bool GameStateRenderer::InitRenderer()
{
    bool isSuccessful = true;
    printf ("GameStateRenderer::InitRenderer starting...\n");


    // init context
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	sSdlWindow = SDL_CreateWindow ("TestGame", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (sSdlWindow == NULL) {
        isSuccessful = false;
		printf ("SDL_CreateWindow failed. SDL_ERROR: %s\n", SDL_GetError());
	}
	assert (sSdlWindow != NULL);

	sSdlGlContext = SDL_GL_CreateContext (GameStateRenderer::sSdlWindow);
    if (sSdlGlContext == NULL) {
        isSuccessful = false;
        printf ("SDL_GL_CreateContext failed. SDL_ERROR: %s\n", SDL_GetError());
    }
    assert (sSdlGlContext != NULL);

    // init glew
    glewExperimental = GL_TRUE;
    GLenum glewInitReturnValue = glewInit();
    if (glewInitReturnValue != GLEW_OK)
    {
        isSuccessful = false;
        printf ("glewInit failed. SDL_ERROR: %s\n", glewGetErrorString (glewInitReturnValue));
    }
    assert (glewInitReturnValue == GLEW_OK);

    // init vsync
    int swapIntervalReturnValue = SDL_GL_SetSwapInterval (1);

    if (swapIntervalReturnValue  < 0)
    {
        isSuccessful = false;
        printf ("swapIntervalReturnValue failed. SDL_ERROR: %s\n", SDL_GetError());
    }


    // init (head-up display) shader program
    sHudTexShaderProgram = glCreateProgram();

    GLuint vertexShader;
    GLuint fragmentShader;

    isSuccessful = isSuccessful && CompileShader (&vertexShader, GL_VERTEX_SHADER, sHudTexVertexShaderSource);
    if (isSuccessful) {
        glAttachShader (sHudTexShaderProgram, vertexShader);

        isSuccessful = isSuccessful && CompileShader (&fragmentShader, GL_FRAGMENT_SHADER, sHudTexFragmentShaderSource);

        if (isSuccessful) {
            glAttachShader (sHudTexShaderProgram, fragmentShader);

            glLinkProgram (sHudTexShaderProgram);

            GLint shaderProgCompilationReturnValue = GL_TRUE;

            glGetProgramiv (sHudTexShaderProgram, GL_LINK_STATUS, &shaderProgCompilationReturnValue);
            if (shaderProgCompilationReturnValue != GL_TRUE)
            {
                isSuccessful = false;
                printf ("Error linking program %d!\n", sHudTexShaderProgram);
            } else {
                sHudTexShaderProgram_position2DLocation      = glGetAttribLocation (sHudTexShaderProgram, "position2D");
                sHudTexShaderProgram_texPosition2DLocation   = glGetAttribLocation (sHudTexShaderProgram, "vertTexCoordinates");
                sHudTexShaderProgram_mvMatrixUniformLocation = glGetUniformLocation (sHudTexShaderProgram, "mvMatrix");
                sHudTexShaderProgram_samplerLocation         = glGetUniformLocation (sHudTexShaderProgram, "texSampler");
                sHudTexShaderProgram_destroyUniLoc           = glGetUniformLocation (sHudTexShaderProgram, "destructionPercentage");
                if (-1 == sHudTexShaderProgram_position2DLocation || -1 == sHudTexShaderProgram_texPosition2DLocation
                        || -1 == sHudTexShaderProgram_mvMatrixUniformLocation || -1 == sHudTexShaderProgram_samplerLocation) {
                    printf ("Error obtaining location (s) for sHudTexShaderProgram.");
                    isSuccessful = false;
                }
            }
        }
    }


    // init vertex buffer
    // generate one buffer object name (it is not yet bound to any buffer)
    glGenBuffers (1, &GameStateRenderer::sHudSquare_VBO);
    // bind the generated object name to a target so that this will be the buffer affected by functions refering the target
    glBindBuffer (GL_ARRAY_BUFFER, sHudSquare_VBO);
    // load vertex data
    glBufferData (GL_ARRAY_BUFFER, 8 * sizeof (GLfloat), sHudSquare_vertices, GL_STATIC_DRAW);

    // repeat for texture coordinates
    glGenBuffers (1, &sHudSquare_textureCoordinatesBo);
    glBindBuffer (GL_ARRAY_BUFFER, sHudSquare_textureCoordinatesBo);
    glBufferData (GL_ARRAY_BUFFER, 8 * sizeof (GLfloat), sHudSquare_textureCoordinates, GL_STATIC_DRAW);

    // init index buffer object
    glGenBuffers (1, &sHudSquare_IBO);
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, sHudSquare_IBO);
    glBufferData (GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof (GLuint), sHudSquare_indices, GL_STATIC_DRAW);


    // load textures
    SDL_Surface* backgroundSdlSurface = IMG_Load (sFilePath_BackGroundJpg);
	if (backgroundSdlSurface  == NULL) {
        isSuccessful = false;
		printf ("IMG_Load failed for path: \"%s\". SDL_ERROR: %s\n", sFilePath_BackGroundJpg, SDL_GetError());
	}
    SDL_Surface* backgroundTransparentSdlSurface = IMG_Load (sFilePath_backgroundTransparent);
	if (backgroundTransparentSdlSurface  == NULL) {
        isSuccessful = false;
		printf ("IMG_Load failed for path: \"%s\". SDL_ERROR: %s\n", sFilePath_backgroundTransparent, SDL_GetError());
	}
    SDL_Surface* selectionSurface = IMG_Load (sFilePath_SelectionPng);
	if (selectionSurface   == NULL) {
        isSuccessful = false;
		printf ("IMG_Load failed for path: \"%s\". SDL_ERROR: %s\n", sFilePath_SelectionPng, SDL_GetError());
	}
	assert (backgroundSdlSurface != NULL);
	assert (backgroundTransparentSdlSurface != NULL);
	assert (selectionSurface != NULL);
    glGenTextures (1, &sTextureObjectName_BackGroundJpg);
    glBindTexture (GL_TEXTURE_2D, sTextureObjectName_BackGroundJpg);
    // in 2D with fixed size of the image, one would probably be better off scaling the jpg, png files than use mipmaps, so we're using linear filtering
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, backgroundSdlSurface->w, backgroundSdlSurface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, backgroundSdlSurface->pixels);
    SDL_FreeSurface (backgroundSdlSurface);

    glGenTextures (1, &sTextureObjectName_backgroundTransparent);
    glBindTexture (GL_TEXTURE_2D, sTextureObjectName_backgroundTransparent);
    // in 2D with fixed size of the image, one would probably be better off scaling the jpg, png files than use mipmaps, so we're using linear filtering
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, backgroundTransparentSdlSurface->w, backgroundTransparentSdlSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, backgroundTransparentSdlSurface->pixels);
    SDL_FreeSurface (backgroundTransparentSdlSurface);

    glGenTextures (1, &sTextureObjectName_Selection);
    glBindTexture (GL_TEXTURE_2D, sTextureObjectName_Selection);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, selectionSurface->w, selectionSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, selectionSurface->pixels);
    SDL_FreeSurface (selectionSurface);

    // load the 5 tile images
    glGenTextures (5, sTextureObjectNames_tileImages);
    for (int i = 0; i<5; i++) {
       SDL_Surface* tileImageSdlSurface = IMG_Load (sFilePath_tileImages[i]);
        if (tileImageSdlSurface  == NULL) {
            isSuccessful = false;
            printf ("IMG_Load failed for path: \"%s\". SDL_ERROR: %s\n", sFilePath_tileImages[i], SDL_GetError());
        }
        assert (tileImageSdlSurface != NULL);
        glBindTexture (GL_TEXTURE_2D, sTextureObjectNames_tileImages[i]);
        // in 2D with fixed size of the image, one would probably be better off scaling the jpg, png files than use mipmaps, so we're using linear filtering
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, tileImageSdlSurface->w, tileImageSdlSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tileImageSdlSurface->pixels);
        SDL_FreeSurface (tileImageSdlSurface);
    }

    // init modelview matrix
    sHudMvMatrix_background = glm::mat4 (1);
    glm::vec3 scaling = glm::vec3 (2.0, 2.0, 1.0);
    glm::vec3 translation = glm::vec3 (-1.0, -1.0, 0.0);
    sHudMvMatrix_background = glm::translate (sHudMvMatrix_background, translation);
    sHudMvMatrix_background = glm::scale (sHudMvMatrix_background, scaling);

    sHudMvMatrix_squareToGridPosition = glm::mat4 (1);
    scaling = glm::vec3 (2.0f* (VERTICAL_RIGHT_GRID_LINE-VERTICAL_LEFT_GRID_LINE), 2.0f* (HORIZONTAL_BOTTOM_GRID_LINE-HORIZONTAL_TOP_GRID_LINE), 1.0f);
    translation = glm::vec3 (-1.0f+2.0f*VERTICAL_LEFT_GRID_LINE,-1.0f+2.0f* (1.0f-HORIZONTAL_BOTTOM_GRID_LINE), 0.0f);
    sHudMvMatrix_squareToGridPosition = glm::translate (sHudMvMatrix_squareToGridPosition, translation);
    sHudMvMatrix_squareToGridPosition = glm::scale (sHudMvMatrix_squareToGridPosition, scaling);

    // init clear color
    glClearColor (0.5f, 0.5f, 0.5f, 1.0f);


    // text rendering
    TTF_Init();
    sTextFont = TTF_OpenFont ("assets/Rainbow2000.ttf", 32);
    glGenTextures (1, &sTextureObjectName_text);

    sHudMvMatrix_squareToTextPosition = glm::mat4 (1);
    scaling = glm::vec3 (0.03f, 0.05f, 1.0f);
    translation = glm::vec3 (-1.0f + 0.10f, -1.0f + 1.45f, 0.0f);
    sHudMvMatrix_squareToTextPosition = glm::translate (sHudMvMatrix_squareToTextPosition, translation);
    sHudMvMatrix_squareToTextPosition = glm::scale (sHudMvMatrix_squareToTextPosition, scaling);


    // print success
    if (isSuccessful) {
        printf ("GameStateRenderer::InitRenderer DONE.\n");
    } else {
        printf ("GameStateRenderer::InitRenderer FAILED.\n");
    }
    return isSuccessful;
}


GLint GameStateRenderer::CompileShader (GLuint* shader, GLenum shaderType, const GLchar** codeString)
{
    bool isSuccessful = true;
    *shader = glCreateShader (shaderType);
    glShaderSource (*shader, 1, codeString, NULL);
    glCompileShader (*shader);
    GLint shaderCompilationReturnValue = GL_FALSE;
    glGetShaderiv (*shader, GL_COMPILE_STATUS, &shaderCompilationReturnValue);

    if (shaderCompilationReturnValue != GL_TRUE)
    {
        printf ("shader compilation failed: %d!\n", *shader);
        isSuccessful = false;
    }
    return isSuccessful;
}


bool GameStateRenderer::Render (const GameState& gameState)
{
    bool isSuccessful = true;
    glClear (GL_COLOR_BUFFER_BIT);
    glDisable (GL_DEPTH_TEST);

    DrawBackground(false);
    DrawGameState (gameState);
    DrawBackground(true);

    // render time left
    int timeLeft = gameState.GetGameplayTimeLeft();
    int score = gameState.GetScore();
    std::string scoreText = "Score: " + std::to_string (score);
    glm::vec3 translateText = glm::vec3 (0.0f, 0.0f, 0.0f);
    RenderText (glm::translate (sHudMvMatrix_squareToTextPosition, translateText), "TestGame", 8);
    translateText = glm::vec3 (0.0f, -2.0f, 0.0f);
    RenderText (glm::translate (sHudMvMatrix_squareToTextPosition, translateText), scoreText.c_str(), scoreText.length());
    scoreText = "Time left: " + std::to_string (timeLeft);
    translateText = glm::vec3 (0.0f, -4.0f, 0.0f);
    RenderText (glm::translate (sHudMvMatrix_squareToTextPosition, translateText), scoreText.c_str(), scoreText.length());

	SDL_GL_SwapWindow (sSdlWindow);
    return isSuccessful;
}


void GameStateRenderer::DrawBackground (bool transparent)
{
    if (transparent) {
        DrawHudSquare (sHudMvMatrix_background, sTextureObjectName_backgroundTransparent);
    } else {
        DrawHudSquare (sHudMvMatrix_background, sTextureObjectName_BackGroundJpg);
    }
}


void GameStateRenderer::DrawGameState (const GameState& gameState)
{
    if (GameState::CollapsingTiles == gameState.GetAnimationState()) {
        // move to separate function for this case to prevent bloating the code here further
        DrawGameStateCollaping (gameState);
        return;
    }

    int rows = gameState.GetRows();
    int columns = gameState.GetColumns();
    //ask gameState about tiles being dragged
    int draggedTileRow = gameState.GetDraggedTileRow();
    int draggedTileColumn = gameState.GetDraggedTileColumn();
    int replacedTileRow = gameState.GetReplacedTileRow();
    int replacedTileColumn = gameState.GetReplacedTileColumn();

    // draw all tiles currently not being dragged nor replaced
    for (int currentRow = 0; currentRow < rows; currentRow++) {
        for (int currentColumn = 0; currentColumn < columns; currentColumn++) {
            GameState::Color tileColor = gameState.GetColorAt (currentRow, currentColumn);
            if (GameState::NotAColor == tileColor || GameState::DestroyedColor == tileColor) {
                continue;
            }
            glm::mat4 tileLocationMatrix = glm::mat4 (1);
            glm::vec3 scaling (1.0f/static_cast<float>(columns), 1.0f/static_cast<float>(rows), 1.0f);
            glm::vec3 translation  = glm::vec3 (1.0f/static_cast<float>(columns) * currentColumn, 1.0f/static_cast<float>(rows) * currentRow, 1.0f);
            if (currentRow == draggedTileRow && currentColumn == draggedTileColumn) {
                continue;
            }
            if (currentRow == replacedTileRow && currentColumn == replacedTileColumn) {
                continue;
            }
            translation.y = 1.0f - 1.0f/static_cast<float>(rows) - translation.y;
            tileLocationMatrix = glm::translate (tileLocationMatrix, translation);
            tileLocationMatrix = glm::scale (tileLocationMatrix, scaling);
            if (gameState.IsTileBeingDestroyed (currentRow, currentColumn)) {
                DrawHudSquareDestroyed (sHudMvMatrix_squareToGridPosition * tileLocationMatrix,
                       sTextureObjectNames_tileImages[static_cast<int>(tileColor)-1],
                       gameState.GetAnimationPercentage());
            } else {
                DrawHudSquare (sHudMvMatrix_squareToGridPosition * tileLocationMatrix,
                       sTextureObjectNames_tileImages[static_cast<int>(tileColor)-1]);
            }
            if (GameState::Idle == gameState.GetAnimationState()) {
                if (currentRow == gameState.GetSelectedTileRow() && currentColumn == gameState.GetSelectedTileColumn()) {
                    DrawHudSquare (sHudMvMatrix_squareToGridPosition * tileLocationMatrix, sTextureObjectName_Selection);
                }
            }
        }
    }

    // draw dragged and replaced tiles
    if (gameState.IsDragActive() || gameState.GetAnimationState() == GameState::SwappingTiles) {
        glm::mat4 tileLocationMatrix = glm::mat4 (1);
        glm::vec3 scaling (1.0f/static_cast<float>(columns), 1.0f/static_cast<float>(rows), 1.0f);

        if (replacedTileRow != -1 && replacedTileColumn != -1) {
            glm::vec3 replacedTileTranslation  = glm::vec3 (1.0f/static_cast<float>(columns) * replacedTileColumn, 1.0f/static_cast<float>(rows) * replacedTileRow, 1.0f);
            replacedTileTranslation -= gameState.GetCurrentDraggedTileDisplacement();
            replacedTileTranslation.y = 1.0f - 1.0f/static_cast<float>(rows) - replacedTileTranslation.y;
            tileLocationMatrix = glm::translate (glm::mat4 (1), replacedTileTranslation);
            tileLocationMatrix = glm::scale (tileLocationMatrix, scaling);
            GameState::Color tileColor = gameState.GetColorAt (replacedTileRow, replacedTileColumn);
            if (GameState::NotAColor != tileColor && GameState::DestroyedColor != tileColor) {
                DrawHudSquare (sHudMvMatrix_squareToGridPosition * tileLocationMatrix,
                       sTextureObjectNames_tileImages[static_cast<int>(tileColor)-1]);
            }
        }

        glm::vec3 draggedTileTranslation  = glm::vec3 (1.0f/static_cast<float>(columns) * draggedTileColumn, 1.0f/static_cast<float>(rows) * draggedTileRow, 1.0f);
        draggedTileTranslation += gameState.GetCurrentDraggedTileDisplacement();
        draggedTileTranslation.y = 1.0f - 1.0f/static_cast<float>(rows) - draggedTileTranslation.y;
        tileLocationMatrix = glm::translate (glm::mat4 (1), draggedTileTranslation);
        tileLocationMatrix = glm::scale (tileLocationMatrix, scaling);
        GameState::Color tileColor = gameState.GetColorAt (draggedTileRow, draggedTileColumn);
        if (GameState::NotAColor != tileColor && GameState::DestroyedColor != tileColor) {
            DrawHudSquare (sHudMvMatrix_squareToGridPosition * tileLocationMatrix,
                   sTextureObjectNames_tileImages[static_cast<int>(tileColor)-1]);
        }
    }
}


void GameStateRenderer::DrawHudSquareDestroyed (const glm::mat4& transformMatrix, const GLuint& textureObject, float destructionPercentage)
{
    // render square
    // choose the shader program to use with the draw call
    glUseProgram (sHudTexShaderProgram);
    // enable position2D and vertTexCoordinates attribute arrays in the shader program by providing their locations
    glEnableVertexAttribArray (sHudTexShaderProgram_position2DLocation);
    // now that the vertex attribute array is enabled, we need to specify what data to use for it
    // this will bind the relevant vertex data to GL_ARRAY_BUFFER target, it will be used as data for
    // vertex attribute pointer
    glBindBuffer (GL_ARRAY_BUFFER, sHudSquare_VBO);
    // now we need to specify how to use the target (GL_ARRAY_BUFFER) with position2D attribute
    glVertexAttribPointer (sHudTexShaderProgram_position2DLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (GLfloat), 0);
    // now we need to specify in what order should the vertices in :
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, sHudSquare_IBO);
    // no addition configuration needed here, we can use the draw call

    // TEXTURE
    glEnableVertexAttribArray (sHudTexShaderProgram_texPosition2DLocation);
    // bind texture coordinates buffer object to array buffer target to set up the pointer for it
    glBindBuffer (GL_ARRAY_BUFFER, sHudSquare_textureCoordinatesBo);
    glVertexAttribPointer (sHudTexShaderProgram_texPosition2DLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (GLfloat), 0);
    // activate the texture image unit 0 and bind
    glActiveTexture (GL_TEXTURE0);
    // initialize the sampler2D uniform
    glUniform1i (sHudTexShaderProgram_samplerLocation, 0);
    // bind the texture object (generated in init) to the appropriate GL_TEXTURE_2D target so it will be associated with the active texture image unit 0.
    glBindTexture (GL_TEXTURE_2D, textureObject);
    // modelview matrix
    glUniformMatrix4fv (sHudTexShaderProgram_mvMatrixUniformLocation, 1, GL_FALSE, &transformMatrix[0][0]);
    // destruction percentage
    glUniform1f (sHudTexShaderProgram_destroyUniLoc, destructionPercentage);

    glEnable (GL_BLEND);
    glBlendEquation (GL_FUNC_ADD);
    glBlendFuncSeparate (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    // the final draw call
    glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);


    // clean up
    glDisable (GL_BLEND);
    glDisableVertexAttribArray (sHudTexShaderProgram_texPosition2DLocation);
    glDisableVertexAttribArray (sHudTexShaderProgram_position2DLocation);
    glUseProgram (0);
}


void GameStateRenderer::DrawHudSquare (const glm::mat4& transformMatrix, const GLuint& textureObject)
{
    DrawHudSquareDestroyed (transformMatrix, textureObject, 0.0f);
}


glm::vec2 GameStateRenderer::GetGridCoordinatesFromScreenLocation (int x, int y)
{
    float xByWidth = static_cast<float>(x)/static_cast<float>(SCREEN_WIDTH);
    float yByWidth = static_cast<float>(y)/static_cast<float>(SCREEN_HEIGHT);
    glm::vec2 gridCoordinates = glm::vec2 (
            (xByWidth - VERTICAL_LEFT_GRID_LINE)/ (VERTICAL_RIGHT_GRID_LINE - VERTICAL_LEFT_GRID_LINE),
            (yByWidth - HORIZONTAL_TOP_GRID_LINE)/ (HORIZONTAL_BOTTOM_GRID_LINE - HORIZONTAL_TOP_GRID_LINE)
       );
    return gridCoordinates;
}



void GameStateRenderer::DrawGameStateCollaping (const GameState& gameState)
{
    int rows = gameState.GetRows();
    int columns = gameState.GetColumns();
    float animationPercentage = gameState.GetAnimationPercentage();
    std::vector<std::vector<int>> columnsToCollapse = gameState.GetColumnsToCollapse();


    for (int currentColumn = 0; currentColumn < columns; currentColumn++) {
        glm::vec3 collapseAdjustment = glm::vec3 (0.0f, 0.0f, 0.0f);
        if (columnsToCollapse[currentColumn][0] != -1) {
            float adjustment = static_cast<float>(columnsToCollapse[currentColumn][1]) * (1.0f/static_cast<float>(rows));
            adjustment *= 1 - animationPercentage;
            collapseAdjustment.y = -adjustment;
        }


        for (int currentRow = 0; currentRow < rows; currentRow++) {
            GameState::Color tileColor = gameState.GetColorAt (currentRow, currentColumn);
            if (GameState::NotAColor == tileColor || GameState::DestroyedColor == tileColor) {
                continue;
            }

            glm::mat4 tileLocationMatrix = glm::mat4 (1);
            glm::vec3 scaling (1.0f/static_cast<float>(columns), 1.0f/static_cast<float>(rows), 1.0f);
            glm::vec3 translation  = glm::vec3 (1.0f/static_cast<float>(columns) * currentColumn, 1.0f/static_cast<float>(rows) * currentRow, 1.0f);
            if (currentRow <= columnsToCollapse[currentColumn][0]) {
                translation += collapseAdjustment;
            }
            translation.y = 1.0f - 1.0f/static_cast<float>(rows) - translation.y;
            tileLocationMatrix = glm::translate (tileLocationMatrix, translation);
            tileLocationMatrix = glm::scale (tileLocationMatrix, scaling);

            DrawHudSquare (sHudMvMatrix_squareToGridPosition * tileLocationMatrix,
                   sTextureObjectNames_tileImages[static_cast<int>(tileColor)-1]);
        }
    }
}


void GameStateRenderer::RenderText (glm::mat4 mvMatrix, const char* text, int textLength)
{
    sTextSurface = TTF_RenderText_Blended (sTextFont, text, sSdlColorWhite);
    glBindTexture (GL_TEXTURE_2D, sTextureObjectName_text);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, sTextSurface->w, sTextSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, sTextSurface->pixels);
    SDL_FreeSurface (sTextSurface);
    glm::vec3 scaling = glm::vec3 (1.0f * textLength, 1.0f, 1.0f);
    DrawHudSquare (glm::scale (mvMatrix, scaling), sTextureObjectName_text);
}
