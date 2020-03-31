#include "noise.hpp"

// Generate texture with comp-shader

// XXX: Texture size seems to have a big performance impact
// Related to level of detail?
int tex_size = 128;
int tex_w = tex_size, tex_h = tex_size, tex_d = tex_size;

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
  int low_detail = 128;
  int med_detail = 256;
  int high_detail = 512;

  generatePoints(compShader, 0, low_detail, "low_detail_points");
  generatePoints(compShader, 1, med_detail, "med_detail_points");
  generatePoints(compShader, 2, high_detail, "high_detail_points");

  glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, (GLuint)tex_d);
  
  return tex_output;
}


void bindTexture(GLuint tex)
{
  // XXX: Should this be here?
  glMemoryBarrier(GL_ALL_BARRIER_BITS);
  glBindTextureUnit(0, tex);
}


void generatePoints(Gloom::Shader* shader, int idx, int num_points, std::string name)
{
  for (int i = 0; i < num_points; i++) {
    int x = rand() % tex_size;
    int y = rand() % tex_size;
    int z = rand() % tex_size;

    glUniform1i(idx, num_points);

    glUniform3fv(shader->getUniformFromName(fmt::format("{}[{}]", name, i)),
        1, glm::value_ptr(glm::vec3(x,y,z)));
  }
}
