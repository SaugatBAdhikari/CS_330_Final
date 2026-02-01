///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

// declaration of the global variables and defines
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// camera object used for viewing and interacting with
	// the 3D scene
	Camera* g_pCamera = nullptr;

	// these variables are used for mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// time between current frame and last frame
	float gDeltaTime = 0.0f; 
	float gLastFrame = 0.0f;

	// the following variable is false when orthographic projection
	// is off and true when it is on
	bool bOrthographicProjection = false;

	// key edge-detection flags (prevents rapid fire toggles while held)
	bool gPKeyDown = false;
	bool gOKeyDown = false;

	// --------------------------------------------------------------------------
	// Saved "free-look" camera state (used for restoring after top-down view)
	// --------------------------------------------------------------------------
	bool gHasSavedFreeLookCamera = false;
	glm::vec3 gSavedPosition(0.0f);
	glm::vec3 gSavedFront(0.0f, 0.0f, -1.0f);
	glm::vec3 gSavedUp(0.0f, 1.0f, 0.0f);
	glm::vec3 gSavedRight(1.0f, 0.0f, 0.0f);
	glm::vec3 gSavedWorldUp(0.0f, 1.0f, 0.0f);
	float gSavedYaw = 0.0f;
	float gSavedPitch = 0.0f;
	float gSavedZoom = 45.0f;
	float gSavedMovementSpeed = 2.5f;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(
	ShaderManager *pShaderManager)
{
	// initialize the member variables
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();
	// default camera view parameters
	g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
	// free up allocated memory
	m_pShaderManager = NULL;
	m_pWindow = NULL;
	if (NULL != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

/***********************************************************
CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	// try to create the displayed OpenGL window
	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	// tell GLFW to capture all mouse events
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// this callback is used to receive mouse moving events
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

	// this callback is used to receive mouse scroll wheel events
	// (Milestone Three: scroll adjusts camera speed dynamically)
	glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Callback);

	// enable blending for supporting tranparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	// In fixed top-down orthographic mode, ignore mouse-look so the view remains
	// explicitly looking straight down (Milestone Three rubric requirement).
	if (bOrthographicProjection)
	{
		return;
	}

	if (gFirstMouse)
	{
		gLastX = (float)xMousePos;
		gLastY = (float)yMousePos;
		gFirstMouse = false;
	}

	float xoffset = (float)xMousePos - gLastX;
	// reversed since y-coordinates go from bottom to top
	float yoffset = gLastY - (float)yMousePos;

	gLastX = (float)xMousePos;
	gLastY = (float)yMousePos;

	if (g_pCamera)
	{
		g_pCamera->ProcessMouseMovement(xoffset, yoffset);
	}
}

/***********************************************************
 *  Mouse_Scroll_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse scroll wheel is moved.
 *
 *  Milestone Three requirement:
 *  - Scroll wheel dynamically adjusts camera speed.
 ***********************************************************/
void ViewManager::Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset)
{
	(void)window;
	(void)xOffset;

	if (g_pCamera)
	{
		// ----------------------------------------------------------------------
		// Milestone Three requirement:
		// - Scroll UP increases camera MovementSpeed
		// - Scroll DOWN decreases camera MovementSpeed
		// - Clamp to keep the camera controllable
		//
		// NOTE: This remains enabled even in Orthographic (movement-locked) mode
		// so the user can set a preferred speed before returning to free-look.
		// ----------------------------------------------------------------------
		const float kMinSpeed = 1.0f;
		const float kMaxSpeed = 20.0f;
		const float kStepPerNotch = 1.0f;

		float newSpeed = g_pCamera->MovementSpeed + (float)yOffset * kStepPerNotch;
		if (newSpeed < kMinSpeed) newSpeed = kMinSpeed;
		if (newSpeed > kMaxSpeed) newSpeed = kMaxSpeed;
		g_pCamera->MovementSpeed = newSpeed;
	}
}

/***********************************************************
 *  processInput()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/
void ViewManager::processInput()
{
	// close the window if the escape key has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	// --------------------------------------------------------------------------
	// Projection toggling (Milestone Three)
	// P: perspective projection (45° FOV)
	// O: orthographic projection
	// --------------------------------------------------------------------------
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS && !gPKeyDown)
	{
		// If we were in orthographic top-down mode, restore the previous free-look
		// camera pose so the user returns exactly to where they were.
		if (bOrthographicProjection && g_pCamera && gHasSavedFreeLookCamera)
		{
			g_pCamera->Position = gSavedPosition;
			g_pCamera->Front = gSavedFront;
			g_pCamera->Up = gSavedUp;
			g_pCamera->Right = gSavedRight;
			g_pCamera->WorldUp = gSavedWorldUp;
			g_pCamera->Yaw = gSavedYaw;
			g_pCamera->Pitch = gSavedPitch;
			g_pCamera->Zoom = gSavedZoom;
			g_pCamera->MovementSpeed = gSavedMovementSpeed;

			// Prevent a large camera "jump" on the next mouse move after teleporting.
			gFirstMouse = true;
		}

		bOrthographicProjection = false;
		if (g_pCamera)
		{
			g_pCamera->Zoom = 45.0f; // required perspective FOV
		}
		gPKeyDown = true;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_RELEASE)
	{
		gPKeyDown = false;
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS && !gOKeyDown)
	{
		// Save the current free-look camera state so P can restore it later.
		if (!bOrthographicProjection && g_pCamera)
		{
			gHasSavedFreeLookCamera = true;
			gSavedPosition = g_pCamera->Position;
			gSavedFront = g_pCamera->Front;
			gSavedUp = g_pCamera->Up;
			gSavedRight = g_pCamera->Right;
			gSavedWorldUp = g_pCamera->WorldUp;
			gSavedYaw = g_pCamera->Yaw;
			gSavedPitch = g_pCamera->Pitch;
			gSavedZoom = g_pCamera->Zoom;
			gSavedMovementSpeed = g_pCamera->MovementSpeed;
		}

		// ----------------------------------------------------------------------
		// Orthographic top-down camera view (Milestone Three requirement)
		// - Position directly above scene center: (0, 10, 0)
		// - Look straight down: Front (0, -1, 0)
		// - Up vector: (0, 0, -1)
		// This makes the view unambiguously top-down.
		// ----------------------------------------------------------------------
		bOrthographicProjection = true;
		if (g_pCamera)
		{
			g_pCamera->Position = glm::vec3(0.0f, 10.0f, 0.0f);
			g_pCamera->Front = glm::vec3(0.0f, -1.0f, 0.0f);
			g_pCamera->WorldUp = glm::vec3(0.0f, 0.0f, -1.0f);

			// Keep the camera basis vectors consistent with the requested Front/Up.
			g_pCamera->Right = glm::normalize(glm::cross(g_pCamera->Front, g_pCamera->WorldUp));
			g_pCamera->Up = glm::normalize(glm::cross(g_pCamera->Right, g_pCamera->Front));

			// Prevent a large camera "jump" on the next mouse move after teleporting.
			gFirstMouse = true;
		}

		gOKeyDown = true;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_RELEASE)
	{
		gOKeyDown = false;
	}

	// --------------------------------------------------------------------------
	// Movement lock in Orthographic mode (Milestone Three rubric requirement)
	// When in top-down orthographic view, keep the camera perfectly centered by
	// ignoring WASD/QE movement inputs.
	// --------------------------------------------------------------------------
	if (bOrthographicProjection)
	{
		return;
	}

	// Move the camera so you can get around objects (e.g., see the handle behind the can).
	// Controls: W/S forward/back, A/D strafe, Q/E down/up.
	if (g_pCamera != NULL)
	{
		if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
		{
			g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
		}
		if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
		{
			g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
		}
		if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
		{
			g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
		}
		if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
		{
			g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);
		}
		if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
		{
			g_pCamera->ProcessKeyboard(DOWN, gDeltaTime);
		}
		if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
		{
			g_pCamera->ProcessKeyboard(UP, gDeltaTime);
		}
	}
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// per-frame timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// process any keyboard events that may be waiting in the 
	// event queue
	processInput();

	// get the current view matrix from the camera
	view = g_pCamera->GetViewMatrix();

	// --------------------------------------------------------------------------
	// Define the current projection matrix
	// - Perspective: uses the camera's Zoom as FOV (P sets this to 45°)
	// - Orthographic: a fixed scale, useful for top-down/front-like viewing
	// --------------------------------------------------------------------------
	const float aspect = (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT;
	if (!bOrthographicProjection)
	{
		projection = glm::perspective(glm::radians(g_pCamera->Zoom), aspect, 0.1f, 100.0f);
	}
	else
	{
		const float orthoScale = 10.0f;
		projection = glm::ortho(-orthoScale * aspect, orthoScale * aspect, -orthoScale, orthoScale, 0.1f, 100.0f);
	}

	// if the shader manager object is valid
	if (NULL != m_pShaderManager)
	{
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ViewName, view);
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		// set the view position of the camera into the shader for proper rendering
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}