#include <glad/glad.h>
#include "glUtils.hpp"
#include <vector>

template <class T>
unsigned int generateAttribute(int id, int elementsPerEntry, std::vector<T> data, bool normalize) {
    unsigned int bufferID;
    glGenBuffers(1, &bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(id, elementsPerEntry, GL_FLOAT, normalize ? GL_TRUE : GL_FALSE, elementsPerEntry * sizeof(float), 0);
    glEnableVertexAttribArray(id);
    return bufferID;
}

unsigned int generateBuffer(Mesh &mesh) {
    unsigned int vaoID;
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    generateAttribute(0, 3, mesh.vertices, false);
    generateAttribute(1, 3, mesh.normals, true);

    if (mesh.textureCoordinates.size() > 0) {
        generateAttribute(2, 2, mesh.textureCoordinates, false);

        for ( int i=0; i<mesh.vertices.size(); i+=3){

          // Shortcuts for vertices
          glm::vec3 & v0 = mesh.vertices[i+0];
          glm::vec3 & v1 = mesh.vertices[i+1];
          glm::vec3 & v2 = mesh.vertices[i+2];

          // Shortcuts for UVs
          glm::vec2 & uv0 = mesh.textureCoordinates[i+0];
          glm::vec2 & uv1 = mesh.textureCoordinates[i+1];
          glm::vec2 & uv2 = mesh.textureCoordinates[i+2];
        }

    }

    unsigned int indexBufferID;
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

    return vaoID;
}
