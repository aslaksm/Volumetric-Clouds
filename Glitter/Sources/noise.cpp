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


// Generate texture with comp-shader

// int tex_w = 512, tex_h = 512, tex_d = 512;
// int tex_w = 128, tex_h = 128, tex_d = 128;
int tex_w = 256, tex_h = 256, tex_d = 256;

GLuint generateTexture()
{


  GLuint tex_output;
  glGenTextures(1, &tex_output);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, tex_output);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, tex_w, tex_h, tex_d, 0, GL_RGBA, GL_FLOAT,
   NULL);
  glBindImageTexture(0, tex_output, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

  Gloom::Shader* compShader;
  compShader = new Gloom::Shader();
  compShader->attach("../Glitter/Shaders/noise.comp");
  compShader->link();
  compShader->activate();

  srand(time(NULL));
  for (int i = 0; i < 80; i++) {
    int x = rand() % 256;
    int y = rand() % 256;
    int z = rand() % 256;

    std::cout << fmt::format("x y z are {} {} {}", x,y,z) << std::endl;
    glUniform3fv(i, 1, glm::value_ptr(glm::vec3(x,y,z)));
  }

  glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, (GLuint)tex_d);
  
  return tex_output;
}


void bindTexture(GLuint tex)
{
  // XXX: Should this be here?
  glMemoryBarrier(GL_ALL_BARRIER_BITS);
  glBindTextureUnit(0, tex);
}
