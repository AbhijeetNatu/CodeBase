// Header files
#include <windows.h>
#include "OGL.h"
#include "Sphere.h"
#include <stdio.h> 
#include <stdlib.h> 

// OpenGL header File
#include <GL/glew.h> 
#include <GL/gl.h>

#include "vmath.h"
using namespace vmath;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
// OpenGL Library
#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"Sphere.lib")

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];

GLuint gNumVertices;
GLuint gNumElements;
GLuint gVao_sphere;
GLuint gVbo_sphere_position;
GLuint gVbo_sphere_normal;
GLuint gVbo_sphere_element;
GLuint vbgVao_sphere;
GLuint vagVbo_sphere_position;


// Global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global variables declaration
HWND ghwnd = NULL;
BOOL gbFullScreen = FALSE;
FILE* gpFile = NULL;
BOOL gbActiveWindow = FALSE;
HDC ghdc = NULL;
HGLRC ghrc = NULL;

// Programable Pipeline Related Variables
GLuint shaderProgramObject;
GLuint shaderProgramObject1;

char chosenShader = 'v';

enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXTURE0
};

//Per Vertex
GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint projectionMatrixUniform;

//Related to Light
GLuint laUniform;
GLuint ldUniform;
GLuint lsUniform;
GLuint lightPositionUniform;

GLuint kaUniform;
GLuint kdUniform;
GLuint ksUniform;
GLuint materialShininessUniform;

GLuint lightingEnabledUniform;

//Per Fragment
GLuint modelMatrixUniform_frag;
GLuint viewMatrixUniform_frag;
GLuint projectionMatrixUniform_frag;

//Related to Light
GLuint laUniform_frag;
GLuint ldUniform_frag;
GLuint lsUniform_frag;
GLuint lightPositionUniform_frag;

GLuint kaUniform_frag;
GLuint kdUniform_frag;
GLuint ksUniform_frag;
GLuint materialShininessUniform_frag;

GLuint lightingEnabledUniform_frag;

BOOL bLight = FALSE;

GLfloat lightAmbient[] = { 0.1f,0.1f,0.1f,1.0 };
GLfloat lightDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightPosition[] = { 100.0f,100.0f,100.0f,1.0f };

GLfloat materialAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat materialDiffuse[] = { 0.5f,0.2f,0.7f,1.0f };
GLfloat materialSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat materialShininess = 128.0f;

GLfloat lightAmbient_frag[] = { 0.1f,0.1f,0.1f,1.0 };
GLfloat lightDiffuse_frag[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightSpecular_frag[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightPosition_frag[] = { 100.0f,100.0f,100.0f,1.0f };

GLfloat materialAmbient_frag[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat materialDiffuse_frag[] = { 0.5f,0.2f,0.7f,1.0f };
GLfloat materialSpecular_frag[] = { 0.7f,0.7f,0.7f,1.0f };
GLfloat materialShininess_frag = 128.0f;

int i = 0;

mat4 perspectiveProjectionMatrix;
// Entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// function declarations
	int initialize(void);
	void display(void);
	void update(void);
	void uninitialize(void);
	// variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("OpenGL");
	BOOL bDone = FALSE;
	int iRetVal = 0;


	if (fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Creation of Log File Failed. Exitting..."), TEXT("File I/O Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log File is Successfully Created.\n");
	}

	// initilization of WNDCLASSEX structure 

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	// Register WNDCLASSEX

	int Screen_Width = GetSystemMetrics(SM_CXSCREEN);
	int Screen_Height = GetSystemMetrics(SM_CYSCREEN);

	int Window_X = (Screen_Width / 2) - (WINDOW_WIDTH / 2);
	int Window_Y = (Screen_Height / 2) - (WINDOW_HEIGHT / 2);

	RegisterClassEx(&wndclass);

	// Create the Window

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("OpenGL Window"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		Window_X,
		Window_Y,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	// Initilize 
	iRetVal = initialize();

	// Show window
	ShowWindow(hwnd, iCmdShow);

	if (iRetVal == -1)
	{
		fprintf(gpFile, "Choose Pixel Format Failed\n");
		uninitialize();
	}
	if (iRetVal == -2)
	{
		fprintf(gpFile, "Set Pixel Format Failed\n");
		uninitialize();
	}
	if (iRetVal == -3)
	{
		fprintf(gpFile, "Creating OpenGL Context Failed\n");
		uninitialize();
	}
	if (iRetVal == -4)
	{
		fprintf(gpFile, "Making OpenGL as Current Contex Failed\n");
		uninitialize();
	}
	if (iRetVal == -5)
	{
		fprintf(gpFile, "GLEW initialization failed");
		uninitialize();
	}

	// Foregrounding and Focusing the Window

	SetForegroundWindow(hwnd); 
	SetFocus(hwnd);

	// Game Loop

	while (bDone == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = TRUE;
			}
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
				// Render The Scene
				display();

				// Update The Scene
				update();

			}
		}
	}
	uninitialize();
	return (int)msg.wParam;
}

