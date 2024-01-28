#define GLEW_STATIC
#include <SDL.h>
#include <iostream>
#include <glew.h>
#include <SDL_opengl.h>
#include <gl/glu.h>
#include <vector>


// GLOBALS ================================================================================
// Window Properties
int gScreenHeight = 640;						// window height
int gScreenWidth = 800;							// window width
SDL_Window* gApplicationWindow = nullptr;		// SDL_Window
SDL_GLContext gOpenGLContext = nullptr;			// GL context for SDL Window

// Main loop flag
bool gQuit = false;						// if true, terminate main loop

// VAO
GLuint gVertexArrayObject = 0;
// VBO
GLuint gVertexBufferObject = 0;


// Shaders Text String Variable
// Vertex Shader
const std::string gVertexShaderSource = 
	"#version 410 core\n"
	"in vec4 position;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(position.x, position.y, position.z, position.w);\n"
	"}\n";
// Fragment Shader
const std::string gFragmentShaderSource = 
	"#version 410 core\n"
	"out vec4 color;\n"
	"void main()\n"
	"{\n"
	"	color = vec4(1.0f, 0.5f, 0.0f, 1.0f);\n"
	"}\n";

// Program Object (for shaders)
GLuint gGraphicsPipelineShaderProgram = 0;


// GLOBALS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


// Function to compile shader of given type and source (Currently only support vertex and fragment shader
GLuint CompileShader(GLuint type, const std::string source)
{
	GLuint shaderObject;

	if (type == GL_VERTEX_SHADER)
	{
		shaderObject = glCreateShader(GL_VERTEX_SHADER);
	}
	else if (type == GL_FRAGMENT_SHADER)
	{
		shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	}

	const char* src = source.c_str();
	// The source of the shader
	glShaderSource(shaderObject, 1, &src, nullptr);
	// Now compile the shader
	glCompileShader(shaderObject);

	// Retrieve the result of the compilation
	int result;
	// Retrieve the compilation status
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		char* errorMessages = new char[length];
		glGetShaderInfoLog(shaderObject, length, &length, errorMessages);

		if (type == GL_VERTEX_SHADER)
		{
			std::cout << "ERROR: GL_VERTEX_SHADER compilation failed!\n" << errorMessages << std::endl;
		}
		else if (type == GL_FRAGMENT_SHADER)
		{
			std::cout << "ERROR: GL_FRAGMENT_SHADER compilation failed!\n" << errorMessages << std::endl;
		}

		// Reclaim the memory
		delete[] errorMessages;

		// Delete the broken shader
		glDeleteShader(shaderObject);

		return 0;
	}

	return shaderObject;
}

// Function to create shader program with given vertex and fragment shader sources
GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource)
{
	GLuint programObject = glCreateProgram();

	GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
	GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	glAttachShader(programObject, myVertexShader);
	glAttachShader(programObject, myFragmentShader);
	glLinkProgram(programObject);

	//validate our program
	glValidateProgram(programObject);
	//glDetachShader, glDeleteShader

	return programObject;
}

// Function to create graphics pipeline (shader program)
void CreateGraphicsPipeline()
{
	gGraphicsPipelineShaderProgram = CreateShaderProgram(gVertexShaderSource, gFragmentShaderSource);
}




void GetOpenGLVersionInfo()
{
	std::cout << "OpenGL INFO" << std::endl;
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}


void VertexSpecification()
{
	//lives on the cpu
	const std::vector<GLfloat> vertexPosition{
		//x      y       z
		-0.8f, -0.8f, 0.0f,		//vertex 1
		0.8f, -0.8f, 0.0f,		//vertex 2
		0.0f, 0.8f, 0.0f		//vertex 3
	};

	//start setting things up on the gpu
	glGenVertexArrays(1, &gVertexArrayObject);
	glBindVertexArray(gVertexArrayObject);

	//start generate VBO
	glGenBuffers(1, &gVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, vertexPosition.size() * sizeof(GLfloat), vertexPosition.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(	
		0,						// Attribute 0 corresponds to the enabled glEnableVertexAttribArray
		3,						// The number of components (e.g. x, y, z = 3 components)
		GL_FLOAT,				// The data type being used to create the vertex
		GL_FALSE,				// Is the data normalized?
		0,						// Stride
		(void*)0				//Offset
	);

	// unbind the currently bound vertex array object (VAO)
	glBindVertexArray(0);
	// Disable any attributes opened in vertex attribute array
	glDisableVertexAttribArray(0);
}


void InitializeProgram()
{
	//initialize SDL (only SDL_Video)
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "Error: SDL Initialization failed!" << std::endl;
		exit(1);
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	//Create SDL Window
	gApplicationWindow = SDL_CreateWindow("SDL OPENGL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, gScreenWidth, gScreenHeight, SDL_WINDOW_OPENGL);
	if (gApplicationWindow == nullptr)
	{
		std::cout << "Error: SDL Window Createn failed!" << std::endl;
	}

	//Create Context for OpenGL
	gOpenGLContext = SDL_GL_CreateContext(gApplicationWindow);
	if (gOpenGLContext == nullptr)
	{
		std::cout << "Error: SDL_GL_Context Createn failed!" << std::endl;
	}

	//Initialize GLEW library
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(1);
	}
	else
	{
		std::cout << "GLEW INITIALIZED!" << std::endl;
		std::cout << "Version: " << glewGetString(GLEW_VERSION) << std::endl;
	}

	GetOpenGLVersionInfo();
}


void Input()
{
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_QUIT)
		{
			std::cout << "SDL_QUIT!" << std::endl;
			gQuit = true;
		}
	}
}


//setting openGL states
void PreDraw()
{
	// Disable depth test and face culling
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// Initialize clear color
	// Background of the screen
	glViewport(0, 0, gScreenWidth, gScreenHeight);
	glClearColor(1.f, 1.f, 0.1f, 1.f);

	// Clear color buffer and depth buffer
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// Use the shader
	glUseProgram(gGraphicsPipelineShaderProgram);
}


void Draw()
{
	// Enable the attributes
	glBindVertexArray(gVertexArrayObject);

	// Select the vertex buffer object to enable
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

	// Render data
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Stop using current graphic pipeline
	//glUseProgram(0);      //not necessary if only using one graphic pipeline
}


void MainLoop()
{
	// While the application is running
	while (!gQuit)
	{
		// Handle Input (via SDL)
		Input();

		// Setup anything (i.e. OpenGL State) that needs to take place before draw calls
		PreDraw();

		// Draw Calls in OpenGL
		Draw();

		// Update screem of the specified window
		SDL_GL_SwapWindow(gApplicationWindow);
	}
}



void CleanUp()
{
	SDL_DestroyWindow(gApplicationWindow);

	SDL_Quit();
}


int main(int argc, char* argv[])
{
	// 1. Setup the graphics program (SDL2, OpenGL Context, GLEW)
	InitializeProgram();

	// 2. Setup the geometry
	VertexSpecification();

	// 3. Create the graphic pipeline
	// A minimum, the vertex and fragment shader
	CreateGraphicsPipeline();

	// 4. Call the main application loop
	MainLoop();

	// 5. Call the cleanup function when our program terminates(main loop ended)
	CleanUp();

	return 0;
}