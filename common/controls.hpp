#ifndef CONTROLS_HPP
#define CONTROLS_HPP

void computeMatricesFromInputs(int width = 1024, int height = 768);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

#endif