//Callback function

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// Function Declarations

	void ToggleFullScreen(void);
	void resize(int, int);

	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = TRUE;
		break;
	case WM_KILLFOCUS:
		gbActiveWindow = FALSE;
		break;
	case WM_ERASEBKGND:
		return 0;
	case WM_CHAR:
		switch (wParam)
		{
		case 'Q':
		case 'q':
			DestroyWindow(hwnd);
			break;

		case 'F':
		case 'f':
			chosenShader = 'f';
			break;

		case 'V':
		case 'v':
			chosenShader = 'v';
			break;

		case 'L':
		case 'l':
			if (bLight == FALSE)
			{
				bLight = TRUE;
			}
			else
			{
				bLight = FALSE;
			}
			break;

		default:
			break;

		}
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case 27:
			ToggleFullScreen();
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

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void ToggleFullScreen(void)
{
	// Variable Declarations
	static DWORD dwStyle;
	static WINDOWPLACEMENT wp;
	MONITORINFO mi;

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

int initialize(void)
{
	// Function declaration
	void resize(int, int);
	void printGLInfo(void);
	void uninitialize(void);

	// Variable declaration
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;

	// code
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	// initilization of PixelFormatDescriptor
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32; 

	// GetDC
	ghdc = GetDC(ghwnd);

	// Chose Pixel Format
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
	{
		return -1;
	}

	// Set Chosen Pixel Format
	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		return -2;
	}

	//Create OpenGL Rendering Context
	ghrc = wglCreateContext(ghdc);

	if (ghrc == NULL)
	{
		return -3;
	}

	//Make the rendering context as current context
	if (wglMakeCurrent(ghdc, ghrc) == FALSE) 
	{
		return -4;
	}

	// GLEW Initialization 
	if (glewInit() != GLEW_OK)
	{
		return -5;
	}

	// Print OpenGL information
	printGLInfo();

	// Vertex Shader
	const GLchar* vertexShaderSourceCode =
		"#version 460 core" \
		"\n" \
		"in vec4 a_position;" \
		"in vec3 a_normal;" \
		"uniform mat4 u_modelMatrix;" \
		"uniform mat4 u_viewMatrix;" \
		"uniform mat4 u_projectionMatrix;" \
		"uniform vec3 u_la;" \
		"uniform vec3 u_ld;" \
		"uniform vec3 u_ls;" \
		"uniform vec4 u_lightPosition;" \
		"uniform vec3 u_ka;" \
		"uniform vec3 u_kd;" \
		"uniform vec3 u_ks;" \
		"uniform float u_materialShininess;" \
		"uniform int u_lightEnabled;" \
		"out vec3 phong_ads_light;" \
		"void main(void)" \
		"{" \
		"if(u_lightEnabled==1)" \
		"{" \
		"vec3 ambient = u_la*u_ka;" \
		"vec4 eyeCordinates = u_viewMatrix * u_modelMatrix * a_position;" \
		"mat3 normalMatrix = mat3(u_viewMatrix * u_modelMatrix);" \
		"vec3 transformedNormals = normalize(normalMatrix * a_normal);" \
		"vec3 lightDirection = normalize(vec3(u_lightPosition) - eyeCordinates.xyz);" \
		"vec3 diffuse = u_ld*u_kd*max(dot(lightDirection,transformedNormals),0.0);" \
		"vec3 reflectionVector = reflect(-lightDirection,transformedNormals);" \
		"vec3 viewervector = normalize(-eyeCordinates.xyz);" \
		"vec3 specular = u_ls*u_ks*pow(max(dot(reflectionVector,viewervector),0.0),u_materialShininess);" \
		"phong_ads_light = ambient + diffuse + specular;" \
		"}" \
		"else" \
		"{" \
		"phong_ads_light = vec3(1.0,1.0,1.0);" \
		"}" \
		"gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" \
		"}";

	// Create Vertex Shedar object
	GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	// give Vertex Shedar code to shedar object
	glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);
	// Compile Vertex Sheder
	glCompileShader(vertexShaderObject);
	// Error Checking 
	GLint status;
	GLint infoLogLength;
	char* log = NULL;

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
				fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", log);
				free(log);
				uninitialize();
			}
		}
	}

	// Fragment Shader

	const GLchar* fragmentShaderSourceCode =
		"#version 460 core" \
		"\n" \
		"in vec3 phong_ads_light;" \
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"FragColor = vec4(phong_ads_light,1.0);" \
		"}";

	GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);

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
				fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", log);
				free(log);
				uninitialize();
			}
		}
	}

	// Shader Program Object
	shaderProgramObject = glCreateProgram();

	glAttachShader(shaderProgramObject, vertexShaderObject);
	glAttachShader(shaderProgramObject, fragmentShaderObject);

	glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "a_position");
	glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "a_normal");

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
				fprintf(gpFile, "Shader Program Link Log : %s\n", log);
				free(log);
				uninitialize();
			}
		}
	}

	modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_modelMatrix");
	viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_viewMatrix");
	projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");

	laUniform = glGetUniformLocation(shaderProgramObject, "u_la");
	ldUniform = glGetUniformLocation(shaderProgramObject, "u_ld");
	lsUniform = glGetUniformLocation(shaderProgramObject, "u_ls");
	lightPositionUniform = glGetUniformLocation(shaderProgramObject, "u_lightPosition");

	kaUniform = glGetUniformLocation(shaderProgramObject, "u_ka");
	kdUniform = glGetUniformLocation(shaderProgramObject, "u_kd");
	ksUniform = glGetUniformLocation(shaderProgramObject, "u_ks");
	materialShininessUniform = glGetUniformLocation(shaderProgramObject, "u_materialShininess");

	lightingEnabledUniform = glGetUniformLocation(shaderProgramObject, "u_lightEnabled");
	
	
	//Per Fragment
	
	// Vertex Shader

	const GLchar* vertexShaderSourceCode1 =
		"#version 460 core" \
		"\n" \
		"in vec4 a_position;" \
		"in vec3 a_normal;" \
		"uniform mat4 u_modelMatrix;" \
		"uniform mat4 u_viewMatrix;" \
		"uniform mat4 u_projectionMatrix;" \
		"uniform vec4 u_lightPosition;" \
		"uniform int u_lightingEnabled;" \
		"out vec3 transformedNormals;" \
		"out vec3 lightDirection;" \
		"out vec3 viewerVector;" \
		"void main(void)" \
		"{" \
		"if(u_lightingEnabled==1)" \
		"{" \
		"vec4 eyeCordinates = u_viewMatrix * u_modelMatrix * a_position;" \
		"mat3 normalMatrix = mat3(u_viewMatrix * u_modelMatrix);" \
		"transformedNormals = normalMatrix * a_normal;" \
		"lightDirection = vec3(u_lightPosition) - eyeCordinates.xyz;" \
		"viewerVector = -eyeCordinates.xyz;" \
		"}" \
		"gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" \
		"}";

	// Create Vertex Shedar object
	GLuint vertexShaderObject1 = glCreateShader(GL_VERTEX_SHADER);
	// give Vertex Shedar code to shedar object
	glShaderSource(vertexShaderObject1, 1, (const GLchar**)&vertexShaderSourceCode1, NULL);
	// Compile Vertex Sheder
	glCompileShader(vertexShaderObject1);
	// Error Checking 
	status;
	infoLogLength;
	log = NULL;
	//	GLint status;
	//	GLint infoLogLength;
	//	char* log = NULL;

	glGetShaderiv(vertexShaderObject1, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		glGetShaderiv(vertexShaderObject1, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0)
		{
			log = (char*)malloc(infoLogLength);

			if (log != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(vertexShaderObject1, infoLogLength, &written, log);
				fprintf(gpFile, "Vertex Shader1 Compilation Log : %s\n", log);
				free(log);
				uninitialize();
			}
		}
	}

	// Fragment Shader

	const GLchar* fragmentShaderSourceCode1 =
		"#version 460 core" \
		"\n" \
		"uniform vec3 u_la;" \
		"uniform vec3 u_ld;" \
		"uniform vec3 u_ls;" \
		"uniform vec3 u_ka;" \
		"uniform vec3 u_kd;" \
		"uniform vec3 u_ks;" \
		"uniform float u_materialShininess;" \
		"uniform int u_lightingEnabled;" \
		"in vec3 transformedNormals;" \
		"in vec3 lightDirection;" \
		"in vec3 viewerVector;" \
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"vec3 phong_ads_color;" \
		"if(u_lightingEnabled == 1)" \
		"{" \
		"vec3 ambient = u_la * u_ka;" \
		"vec3 normalized_transformed_normals = normalize(transformedNormals);" \
		"vec3 normalized_light_direction = normalize(lightDirection);" \
		"vec3 diffuse = u_ld*u_kd*max(dot(normalized_light_direction,normalized_transformed_normals),0.0);" \
		"vec3 reflectionVector = reflect(-normalized_light_direction,normalized_transformed_normals);" \
		"vec3 normalized_viewer_vector = normalize(viewerVector);" \
		"vec3 specular = u_ls*u_ks*pow(max(dot(reflectionVector,normalized_viewer_vector),0.0),u_materialShininess);" \
		"phong_ads_color = ambient + diffuse + specular;" \
		"}" \
		"else" \
		"{" \
		"phong_ads_color = vec3(1.0,1.0,1.0);" \
		"}" \
		"FragColor = vec4(phong_ads_color,1.0);" \
		"}";

	GLuint fragmentShaderObject1 = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fragmentShaderObject1, 1, (const GLchar**)&fragmentShaderSourceCode1, NULL);

	glCompileShader(fragmentShaderObject1);

	status = 0;
	infoLogLength = 0;
	log = NULL;

	glGetShaderiv(fragmentShaderObject1, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		glGetShaderiv(fragmentShaderObject1, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0)
		{
			log = (char*)malloc(infoLogLength);

			if (log != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(fragmentShaderObject1, infoLogLength, &written, log);
				fprintf(gpFile, "Fragment Shader 1 Compilation Log : %s\n", log);
				free(log);
				uninitialize();
			}
		}
	}

	// Shader Program Object
	shaderProgramObject1 = glCreateProgram();

	glAttachShader(shaderProgramObject1, vertexShaderObject1);
	glAttachShader(shaderProgramObject1, fragmentShaderObject1);

	glBindAttribLocation(shaderProgramObject1, AMC_ATTRIBUTE_POSITION, "a_position");
	glBindAttribLocation(shaderProgramObject1, AMC_ATTRIBUTE_NORMAL, "a_normal");

	glLinkProgram(shaderProgramObject1);

	status = 0;
	infoLogLength = 0;
	log = NULL;

	glGetProgramiv(shaderProgramObject1, GL_LINK_STATUS, &status);

	if (status == GL_FALSE)
	{
		glGetProgramiv(shaderProgramObject1, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0)
		{
			log = (char*)malloc(infoLogLength);
			if (log != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(shaderProgramObject1, infoLogLength, &written, log);
				fprintf(gpFile, "Shader Program Link Log : %s\n", log);
				free(log);
				uninitialize();
			}
		}
	}

	modelMatrixUniform_frag = glGetUniformLocation(shaderProgramObject1, "u_modelMatrix");
	viewMatrixUniform_frag = glGetUniformLocation(shaderProgramObject1, "u_viewMatrix");
	projectionMatrixUniform_frag = glGetUniformLocation(shaderProgramObject1, "u_projectionMatrix");
	laUniform_frag = glGetUniformLocation(shaderProgramObject1, "u_la");
	ldUniform_frag = glGetUniformLocation(shaderProgramObject1, "u_ld");
	lsUniform_frag = glGetUniformLocation(shaderProgramObject1, "u_ls");
	lightPositionUniform_frag = glGetUniformLocation(shaderProgramObject1, "u_lightPosition");
	kaUniform_frag = glGetUniformLocation(shaderProgramObject1, "u_ka");
	kdUniform_frag = glGetUniformLocation(shaderProgramObject1, "u_kd");
	ksUniform_frag = glGetUniformLocation(shaderProgramObject1, "u_ks");
	materialShininessUniform_frag = glGetUniformLocation(shaderProgramObject1, "u_materialShininess");
	lightingEnabledUniform_frag = glGetUniformLocation(shaderProgramObject1, "u_lightingEnabled");


	// declaration of vertex data arrays
	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();

	// VAO and VBO related code
	 // vao
	glGenVertexArrays(1, &gVao_sphere);
	glBindVertexArray(gVao_sphere);

	// position vbo
	glGenBuffers(1, &gVbo_sphere_position);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// normal vbo
	glGenBuffers(1, &gVbo_sphere_normal);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_normal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);

	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// element vbo
	glGenBuffers(1, &gVbo_sphere_element);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// unbind vao
	glBindVertexArray(0);

	//Depth Related Changes
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Here Starts OpenGL Code
	// Clear the Screen using black color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	perspectiveProjectionMatrix = mat4::identity();

	resize(WINDOW_WIDTH, WINDOW_HEIGHT);
	return 0;
}

