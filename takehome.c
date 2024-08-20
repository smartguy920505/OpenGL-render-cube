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

typedef struct MeshData {
    int32_t vertex_count;
    int32_t triangle_count;
    float* vertex_data; // position (3 floats), normals (3 floats)
    uint32_t* triangles; // 3 x triangle_count

    // Vertex Layout info
    int32_t vertex_size;
    int32_t positions_size;
    int32_t positions_offset;
    int32_t normals_size;
    int32_t normals_offset;
} MeshData;

// Forward declare loading code
int32_t load_mesh_data(const char* filename, MeshData* out_data); 

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
void init(SceneData* scene, MeshData* mesh_data){

    // Initialize VAO, VBO, EBO
    glGenVertexArrays(1, &scene->cube_vao);
    GLuint vbo, ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(scene->cube_vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_data->vertex_count * mesh_data->vertex_size, mesh_data->vertex_data, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_data->triangle_count * 3 * sizeof(uint32_t), mesh_data->triangles, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, mesh_data->vertex_size, (void*)mesh_data->positions_offset);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, mesh_data->vertex_size, (void*)mesh_data->normals_offset);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Create the program
    GLuint vrtx_shdr = glh_compile_shader_src(GL_VERTEX_SHADER, cube_vrtx_shdr_src);
    GLuint frag_shdr = glh_compile_shader_src(GL_FRAGMENT_SHADER, basic_frag_shdr_src);
    scene->basic_program = glh_link_program(vrtx_shdr, 0, frag_shdr);
}

// Frame function - called on every frame, performs the rendering
void frame(SceneData* scene, MeshData* mesh_data) {
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

    // Render the mesh
    glBindVertexArray(scene->cube_vao);
    glDrawElements(GL_TRIANGLES, mesh_data->triangle_count * 3, GL_UNSIGNED_INT, 0);
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

    // Read mesh data 
    MeshData mesh_data = {0};
    if (!load_mesh_data("data/armadillo.bin", &mesh_data)){
        printf("Loaded the mesh with %d vertices and %d triangles!\n", mesh_data.vertex_count, mesh_data.triangle_count);
        printf("Vertex Layout: %d bytes per vertex\n", mesh_data.vertex_size);
        printf("  Position Size: %d bytes | Offset: %d bytes\n", mesh_data.positions_size, mesh_data.positions_offset);
        printf("  Normal Size:   %d bytes | Offset: %d bytes\n", mesh_data.normals_size, mesh_data.normals_offset);
    }

    // Initialize scene
    SceneData scene = {0};
    init(&scene, &mesh_data);

    // Run the rendering loop
    while (!glfwWindowShouldClose(window)) {
        frame(&scene, &mesh_data);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &scene.cube_vao);
    glDeleteProgram(scene.basic_program);
    free(mesh_data.vertex_data);
    free(mesh_data.triangles);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// Implementation of data loading, out of the way
int32_t load_mesh_data(const char* filename, MeshData* out_data) {
    FILE *file = fopen(filename, "rb");
    
    if (!file) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    if (fread(&out_data->vertex_count, sizeof(uint32_t), 1, file) != 1) {
        perror("Failed to read vertex count");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Read triangle count (1 byte)
    if (fread(&out_data->triangle_count, sizeof(uint32_t), 1, file) != 1) {
        perror("Failed to read triangle count");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Allocate memory for vertex data 
    out_data->vertex_size = 6 * sizeof(float);
    size_t vertex_data_size = out_data->vertex_count * out_data->vertex_size;
    out_data->vertex_data = (float *)malloc(vertex_data_size);
    if (!out_data->vertex_data) {
        perror("Failed to allocate memory for vertices");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Read vertex data
    if (fread(out_data->vertex_data, 1, vertex_data_size, file) != vertex_data_size) {
        perror("Failed to read vertex data");
        free(out_data->vertex_data);
        fclose(file);
        return EXIT_FAILURE;
    }

    // Allocate memory for triangle data 
    size_t triangle_data_size = out_data->triangle_count * 3 * sizeof(uint32_t);
    out_data->triangles = (uint32_t *)malloc(triangle_data_size);
    if (!out_data->triangles) {
        perror("Failed to allocate memory for triangles");
        free(out_data->vertex_data);
        fclose(file);
        return EXIT_FAILURE;
    }

    // Read triangle data
    if (fread(out_data->triangles, 1, triangle_data_size, file) != triangle_data_size) {
        perror("Failed to read triangle data");
        free(out_data->vertex_data);
        free(out_data->triangles);
        fclose(file);
        return EXIT_FAILURE;
    }

    // Close the file
    fclose(file);
    out_data->positions_size = 3 * sizeof(float);
    out_data->positions_offset = 0;
    out_data->normals_size = 3 * sizeof(float);
    out_data->normals_offset = 3 * sizeof(float);
    return 0;
}

