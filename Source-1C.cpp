// ********************************
// Αυτό το αρχείο θα χρησιμοποιήσετε για να υλοποιήσετε την άσκηση 1Γ της OpenGL
//
// ΑΜ: 2653 Όνομα: ΠΑΝΑΓΙΩΤΗΣ ΒΟΥΖΑΛΗΣ
// ΑΜ: 2732 Όνομα: ΓΕΩΡΓΙΟΣ ΚΟΥΚΟΥΒΙΝΟΣ
// 
// *********************************

// Include standard headers
#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
using namespace std;

#include <stdlib.h>
#include <string.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
using namespace glm;

#include "controls.hpp"
#include "objloader.hpp"

// Include windows.h
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "Winmm.lib")

// Number of objects to be drawn on screen.
// This number includes the crater textures.
// Depending on the number of polygons the crater texture has
// and the number of craters we wish to display
// the startup loading time of our app will increase.
// We create an array, so 0 refers to ground plane, 1 to fireball
// 2 to 802 refer to craters when a fireball hits an area for the 1st time
#define number_of_objects 802
int objCounter = number_of_objects;

#define number_of_big_craters 400

#define windowWidth 1000
#define windowHeight 1000

//************************************
// Η LoadShaders είναι black box για σας
//************************************
GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

//*******************************************************************************
// Η παρακάτω συνάρτηση είναι από http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
// H συνάρτηση loadOBJ φορτώνει ένα αντικείμενο από το obj αρχείο του και φορτώνει και normals kai uv συντεταγμένες
// Την χρησιμοποιείτε όπως το παράδειγμα που έχω στην main

// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide : 
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

bool loadOBJ(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals
) {
	objCounter -= 1;
	printf("Loading OBJ file %s... %i objects remaining...\n", path, objCounter);
	
	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}
	fclose(file);
	return true;
}

glm::vec3 calculateFireballSpawnPoint() {

	// Orismos lower kai upper bound gia ta spawn coords tou fireball
	float spawnLowerBound = 0.0f; 
	float spawnUpperBound = 100.0f;
	
	// Choose a random value between [lowerBound, upperBound]
	// We choose the 500th and 1000th number that are provided by the seed
	float x;
	float y;
	for (int i = 0; i < 500; i++) { x = spawnLowerBound + (float)rand() * (float)(spawnUpperBound - spawnLowerBound) / (float)RAND_MAX; }
	for (int i = 0; i < 1000; i++) { y = spawnLowerBound + (float)rand() * (float)(spawnUpperBound - spawnLowerBound) / (float)RAND_MAX; }
	
	glm::vec3 spawnPoint = glm::vec3(x, y, 20.0f); // The fireball always starts at a height of z = 20
	//glm::vec3 spawnPoint = glm::vec3(20.0, 20.0f, 20.0f); // Debugging
	//std::cout << "Fireball spawn point: " + glm::to_string(spawnPoint) << std::endl; // Debugging
	return spawnPoint;
}

float zOffset_initial = 0.001f; // initial speed offset for fireball descent 0.0001f
float zOffset = zOffset_initial; // speed offset for fireball descent
void checkForSpeedPresses() {
	// Make the fireball descend faster or slower

	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
		zOffset = zOffset * 1.025f;
		//if (zOffset > 0.1f) { zOffset = 0.0001f; } // Reset offset if too fast
	}
	else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		zOffset = zOffset / 1.02f;
		if (zOffset < 0.00001f) { zOffset = 0.0001f; } // Reset offset if too slow
	}
}

float fireball_radius = 1.0; // Taken from meshlab - BALLZ.obj attributes
glm::vec3 ground_plane = glm::vec3(0.0f, 0.0f, 0.0f); // Our ground plane sits at the xy plane where z = 0

// Array with flag variables used to indicate whether to draw a crater or not
int should_draw_craters_array[number_of_objects - 2] = {};
// Array with the coords of each crater
glm::vec3 craters_coords_array[number_of_objects - 2] = {};
// Count the number of craters
int craterCounter = 0;

// Initialize a coords vector
glm::vec3 crater_impact_coords = glm::vec3(0.0f, 0.0f, 0.0f);