void printGLInfo(void)
{
	// Local variable declrations
	GLint numExtensions = 0;

	// Code
	fprintf(gpFile, "OpenGL Vendor	: %s\n", glGetString(GL_VENDOR));
	fprintf(gpFile, "OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(gpFile, "OpenGL Version	: %s\n", glGetString(GL_VERSION));
	fprintf(gpFile, "GLSL Version	: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

	fprintf(gpFile, "Number of Supported Extensions : %d\n", numExtensions);

	for (int i = 0; i < numExtensions; i++)
	{
		fprintf(gpFile, "%s\n", glGetStringi(GL_EXTENSIONS, i));
	}
}

void resize(int width, int height)
{
	// code
	if (height == 0)				// to avoid divide by zero 
	{
		height = 1;
	}
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}
void display(void)
{
	// code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (chosenShader == 'v')
	{
		// Use Shader Program Object
		glUseProgram(shaderProgramObject);

		// Transformation
		mat4 TranslationMatrix = mat4::identity();
		mat4 ModelMatrix = mat4::identity();
		mat4 ViewMatrix = mat4::identity();

		TranslationMatrix = vmath::translate(0.0f, 0.0f, -2.0f); 

		ModelMatrix = TranslationMatrix;

		glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, ModelMatrix);
		glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, ViewMatrix);
		glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

		if (bLight == TRUE)
		{
			glUniform1i(lightingEnabledUniform, 1);
			glUniform3fv(laUniform, 1, lightAmbient);
			glUniform3fv(ldUniform, 1, lightDiffuse);
			glUniform3fv(lsUniform, 1, lightSpecular);
			glUniform4fv(lightPositionUniform, 1, lightPosition);

			glUniform3fv(kaUniform, 1, materialAmbient);
			glUniform3fv(kdUniform, 1, materialDiffuse);
			glUniform3fv(ksUniform, 1, materialSpecular);
			glUniform1f(materialShininessUniform, materialShininess);
		}
		else
		{
			glUniform1i(lightingEnabledUniform, 0);
		}

		//Bind Vao
		glBindVertexArray(gVao_sphere);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
		glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

		//Unbind Vao
		glBindVertexArray(0);

		//Unuse The Shader Program Object
		glUseProgram(0);
	}
	else if (chosenShader == 'f')
	{
		// Use Shader Program Object
		glUseProgram(shaderProgramObject1);

		// Transformation
		mat4 TranslationMatrix = mat4::identity();
		mat4 ModelMatrix = mat4::identity();
		mat4 ViewMatrix = mat4::identity();

		TranslationMatrix = vmath::translate(0.0f, 0.0f, -2.0f); 

		ModelMatrix = TranslationMatrix;

		glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, ModelMatrix);
		glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, ViewMatrix);
		glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

		if (bLight == TRUE)
		{
			glUniform1i(lightingEnabledUniform_frag, 1);
			glUniform3fv(laUniform_frag, 1, lightAmbient_frag);
			glUniform3fv(ldUniform_frag, 1, lightDiffuse_frag);
			glUniform3fv(lsUniform_frag, 1, lightSpecular_frag);
			glUniform4fv(lightPositionUniform_frag, 1, lightPosition_frag);

			glUniform3fv(kaUniform_frag, 1, materialAmbient_frag);
			glUniform3fv(kdUniform_frag, 1, materialDiffuse_frag);
			glUniform3fv(ksUniform_frag, 1, materialSpecular_frag);
			glUniform1f(materialShininessUniform_frag, materialShininess_frag);
		}
		else
		{
			glUniform1i(lightingEnabledUniform_frag, 0);
		}

		//Bind vao
		glBindVertexArray(gVao_sphere);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
		
		glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

		//Unbind vao
		glBindVertexArray(0);

		// Unuse The Shader Program Object
		glUseProgram(0);
	}

	SwapBuffers(ghdc);
}
void update(void)
{
	// code

}
void uninitialize(void)
{
	// Function Declarations
	void ToggleFullScreen(void);

	// Code
	if (gbFullScreen)
	{
		ToggleFullScreen();
	}

	if (gVao_sphere)
	{
		glDeleteBuffers(1, &gVao_sphere);
		gVao_sphere = 0;
	}
	if (gVbo_sphere_position)
	{
		glDeleteVertexArrays(1, &gVbo_sphere_position);
		gVbo_sphere_position = 0;
	}
	if (gVbo_sphere_normal)
	{
		glDeleteBuffers(1, &gVbo_sphere_normal);
		gVbo_sphere_normal = 0;
	}
	if (gVbo_sphere_element)
	{
		glDeleteVertexArrays(1, &gVbo_sphere_element);
		gVbo_sphere_element = 0;
	}
	if (vbgVao_sphere)
	{
		glDeleteVertexArrays(1, &vbgVao_sphere);
		vbgVao_sphere = 0;
	}
	if (vagVbo_sphere_position)
	{
		glDeleteVertexArrays(1, &vagVbo_sphere_position);
		vagVbo_sphere_position = 0;
	}

	// Shader Uninitialization
	if (shaderProgramObject)
	{
		glUseProgram(shaderProgramObject);
		GLsizei numAttachedShaders;
		glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numAttachedShaders);
		GLuint* shaderObjects = NULL;
		shaderObjects = (GLuint*)malloc(numAttachedShaders * sizeof(GLuint));
		glGetAttachedShaders(shaderProgramObject, numAttachedShaders, &numAttachedShaders, shaderObjects);
		for (GLsizei i = 0; i < numAttachedShaders; i++)
		{
			glDetachShader(shaderProgramObject, shaderObjects[i]);
			glDeleteShader(shaderObjects[i]);
			shaderObjects[i] = 0;
		}
		free(shaderObjects);
		shaderObjects = NULL;
		glUseProgram(0);
		glDeleteProgram(shaderProgramObject);
		shaderProgramObject = 0;
	}

	// Shader Uninitialization
	if (shaderProgramObject1)
	{
		glUseProgram(shaderProgramObject1);
		GLsizei numAttachedShaders;
		glGetProgramiv(shaderProgramObject1, GL_ATTACHED_SHADERS, &numAttachedShaders);
		GLuint* shaderObjects = NULL;
		shaderObjects = (GLuint*)malloc(numAttachedShaders * sizeof(GLuint));
		glGetAttachedShaders(shaderProgramObject1, numAttachedShaders, &numAttachedShaders, shaderObjects);
		for (GLsizei i = 0; i < numAttachedShaders; i++)
		{
			glDetachShader(shaderProgramObject1, shaderObjects[i]);
			glDeleteShader(shaderObjects[i]);
			shaderObjects[i] = 0;
		}
		free(shaderObjects);
		shaderObjects = NULL;
		glUseProgram(0);
		glDeleteProgram(shaderProgramObject1);
		shaderProgramObject1 = 0;
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
		fprintf(gpFile, "Log File is Successfully Closed.\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}
