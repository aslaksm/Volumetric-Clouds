// Local Headers
#include "glitter.hpp"
#include "shader.hpp"
#include "shapes.hpp"
#include "glUtils.hpp"
#include "program.hpp"
#include "noise.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <time.h>

int main(int argc, char * argv[]) {

    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(mWidth, mHeight, "OpenGL", nullptr, nullptr);

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

    GLuint texID = generateTexture();


    Gloom::Shader* shader;
    shader = new Gloom::Shader();
    shader->makeBasicShader("../Glitter/Shaders/shader.vert", "../Glitter/Shaders/shader.frag");
    shader->activate();

    // Mesh clouds = plane(glm::vec3(0.6, 0.6, 1.0));
    Mesh clouds = plane(glm::vec3(1.0, 1.0, 1.0));
    unsigned int cloudsVAO = generateBuffer(clouds);
    printGLError();

    float t = 0.0;

    bindTexture(texID);

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.35f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        printGLError();

        int windowWidth, windowHeight;
        glfwGetWindowSize(mWindow, &windowWidth, &windowHeight);
        glViewport(0, 0, windowWidth, windowHeight);

        glUniform2fv(4, 1, glm::value_ptr(glm::vec2(windowWidth, windowHeight)));

        glUniform1f(5, t);
        // t+= 1.00;
        t+= 0.001;

        glBindVertexArray(cloudsVAO);
        glDrawElements(GL_TRIANGLES, clouds.indices.size(), GL_UNSIGNED_INT, nullptr);

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);

        glfwPollEvents();
    }   glfwTerminate();
    return EXIT_SUCCESS;
}