// Array with flag variables used to indicate whether to draw a bigger crater or not
int should_draw_2ndcraters_array[number_of_big_craters] = {};
// Array with the coords of each bigger crater
glm::vec3 second_craters_coords_array[number_of_big_craters] = {};
// Count the number of 2nd craters
int secondCraterCounter = 0;

bool checkForCollision(glm::vec3 fireballPos, glm::vec3 ground_plane, float fireball_radius) {
	// Check if the fireball collides with the ground plane

	float distance = fireballPos.z - ground_plane.z; // simple distance of z coords
	if (distance < fireball_radius) {
		
		//crater_impact_coords = glm::vec3(fireballPos.x, fireballPos.y, -1.0f); // complex crater
		crater_impact_coords = glm::vec3(fireballPos.x, fireballPos.y, -1.5f); // simple crater
		float offset = 5.0f;
		if ((100.0f - fireballPos.x) < offset) { crater_impact_coords = glm::vec3(crater_impact_coords.x - offset, crater_impact_coords.y, crater_impact_coords.z); }
		if ((100.0f - fireballPos.y) < offset) { crater_impact_coords = glm::vec3(crater_impact_coords.x, crater_impact_coords.y - offset, crater_impact_coords.z); }

		//std::cout << "Fireball impact point:" + glm::to_string(crater_impact_coords) << std::endl; // Debugging

		float proximity = 10.0f;
		for (int i = 0; i < craterCounter; i++) {
			if ((crater_impact_coords.x - craters_coords_array[i].x == 0.0f) && (crater_impact_coords.y - craters_coords_array[i].y == 0.0f)) { // Fireball impact point is the same as before
			//if (((crater_impact_coords.x - craters_coords_array[i].x >= 0.0f) && (crater_impact_coords.x - craters_coords_array[i].x < proximity)) && ((crater_impact_coords.y - craters_coords_array[i].y >= 0.0f) && (crater_impact_coords.y - craters_coords_array[i].y < proximity))) {
			
				
				should_draw_2ndcraters_array[secondCraterCounter] = 1;
				second_craters_coords_array[secondCraterCounter] = crater_impact_coords;
				secondCraterCounter += 1;
				
				printf("hi - i is %d\n", i);
				std::cout << "crater_impact_coords:" + glm::to_string(crater_impact_coords) << std::endl; // Debugging
				std::cout << "craters_coords_array:" + glm::to_string(craters_coords_array[i]) << std::endl; // Debugging

				return true;
			}
		}

		should_draw_craters_array[craterCounter] = 1;
		craters_coords_array[craterCounter] = crater_impact_coords;
		craterCounter += 1;

		return true;
	}
	return false;
}

