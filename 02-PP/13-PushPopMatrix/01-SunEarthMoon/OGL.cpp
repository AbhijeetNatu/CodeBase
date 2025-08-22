// header file
#include <windows.h>
#include "OGL.h"
// OpenGL Header File
#include<gl/glew.h> // this must be above gl.h
#include <GL/gl.h>
#include <stdio.h>
#include"vmath.h"
#include "Sphere.h"
using namespace vmath;


#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// OpenGL Libraries
#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"OpenGl32.lib")
#pragma comment(lib,"Sphere.lib")

// global function declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global Variable Declaration
HWND ghwnd = NULL;
BOOL gbFullScreen = FALSE;
FILE* gpFile = NULL;
BOOL gbActiveWindow = FALSE;
HDC ghdc = NULL;
HGLRC ghrc = NULL;

// programmable pipeline variable
GLuint shaderProgramObject;
enum
{
	AAN_ATTRIBUTE_POSITION = 0,
	AAN_ATTRIBUTE_COLOR,
	AAN_ATTRIBUTE_NORMAL,
	AAN_ATTRIBUTE_TEXTURE0,

};

GLuint vao_sphere;
GLuint vbo_sphere_position;
GLuint vbo_sphere_normal;
GLuint vbo_sphere_element;

GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint projectionMatrixUniform;
GLuint colorUniform;

mat4 perspectiveProjectionMatrix;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];
unsigned int gNumVertices;
unsigned int gNumElements;

int day = 0;
int year = 0;
int moon = 0;

// stack implementation

#define SIZE 5
mat4 stack[SIZE];
int top = -1;

int IsEmpty(void);
int IsFull(void);
void Push(mat4);
void Pop(void);
mat4 Peek(void);


// Entry Point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdline, int iCmdShow)
{
	// Function Declaration
	int initialise(void);
	void display(void);
	void update(void);
	void uninitialize(void);

	// variable declaration
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyWindow");
	BOOL bDone = FALSE;

	int iRetVal = 0;



	// code
	if (fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Creation Of Log.txt File Failed.Exitting"), TEXT("File I/O Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log File is Successfully Created. \n");
	}



	//initialize of WNDCLASSEX structure
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_HAND);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	// Registering above wndclass 

	RegisterClassEx(&wndclass);

	// create the Window

	hwnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		szAppName,
		TEXT("AAN: SUN EARTH MOON"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		(GetSystemMetrics(SM_CXSCREEN) / 2) - (WIN_WIDTH / 2),
		(GetSystemMetrics(SM_CYSCREEN) / 2) - (WIN_HEIGHT / 2),
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	ghwnd = hwnd;

	// initialize
	iRetVal = initialise();

	if (iRetVal == -1)
	{
		fprintf(gpFile, "Choose Pixel Format failed. \n");
		uninitialize();
	}
	else if (iRetVal == -2)
	{
		fprintf(gpFile, "Set Pixel Format failed. \n");
		uninitialize();
	}

	else if (iRetVal == -3)
	{
		fprintf(gpFile, "Create OpenGL Context failed. \n");
		uninitialize();
	}
	else if (iRetVal == -4)
	{
		fprintf(gpFile, "Making OpenGL as Current Context failed. \n");
		uninitialize();
	}
	else if (iRetVal == -4)
	{
		fprintf(gpFile, "GLEW initialization failed. \n");
		uninitialize();
	}
	else
	{
		fprintf(gpFile, "Successfully Executed.... \n");
	}


	// Show Window

	ShowWindow(hwnd, iCmdShow);

	// Forgrounding And Focusing the Window
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// Special Loop / Game Loop

	while (bDone == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == TRUE)
			{
				// Render The scen
				display();

				// Update The scene
				update();

			}
		}
	}
	uninitialize();
	return((int)msg.wParam);

}

// Callback function

