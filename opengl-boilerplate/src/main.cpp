#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Link statically with GLEW
#define GLEW_STATIC

struct ShaderProgramSource
{
    std::string vertexSource;
    std::string fragmentSource;
};

ShaderProgramSource parseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);
    
    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };
    
    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    
    while (getline(stream, line))
    {
        if (line.find("shader") != std::string::npos)
        {
            if (line.find("#Vertex") != std::string::npos)
            {
                type = ShaderType::VERTEX;
            } else if (line.find("#Fragment") != std::string::npos)
            {
                type = ShaderType::FRAGMENT;
            }
        } else {
            ss[(int)type] << line << '\n';
        }
    }

    return { ss[0].str(), ss[1].str() };
}

GLuint compileShader(GLenum type, const std::string& source)
{
    GLuint id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);
    
    GLint status;
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout <<"Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader"<< std::endl;
        std::cout << message << std::endl;
    }
    
    return id;
}

int main(void)
{
    /* Initialize GLFW */
    if (!glfwInit())
        return -1;
    
    // Use OpenGL 4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    // Should be true for macOS, according to GLFW docs, to get core profile.
    // https://stackoverflow.com/a/46552181
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // According to Apple docs, non-core profiles are limited to version 2.1.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Boilerplate", NULL, NULL);
    
    // Fullscreen
    // GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", glfwGetPrimaryMonitor(), NULL);
    // Escape key to easily return to the desktop.
    // if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    //     glfwSetWindowShouldClose(window, GL_TRUE);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    /* the OpenGL context has to be made active */
    glfwMakeContextCurrent(window);
    
    fprintf(stdout, "OpenGL version %s\n", glGetString(GL_VERSION));
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
      /* Problem: glewInit failed. */
      fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    fprintf(stdout, "Using GLEW %s\n", glewGetString(GLEW_VERSION));
    
    // Create a Vertex Array Object (VAO)
    // VAOs store all of the links between the attributes and your VBOs with raw vertex data.
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    // Create a Vertex Buffer Object (VBO)
    GLuint vbo;
    glGenBuffers(1, &vbo); // Generate 1 buffer
    GLfloat vertices[] = {
        -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // Top-left, Vertex 1 (X, Y, Red)
         0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // Top-right, Vertex 2 (X, Y, Green)
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Bottom-right Vertex 3 (X, Y, Blue)
        -0.5f, -0.5f, 1.0f, 1.0f, 1.0f  // Bottom-left, Vertex 4 (X, Y, White)
    };
    glBindBuffer(GL_ARRAY_BUFFER, vbo); // make vbo the active array buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // copy vertex data to graphics card
    
    // Create an element array
    GLuint ebo;
    glGenBuffers(1, &ebo);
    // Using an element buffer allows you to reuse data
    // Some vertices are used from both triangles to draw the rectangle
    GLuint elements[] = {
        0, 1, 2,
        2, 3, 0
    };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
    
    // Copy files build phase in Xcode project settings.
    // When Xcode builds the project, the input files (shaders) will be copied to the same folder as the executable.
    // https://stackoverflow.com/a/23449331
    ShaderProgramSource source = parseShader("basic.shader.glsl");
    
    // Create program
    GLuint shaderProgram = glCreateProgram();
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, source.vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, source.fragmentSource);
    
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Just like a vertex buffer, only one program can be active at a time.
    glUseProgram(shaderProgram);
    
    // Retrieve a reference to the position input in the vertex shader.
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    // With the reference to the input, you can specify how the data
    // for that input is retrieved from the array.
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
    // Vertex attribute array needs to be enabled.
    glEnableVertexAttribArray(posAttrib);
    
    GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));
    
    // Get the location of the color uniform.
    GLint uniColor = glGetUniformLocation(shaderProgram, "uniColor");

    auto t_start = std::chrono::high_resolution_clock::now();
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
        glUniform3f(uniColor, (sin(time * 4.0f) + 3.0f) / 4.0f, 0.0f, 0.0f);
        
        // Clear color
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Draw your scene here
        // Draw a triangle from the 3 vertices
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        
        // The only real difference is that you're talking about indices instead of vertices now
        // Allow to reuse vertices, element buffers will be an important area of optimization for bigger applications
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    
    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    glDeleteBuffers(1, &vbo);

    glDeleteVertexArrays(1, &vao);

    glfwTerminate();
    return 0;
}
