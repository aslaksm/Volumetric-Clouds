#include "noise.hpp"

// Generate textures with comp-shader

int tex_size = 96;
int detail_size = 32;
int tex_w = tex_size, tex_h = tex_size, tex_d = tex_size;

// 128^2 RGBA coverage texture
GLuint generateCoverageAndHeightTexture()
{
  GLuint tex_output;
  glGenTextures(1, &tex_output);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_output);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
  glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

  Gloom::Shader* compShader;
  compShader = new Gloom::Shader();
  compShader->attach("../Glitter/Shaders/coverage.comp");
  compShader->link();
  compShader->activate();

  glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, (GLuint)1);

  return tex_output;

}

// 128^3 RGBA main texure
GLuint generateTexture()
{
  GLuint tex_output;
  glGenTextures(1, &tex_output);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_3D, tex_output);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, tex_w, tex_h, tex_d, 0, GL_RGBA, GL_FLOAT,
      NULL);
  glBindImageTexture(1, tex_output, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

  Gloom::Shader* compShader;
  compShader = new Gloom::Shader();
  compShader->attach("../Glitter/Shaders/noise.comp");
  compShader->link();
  compShader->activate();

  glUniform3fv(0, 1, glm::value_ptr(glm::vec3(tex_size)));

  glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, (GLuint)tex_d);

  return tex_output;
}

// 32^3 RGB(A) texture of FBM Worleys
GLuint generateDetailTexture()
{
  GLuint tex_output;
  glGenTextures(1, &tex_output);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_3D, tex_output);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, detail_size, detail_size, detail_size, 0, GL_RGBA, GL_FLOAT,
      NULL);
  glBindImageTexture(2, tex_output, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

  Gloom::Shader* compShader;
  compShader = new Gloom::Shader();
  compShader->attach("../Glitter/Shaders/high_res.comp");
  compShader->link();
  compShader->activate();

  glUniform3fv(0, 1, glm::value_ptr(glm::vec3(detail_size)));

  glDispatchCompute((GLuint)detail_size, (GLuint)detail_size, (GLuint)detail_size);

  return tex_output;
}


void bindTexture(int idx, GLuint tex)
{
  glMemoryBarrier(GL_ALL_BARRIER_BITS);
  glBindTextureUnit(idx, tex);
}