LRESULT CALLBACK WndProc(HWND hwnd, UINT imsg, WPARAM wParam, LPARAM lParam)
{
	// Local Function Prototype
	void ToggleFullScreen(void);
	void resize(int, int);

	//code
	switch (imsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = TRUE;
		break;
	case WM_KILLFOCUS:
		gbActiveWindow = FALSE;
		break;

	case WM_ERASEBKGND:
		return(0); 

	case WM_CHAR:
		switch (wParam)
		{
		case 27:
			DestroyWindow(hwnd);
			break;

		case 'F':
		case 'f':
			ToggleFullScreen();
			break;

		case 'D':
			day = (day + 6) % 360;
			break;

		case 'd':
			day = (day - 6) % 360;
			break;

		case 'Y':
			year = (year + 3) % 360;
			moon = (moon + 6) % 360;
			day = (day + 20) % 360;
			break;

		case 'y':
			year = (year - 3) % 360;
			moon = (moon - 6) % 360;
			day = (day - 20) % 360;
			break;

		default:
			break;
		}
		break;

	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}
	return(DefWindowProc(hwnd, imsg, wParam, lParam));
}


void ToggleFullScreen(void)
{
	// Variable Declaration
	static DWORD dwStyle;
	static WINDOWPLACEMENT wp;
	MONITORINFO mi;

	// Code
	wp.length = sizeof(WINDOWPLACEMENT);
	if (gbFullScreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(MONITORINFO);
			if (GetWindowPlacement(ghwnd, &wp) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);

			}
			ShowCursor(FALSE);
			gbFullScreen = TRUE;
		}
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wp);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
		gbFullScreen = FALSE;
	}
}


int initialise(void)
{
	// Function Declaration
	void resize(int, int);
	void printGLInfo(void);
	void uninitialize(void);

	// Variable Declaration
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;

	// Code

	// Initialization of PIXELFORMATDESCRIPTOR
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32; // 24 also can be done

	// GetDC
	ghdc = GetDC(ghwnd);

	// Choose Pixel Format

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
	{
		return(-1);
	}

	// Set Chosen Pixel Format

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		return(-2);
	}

	// Create OpenGL Rendering Contect

	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL)
	{
		return(-3);
	}

	// Make The Rendering Context as Current Context

	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
	{
		return(-4);
	}

	// Glew Initialization
	if (glewInit() != GLEW_OK)
	{
		return(-5);
	}
	// Print OpenGL info
	//printGLInfo();


	// Vertex Shader 
	
	//step 1
	const GLchar* vertexShaderSourceCode =
		"#version 460 core" \
		"\n" \
		"in vec4 a_position;" \
		"uniform mat4 u_modelMatrix;" \
		"uniform mat4 u_viewMatrix;" \
		"uniform mat4 u_projectionMatrix;" \
		"void main(void)" \
		"{" \
		"gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" \
		"}";

	//step 2
	GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	//step 3
	glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);

	// step 4
	glCompileShader(vertexShaderObject);

	//step 5
	GLint status;
	GLint infoLogLength;
	char* log = NULL;
	// sub 1
	glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char*)malloc(infoLogLength);
			if (log != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(vertexShaderObject, infoLogLength, &written, log);
				fprintf(gpFile, "Vertex Shader Compilation Log : %s \n", log);
				free(log);
				uninitialize();
			}
		}
	}
	
	//Fragment Shader

	const GLchar* fragmentShaderSourceCode =
		"#version 460 core" \
		"\n" \
		"uniform vec4 u_color;" \
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"FragColor = vec4(u_color);" \
		"}";

	//step 2
	GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	//step 3
	glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);

	// step 4
	glCompileShader(fragmentShaderObject);

	status = 0;
	infoLogLength = 0;
	log = NULL;
	glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char*)malloc(infoLogLength);
			if (log != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(fragmentShaderObject, infoLogLength, &written, log);
				fprintf(gpFile, "Fragment Shader Compilation Log : %s \n", log);
				free(log);
				uninitialize();
			}
		}
	}


	// shader program object
	shaderProgramObject = glCreateProgram();

	glAttachShader(shaderProgramObject, vertexShaderObject);
	glAttachShader(shaderProgramObject, fragmentShaderObject);

	glBindAttribLocation(shaderProgramObject,AAN_ATTRIBUTE_POSITION,"a_position");

	glLinkProgram(shaderProgramObject);

	status = 0;
	infoLogLength = 0;
	log = NULL;
	glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char*)malloc(infoLogLength);
			if (log != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(shaderProgramObject, infoLogLength, &written, log);
				fprintf(gpFile, "Shader Program info Log : %s\n", log);
				free(log);
				uninitialize();
			}
		}
	}

	// post linking
	modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_modelMatrix");
	viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_viewMatrix");
	projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");
	colorUniform = glGetUniformLocation(shaderProgramObject, "u_color");

	// declaration of vertex data arrays

	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();


	// vao  and vbo related code

	glGenVertexArrays(1, &vao_sphere);
	glBindVertexArray(vao_sphere);

	//POSITON
	glGenBuffers(1, &vbo_sphere_position);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(AAN_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(AAN_ATTRIBUTE_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo


	//NORMAL
	glGenBuffers(1, &vbo_sphere_normal);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_normal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);

	glVertexAttribPointer(AAN_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(AAN_ATTRIBUTE_NORMAL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//ELEMENT
	glGenBuffers(1, &vbo_sphere_element);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	// unbind vao
	glBindVertexArray(0);


	// depth related code
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);


	// Clear the screen using Black color

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	perspectiveProjectionMatrix = mat4::identity();

	// WarmUp resize call
	resize(WIN_WIDTH, WIN_HEIGHT);
	return(0);
}

void printGLInfo(void)
{
	//Local variable declarations
	GLint numExtensions = 0;

	// Code

	fprintf(gpFile, "OpenGL Vendor :%s \n", glGetString(GL_VENDOR));
	fprintf(gpFile, "OpenGL Renderer :%s \n", glGetString(GL_RENDERER));
	fprintf(gpFile, "OpenGL Version :%s \n", glGetString(GL_VERSION));
	fprintf(gpFile, "OpenGL GLSL Version :%s \n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	fprintf(gpFile, "Number Of Supported extension :%d \n", numExtensions);

	for (int i = 0; i < numExtensions; i++)
	{
		fprintf(gpFile, " %s \n", glGetStringi(GL_EXTENSIONS, i));
	}

}

void resize(int width, int height)
{
	// Code
	if (height == 0)
		height = 1;  

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);


	perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);


}

