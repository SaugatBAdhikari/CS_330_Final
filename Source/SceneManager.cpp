///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
	m_loadedTextures = 0;
}

/***********************************************************
 *  SetResourceBasePath()
 *
 *  Sets the base directory for loading shaders and textures
 *  (e.g. the exe directory so the app works when run from anywhere).
 ***********************************************************/
void SceneManager::SetResourceBasePath(const std::string& basePath)
{
	m_resourceBasePath = basePath;
	if (!m_resourceBasePath.empty() && m_resourceBasePath.back() != '/' && m_resourceBasePath.back() != '\\')
		m_resourceBasePath += '/';
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;

	// Free the custom ground plane VAO/VBO (Milestone Three).
	DestroyGroundPlaneMesh();

	// Free loaded textures (Milestone Four).
	DestroyGLTextures();

	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glDeleteTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/***********************************************************
 *  SetSceneLights()
 *
 *  Sets Phong light source uniforms: one key light and one
 *  fill light so the scene is clearly lit with no complete shadow.
 ***********************************************************/
void SceneManager::SetSceneLights()
{
	if (m_pShaderManager == NULL) return;

	// Key light: pinkish point light above and in front of the scene.
	const glm::vec3 keyPos(4.0f, 8.0f, 5.0f);
	const glm::vec3 keyAmbient(0.35f, 0.18f, 0.28f);   // pink-tinted ambient
	const glm::vec3 keyDiffuse(0.95f, 0.55f, 0.75f);    // strong pink diffuse
	const glm::vec3 keySpecular(1.0f, 0.7f, 0.85f);     // pink-white specular
	m_pShaderManager->setVec3Value("lightSources[0].position", keyPos);
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", keyAmbient);
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", keyDiffuse);
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", keySpecular);
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 1.0f);

	// Fill light: softer pink/mauve from the other side so nothing is in complete shadow.
	const glm::vec3 fillPos(-5.0f, 5.0f, 4.0f);
	const glm::vec3 fillAmbient(0.2f, 0.12f, 0.2f);     // mauve ambient
	const glm::vec3 fillDiffuse(0.5f, 0.3f, 0.45f);    // soft pink diffuse
	const glm::vec3 fillSpecular(0.6f, 0.4f, 0.55f);   // pink specular
	m_pShaderManager->setVec3Value("lightSources[1].position", fillPos);
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", fillAmbient);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", fillDiffuse);
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", fillSpecular);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 24.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.6f);

	// Remaining light slots: zero contribution (no extra lights used).
	const glm::vec3 zero(0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[2].position", zero);
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", zero);
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", zero);
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", zero);
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 1.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.0f);
	m_pShaderManager->setVec3Value("lightSources[3].position", zero);
	m_pShaderManager->setVec3Value("lightSources[3].ambientColor", zero);
	m_pShaderManager->setVec3Value("lightSources[3].diffuseColor", zero);
	m_pShaderManager->setVec3Value("lightSources[3].specularColor", zero);
	m_pShaderManager->setFloatValue("lightSources[3].focalStrength", 1.0f);
	m_pShaderManager->setFloatValue("lightSources[3].specularIntensity", 0.0f);
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

/***********************************************************
 *  TryLoadTexture()
 *
 *  Milestone Four: Tries multiple paths to load a texture so
 *  the app works when run from the project dir or from the
 *  output (Debug/Release) directory.
 ***********************************************************/
