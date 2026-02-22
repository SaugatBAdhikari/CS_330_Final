#include <iostream>         // error handling and output
#include <cstdlib>          // EXIT_FAILURE
#include <string>

#include <GL/glew.h>        // GLEW library
#include "GLFW/glfw3.h"     // GLFW library

#ifdef _WIN32
#include <windows.h>
#endif

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneManager.h"
#include "ViewManager.h"
#include "ShapeMeshes.h"
#include "ShaderManager.h"

// Namespace for declaring global variables
namespace
{
	// Macro for window title
	const char* const WINDOW_TITLE = "7-1 FinalProject and Milestones"; 

	// Main GLFW window
	GLFWwindow* g_Window = nullptr;

	// scene manager object for managing the 3D scene prepare and render
	SceneManager* g_SceneManager = nullptr;
	// shader manager object for dynamic interaction with the shader code
	ShaderManager* g_ShaderManager = nullptr;
	// view manager object for managing the 3D view setup and projection to 2D
	ViewManager* g_ViewManager = nullptr;
}

// Function declarations - all functions that are called manually
// need to be pre-declared at the beginning of the source code.
bool InitializeGLFW();
bool InitializeGLEW();

// Returns the directory containing the executable (with trailing slash), or empty string on failure.
std::string GetExeDirectory();


/***********************************************************
 *  main(int, char*)
 *
 *  This function gets called after the application has been
 *  launched.
 ***********************************************************/
int main(int argc, char* argv[])
{
	// if GLFW fails initialization, then terminate the application
	if (InitializeGLFW() == false)
	{
		return(EXIT_FAILURE);
	}

	// try to create a new shader manager object
	g_ShaderManager = new ShaderManager();
	// try to create a new view manager object
	g_ViewManager = new ViewManager(
		g_ShaderManager);

	// try to create the main display window
	g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);

	// if GLEW fails initialization, then terminate the application
	if (InitializeGLEW() == false)
	{
		return(EXIT_FAILURE);
	}

	// Load shader code: look next to the exe first (so extracted/standalone exe works), then CWD, then Utilities.
	std::string exeDir = GetExeDirectory();
	std::string vertexPath = exeDir + "shaders/vertexShader.glsl";
	std::string fragmentPath = exeDir + "shaders/fragmentShader.glsl";
	GLuint programId = g_ShaderManager->LoadShaders(vertexPath.c_str(), fragmentPath.c_str());
	if (programId == 0)
	{
		programId = g_ShaderManager->LoadShaders(
			"shaders/vertexShader.glsl",
			"shaders/fragmentShader.glsl");
	}
	if (programId == 0)
	{
		programId = g_ShaderManager->LoadShaders(
			"../../Utilities/shaders/vertexShader.glsl",
			"../../Utilities/shaders/fragmentShader.glsl");
	}
	g_ShaderManager->use();

	// Create scene manager and set resource base to exe directory so textures load next to exe.
	g_SceneManager = new SceneManager(g_ShaderManager);
	g_SceneManager->SetResourceBasePath(exeDir);
	g_SceneManager->PrepareScene();

	// loop will keep running until the application is closed 
	// or until an error has occurred
	while (!glfwWindowShouldClose(g_Window))
	{
		// Enable z-depth
		glEnable(GL_DEPTH_TEST);

		// Clear the frame and z buffers
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// convert from 3D object space to 2D view
		g_ViewManager->PrepareSceneView();

		// refresh the 3D scene
		g_SceneManager->RenderScene();


		// Flips the the back buffer with the front buffer every frame.
		glfwSwapBuffers(g_Window);

		// query the latest GLFW events
		glfwPollEvents();
	}

	// clear the allocated manager objects from memory
	if (NULL != g_SceneManager)
	{
		delete g_SceneManager;
		g_SceneManager = NULL;
	}
	if (NULL != g_ViewManager)
	{
		delete g_ViewManager;
		g_ViewManager = NULL;
	}
	if (NULL != g_ShaderManager)
	{
		delete g_ShaderManager;
		g_ShaderManager = NULL;
	}

	// Terminates the program successfully
	exit(EXIT_SUCCESS); 
}

/***********************************************************
 *	InitializeGLFW()
 * 
 *  This function is used to initialize the GLFW library.   
 ***********************************************************/
bool InitializeGLFW()
{
	// GLFW: initialize and configure library
	// --------------------------------------
	glfwInit();

#ifdef __APPLE__
	// set the version of OpenGL and profile to use
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	// set the version of OpenGL and profile to use
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	// GLFW: end -------------------------------

	return(true);
}

/***********************************************************
 *	InitializeGLEW()
 *
 *  This function is used to initialize the GLEW library.
 ***********************************************************/
bool InitializeGLEW()
{
	// GLEW: initialize
	// -----------------------------------------
	GLenum GLEWInitResult = GLEW_OK;

	// try to initialize the GLEW library
	GLEWInitResult = glewInit();
	if (GLEW_OK != GLEWInitResult)
	{
		std::cerr << glewGetErrorString(GLEWInitResult) << std::endl;
		return false;
	}
	// GLEW: end -------------------------------

	// Displays a successful OpenGL initialization message
	std::cout << "INFO: OpenGL Successfully Initialized\n";
	std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << "\n" << std::endl;

	return(true);
}

/***********************************************************
 *	GetExeDirectory()
 *
 *  Returns the directory containing the executable, with a
 *  trailing slash, so resources (shaders, textures) can be
 *  loaded when the exe is run from any folder or extracted.
 ***********************************************************/
std::string GetExeDirectory()
{
#ifdef _WIN32
	char path[MAX_PATH];
	HMODULE hModule = GetModuleHandleA(NULL);
	if (hModule == NULL) return std::string();
	DWORD len = GetModuleFileNameA(hModule, path, MAX_PATH);
	if (len == 0 || len >= MAX_PATH) return std::string();
	std::string exePath(path);
	std::size_t lastSlash = exePath.find_last_of("\\/");
	if (lastSlash != std::string::npos)
		return exePath.substr(0, lastSlash + 1);
#endif
	return std::string();
}