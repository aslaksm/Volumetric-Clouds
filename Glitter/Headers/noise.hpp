#pragma once
// Local Headers
#include "glitter.hpp"
#include "shader.hpp"
#include "shapes.hpp"
#include "glUtils.hpp"
#include "program.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <iostream>
#include <ctime>

GLuint generateCoverageAndHeightTexture();
GLuint generateTexture();
GLuint generateDetailTexture();
void bindTexture(int idx, GLuint tex);
void generatePoints(Gloom::Shader* shader, int idx, int num_points, std::string name);
