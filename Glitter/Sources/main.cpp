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


double mouseSensitivity = 1.0;
double lastMouseX = mWidth / 2;
double lastMouseY = mHeight / 2;
float X = 0.0;
float Y = 0.0;
void mouseCallback(GLFWwindow* window, double x, double y) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    double deltaX = x - lastMouseX;
    double deltaY = y - lastMouseY;
    X += deltaX/1000;
    Y += deltaY/1000;

    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
}





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
    GLuint detID = generateDetailTexture();
    bindTexture(0, covID);
    bindTexture(1, texID);
    bindTexture(2, detID);

    // FIXME: We don't need a separate shader for this, unless we're doing atmospheric stuff
    Gloom::Shader* bgShader;
    bgShader = new Gloom::Shader();
    bgShader->makeBasicShader("../Glitter/Shaders/shader.vert", "../Glitter/Shaders/bg.frag");

    Gloom::Shader* shader;
    shader = new Gloom::Shader();
    shader->makeBasicShader("../Glitter/Shaders/shader.vert", "../Glitter/Shaders/shader.frag");

    Mesh clouds = plane(glm::vec3(1.0, 1.0, 1.0));
    unsigned int cloudsVAO = generateBuffer(clouds);
    glBindVertexArray(cloudsVAO);
    // printGLError();

    float t = 0.0;

    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
    float height = 10000.0;
    bool coverage_only = false;
    float look_y = 0.0;
    float trans = 1.0;
    float scat = 0.11;
    int num_steps = 20;
    float light_step_size = 100.f;
    
    float scale_cov = 3.0;
    float scale_base = 3.0;
    float scale_detail = 3.0;

    float pass_time = false;

    bool menu_mode = false;


    glfwSetCursorPosCallback(mWindow, mouseCallback);
    glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);


    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        if (glfwGetKey(mWindow, GLFW_KEY_M) == GLFW_PRESS)
            menu_mode = !menu_mode;
        if (glfwGetKey(mWindow, GLFW_KEY_E) == GLFW_PRESS)
            y += 100;
        if (glfwGetKey(mWindow, GLFW_KEY_Q) == GLFW_PRESS)
            y -= 100;
        if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS)
            x -= 100;
        if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS)
            x += 100;
        if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
            z += 100;
        if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
            z -= 100;

        if (menu_mode)
        {
          glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
          glfwSetCursorPosCallback(mWindow, NULL);
        }
        else
        {
          glfwSetCursorPosCallback(mWindow, mouseCallback);
          glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }


        // Set viewport and get resolution
        int windowWidth, windowHeight;
        glfwGetWindowSize(mWindow, &windowWidth, &windowHeight);
        glViewport(0, 0, windowWidth, windowHeight);

        lastMouseX = windowWidth / 2;
        lastMouseY = windowHeight / 2;


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
        glUniform1f(7, height);
        glUniform1i(8, coverage_only);
        glUniform1f(9, trans);
        glUniform1f(10, scat);
        glUniform1i(11, num_steps);
        glUniform1f(12, light_step_size);
        glUniform3fv(13, 1, glm::value_ptr(glm::vec3(scale_cov, scale_base, scale_detail)));
        glUniform2fv(14, 1, glm::value_ptr(glm::vec2(X, Y)));

        glDrawElements(GL_TRIANGLES, clouds.indices.size(), GL_UNSIGNED_INT, nullptr);


        // Setup ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (pass_time) t += 0.01;
        // Define GUI
        ImGui::Begin("Demo window");
        if (ImGui::Button("Time"))
        {
          pass_time = !pass_time;
        }

        ImGui::SliderFloat("X", &x, -1000.0f, 1000.0f);
        ImGui::SliderFloat("Y", &y, -100000.0f, 10000.0f);
        ImGui::SliderFloat("Z", &z, -1000.0f, 1000.0f);
        ImGui::SliderFloat("Scale coverage", &scale_cov, 0.01f, 10.0f);
        ImGui::SliderFloat("Scale base", &scale_base, 0.01f, 10.0f);
        ImGui::SliderFloat("Scale detail", &scale_detail, 0.01f, 10.0f);
        ImGui::SliderFloat("Max Height", &height, 0.0f, 10000.0f);
        ImGui::Checkbox("Coverage only", &coverage_only);
        ImGui::SliderFloat("Transmittance", &trans, 0.0f, 10.0f);
        ImGui::SliderFloat("Scattering", &scat, 0.0f, 3.0f);
        ImGui::SliderInt("Num steps", &num_steps, 1, 100);
        ImGui::SliderFloat("Lightmarch step size", &light_step_size, 5.f, 500.f);
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


