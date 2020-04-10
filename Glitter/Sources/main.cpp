// Local Headers
#include "glitter.hpp"
#include "shader.hpp"
#include "shapes.hpp"
#include "glUtils.hpp"
#include "program.hpp"
#include "noise.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fmt/format.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <iostream>

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


    // Setup and load IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
    ImGui_ImplOpenGL3_Init("#version 430 core");
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    glEnable(GL_BLEND);  
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    GLuint covID = generateCoverageAndHeightTexture();
    GLuint texID = generateTexture();
    // GLuint detID = generateDetailTexture();
    bindTexture(0, covID);
    bindTexture(1, texID);
    // bindTexture(2, detID);

    Gloom::Shader* bgShader;
    bgShader = new Gloom::Shader();
    bgShader->makeBasicShader("../Glitter/Shaders/shader.vert", "../Glitter/Shaders/bg.frag");
    // shader->activate();

    Gloom::Shader* shader;
    shader = new Gloom::Shader();
    shader->makeBasicShader("../Glitter/Shaders/shader.vert", "../Glitter/Shaders/shader.frag");
    // shader->activate();

    // Mesh clouds = plane(glm::vec3(0.6, 0.6, 1.0));
    Mesh clouds = plane(glm::vec3(1.0, 1.0, 1.0));
    unsigned int cloudsVAO = generateBuffer(clouds);
    glBindVertexArray(cloudsVAO);
    // printGLError();

    float t = 0.0;

    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
    float dist = 0.0;
    float look_x = 0.0;
    float look_y = 0.0;
    float step1 = 1.0;
    float step2 = 1.0;
    int num_steps = 5;

    float pass_time = false;


    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Set viewport and get resolution
        int windowWidth, windowHeight;
        glfwGetWindowSize(mWindow, &windowWidth, &windowHeight);
        glViewport(0, 0, windowWidth, windowHeight);

        // Background Color
        glClear(GL_COLOR_BUFFER_BIT);
        bgShader->activate();
        glUniform2fv(4, 1, glm::value_ptr(glm::vec2(windowWidth, windowHeight)));
        glDrawElements(GL_TRIANGLES, clouds.indices.size(), GL_UNSIGNED_INT, nullptr);

        // Render clouds
        shader->activate();

        glUniform2fv(4, 1, glm::value_ptr(glm::vec2(windowWidth, windowHeight)));
        glUniform1f(5, t);
        glUniform3fv(6, 1, glm::value_ptr(glm::vec3(x,y,z)));
        glUniform1f(7, dist);
        glUniform2fv(8, 1, glm::value_ptr(glm::vec2(look_x, look_y)));
        glUniform2fv(9, 1, glm::value_ptr(glm::vec2(step1, step2)));
        glUniform1i(10, num_steps);
        // t+= 0.01;
        glDrawElements(GL_TRIANGLES, clouds.indices.size(), GL_UNSIGNED_INT, nullptr);


        // Setup ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (pass_time) t += 0.01;
        // render GUI
        ImGui::Begin("Demo window");
        if (ImGui::Button("Time"))
        {
          pass_time = !pass_time;
        }

        if (ImGui::Button("Over"))
        {
          y = 100.0;
          look_y = -1.0;
        }

        if (ImGui::Button("Under"))
        {
          y = -100.0;
          look_y = 1.0;
        }

        ImGui::SliderFloat("X", &x, -1000.0f, 1000.0f);
        ImGui::SliderFloat("Y", &y, -1000.0f, 2000.0f);
        ImGui::SliderFloat("Z", &z, -1000.0f, 1000.0f);
        ImGui::SliderFloat("Distance", &dist, 0.0f, 1000.0f);
        ImGui::SliderFloat("Look x", &look_x, -1.0f, 1.0f);
        ImGui::SliderFloat("Look y", &look_y, -1.0f, 1.0f);
        ImGui::SliderFloat("Step 1", &step1, 0.0f, 3.0f);
        ImGui::SliderFloat("Step 2", &step2, 0.0f, 2.0f);
        ImGui::SliderInt("Num steps", &num_steps, 1, 100);
        ImGui::End();

        // Render imgui into screen
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);

        glfwPollEvents();
    }   glfwTerminate();
    return EXIT_SUCCESS;
}