void display(void)
{
	// Code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the shader program object
	glUseProgram(shaderProgramObject);

	//Transformation
	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();
	mat4 translationMatrix = mat4::identity();
	mat4 scaleMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();
	
	viewMatrix = vmath::lookat(vec3(0.0f, 0.0f, 5.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

	modelMatrix = translationMatrix * scaleMatrix * rotationMatrix;
	Push(modelMatrix);

	glUniformMatrix4fv(modelMatrixUniform,1,GL_FALSE,modelMatrix);

	glUniformMatrix4fv(viewMatrixUniform,1,GL_FALSE, viewMatrix);

	glUniformMatrix4fv(projectionMatrixUniform,1,GL_FALSE, perspectiveProjectionMatrix);

	//SUN

	float suncolor[] = { 1.0f,1.0f,0.0f,1.0f };
	glUniform4fv(colorUniform, 1, suncolor);

	glBindVertexArray(vao_sphere);

	// Here should be drawing code

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// unbind vao 
	glBindVertexArray(0);

	Pop();

	// EARTH

	Push(modelMatrix);
	mat4 rotationMatrix_y = vmath::rotate((GLfloat)year, 0.0f, 1.0f, 0.0f);

	mat4 rotationMatrix_d = vmath::rotate((GLfloat)day, 0.0f, 1.0f, 0.0f);
	rotationMatrix = rotationMatrix_y;
	rotationMatrix *= vmath::translate(2.0f, 0.0f, 0.0f);
	scaleMatrix = vmath::scale(0.6f, 0.6f, 0.6f);

	modelMatrix *= Peek() * translationMatrix * scaleMatrix * rotationMatrix * rotationMatrix_d;
	Push(modelMatrix);
	glUniformMatrix4fv(modelMatrixUniform,1,GL_FALSE, modelMatrix);

	glUniformMatrix4fv(viewMatrixUniform,1,GL_FALSE,viewMatrix);

	glUniformMatrix4fv(projectionMatrixUniform,1,GL_FALSE, perspectiveProjectionMatrix);

	float earthcolor[] = { 0.4f, 0.9f, 1.0f,1.0f };
	glUniform4fv(colorUniform, 1, earthcolor);

	glBindVertexArray(vao_sphere);

	// Here should be the drawing code

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
	glDrawElements(GL_LINES, gNumElements, GL_UNSIGNED_SHORT, 0);


	// unbind vao
	glBindVertexArray(0);

	// CODE FOR EARTH ends here


	//MOON

	modelMatrix *= Peek(); //* scaleMatrix * rotationMatrix;

	glUniformMatrix4fv(modelMatrixUniform,1,GL_FALSE, modelMatrix);

	glUniformMatrix4fv(viewMatrixUniform,1,GL_FALSE, viewMatrix);

	glUniformMatrix4fv(projectionMatrixUniform,1,GL_FALSE, perspectiveProjectionMatrix);

	float mooncolor[] = { 1.0f, 1.0f, 1.0f,1.0f };
	glUniform4fv(colorUniform, 1, mooncolor);

	glBindVertexArray(vao_sphere);

	// Here should be drawing code

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	Pop();
	Pop();

	// Unbind vao
	glBindVertexArray(0);

	//CODE FOR MOON ENDS HERE

	// Unuse the shader program object
	glUseProgram(0);

	SwapBuffers(ghdc);
}



int IsEmpty()
{
	if (top == -1)
		return TRUE;
	else
		return FALSE;

}

int IsFull()
{
	if (top == (SIZE - 1))
		return TRUE;
	else
		return FALSE;

}

void Push(mat4 curmat)
{
	int value;
	if (IsFull() == TRUE)
	{
		printf("\n\n");
		printf("Stack Is Full .");
		return;
	}

	top++;
	stack[top] = curmat;
	return;
}
void Pop(void)
{

	top--;

	return;
}

mat4 Peek()
{
	if (IsEmpty() == TRUE)
	{
		printf("Stack is Empty \n");
	}
	return stack[top];
}

void update(void)
{
	
}

void uninitialize(void)
{
	// Function Declaration
	void ToggleFullScreen(void);

	// Code
	if (gbFullScreen)
	{
		ToggleFullScreen();
	}

	if (gpFile)
	{
		fprintf(gpFile, "Log.txt File is Successfully Close.\n");
		fclose(gpFile);
		gpFile = NULL;
	}


	// DELETION AND UNINITIALIZATION OF vbo

	if (vbo_sphere_position)
	{
		glDeleteBuffers(1, &vbo_sphere_position);
		vbo_sphere_position = 0;
	}

	if (vbo_sphere_normal)
	{
		glDeleteBuffers(1, &vbo_sphere_normal);
		vbo_sphere_normal = 0;
	}

	// DELETION AND UNINITIALIZATION OF vao

	if (vao_sphere)
	{
		glDeleteVertexArrays(1, &vao_sphere);
		vao_sphere = 0;
	}

	// Shader Uninitialization
	if (shaderProgramObject)
	{
		glUseProgram(shaderProgramObject);
		GLsizei  numAttachShader;

		glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numAttachShader);
		GLuint* shaderObject = NULL;
		shaderObject = (GLuint*)malloc(numAttachShader * sizeof(GLuint));
		// 
		glGetAttachedShaders(shaderProgramObject, numAttachShader, &numAttachShader, shaderObject);

		for (GLsizei i = 0; i < numAttachShader; i++)
		{
			glDetachShader(shaderProgramObject, shaderObject[i]);
			glDeleteShader(shaderObject[i]);
			shaderObject[i] = 0;
			free(shaderObject);
			shaderObject = NULL;
		}
		glUseProgram(0);
		glDeleteProgram(shaderProgramObject);
		shaderProgramObject = NULL;
	}

	if (wglGetCurrentContext() == ghrc)
	{
		wglMakeCurrent(NULL, NULL);

	}

	if (ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	if (ghdc)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}


	if (ghwnd)
	{
		DestroyWindow(ghwnd);
		ghwnd = NULL;
	}

	if (gpFile)
	{
		fprintf(gpFile, "Log.txt File is Successfully Close.\n");
		fclose(gpFile);
		gpFile = NULL;
	}


}