bool SceneManager::TryLoadTexture(const char* primaryPath,
	const char* fallbackPath, const std::string& tag)
{
	if (CreateGLTexture(primaryPath, tag))
		return true;
	if (fallbackPath && fallbackPath[0] != '\0')
		return CreateGLTexture(fallbackPath, tag);
	return false;
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering.
 *
 *  Milestone Four: Loads textures for the ground plane and
 *  for the composite watering can (metal body/spout/bracket,
 *  wood handle). Tries paths relative to output dir first,
 *  then relative to project/solution (Utilities/textures).
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	// --------------------------------------------------------------------------
	// NOTE: We intentionally do NOT use ShapeMeshes' plane for Milestone Three.
	// The ground/floor is a custom VAO/VBO created in CreateGroundPlaneMesh().
	// --------------------------------------------------------------------------
	m_basicMeshes->LoadBoxMesh(); // bracket connector
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadTorusMesh();
	m_basicMeshes->LoadPyramid4Mesh(); // pyramid (e.g. plant pot)

	// Create the custom ground plane geometry (VAO/VBO).
	CreateGroundPlaneMesh();

	// --------------------------------------------------------------------------
	// Load textures from exe-relative path (m_resourceBasePath) or CWD.
	// Keeps shaders/textures next to the exe when you extract or run from anywhere.
	// --------------------------------------------------------------------------
	std::string texBase = m_resourceBasePath + "textures/";
	TryLoadTexture((texBase + "pavers.jpg").c_str(), "textures/pavers.jpg", "floor");
	TryLoadTexture((texBase + "stainless.jpg").c_str(), "textures/stainless.jpg", "metal");
	TryLoadTexture((texBase + "rusticwood.jpg").c_str(), "textures/rusticwood.jpg", "wood");

	// --------------------------------------------------------------------------
	// Materials for Phong lighting (Milestone: lighting).
	// Used so the plane reflects light and the watering can parts are lit.
	// --------------------------------------------------------------------------
	OBJECT_MATERIAL mat;
	// Floor (pavers): neutral, moderate specular so the plane reflects light.
	mat.tag = "floor";
	mat.ambientStrength = 0.25f;
	mat.ambientColor = glm::vec3(0.35f, 0.35f, 0.35f);
	mat.diffuseColor = glm::vec3(0.6f, 0.6f, 0.6f);
	mat.specularColor = glm::vec3(0.4f, 0.4f, 0.4f);
	mat.shininess = 32.0f;
	m_objectMaterials.push_back(mat);
	// Metal (body/spout/bracket): brighter specular for a metallic look.
	mat.tag = "metal";
	mat.ambientStrength = 0.2f;
	mat.ambientColor = glm::vec3(0.5f, 0.5f, 0.5f);
	mat.diffuseColor = glm::vec3(0.7f, 0.7f, 0.7f);
	mat.specularColor = glm::vec3(0.9f, 0.9f, 0.9f);
	mat.shininess = 64.0f;
	m_objectMaterials.push_back(mat);
	// Wood (handle): warmer, lower specular.
	mat.tag = "wood";
	mat.ambientStrength = 0.3f;
	mat.ambientColor = glm::vec3(0.4f, 0.3f, 0.2f);
	mat.diffuseColor = glm::vec3(0.5f, 0.35f, 0.2f);
	mat.specularColor = glm::vec3(0.15f, 0.1f, 0.05f);
	mat.shininess = 16.0f;
	m_objectMaterials.push_back(mat);
	// Pyramid: e.g. planter, matte.
	mat.tag = "pyramid";
	mat.ambientStrength = 0.3f;
	mat.ambientColor = glm::vec3(0.35f, 0.25f, 0.2f);
	mat.diffuseColor = glm::vec3(0.5f, 0.35f, 0.25f);
	mat.specularColor = glm::vec3(0.2f, 0.15f, 0.1f);
	mat.shininess = 20.0f;
	m_objectMaterials.push_back(mat);
	// Glass: transparent water glass (high specular, light blue tint).
	mat.tag = "glass";
	mat.ambientStrength = 0.15f;
	mat.ambientColor = glm::vec3(0.6f, 0.8f, 1.0f);
	mat.diffuseColor = glm::vec3(0.7f, 0.88f, 1.0f);
	mat.specularColor = glm::vec3(0.9f, 0.95f, 1.0f);
	mat.shininess = 96.0f;
	m_objectMaterials.push_back(mat);
	// Green cube box (right of pyramid).
	mat.tag = "greenbox";
	mat.ambientStrength = 0.3f;
	mat.ambientColor = glm::vec3(0.15f, 0.5f, 0.15f);
	mat.diffuseColor = glm::vec3(0.2f, 0.7f, 0.2f);
	mat.specularColor = glm::vec3(0.3f, 0.6f, 0.3f);
	mat.shininess = 32.0f;
	m_objectMaterials.push_back(mat);
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// Milestone Four: Bind all loaded textures to their slots so they are
	// available when we call SetShaderTexture() for each mesh.
	BindGLTextures();

	// Lighting: set light sources once per frame (Phong model; nothing in complete shadow).
	SetSceneLights();
	if (m_pShaderManager != NULL)
		m_pShaderManager->setIntValue(g_UseLightingName, true);

	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	// Move the floor down so the taller watering can sits on it (body height 4.5 => spans ~[-2.25, +2.25]).
	positionXYZ = glm::vec3(0.0f, -2.25f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Ground plane: texture + Phong lighting so light reflects off the plane.
	SetShaderMaterial("floor");
	if (FindTextureSlot("floor") >= 0)
	{
		SetShaderTexture("floor");
		SetTextureUVScale(4.0f, 4.0f);
	}
	else
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		SetShaderColor(0.0f, 0.1f, 0.4f, 1.0f);
	}

	// draw the custom ground plane VAO/VBO (Milestone Three)
	if (m_groundVAO != 0)
	{
		glBindVertexArray(m_groundVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
	/****************************************************************/

	// --- Pyramid (e.g. planter, left of scene), 3x scale ---
	scaleXYZ = glm::vec3(2.1f, 2.7f, 2.1f);  // 3x (0.7, 0.9, 0.7)
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-2.5f, 1.35f, 2.0f);  // Y = half height so base on ground
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderMaterial("pyramid");
	SetShaderColor(0.5f, 0.35f, 0.25f, 1.0f);
	m_basicMeshes->DrawPyramid4Mesh();

	// --- Green cube box (right of pyramid) ---
	scaleXYZ = glm::vec3(0.8f, 0.8f, 0.8f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.5f, 0.4f, 2.0f);  // right of pyramid (pyramid at x=-2.5)
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderMaterial("greenbox");
	SetShaderColor(0.2f, 0.7f, 0.2f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	// Draw the composite "Watering Can" object with per-part textures (Milestone Four).
	RenderComplexObject();

	// --- Transparent water glass (left of pyramid), drawn last for correct blending ---
	scaleXYZ = glm::vec3(0.35f, 1.1f, 0.35f);  // tall cylinder
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-5.0f, 1.1f, 2.0f);  // left of pyramid (pyramid at x=-2.5), base on ground
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderMaterial("glass");
	SetShaderColor(0.65f, 0.85f, 1.0f, 0.38f);  // light blue, transparent
	m_basicMeshes->DrawCylinderMesh(true, true, true);
}

// --------------------------------------------------------------------------
// Function: CreateGroundPlaneMesh
// Purpose : Creates a custom plane VAO/VBO for the scene floor.
// Vertex format: position (3), normal (3), texCoord (2)
// Shader locations: 0=position, 1=normal, 2=texCoord
// --------------------------------------------------------------------------
void SceneManager::CreateGroundPlaneMesh()
{
	// If already created (e.g., hot-reload style), avoid leaking buffers.
	DestroyGroundPlaneMesh();

	// Unit plane on the XZ axis (Y=0). We scale/translate it in RenderScene().
	const float groundVertices[] = {
		// positions            // normals         // UVs
		-0.5f, 0.0f, -0.5f,     0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
		 0.5f, 0.0f, -0.5f,     0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
		 0.5f, 0.0f,  0.5f,     0.0f, 1.0f, 0.0f,  1.0f, 1.0f,

		 0.5f, 0.0f,  0.5f,     0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		-0.5f, 0.0f,  0.5f,     0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
		-0.5f, 0.0f, -0.5f,     0.0f, 1.0f, 0.0f,  0.0f, 0.0f
	};

	glGenVertexArrays(1, &m_groundVAO);
	glGenBuffers(1, &m_groundVBO);

	glBindVertexArray(m_groundVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_groundVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

	const GLsizei stride = 8 * (GLsizei)sizeof(float);
	// position attribute (location = 0)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	// normal attribute (location = 1)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	// texture coordinate attribute (location = 2)
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// --------------------------------------------------------------------------
// Function: DestroyGroundPlaneMesh
// Purpose : Frees the custom ground plane VAO/VBO.
// --------------------------------------------------------------------------
void SceneManager::DestroyGroundPlaneMesh()
{
	if (m_groundVBO != 0)
	{
		glDeleteBuffers(1, &m_groundVBO);
		m_groundVBO = 0;
	}
	if (m_groundVAO != 0)
	{
		glDeleteVertexArrays(1, &m_groundVAO);
		m_groundVAO = 0;
	}
}

// --------------------------------------------------------------------------
// Function: RenderComplexObject
// Purpose : Draws a composite 3D Watering Can using a Cylinder, Cone, Box,
//           and Cylinder. Milestone Four: each part uses a different texture
//           (metal for body/spout/bracket, wood for handle) for a cohesive
//           but visually distinct object.
// --------------------------------------------------------------------------
void SceneManager::RenderComplexObject()
{
	if (m_pShaderManager == NULL || m_basicMeshes == NULL) return;

	const GLint modelLoc = glGetUniformLocation(m_pShaderManager->m_programID, "model");
	const GLint colorLoc = glGetUniformLocation(m_pShaderManager->m_programID, "objectColor");
	const GLint useTextureLoc = glGetUniformLocation(m_pShaderManager->m_programID, "bUseTexture");

	glm::mat4 model;
	const int metalSlot = FindTextureSlot("metal");
	const int woodSlot = FindTextureSlot("wood");
	const bool useTextures = (metalSlot >= 0 || woodSlot >= 0);

	// Default UV scale for watering can parts (no tiling; one-to-one mapping).
	SetTextureUVScale(1.0f, 1.0f);

	// --- PART 1: MAIN BODY (Tall Cylinder) - metal texture + lighting ---
	SetShaderMaterial("metal");
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.5f, 4.5f, 1.5f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	if (useTextures && metalSlot >= 0)
	{
		if (useTextureLoc >= 0) glUniform1i(useTextureLoc, GL_TRUE);
		SetShaderTexture("metal");
	}
	else
	{
		if (useTextureLoc >= 0) glUniform1i(useTextureLoc, GL_FALSE);
		glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	m_basicMeshes->DrawCylinderMesh(true, true, true);

	// --- PART 2: SPOUT (Cone) - metal texture + lighting ---
	SetShaderMaterial("metal");
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(1.0f, 1.2f, 0.0f));
	model = glm::rotate(model, glm::radians(-40.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.4f, 4.0f, 0.4f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	if (useTextures && metalSlot >= 0)
	{
		if (useTextureLoc >= 0) glUniform1i(useTextureLoc, GL_TRUE);
		SetShaderTexture("metal");
	}
	else
	{
		if (useTextureLoc >= 0) glUniform1i(useTextureLoc, GL_FALSE);
		glUniform4f(colorLoc, 0.95f, 0.95f, 0.95f, 1.0f);
	}
	m_basicMeshes->DrawConeMesh(true);

	// --- PART 3: BRACKET (Box connector) - metal texture + lighting ---
	SetShaderMaterial("metal");
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 1.8f, 0.0f));
	model = glm::scale(model, glm::vec3(0.9f, 0.3f, 0.5f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	if (useTextures && metalSlot >= 0)
	{
		if (useTextureLoc >= 0) glUniform1i(useTextureLoc, GL_TRUE);
		SetShaderTexture("metal");
	}
	else
	{
		if (useTextureLoc >= 0) glUniform1i(useTextureLoc, GL_FALSE);
		glUniform4f(colorLoc, 0.9f, 0.9f, 0.9f, 1.0f);
	}
	m_basicMeshes->DrawBoxMesh();

	// --- PART 4: HANDLE (Cylinder) - wood texture + lighting ---
	SetShaderMaterial("wood");
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.45f, 0.6f, 0.0f));
	model = glm::scale(model, glm::vec3(0.3f, 2.6f, 0.3f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	if (useTextures && woodSlot >= 0)
	{
		if (useTextureLoc >= 0) glUniform1i(useTextureLoc, GL_TRUE);
		SetShaderTexture("wood");
	}
	else
	{
		if (useTextureLoc >= 0) glUniform1i(useTextureLoc, GL_FALSE);
		glUniform4f(colorLoc, 0.6f, 0.4f, 0.2f, 1.0f);
	}
	m_basicMeshes->DrawCylinderMesh(true, true, true);
}
