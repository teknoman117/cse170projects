#include <iostream>

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <project2/directories.hpp>

GLuint BuildShaderProgram(const char *vsPath, const char *fsPath);
GLuint CreateShader(GLenum eShaderType, const char *strShaderFile);

int main()
{
    std::cout << "Dir = " << GetApplicationResourcesDirectory() << std::endl;
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return 1;
    
    SDL_Window *window = SDL_CreateWindow("My Game Window",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          640, 480,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (glContext == NULL)
    {
        printf("There was an error creating the OpenGL context!\n");
        return 0;
    }
    
    const unsigned char *version = glGetString(GL_VERSION);
    if (version == NULL)
    {
        printf("There was an error creating the OpenGL context!\n");
        return 1;
    }
    
    SDL_GL_MakeCurrent(window, glContext);
    
    
    //MUST make a context AND make it current BEFORE glewInit()!
    glewExperimental = GL_TRUE;
    GLenum glew_status = glewInit();
    if (glew_status != 0)
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
        return 1;
    }
    
    const float triangleVertices[] = {
        0.0f, 0.5f, 0.0f, 1.0f,
        0.5f, -0.366f, 0.0f, 1.0f,
        -0.5f, -0.366f, 0.0f, 1.0f,
        //next part contains vertex colors
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f
    }; //we love you vertices!
    
    GLuint theShaderProgram;
    std::string vertexShaderPath = GetApplicationResourcesDirectory() + "/vs1.glsl";
    std::string fragmentShaderPath = GetApplicationResourcesDirectory() + "/fs1.glsl";
    theShaderProgram = BuildShaderProgram(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    if (theShaderProgram == -1)
    {
        SDL_Quit();
        return 0;
    }
    
    printf("Using program %d\n", theShaderProgram);
    
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao); //make our vertex array object, we need it to restore state we set after binding it. Re-binding reloads the state associated with it.
    
    GLuint triangleBufferObject;
    glGenBuffers(1, &triangleBufferObject); //create the buffer
    glBindBuffer(GL_ARRAY_BUFFER, triangleBufferObject); //we're "using" this one now
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW); //formatting the data for the buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind any buffers
    
    printf("glError: %d\n", glGetError());
    
    char bGameLoopRunning = 1;
    while (bGameLoopRunning)
    {
        SDL_Event e;
        if ( SDL_PollEvent(&e) )
        {
            if (e.type == SDL_QUIT)
                bGameLoopRunning = 0;
            else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
                bGameLoopRunning = 0;
        }
        
        glClearColor(0.0,0.0,0.0,0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        /* drawing code in here! */
        glUseProgram(theShaderProgram);
        glBindBuffer(GL_ARRAY_BUFFER, triangleBufferObject); //bind the buffer we're applying attributes to
        glEnableVertexAttribArray(0); //0 is our index, refer to "location = 0" in the vertex shader
        glEnableVertexAttribArray(1); //attribute 1 is for vertex color data
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0); //tell gl (shader!) how to interpret our vertex data
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)48); //color data is 48 bytes in to the array
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glUseProgram(0);
        /* drawing code above here! */
        
        SDL_GL_SwapWindow(window);
        SDL_Delay(20);
    }
    
    SDL_GL_DeleteContext(glContext);
    SDL_Quit();
    return 0;
}

GLuint BuildShaderProgram(const char *vsPath, const char *fsPath)
{
    GLuint vertexShader;
    GLuint fragmentShader;
    
    vertexShader = CreateShader(GL_VERTEX_SHADER, vsPath);
    fragmentShader = CreateShader(GL_FRAGMENT_SHADER, fsPath);
    
    /* So we've compiled our shaders, now we need to link them in to the program
     that will be used for rendering. */
    
    /*This section could be broken out into a separate function, but we're doing it here
     because I'm not using C++ STL features that would make this easier. Tutorial doing so is
     here: http://www.arcsynthesis.org/gltut/Basics/Tut01%20Making%20Shaders.html */
    
    GLuint tempProgram;
    tempProgram = glCreateProgram();
    
    glAttachShader(tempProgram, vertexShader);
    glAttachShader(tempProgram, fragmentShader);
    
    glLinkProgram(tempProgram); //linking!
    
    //error checking
    GLint status;
    glGetProgramiv(tempProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetProgramiv(tempProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
        
        GLchar strInfoLog[4096];
        glGetProgramInfoLog(tempProgram, infoLogLength, NULL, strInfoLog);
        printf("Shader linker failure: %s\n", strInfoLog);
        return -1;
    }
    else
        puts("Shader linked sucessfully!");
    
    glDetachShader(tempProgram, vertexShader);
    glDetachShader(tempProgram, fragmentShader);
    
    return tempProgram;
}

GLuint CreateShader(GLenum eShaderType, const char *strShaderFile)
{
    char shaderSource[4096];
    char inChar;
    FILE *shaderFile;
    int i = 0;
    
    shaderFile = fopen(strShaderFile, "r");
    while(fscanf(shaderFile,"%c",&inChar) > 0)
    {
        shaderSource[i++] = inChar; //loading the file's chars into array
    }
    shaderSource[i - 1] = '\0';
    fclose(shaderFile);
    puts(shaderSource); //print to make sure the string is loaded
    
    GLuint shader = glCreateShader(eShaderType);
    const char *ss = shaderSource;
    glShaderSource(shader, 1, &ss, NULL);
    
    glCompileShader(shader);
    
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        
        GLchar strInfoLog[4096];
        glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
        
        char strShaderType[16];
        switch(eShaderType)
        {
            case GL_VERTEX_SHADER: sprintf(strShaderType, "vertex"); break;
            case GL_GEOMETRY_SHADER: sprintf(strShaderType, "geometry"); break;
            case GL_FRAGMENT_SHADER: sprintf(strShaderType, "fragment"); break;
        }
        
        printf("Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
        return -1;
    }
    else
        puts("Shader compiled sucessfully!");
    
    return shader;
}
