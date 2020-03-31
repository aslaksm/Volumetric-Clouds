#include "shapes.hpp"

Mesh plane(glm::vec3 scale) {
    glm::vec3 points[4];
    int indices[6] = {0, 1, 2, 1, 3, 2};

    for (int y = 0; y <= 1; y++)
    for (int x = 0; x <= 1; x++) {
        points[x+y*2] = glm::vec3(x*2-1, y*2-1, 1) * scale;
    }

    Mesh m;
    for (int i = 0; i < 6; i++) {
      m.vertices.push_back(points[indices[i]]);
      m.indices.push_back(i);
      m.normals.push_back(glm::vec3(0,0,1));
    }

    return m;
}
