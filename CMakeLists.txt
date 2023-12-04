set(title "TestGame")

set(CMAKE_CXX_STANDARD 11)

project(${title})


set(TARGET "MSVC")
if (MSVC)
    cmake_minimum_required(VERSION 3.7.2)
    add_definitions(-DTARGET_MSVC)
    include_directories("${CMAKE_SOURCE_DIR}/external/${TARGET}/SDL/include")
	include_directories("${CMAKE_SOURCE_DIR}/external/${TARGET}/GLEW/include")
	include_directories("${CMAKE_SOURCE_DIR}/external/${TARGET}/GLM")
	link_directories("${CMAKE_SOURCE_DIR}/external/${TARGET}/SDL/lib/x86")
	link_directories("${CMAKE_SOURCE_DIR}/external/${TARGET}/GLEW/lib/Release/Win32")
elseif(UNIX)
set(TARGET "UNIX")
    cmake_minimum_required(VERSION 3.5.1)
    add_definitions(-DTARGET_UNIX)
endif(MSVC)


# add TestGame and GameState Renderer class definitions
include_directories("${CMAKE_SOURCE_DIR}/src/testgame")
add_executable(${title} src/main.cpp)
target_sources(${title} PUBLIC "${CMAKE_SOURCE_DIR}/src/testgame/TestGame.cpp")
target_sources(${title} PUBLIC "${CMAKE_SOURCE_DIR}/src/testgame/GameStateRenderer.cpp")
target_sources(${title} PUBLIC "${CMAKE_SOURCE_DIR}/src/testgame/GameState.cpp")
target_sources(${title} PUBLIC "${CMAKE_SOURCE_DIR}/src/testgame/GameStateLogic.cpp")

if (MSVC)
	# link glew libraries
	target_link_libraries(${title} opengl32)
	target_link_libraries(${title} glew32s)
	# glew32s is for static linking and that requres the define below
	add_definitions(-DGLEW_STATIC)
	target_link_libraries(${title} SDL2)
	target_link_libraries(${title} SDL2main)
	target_link_libraries(${title} SDL2_image)
    target_link_libraries(${title} SDL2_ttf)
endif (MSVC)

if (UNIX)
    target_link_libraries(${title} GL)
    target_link_libraries(${title} GLEW)
    target_link_libraries(${title} SDL2)
    target_link_libraries(${title} SDL2main)
    target_link_libraries(${title} SDL2_image)
    target_link_libraries(${title} SDL2_ttf)
endif(UNIX)


if (MSVC)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${title})
# copy assets to where Visual studio builds executable by default and where it typically runs it from
	add_custom_command(TARGET ${title} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_SOURCE_DIR}/assets/"
		"$<TARGET_FILE_DIR:${title}>/assets")
	add_custom_command(TARGET ${title} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_SOURCE_DIR}/assets/"
		"$<TARGET_FILE_DIR:${title}>/../assets")

	add_custom_command(TARGET ${title} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_SOURCE_DIR}/external/${TARGET}/SDL/lib/x86"
		$<TARGET_FILE_DIR:${title}>)
	add_custom_command(TARGET ${title} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/external/${TARGET}/GLEW/bin/Release/Win32/glew32.dll"
		$<TARGET_FILE_DIR:${title}>)
endif (MSVC)
