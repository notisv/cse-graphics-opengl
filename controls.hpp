#ifndef CONTROLS_HPP
#define CONTROLS_HPP

#define windowWidth 1000
#define windowHeight 1000

void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
glm::vec3 getCameraPosition();

#endif