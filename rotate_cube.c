// Request implementations
#define _GLAD_IMPLEMENTATION_
#define _GLFW_IMPLEMENTATION_
#define _GL_HELPERS_IMPLEMENTATION_
#define _VEC_MATH_IMPLEMENTATION_

// Detect OS
#define PLATFORM_WINDOWS 0
#define PLATFORM_LINUX   0
#define PLATFORM_MACOS   0

#if defined(_WIN32) || defined(_WIN64)
#undef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS 1
#elif defined(__linux__)
#undef PLATFORM_LINUX
#define PLATFORM_LINUX 1
#elif defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#undef PLATFORM_MACOS
#define PLATFORM_MACOS 1
#else
#error "Platform not recognized!"
#endif

// Specify windowing backend based on the OS
#if PLATFORM_LINUX
#define PLATFORM_NAME "Linux"
#define _GLFW_X11
#define _POSIX_C_SOURCE 199309L
#define GLFW_INCLUDE_NONE
#elif PLATFORM_MACOS
#define PLATFORM_NAME "OSX"
#define _GLFW_COCOA
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_NONE
#elif PLATFORM_WINDOWS
#define PLATFORM_NAME "Windows"
#define _GLFW_WIN32
#define GLFW_INCLUDE_NONE
#else
#error "Unknown platform!"
#endif

// Clib includes
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// Include libraries
#include "libs/glfw.h"
#include "libs/glad.h"
#include "libs/gl_helpers.h"
#include "libs/vec_math.h"
#include <cglm/cglm.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define GLH_MAT4_ROTATE_X(angle) { \
    1, 0, 0, 0, \
    0, cos(angle), -sin(angle), 0, \
    0, sin(angle), cos(angle), 0, \
    0, 0, 0, 1 \
}

void GLH_MAT4_LOOK_AT(vec3 eye, vec3 center, vec3 up, mat4 dest) {
    glm_lookat(eye, center, up, dest);
}

void GLH_MAT4_PERSPECTIVE(float fovy, float aspect, float near, float far, mat4 dest) {
    glm_perspective(glm_rad(fovy), aspect, near, far, dest);
}

// Basic datastructures
typedef struct SceneData {
    GLuint cube_vao;
    GLuint basic_program;
} SceneData;

// Cube vertex and index data
GLfloat cube_vertices[] = {
    // Positions        // Colors
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
};

GLuint cube_indices[] = {
    0, 1, 2, 2, 3, 0,  // Front face
    4, 5, 6, 6, 7, 4,  // Back face
    0, 1, 5, 5, 4, 0,  // Bottom face
    2, 3, 7, 7, 6, 2,  // Top face
    0, 3, 7, 7, 4, 0,  // Left face
    1, 2, 6, 6, 5, 1   // Right face
};

// Shaders
const char* cube_vrtx_shdr_src =
    GLH_SHADER_HEADER
    GLH_STRINGIFY(
    layout(location = 0) in vec3 position;
    layout(location = 1) in vec3 color;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    out vec3 v_color;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0);
        v_color = color;
    }
);

const char* basic_frag_shdr_src =
    GLH_SHADER_HEADER
    GLH_STRINGIFY(
    in vec3 v_color;

    out vec4 out_color;

    void main()
    {
        out_color = vec4(v_color, 1.0);
    }
);


// Initialize function - called once, sets up data for rendering
void init(SceneData* scene){

    // Initialize cube VAO
    GLuint cube_vao, cube_vbo, cube_ebo;
    glGenVertexArrays(1, &cube_vao);
    glGenBuffers(1, &cube_vbo);
    glGenBuffers(1, &cube_ebo);

    glBindVertexArray(cube_vao);

    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);

    // Create the program
    GLuint vrtx_shdr = glh_compile_shader_src(GL_VERTEX_SHADER, cube_vrtx_shdr_src);
    GLuint frag_shdr = glh_compile_shader_src(GL_FRAGMENT_SHADER, basic_frag_shdr_src);
    scene->basic_program = glh_link_program(vrtx_shdr, 0, frag_shdr);

    // Store cube VAO
    scene->cube_vao = cube_vao;
}

// Frame function - called on every frame, performs the rendering
void frame(SceneData* scene) {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.3f, 0.3f, 0.45f, 1.0f);

    vec3 eye = {0.0f, 0.0f, 3.0f};
    vec3 center = {0.0f, 0.0f, 0.0f};
    vec3 up = {0.0f, 1.0f, 0.0f};

    // Use the program
    glUseProgram(scene->basic_program);

    // Create rotation matrices
    float angle = (float)glfwGetTime() * 0.1f; // Rotate at 0.1 degrees per second
    mat4 model = GLH_MAT4_ROTATE_X(angle);
    
    mat4 view;
    GLH_MAT4_LOOK_AT(eye, center, up, view);
    
    mat4 projection;
    GLH_MAT4_PERSPECTIVE(45.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f, projection);

    GLuint model_loc = glGetUniformLocation(scene->basic_program, "model");
    GLuint view_loc = glGetUniformLocation(scene->basic_program, "view");
    GLuint proj_loc = glGetUniformLocation(scene->basic_program, "projection");

    glUniformMatrix4fv(model_loc, 1, GL_FALSE, (const GLfloat*)&model);
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, (const GLfloat*)&view);
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, (const GLfloat*)&projection);

    // Render the cube
    glBindVertexArray(scene->cube_vao);
    glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);   
}

int32_t main(int32_t argc, char** argv) {

    // Initialize GLFW
    if (!glfwInit()) {
        printf("Failed to initialize GLFW! Terminating\n");
        exit(EXIT_FAILURE);
    }

    // Create window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Rotating Cube", NULL, NULL);
    if (!window) {
        glfwTerminate();
        printf("Failed to create window! Terminating!\n");
        exit(EXIT_FAILURE);
    }

    // Initialize OpenGL
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "[ERROR] Failed to initialize OpenGL context!\n");
        return 1;
    }

    // Initialize scene
    SceneData scene = {0};
    init(&scene);

    // Run the rendering loop
    while (!glfwWindowShouldClose(window)) {
        frame(&scene);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &scene.cube_vao);
    glDeleteProgram(scene.basic_program);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
