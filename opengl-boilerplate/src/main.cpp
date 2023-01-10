#include <iostream>
#include <chrono>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Link statically with GLEW
#define GLEW_STATIC

// Shader sources
const GLchar* vertexSource = R"glsl(
    #version 410 core

    in vec2 position;
    in vec3 color;

    out vec3 Color;

    void main()
    {
        Color = color;
        gl_Position = vec4(position, 0.0, 1.0);
    }
)glsl";
const GLchar* fragmentSource = R"glsl(
    #version 410 core
    
    in vec3 Color;
    uniform vec3 uniColor;
    out vec4 outColor;

    void main()
    {
        outColor = vec4(uniColor.x, Color.y, Color.z, 1.0);
    }
)glsl";

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
    
    // Create and compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    // Compile the shader into code that can be executed by the graphics card.
    glCompileShader(vertexShader);
    GLint status;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (status == GL_TRUE)
    {
        printf("Vertex shader compiled successfully\n");
    }
    
    // Create and compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    if (status == GL_TRUE)
    {
        printf("Fragment shader compiled successfully\n");
    }
    
    // Create program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    // Since this is 0 by default and there's only one output right now,
    // the following line of code is not necessary:
    glBindFragDataLocation(shaderProgram, 0, "outColor");
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
