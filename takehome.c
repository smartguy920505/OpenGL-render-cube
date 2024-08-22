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

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Basic datastructures
typedef struct SceneData {
    GLuint cube_vao;
    GLuint basic_program;
    GLuint model_vao;
    GLuint model_program;
    GLuint framebuffer;
    GLuint texture;
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

float cube_vertices[] = {
		// positions          // normals           // texture coords
		// Back face
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		// Front face
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

		// Left face
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		// Right face
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		// Bottom face
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,

		// Top face
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
	};

// Shaders for cube
const char* cube_vrtx_shdr_src =
    GLH_SHADER_HEADER
    GLH_STRINGIFY(
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexCoords;

    out vec3 FragPos;
    out vec3 Normal;
    out vec2 TexCoords;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        TexCoords = aTexCoords;
        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
);

const char* cube_frag_shdr_src =
    GLH_SHADER_HEADER
    GLH_STRINGIFY(
    out vec4 FragColor;

    in vec3 FragPos;
    in vec3 Normal;
    in vec2 TexCoords;

    uniform sampler2D simple_texture;

    void main()
    {
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(vec3(1, 1.0, 1) - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
        vec3 ambient = vec3(1.0, 1.0, 1.0);
        
        vec4 texColor = texture(simple_texture, TexCoords);
        vec4 result = texColor * vec4((ambient + diffuse), 0.8);
        FragColor = result;
    }
);

// Shaders for model
const char* model_vrtx_shdr_src =
    GLH_SHADER_HEADER
    GLH_STRINGIFY(

    layout(location = 0) in vec3 aPos;   // Position attribute
    layout(location = 1) in vec3 aNormal; // Normal attribute

    out vec3 FragPos;  // Position of the fragment in world space
    out vec3 Normal;   // Normal of the fragment

    uniform mat4 model;       // Model matrix
    uniform mat4 view;        // View matrix
    uniform mat4 projection;  // Projection matrix

    void main()
    {
        // Calculate the position of the fragment in world space
        FragPos = vec3(model * vec4(aPos, 1.0));

        // Calculate the normal for this fragment
        Normal = mat3(transpose(inverse(model))) * aNormal;  

        // Final position of the vertex in screen space
        gl_Position = projection * view * vec4(FragPos, 0.5); // zoom
    }
);

const char* model_frag_shdr_src =
    GLH_SHADER_HEADER
    GLH_STRINGIFY(

    out vec4 FragColor;

    in vec3 FragPos;
    in vec3 Normal;
    in vec2 TexCoord;

    uniform vec3 lightPos;
    uniform vec3 viewPos;
    uniform vec3 lightColor;
    uniform vec3 objectColor;
    uniform float roughness;
    uniform float metalness;

    void main()
    {
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 lightDir = normalize(lightPos - FragPos);
        vec3 halfDir = normalize(viewDir + lightDir);

        // Ambient
        vec3 ambient = 0.1 * lightColor;

        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // Specular (using Blinn-Phong model)
        float spec = pow(max(dot(norm, halfDir), 0.0), 32.0);
        vec3 specular = spec * lightColor;

        // Apply roughness and metalness (Simple PBR approximation)
        vec3 albedo = objectColor * (1.0 - metalness);
        vec3 ambientColor = ambient * albedo;
        vec3 diffuseColor = diffuse * albedo;
        vec3 specularColor = specular * metalness;

        // Final color
        vec3 result = ambientColor + diffuseColor + specularColor;
        FragColor = vec4(result, 1.0);
    }
);

// Implementation of data loading, out of the way
int32_t load_mesh_data(const char* filename, MeshData* out_data);

// Initialize function - called once, sets up data for rendering
void init_cube(SceneData* scene){
    // Initialize cube VAO
    GLuint cube_vbo;
    glGenVertexArrays(1, &scene->cube_vao);
    glGenBuffers(1, &cube_vbo);

    glBindVertexArray(scene->cube_vao);

    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    // Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// Texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);

    // Create the program
    GLuint vrtx_shdr = glh_compile_shader_src(GL_VERTEX_SHADER, cube_vrtx_shdr_src);
    GLuint frag_shdr = glh_compile_shader_src(GL_FRAGMENT_SHADER, cube_frag_shdr_src);
    scene->basic_program = glh_link_program(vrtx_shdr, 0, frag_shdr);
}

void init_model(SceneData* scene, MeshData* mesh_data) {
    // Initialize VAO, VBO, EBO
    glGenVertexArrays(1, &scene->model_vao);
    GLuint vbo, ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(scene->model_vao);

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
    GLuint vrtx_shdr = glh_compile_shader_src(GL_VERTEX_SHADER, model_vrtx_shdr_src);
    GLuint frag_shdr = glh_compile_shader_src(GL_FRAGMENT_SHADER, model_frag_shdr_src);
    scene->model_program = glh_link_program(vrtx_shdr, 0, frag_shdr);
}

// Frame function - called on every frame, performs the rendering
void frame(SceneData* scene, MeshData* mesh_data) {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.3f, 0.3f, 0.45f, 1.0f);

    vec3_t eye = vec3(0.0f, 0.0f, 3.0f);
    vec3_t center = vec3(0.0f, 0.0f, 0.0f);
    vec3_t up = vec3(0.0f, 1.0f, 0.0f);

    // Use the program
    glUseProgram(scene->basic_program);

    /// Create rotation matrices
    float angle = (float)glfwGetTime() * 0.15f; // Rotate at 0.1 radians per second
    vec3_t axis = vec3(0.7071068f, 0.7071068f, 0.0f); // Normalized axis
    mat4_t model = mat4_make_rotation(axis, angle);
    mat4_t view = look_at(eye, center, up);
    mat4_t projection = perspective(deg2rad(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);

    GLuint model_loc = glGetUniformLocation(scene->basic_program, "model");
    GLuint view_loc = glGetUniformLocation(scene->basic_program, "view");
    GLuint proj_loc = glGetUniformLocation(scene->basic_program, "projection");
    GLuint textureLoc = glGetUniformLocation(scene->basic_program, "simple_texture");

    glUniformMatrix4fv(model_loc, 1, GL_FALSE, (const GLfloat*)&model);
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, (const GLfloat*)&view);
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, (const GLfloat*)&projection);
    glUniform1i(textureLoc, 0); // Texture unit 0

    // Render the cube
    glBindVertexArray(scene->cube_vao);
    glBindTexture(GL_TEXTURE_2D, scene->texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

void set_texture(SceneData* scene) {
    // Set up light properties
    vec3_t lightPos = vec3(0.0f, 1.0f, 2.0f);
    vec3_t lightColor = vec3(1.0f, 1.0f, 1.0f);
    vec3_t objectColor = vec3(0.6f, 1.0f, 0.3f);
    float ambientStrength = 0.1f;
    float roughness = 0.5f;  // Example roughness
    float metalness = 0.5f;  // Example metalness
    
    // Pass light properties to the fragment shader
    glUniform3fv(glGetUniformLocation(scene->model_program, "lightPos"), 1, (const GLfloat*)&lightPos);
    glUniform3fv(glGetUniformLocation(scene->model_program, "lightColor"), 1, (const GLfloat*)&lightColor);
    glUniform3fv(glGetUniformLocation(scene->model_program, "objectColor"), 1, (const GLfloat*)&objectColor);
    glUniform1f(glGetUniformLocation(scene->model_program, "ambientStrength"), ambientStrength);
    glUniform1f(glGetUniformLocation(scene->model_program, "metalness"), roughness);
    glUniform1f(glGetUniformLocation(scene->model_program, "roughness"), metalness);
}

void init_texture(SceneData* scene, MeshData* mesh) {
    glGenFramebuffers(1, &scene->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, scene->framebuffer);

    glGenTextures(1, &scene->texture);
    glBindTexture(GL_TEXTURE_2D, scene->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene->texture, 0);

    // Create a depth buffer
    GLuint depth_buffer;
    glGenRenderbuffers(1, &depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Framebuffer is not complete!\n");
    }
    
    // Set the viewport to the size of the texture
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Clear the framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render the model
    vec3_t eye = vec3(0.0f, 0.0f, 3.0f);
    vec3_t center = vec3(0.0f, 0.0f, 0.0f);
    vec3_t up = vec3(0.0f, 1.0f, 0.0f);

    // Use the program
    glUseProgram(scene->model_program);

    /// Create rotation matrices
    float angle = (float)glfwGetTime() * 0.5f; // Rotate at 0.1 radians per second
    vec3_t axis = vec3(0.7071068f, 0.7071068f, 0.0f); // Normalized axis
    mat4_t model = mat4_make_rotation(axis, angle);
    mat4_t view = look_at(eye, center, up);
    mat4_t projection = perspective(deg2rad(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);

    GLuint model_loc = glGetUniformLocation(scene->model_program, "model");
    GLuint view_loc = glGetUniformLocation(scene->model_program, "view");
    GLuint proj_loc = glGetUniformLocation(scene->model_program, "projection");

    glUniformMatrix4fv(model_loc, 1, GL_FALSE, (const GLfloat*)&model);
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, (const GLfloat*)&view);
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, (const GLfloat*)&projection);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glUseProgram(scene->model_program);

    set_texture(scene);

    glBindVertexArray(scene->model_vao);
    glDrawElements(GL_TRIANGLES, mesh->triangle_count * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Unbind the FBO to render to the default framebuffer (the screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Optionally disable face culling if needed for other objects
}

void render_model(SceneData* scene, MeshData* mesh) {
    glBindFramebuffer(GL_FRAMEBUFFER, scene->framebuffer);
    // Attach the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene->texture, 0);
    // Clear the framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    // Render the model

    vec3_t eye = vec3(0.0f, 0.0f, 3.0f);
    vec3_t center = vec3(0.0f, 0.0f, 0.0f);
    vec3_t up = vec3(0.0f, 1.0f, 0.0f);

    // Use the program
    glUseProgram(scene->model_program);

    /// Create rotation matrices
    float angle = (float)glfwGetTime() * 0.5f; // Rotate at 0.1 radians per second
    vec3_t axis = vec3(1.0f, 0.0f, 0.0f); // Normalized axis

    mat4_t model = mat4_make_rotation(axis, angle);
    
    mat4_t view = look_at(eye, center, up);
    
    mat4_t projection = perspective(deg2rad(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);

    GLuint model_loc = glGetUniformLocation(scene->model_program, "model");
    GLuint view_loc = glGetUniformLocation(scene->model_program, "view");
    GLuint proj_loc = glGetUniformLocation(scene->model_program, "projection");

    glUniformMatrix4fv(model_loc, 1, GL_FALSE, (const GLfloat*)&model);
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, (const GLfloat*)&view);
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, (const GLfloat*)&projection);

    set_texture(scene);

    glBindVertexArray(scene->model_vao);
    glDrawElements(GL_TRIANGLES, mesh->triangle_count * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    // Unbind the FBO to render to the default framebuffer (the screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
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
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Yembo", NULL, NULL);
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
    // Enable depth testing for proper 3D rendering
    glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LESS);

    // Enable back-face culling
    // glEnable(GL_CULL_FACE);   // Enable face culling
    // glCullFace(GL_BACK);      // Cull back faces
    // glFrontFace(GL_COW);      // Use counter-clockwise winding order for front faces

    // Read mesh data 
    MeshData mesh = {0};
    if (!load_mesh_data("data/armadillo.bin", &mesh)){
        printf("Loaded the mesh with %d vertices and %d triangles!\n", mesh.vertex_count, mesh.triangle_count);
        printf("Vertex Layout: %d bytes per vertex\n", mesh.vertex_size);
        printf("  Position Size: %d bytes | Offset: %d bytes\n", mesh.positions_size, mesh.positions_offset);
        printf("  Normal Size:   %d bytes | Offset: %d bytes\n", mesh.normals_size, mesh.normals_offset);
    }

    // Initialize scene
    SceneData scene = {0};
    init_cube(&scene);
    init_model(&scene, &mesh);
    init_texture(&scene, &mesh);
    
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    // Run the rendering loop
    while (!glfwWindowShouldClose(window)) {
        render_model(&scene, &mesh);
        frame(&scene, &mesh);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &scene.cube_vao);
    glDeleteVertexArrays(1, &scene.model_vao);
    glDeleteProgram(scene.basic_program);
    glDeleteProgram(scene.model_program);
    free(mesh.vertex_data);
    free(mesh.triangles);
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