int main( void )
{
	// Initialize our arrays because they contain trash
	for (int i = 2; i < number_of_objects; i++) {
		should_draw_craters_array[i-2] = 0;
		craters_coords_array[i-2] = glm::vec3(0.0f, 0.0f, 0.0f);
	}
	for (int i = 0; i < number_of_big_craters; i++) {
		should_draw_2ndcraters_array[i] = 0;
		second_craters_coords_array[i] = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	// Initialize a seed
	srand(time(NULL));

	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(windowWidth, windowHeight, u8"Εργασία 1Γ – Καταστροφή", NULL, NULL);

	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, windowWidth/2, windowHeight/2);

	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	//GLuint programID = LoadShaders( "TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader" );
	GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader"); // From OpenGL tutorial 8

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Load the textures
	int width[number_of_objects], height[number_of_objects], nrChannels[number_of_objects];
	unsigned char* data[number_of_objects];
	// Load the ground obj text
	data[0] = stbi_load("textures/ground1.jpg", &width[0], &height[0], &nrChannels[0], 0);
	// Load the fireball obj text
	data[1] = stbi_load("textures/fire1.jpg", &width[1], &height[1], &nrChannels[1], 0);
	// Load the crater objs text 
	for (int i = 2; i < number_of_objects; i++) {
		data[i] = stbi_load("textures/crater1.jpg", &width[i], &height[i], &nrChannels[i], 0);
	}
	// Load the bigger crater objs text
	int width_bigC[number_of_big_craters], height_bigC[number_of_big_craters], nrChannels_bigC[number_of_big_craters];
	unsigned char* data_bigC[number_of_big_craters];
	for (int i = 0; i < number_of_big_craters; i++) {
		data_bigC[i] = stbi_load("textures/crater1.jpg", &width_bigC[i], &height_bigC[i], &nrChannels_bigC[i], 0);
	}

	if (data)
	{
	
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	if (data_bigC)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	std::vector<glm::vec3> vertices[number_of_objects];
	std::vector<glm::vec3> normals[number_of_objects];
	std::vector<glm::vec2> uvs[number_of_objects];

	std::vector<glm::vec3> vertices_bigC[number_of_big_craters];
	std::vector<glm::vec3> normals_bigC[number_of_big_craters];
	std::vector<glm::vec2> uvs_bigC[number_of_big_craters];
	

	// Read our .obj files
	bool resOBJS[number_of_objects];
	// Load the ground obj 
	resOBJS[0] = loadOBJ("objs/3dplane_445vert_810faces_100x100.obj", vertices[0], uvs[0], normals[0]);
	// Load the fireball obj 
	// Slightly bigger ball to not make our eyes hurt
	resOBJS[1] = loadOBJ("objs/BALLZ.obj", vertices[1], uvs[1], normals[1]);
	// Original ball
	// resOBJS[1] = loadOBJ("objs/ball.obj", vertices[1], uvs[1], normals[1]);
	// Load the crater objs 
	for (int i = 2; i < number_of_objects; i++) {
		resOBJS[i] = loadOBJ("objs/simple_crater.obj", vertices[i], uvs[i], normals[i]);
	}

	// Load the big crater objs 
	bool resOBJS_bigC[number_of_big_craters];
	for (int i = 0; i < number_of_big_craters; i++) {
		resOBJS[i] = loadOBJ("objs/simple_crater_2nd_impact.obj", vertices_bigC[i], uvs_bigC[i], normals_bigC[i]);
	}

	GLuint textureID[number_of_objects];
	GLuint uniformTextureID[number_of_objects]; // handle for uniforms
	GLuint vertexbuffer[number_of_objects];
	GLuint uvbuffer[number_of_objects];
	GLuint normalbuffer[number_of_objects];

	GLuint textureID_bigC[number_of_big_craters];
	GLuint uniformTextureID_bigC[number_of_big_craters]; // handle for uniforms
	GLuint vertexbuffer_bigC[number_of_big_craters];
	GLuint uvbuffer_bigC[number_of_big_craters];
	GLuint normalbuffer_bigC[number_of_big_craters];

	for (int i = 0; i < number_of_objects; i++) {

		glGenTextures(1, &textureID[i]);

		// "Bind" the newly created texture : all future texture functions will modify this texture
		glBindTexture(GL_TEXTURE_2D, textureID[i]);

		// Give the image to OpenGL
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width[i], height[i], 0, GL_RGB, GL_UNSIGNED_BYTE, data[i]);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Get a handle for our "myTextureSampler" uniform
		uniformTextureID[i] = glGetUniformLocation(programID, "myTextureSampler");

		// Load it into a VBO

		glGenBuffers(1, &vertexbuffer[i]);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, vertices[i].size() * sizeof(glm::vec3), &vertices[i][0], GL_STATIC_DRAW);

		glGenBuffers(1, &uvbuffer[i]);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, uvs[i].size() * sizeof(glm::vec2), &uvs[i][0], GL_STATIC_DRAW);

		glGenBuffers(1, &normalbuffer[i]);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, normals[i].size() * sizeof(glm::vec2), &normals[i][0], GL_STATIC_DRAW);
	}

	for (int i = 0; i < number_of_big_craters; i++) {

		glGenTextures(1, &textureID_bigC[i]);

		// "Bind" the newly created texture : all future texture functions will modify this texture
		glBindTexture(GL_TEXTURE_2D, textureID_bigC[i]);

		// Give the image to OpenGL
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_bigC[i], height_bigC[i], 0, GL_RGB, GL_UNSIGNED_BYTE, data_bigC[i]);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Get a handle for our "myTextureSampler" uniform
		uniformTextureID_bigC[i] = glGetUniformLocation(programID, "myTextureSampler");

		// Load it into a VBO

		glGenBuffers(1, &vertexbuffer_bigC[i]);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_bigC[i]);
		glBufferData(GL_ARRAY_BUFFER, vertices_bigC[i].size() * sizeof(glm::vec3), &vertices_bigC[i][0], GL_STATIC_DRAW);

		glGenBuffers(1, &uvbuffer_bigC[i]);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_bigC[i]);
		glBufferData(GL_ARRAY_BUFFER, uvs_bigC[i].size() * sizeof(glm::vec2), &uvs_bigC[i][0], GL_STATIC_DRAW);

		glGenBuffers(1, &normalbuffer_bigC[i]);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer_bigC[i]);
		glBufferData(GL_ARRAY_BUFFER, normals_bigC[i].size() * sizeof(glm::vec2), &normals_bigC[i][0], GL_STATIC_DRAW);
	}

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	glm::vec3 fireballPos = calculateFireballSpawnPoint();

	int should_launch_fireball = 0;		// Flag to launch fireball
	int dont_draw_fireball = 0;			// Flag to "destroy" fireball i.e not draw it

	glm::vec3 lightPosition = glm::vec3(0, 0, 100); // Let there be light at these coords
	
	do{

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glUniform3f(LightID, lightPosition.x, lightPosition.y, lightPosition.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);

		//glBindTexture(GL_TEXTURE_2D, textureID);
		glBindTexture(GL_TEXTURE_2D, textureID[0]);

		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(uniformTextureID[0], 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[0]);
		glVertexAttribPointer(
			1,                                // attribute uv
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[0]);
		glVertexAttribPointer(
			2,								// attribute normals
			3,								// size
			GL_FLOAT,						// type
			GL_FALSE,						// normalized?
			0,								// stride
			(void*)0);						// array buffer offset

		// Draw the plane
		glDrawArrays(GL_TRIANGLES, 0, vertices[0].size());

		// If flag is NOT raised then draw the fireball
		if (dont_draw_fireball != 1) { glDrawArrays(GL_TRIANGLES, 0, vertices[1].size()); }

		// Launch the fireball
		if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
			should_launch_fireball = 1;
			// Preload some coords for the fireball
			glm::vec3 fireballPos = calculateFireballSpawnPoint();
		}

		if (should_launch_fireball == 1) {
			glBindTexture(GL_TEXTURE_2D, textureID[1]);
			glUniform1i(uniformTextureID[1], 0);

			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
			glVertexAttribPointer(
				0,							// attribute
				3,							// size
				GL_FLOAT,					// type
				GL_FALSE,					// normalized?
				0,							// stride
				(void*)0);					// array buffer offset

			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[1]);
			glVertexAttribPointer(
				1,							// attribute uv
				2,							// size
				GL_FLOAT,					// type
				GL_FALSE,					// normalized?
				0,							// stride
				(void*)0);					// array buffer offset

			glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[1]);
			glVertexAttribPointer(
				2,							// attribute normals
				3,							// size
				GL_FLOAT,					// type
				GL_FALSE,					// normalized?
				0,							// stride
				(void*)0);					// array buffer offset

			glm::mat4 fireballModelMatrix = glm::translate(glm::mat4(1.0f), fireballPos);

			// Move z fireball coord by zOffset
			glm::mat4 newFireballModelMatrix =
				translate(glm::mat4(1.0f), -glm::vec3(0, 0, fireballPos.z * zOffset)) * fireballModelMatrix;

			// Calculate MVP
			MVP = ProjectionMatrix * ViewMatrix * newFireballModelMatrix;

			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &newFireballModelMatrix[0][0]);
			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

			glUniform3f(LightID, lightPosition.x, lightPosition.y, lightPosition.z);

			// Draw the fireball 
			glDrawArrays(GL_TRIANGLES, 0, vertices[1].size());
			checkForSpeedPresses(); // Adjust the fireball's speed at will
			printf("Fireball speed: %f\n", zOffset); // Debugging
			//printf("Fireball Z: %f\n", fireballPos.z); // Debugging
			fireballPos.z -= fireballPos.z * zOffset; // The fireball always comes closer to the ground plane
		}

		// Check if we have a collision
		if (dont_draw_fireball != 1 && checkForCollision(fireballPos, ground_plane, fireball_radius)) {
			// Play explosion sound
			sndPlaySound(TEXT("C4_explosion.wav"), SND_ASYNC);

			// Halt the launching of fireballs
			should_launch_fireball = 0;

			// Erase the fireball that collided
			dont_draw_fireball = 0; 

			// Initialize new coords for the fireball and reset the zOffset
			fireballPos = calculateFireballSpawnPoint();
			zOffset = zOffset_initial;
			
		}
		checkForCollision(fireballPos, ground_plane, fireball_radius); // check collision between fireball and ground plane
		
		// Draw craters dynamically only if they should be drawn, i.e the flag variable is raised
		for (int i = 2; i < number_of_objects; i++) {
			int craterID = i - 2;
			if (should_draw_craters_array[craterID] == 1) {

				// Draw the crater
				glBindTexture(GL_TEXTURE_2D, textureID[i]);
				glUniform1i(uniformTextureID[i], 0);

				glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[i]);
				glVertexAttribPointer(
					0,							// attribute
					3,							// size
					GL_FLOAT,					// type
					GL_FALSE,					// normalized?
					0,							// stride
					(void*)0);					// array buffer offset

				glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[i]);
				glVertexAttribPointer(
					1,							// attribute uv
					2,							// size
					GL_FLOAT,					// type
					GL_FALSE,					// normalized?
					0,							// stride
					(void*)0);					// array buffer offset

				glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[i]);
				glVertexAttribPointer(
					2,							// attribute normals
					3,							// size
					GL_FLOAT,					// type
					GL_FALSE,					// normalized?
					0,							// stride
					(void*)0);					// array buffer offset

				glm::mat4 craterModelMatrix = glm::mat4(1.0f);

				// Move the crater to the correct position
				glm::mat4 newCraterModelMatrix =
					translate(glm::mat4(1.0f), glm::vec3(craters_coords_array[craterID].x, craters_coords_array[craterID].y, craters_coords_array[craterID].z)) * craterModelMatrix;

				// Calculate MVP
				MVP = ProjectionMatrix * ViewMatrix * newCraterModelMatrix;

				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &newCraterModelMatrix[0][0]);
				glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

				// Draw the crater
				glDrawArrays(GL_TRIANGLES, 0, vertices[i].size());
			}
		}

		// Draw bigger craters dynamically only if they should be drawn, i.e the flag variable is raised
		for (int i = 0; i < number_of_big_craters; i++) {
			if (should_draw_2ndcraters_array[i] == 1) {

				// Draw the crater
				glBindTexture(GL_TEXTURE_2D, textureID_bigC[i]);
				glUniform1i(uniformTextureID_bigC[i], 0);

				glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_bigC[i]);
				glVertexAttribPointer(
					0,							// attribute
					3,							// size
					GL_FLOAT,					// type
					GL_FALSE,					// normalized?
					0,							// stride
					(void*)0);					// array buffer offset

				glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_bigC[i]);
				glVertexAttribPointer(
					1,							// attribute uv
					2,							// size
					GL_FLOAT,					// type
					GL_FALSE,					// normalized?
					0,							// stride
					(void*)0);					// array buffer offset

				glBindBuffer(GL_ARRAY_BUFFER, normalbuffer_bigC[i]);
				glVertexAttribPointer(
					2,							// attribute normals
					3,							// size
					GL_FLOAT,					// type
					GL_FALSE,					// normalized?
					0,							// stride
					(void*)0);					// array buffer offset

				glm::mat4 craterModelMatrix = glm::mat4(1.0f);

				// Move the crater to the correct position
				glm::mat4 newCraterModelMatrix =
					translate(glm::mat4(1.0f), glm::vec3(second_craters_coords_array[i].x, second_craters_coords_array[i].y, second_craters_coords_array[i].z)) * craterModelMatrix;

				// Calculate MVP
				MVP = ProjectionMatrix * ViewMatrix * newCraterModelMatrix;

				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &newCraterModelMatrix[0][0]);
				glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

				// Draw the crater
				glDrawArrays(GL_TRIANGLES, 0, vertices_bigC[i].size());
			}
		}
		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the SPACE key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_SPACE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 );

	// Cleanup VBOs and shaders
	for (int k = 0; k < number_of_objects; k++) {
		glDeleteBuffers(1, &vertexbuffer[k]);
		glDeleteBuffers(1, &uvbuffer[k]);
		glDeleteBuffers(1, &normalbuffer[k]);
		glDeleteTextures(1, &textureID[k]);
	}
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}