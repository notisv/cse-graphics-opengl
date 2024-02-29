// Include GLFW
#include <GLFW/glfw3.h>

// The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp.
// This is a hack to keep the tutorials simple. Please avoid this.
extern GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

unsigned int cameraMode = 1;

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

// Initial camera position : on +Z
glm::vec3 cameraPosition = glm::vec3(50.0f, -60.0f, 65.0f);				// Camera coords in in world space
glm::vec3 cameraCenterPoint = glm::vec3(50.0f, 50.0f, 0.5f);			// Camera looks at (10.0, 10.0, -0.5)
glm::vec3 cameraZoomReferencePoint = glm::vec3(50.0f, 50.0f, 0.5f);	// Camera zooms in/out in relation to the center of the grid
float zoomingStep = 0.1f;

int differentCameraAngle = 0; // alternate camera view flag

glm::vec3 upVector = glm::vec3(0.0f, 0.0f, 1.0f);						// Camera is using an up vector of (0.0, 0.0, 1.0) - head looks at z axis
float initialFoV = 60.0f;												// Initial Field of View - WAS 45

float degree = glm::radians(0.0174532925f);								// 1 degree = 0.0174532925 radians
float rotationAngle = 10.0f * degree;									// Rotation "speed" is 10 degrees

glm::mat4 getViewMatrix() { return ViewMatrix; }
glm::mat4 getProjectionMatrix() { return ProjectionMatrix; }
glm::vec3 getCameraPosition() { return cameraPosition; }

// Rotation on x axis means that the coords y,z of the yz plane change while x coord stays the same
glm::mat3 rotationMatrixX = mat3(
	1, 0, 0,
	0, cos(rotationAngle), -sin(rotationAngle),
	0, sin(rotationAngle), cos(rotationAngle)
);

// Rotation on z axis means that the coords x,y of the xy plane change while z coord stays the same
glm::mat3 rotationMatrixZ = mat3(
	cos(rotationAngle), -sin(rotationAngle), 0,
	sin(rotationAngle), cos(rotationAngle), 0,
	0, 0, 1
);

void computeMatricesFromInputs() {

	glm::vec3 normalizedVector = glm::normalize((cameraPosition - cameraCenterPoint));
	// Zoom in
	if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) { cameraPosition -= zoomingStep * normalizedVector; }
	// Zoom out
	if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) { cameraPosition += zoomingStep * normalizedVector; }

	// Rotate counterclockwise in regards to x axis
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cameraPosition = rotationMatrixX * cameraPosition;
		upVector = rotationMatrixX * upVector;
	}
	// Rotate clockwise in regards to x axis
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		cameraPosition = cameraPosition * rotationMatrixX;
		upVector = upVector * rotationMatrixX;
	}

	// Rotate counterclockwise in regards to z axis
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cameraPosition = rotationMatrixZ * cameraPosition;
		upVector = rotationMatrixZ * upVector;
	}
	// Rotate clockwise in regards to z axis
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cameraPosition = cameraPosition * rotationMatrixZ;
		upVector = upVector * rotationMatrixZ;
	}

	// Change camera angles depending on what number row key is pressed

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		cameraPosition = cameraPosition;
		cameraCenterPoint = glm::vec3(55.0f, 55.0f, 0.5f);
	}

	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
		cameraPosition = cameraPosition;
		cameraCenterPoint = glm::vec3(0.0f, 0.0f, 0.5f);
	}

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel();

	// Projection matrix : 45° Field of View, 1:1 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 4.0f, 0.1f, 500.0f);

	// Camera matrix
	ViewMatrix = glm::lookAt(
		cameraPosition,		// Camera is here
		cameraCenterPoint,	// and looks here : at the same position
		upVector			// Camera is using the modified up vector
	);

	// Rotate on x axis means that the coords y,z of the yz plane change while x coord stays the same
	// Rotate on z axis means that the coords x,y of the xy plane change while z coord stays the same
	//printf("X: %f, Y: %f, Z: %f\n", cameraPosition.x, cameraPosition.y, cameraPosition.z); // Debugging
}