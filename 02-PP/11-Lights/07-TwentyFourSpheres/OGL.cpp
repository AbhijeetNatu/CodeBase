// Header files
#include <windows.h>
#include <stdio.h>
#include "OGL.h"
#include "Sphere.h"

// OpenGL Header File
#include<gl/glew.h> // this must be above gl.h
#include <GL/gl.h>
#include"vmath.h"
using namespace vmath;

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// OpenGL Libraries
#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"OpenGl32.lib")
#pragma comment(lib,"Sphere.lib")

// Global function declaration
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
	AAN_ATTRIBUTE_VERTEX =0,
	AAN_ATTRIBUTE_COLOR,
	AAN_ATTRIBUTE_NORMAL,
	AAN_ATTRIBUTE_TEXTURE0,

};

GLuint gVao_sphere;
GLuint gVbo_sphere_position;
GLuint gVbo_sphere_normal;
GLuint gVbo_sphere_element;

mat4 perspectiveProjectionMatrix;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];
unsigned int gNumVertices;
unsigned int gNumElements;


BOOL bLight = FALSE;

// Uniform
GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint projectionMatrixUniform;

GLuint laUniform;
GLuint ldUniform;
GLuint lsUniform;

GLuint lightPositionUniform;

GLuint kaUniform;
GLuint kdUniform;
GLuint ksUniform;
GLuint materialShinenessUniform;

GLuint lightingEnabledUniforn;

GLfloat lightAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat lightDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightPosition[] = { 0.0f,0.0f,0.0f,1.0f };


GLfloat materialAmbient[6][4][4];
GLfloat materialDiffuse[6][4][4];
GLfloat materialSpecular[6][4][4];
GLfloat materialShininess[6][4][1];


// Light
GLfloat AngleForXRotation = 0.0f;
GLfloat AngleForYRotation = 0.0f;
GLfloat AngleForZRotation = 0.0f;

GLint KeyPressed = 0;
GLfloat radius = 25.0f;

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
	TCHAR szAppName[]=TEXT("MyWindow");
	BOOL bDone = FALSE;

	int iRetVal = 0;



	// code
	if (fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Creation Of Log.txt File Failed. Exiting"), TEXT("File I/O Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log.txt File is Successfully Created. \n");
	}



	//initialize of WNDCLASSEX structure
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(MYICON));
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
		TEXT("AAN : 24 Spheres"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |WS_VISIBLE,
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
		fprintf(gpFile, "Making OpenGl as Current Context failed. \n");
		uninitialize();
	}
	else if (iRetVal == -4)
	{
		fprintf(gpFile, "Glew initialize failed. \n");
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
			
				display();
			
				update();

			}
		}
	}
	uninitialize();
	return((int) msg.wParam);

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

				case 'X':
				case 'x':
					KeyPressed = 1;
					AngleForXRotation = 0.0f;

					break;

				case 'Y':
				case 'y':
					KeyPressed = 2;
					AngleForYRotation = 0.0f;

					break;

				case 'Z':
				case 'z':
					KeyPressed = 3;
					AngleForZRotation = 0.0f;

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

		default :
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
	void SetMaterialFor24Spheres(void);

	// Variable Declaration
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormateIndex = 0;

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

	iPixelFormateIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormateIndex == 0)
	{
		return(-1);
	}

	// Set Chosen Pixel Format

	if (SetPixelFormat(ghdc, iPixelFormateIndex, &pfd) == FALSE)
	{
		return(-2);
	}

	// Create OpenGL Rendering Context

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

	// glew Initialization
	if (glewInit() != GLEW_OK)
	{
		return(-5);
	}

	// Print OpenGL info
	printGLInfo();

	// Vertex Shader 
	
	//step 1
	const GLchar* vertexShaderSourceCode =
		"#version 460 core" \
		"\n" \
		"in vec4 a_position;" \
		"in vec3 a_normal;" \
		"uniform mat4 u_modelMatrix;" \
		"uniform mat4 u_viewMatrix;" \
		"uniform mat4 u_projectionMatrix;" \
		"uniform vec4 u_lightPosition;" \
		"out vec3 lightDirection;" \
		"uniform int u_LightingEnabled;" \
		"out vec3 transformedNormals;" \
		"out vec3 viewerVector;" \
		"void main(void)" \
		"{" \
		"if(u_LightingEnabled == 1)" \
		"{" \
		"vec4 eyeCoordinates = u_viewMatrix * u_modelMatrix * a_position;" \
		"mat3 normalMatrix = mat3(u_viewMatrix * u_modelMatrix);" \
		"transformedNormals = normalMatrix * a_normal;" \
		"lightDirection = vec3(u_lightPosition) - eyeCoordinates.xyz;" \
		"viewerVector = -eyeCoordinates.xyz;" \
		"}" \
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
		"uniform vec3 u_la;" \
		"uniform vec3 u_ld;" \
		"uniform vec3 u_ls;" \
		"uniform vec3 u_ka;" \
		"uniform vec3 u_kd;" \
		"uniform vec3 u_ks;" \
		"uniform float u_materialShininess;" \
		"out vec4 FragColor;" \
		"in vec3 transformedNormals;" \
		"in vec3 viewerVector;" \
		"in vec3 lightDirection;" \
		"uniform int u_LightingEnabled;" \
		"void main(void)" \
		"{" \
		"vec3 phong_ads_color;" \
		"if(u_LightingEnabled == 1)" \
		"{" \
		"vec3 ambient = u_la * u_ka;" \
		"vec3 normalised_transformed_normal = normalize(transformedNormals);" \
		"vec3 normalised_light_direction = normalize(lightDirection);" \
		"vec3 diffuse = u_ld * u_kd * max(dot(normalised_light_direction, normalised_transformed_normal),0.0);" \
		"vec3 reflectionVector = reflect(-normalised_light_direction,normalised_transformed_normal);" \
		"vec3 normalised_viewerVector = normalize(viewerVector);" \
		"vec3 specular = u_ls * u_ks * pow(max(dot(reflectionVector,normalised_viewerVector),0.0),u_materialShininess);" \
		"phong_ads_color = ambient + diffuse + specular;" \
		"}" \
		"else" \
		"{" \
		"phong_ads_color = vec3(1.0f,1.0f,1.0f);" \
		"}" \
		"FragColor = vec4(phong_ads_color, 1.0);" \
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

	glBindAttribLocation(shaderProgramObject,AAN_ATTRIBUTE_VERTEX,"a_position");
	glBindAttribLocation(shaderProgramObject,AAN_ATTRIBUTE_NORMAL,"a_normal");

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
	
	materialShinenessUniform = glGetUniformLocation(shaderProgramObject, "u_materialShininess");
	lightingEnabledUniforn = glGetUniformLocation(shaderProgramObject, "u_LightingEnabled");

	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();


	glGenVertexArrays(1, &gVao_sphere);
	glBindVertexArray(gVao_sphere);

	glGenBuffers(1, &gVbo_sphere_position);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(AAN_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(AAN_ATTRIBUTE_VERTEX);

	glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo
	
	//NORMAL
	glGenBuffers(1, &gVbo_sphere_normal);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_normal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);

	glVertexAttribPointer(AAN_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(AAN_ATTRIBUTE_NORMAL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//ELEMENT
	glGenBuffers(1, &gVbo_sphere_element);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	// unbind vao
	glBindVertexArray(0);

	//Set material
	SetMaterialFor24Spheres();

	// depth related code
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_CULL_FACE);		//IMP

	// Clear the screen using Gray color

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	
	perspectiveProjectionMatrix = mat4::identity();
		
	// WarmUp resize call
	resize(WIN_WIDTH, WIN_HEIGHT);
	return(0);
}

void printGLInfo(void)
{
	//Local variable declarations
	GLint numExtensions=0;

	// Code

	fprintf(gpFile, "OpenGL Vendor :%s \n", glGetString(GL_VENDOR));
	fprintf(gpFile, "OpenGL Renderer :%s \n", glGetString(GL_RENDERER));
	fprintf(gpFile, "OpenGL Version :%s \n", glGetString(GL_VERSION));
	fprintf(gpFile, "OpenGL GLSL Version :%s \n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	fprintf(gpFile, "Number Of Supported extension :%d \n", numExtensions);

	for (int i = 0; i < numExtensions; i++)
	{
		fprintf(gpFile, " %s \n", glGetStringi(GL_EXTENSIONS,i));
	}

}

void resize(int width, int height)
{
	// Code
	if (height == 0)
		height = 1;  //to avoid divide by zero in future

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	
	
	perspectiveProjectionMatrix = vmath::perspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);


}

void display(void)
{
	// Code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the shader program object
	glUseProgram(shaderProgramObject);


	if (bLight)
	{
		if (KeyPressed == 1)
		{
			lightPosition[0] = radius * sin(AngleForXRotation);
			lightPosition[2] = radius * cos(AngleForXRotation);

		}
		else if (KeyPressed == 2) // y rotation
		{
			lightPosition[1] = radius * sin(AngleForYRotation);
			lightPosition[2] = radius * cos(AngleForYRotation);
		}
		else if (KeyPressed == 3) // z axis rotatation
		{
			lightPosition[0] = radius * sin(AngleForZRotation);
			lightPosition[1] = radius * cos(AngleForZRotation);

		}
		else
		{
			lightPosition[0] = 0.0f;
		}

		glUniform1i(lightingEnabledUniforn, 1);

		glUniform3fv(laUniform, 1, lightAmbient);
		glUniform3fv(ldUniform, 1, lightDiffuse);
		glUniform3fv(lsUniform, 1, lightSpecular);

		glUniform4fv(lightPositionUniform, 1, lightPosition);

	}
	else
	{
		glUniform1i(lightingEnabledUniforn, 0);
	}
	

	//Transformation
	
	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();
	float x = -6.5f;
	float y = 4.0f;
	for (int i = 0; i < 4; i++)
	{
		x = x + 2.50f;
		y = 3.0f;
		for (int j = 0; j < 6; j++)
		{
			glUniform3fv(kaUniform, 1, materialAmbient[i][j]);
			glUniform3fv(kdUniform, 1, materialDiffuse[i][j]);
			glUniform3fv(ksUniform, 1, materialSpecular[i][j]);
			glUniform1f(materialShinenessUniform, materialShininess[i][j][0]);
			mat4 translationMatrix = mat4::identity();
			translationMatrix = vmath::translate(x, y, -12.0f);

			modelMatrix = translationMatrix;

			glUniformMatrix4fv(modelMatrixUniform,1,GL_FALSE, modelMatrix);

			glUniformMatrix4fv(viewMatrixUniform,1,GL_FALSE, viewMatrix);

			glUniformMatrix4fv(projectionMatrixUniform,1,GL_FALSE, perspectiveProjectionMatrix);

			glBindVertexArray(gVao_sphere);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
			glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

			glBindVertexArray(0);
			y = y - 1.4f;
		}
	}
	

	// Unuse the shader program object
	glUseProgram(0);

	
	SwapBuffers(ghdc);
}

void update(void)
{
	// Code
	if (KeyPressed == 1)
	{
		
		AngleForXRotation = AngleForXRotation + 0.02f;
		if (AngleForXRotation >= 360.0f)
			AngleForXRotation = AngleForXRotation - 360.0f;
	}
	else if (KeyPressed == 2)
	{
		AngleForYRotation = AngleForYRotation + 0.02f;
		if (AngleForYRotation >= 360.0f)
			AngleForYRotation = AngleForYRotation - 360.0f;
	}
	else if (KeyPressed == 3)
	{
		AngleForZRotation = AngleForZRotation + 0.02f;
		if (AngleForZRotation >= 360.0f)
			AngleForZRotation = AngleForZRotation - 360.0f;
	}

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
	
	// Delete and uninitialize vbo
	if (gVbo_sphere_position)
	{
		glDeleteBuffers(1, &gVbo_sphere_position);
		gVbo_sphere_position = 0;
	}

	if (gVbo_sphere_normal)
	{
		glDeleteBuffers(1, &gVbo_sphere_normal);
		gVbo_sphere_normal = 0;
	}

	// Delete and uninitialize vao
	if (gVao_sphere)
	{
		glDeleteVertexArrays(1, &gVao_sphere);
		gVao_sphere = 0;
	}

	// Shader Uninitialization
	if (shaderProgramObject)
	{
		glUseProgram(shaderProgramObject);
		GLsizei  numAttachShader;

		glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numAttachShader);
		GLuint* shaderObject = NULL;
		shaderObject = (GLuint*)malloc(numAttachShader*sizeof(GLuint));
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

void SetMaterialFor24Spheres(void)
{

	// Code
	// ***** 1st sphere on 1st column, emerald *****
	// ambient material
	materialAmbient[0][0][0] = 0.0215; // r
	materialAmbient[0][0][1] = 0.1745; // g
	materialAmbient[0][0][2] = 0.0215; // b
	materialAmbient[0][0][3] = 1.0f;   // a

	// diffuse material
	materialDiffuse[0][0][0] = 0.07568; // r
	materialDiffuse[0][0][1] = 0.61424; // g
	materialDiffuse[0][0][2] = 0.07568; // b
	materialDiffuse[0][0][3] = 1.0f;    // a

	// specular material
	materialSpecular[0][0][0] = 0.633;    // r
	materialSpecular[0][0][1] = 0.727811; // g
	materialSpecular[0][0][2] = 0.633;    // b
	materialSpecular[0][0][3] = 1.0f;     // a

	// shininess
	materialShininess[0][0][0] = 0.6 * 128;

	// *******************************************************

	// ***** 2nd sphere on 1st column, jade *****
	// ambient material
	materialAmbient[0][1][0] = 0.135;  // r
	materialAmbient[0][1][1] = 0.2225; // g
	materialAmbient[0][1][2] = 0.1575; // b
	materialAmbient[0][1][3] = 1.0f;   // a

	// diffuse material
	materialDiffuse[0][1][0] = 0.54; // r
	materialDiffuse[0][1][1] = 0.89; // g
	materialDiffuse[0][1][2] = 0.63; // b
	materialDiffuse[0][1][3] = 1.0f; // a

	// specular material
	materialSpecular[0][1][0] = 0.316228; // r
	materialSpecular[0][1][1] = 0.316228; // g
	materialSpecular[0][1][2] = 0.316228; // b
	materialSpecular[0][1][3] = 1.0f;     // a

	// shininess
	materialShininess[0][1][0] = 0.1 * 128;

	// *******************************************************

	// ***** 3rd sphere on 1st column, obsidian *****
	// ambient material
	materialAmbient[0][2][0] = 0.05375; // r
	materialAmbient[0][2][1] = 0.05;    // g
	materialAmbient[0][2][2] = 0.06625; // b
	materialAmbient[0][2][3] = 1.0f;    // a

	// diffuse material
	materialDiffuse[0][2][0] = 0.18275; // r
	materialDiffuse[0][2][1] = 0.17;    // g
	materialDiffuse[0][2][2] = 0.22525; // b
	materialDiffuse[0][2][3] = 1.0f;    // a

	// specular material
	materialSpecular[0][2][0] = 0.332741; // r
	materialSpecular[0][2][1] = 0.328634; // g
	materialSpecular[0][2][2] = 0.346435; // b
	materialSpecular[0][2][3] = 1.0f;     // a

	// shininess
	materialShininess[0][2][0] = 0.3 * 128;

	// *******************************************************

	// ***** 4th sphere on 1st column, pearl *****
	// ambient material
	materialAmbient[0][3][0] = 0.25;    // r
	materialAmbient[0][3][1] = 0.20725; // g
	materialAmbient[0][3][2] = 0.20725; // b
	materialAmbient[0][3][3] = 1.0f;    // a

	// diffuse material
	materialDiffuse[0][3][0] = 1.0;   // r
	materialDiffuse[0][3][1] = 0.829; // g
	materialDiffuse[0][3][2] = 0.829; // b
	materialDiffuse[0][3][3] = 1.0f;  // a

	// specular material
	materialSpecular[0][3][0] = 0.296648; // r
	materialSpecular[0][3][1] = 0.296648; // g
	materialSpecular[0][3][2] = 0.296648; // b
	materialSpecular[0][3][3] = 1.0f;     // a

	// shininess
	materialShininess[0][3][0] = 0.088 * 128;
	
	// *******************************************************

	// ***** 5th sphere on 1st column, ruby *****
	// ambient material
	materialAmbient[0][4][0] = 0.1745;  // r
	materialAmbient[0][4][1] = 0.01175; // g
	materialAmbient[0][4][2] = 0.01175; // b
	materialAmbient[0][4][3] = 1.0f;    // a

	// diffuse material
	materialDiffuse[0][4][0] = 0.61424; // r
	materialDiffuse[0][4][1] = 0.04136; // g
	materialDiffuse[0][4][2] = 0.04136; // b
	materialDiffuse[0][4][3] = 1.0f;    // a

	// specular material
	materialSpecular[0][4][0] = 0.727811; // r
	materialSpecular[0][4][1] = 0.626959; // g
	materialSpecular[0][4][2] = 0.626959; // b
	materialSpecular[0][4][3] = 1.0f;     // a

	// shininess
	materialShininess[0][4][0] = 0.6 * 128;

	// *******************************************************

	// ***** 6th sphere on 1st column, turquoise *****
	// ambient material
	materialAmbient[0][5][0] = 0.1;     // r
	materialAmbient[0][5][1] = 0.18725; // g
	materialAmbient[0][5][2] = 0.1745;  // b
	materialAmbient[0][5][3] = 1.0f;    // a

	// diffuse material
	materialDiffuse[0][5][0] = 0.396;   // r
	materialDiffuse[0][5][1] = 0.74151; // g
	materialDiffuse[0][5][2] = 0.69102; // b
	materialDiffuse[0][5][3] = 1.0f;    // a

	// specular material
	materialSpecular[0][5][0] = 0.297254; // r
	materialSpecular[0][5][1] = 0.30829;  // g
	materialSpecular[0][5][2] = 0.306678; // b
	materialSpecular[0][5][3] = 1.0f;     // a

	// shininess
	materialShininess[0][5][0] = 0.1 * 128;

	// *******************************************************
	// *******************************************************
	// *******************************************************

	// ***** 1st sphere on 2nd column, brass *****
	// ambient material
	materialAmbient[1][0][0] = 0.329412; // r
	materialAmbient[1][0][1] = 0.223529; // g
	materialAmbient[1][0][2] = 0.027451; // b
	materialAmbient[1][0][3] = 1.0f;     // a

	// diffuse material
	materialDiffuse[1][0][0] = 0.780392; // r
	materialDiffuse[1][0][1] = 0.568627; // g
	materialDiffuse[1][0][2] = 0.113725; // b
	materialDiffuse[1][0][3] = 1.0f;     // a

	// specular material
	materialSpecular[1][0][0] = 0.992157; // r
	materialSpecular[1][0][1] = 0.941176; // g
	materialSpecular[1][0][2] = 0.807843; // b
	materialSpecular[1][0][3] = 1.0f;     // a

	// shininess
	materialShininess[1][0][0] = 0.21794872 * 128;

	// *******************************************************
	// ***** 2nd sphere on 2nd column, bronze *****
	// ambient material
	materialAmbient[1][1][0] = 0.2125; // r
	materialAmbient[1][1][1] = 0.1275; // g
	materialAmbient[1][1][2] = 0.054;  // b
	materialAmbient[1][1][3] = 1.0f;   // a

	// diffuse material
	materialDiffuse[1][1][0] = 0.714;   // r
	materialDiffuse[1][1][1] = 0.4284;  // g
	materialDiffuse[1][1][2] = 0.18144; // b
	materialDiffuse[1][1][3] = 1.0f;    // a

	// specular material
	materialSpecular[1][1][0] = 0.393548; // r
	materialSpecular[1][1][1] = 0.271906; // g
	materialSpecular[1][1][2] = 0.166721; // b
	materialSpecular[1][1][3] = 1.0f;     // a

	// shininess
	materialShininess[1][1][0] = 0.2 * 128;

	// *******************************************************

	// ***** 3rd sphere on 2nd column, chrome *****
	// ambient material
	materialAmbient[1][2][0] = 0.25; // r
	materialAmbient[1][2][1] = 0.25; // g
	materialAmbient[1][2][2] = 0.25; // b
	materialAmbient[1][2][3] = 1.0f; // a

	// diffuse material
	materialDiffuse[1][2][0] = 0.4;  // r
	materialDiffuse[1][2][1] = 0.4;  // g
	materialDiffuse[1][2][2] = 0.4;  // b
	materialDiffuse[1][2][3] = 1.0f; // a

	// specular material
	materialSpecular[1][2][0] = 0.774597; // r
	materialSpecular[1][2][1] = 0.774597; // g
	materialSpecular[1][2][2] = 0.774597; // b
	materialSpecular[1][2][3] = 1.0f;     // a

	// shininess
	materialShininess[1][2][0] = 0.6 * 128;

	// *******************************************************

	// ***** 4th sphere on 2nd column, copper *****
	// ambient material
	materialAmbient[1][3][0] = 0.19125; // r
	materialAmbient[1][3][1] = 0.0735;  // g
	materialAmbient[1][3][2] = 0.0225;  // b
	materialAmbient[1][3][3] = 1.0f;    // a

	// diffuse material
	materialDiffuse[1][3][0] = 0.7038;  // r
	materialDiffuse[1][3][1] = 0.27048; // g
	materialDiffuse[1][3][2] = 0.0828;  // b
	materialDiffuse[1][3][3] = 1.0f;    // a

	// specular material
	materialSpecular[1][3][0] = 0.256777; // r
	materialSpecular[1][3][1] = 0.137622; // g
	materialSpecular[1][3][2] = 0.086014; // b
	materialSpecular[1][3][3] = 1.0f;     // a

	// shininess
	materialShininess[1][3][0] = 0.1 * 128;

	// *******************************************************

	// ***** 5th sphere on 2nd column, gold *****
	//materialAmbientambient material
	materialAmbient[1][4][0] = 0.24725; // r
	materialAmbient[1][4][1] = 0.1995;  // g
	materialAmbient[1][4][2] = 0.0745;  // b
	materialAmbient[1][4][3] = 1.0f;    // a

	// diffuse material
	materialDiffuse[1][4][0] = 0.75164; // r
	materialDiffuse[1][4][1] = 0.60648; // g
	materialDiffuse[1][4][2] = 0.22648; // b
	materialDiffuse[1][4][3] = 1.0f;    // a

	// specular material
	materialSpecular[1][4][0] = 0.628281; // r
	materialSpecular[1][4][1] = 0.555802; // g
	materialSpecular[1][4][2] = 0.366065; // b
	materialSpecular[1][4][3] = 1.0f;     // a

	// shininess
	materialShininess[1][4][0] = 0.4 * 128;

	// *******************************************************

	// ***** 6th sphere on 2nd column, silver *****
	// ambient material
	materialAmbient[1][5][0] = 0.19225; // r
	materialAmbient[1][5][1] = 0.19225; // g
	materialAmbient[1][5][2] = 0.19225; // b
	materialAmbient[1][5][3] = 1.0f;    // a

	// diffuse material
	materialDiffuse[1][5][0] = 0.50754; // r
	materialDiffuse[1][5][1] = 0.50754; // g
	materialDiffuse[1][5][2] = 0.50754; // b
	materialDiffuse[1][5][3] = 1.0f;    // a

	// specular material
	materialSpecular[1][5][0] = 0.508273; // r
	materialSpecular[1][5][1] = 0.508273; // g
	materialSpecular[1][5][2] = 0.508273; // b
	materialSpecular[1][5][3] = 1.0f;     // a

	// shininess
	materialShininess[1][5][0] = 0.4 * 128;



	// *******************************************************
	// *******************************************************
	// *******************************************************

	// ***** 1st sphere on 3rd column, black *****
	// ambient material
	materialAmbient[2][0][0] = 0.0;  // r
	materialAmbient[2][0][1] = 0.0;  // g
	materialAmbient[2][0][2] = 0.0;  // b
	materialAmbient[2][0][3] = 1.0f; // a

	// diffuse material
	materialDiffuse[2][0][0] = 0.01; // r
	materialDiffuse[2][0][1] = 0.01; // g
	materialDiffuse[2][0][2] = 0.01; // b
	materialDiffuse[2][0][3] = 1.0f; // a

	// specular material
	materialSpecular[2][0][0] = 0.50; // r
	materialSpecular[2][0][1] = 0.50; // g
	materialSpecular[2][0][2] = 0.50; // b
	materialSpecular[2][0][3] = 1.0f; // a

	// shininess
	materialShininess[2][0][0] = 0.25 * 128;

	// *******************************************************
	// ***** 2nd sphere on 3rd column, cyan *****
	// ambient material
	materialAmbient[2][1][0] = 0.0;  // r
	materialAmbient[2][1][1] = 0.1;  // g
	materialAmbient[2][1][2] = 0.06; // b
	materialAmbient[2][1][3] = 1.0f; // a

	// diffuse material
	materialDiffuse[2][1][0] = 0.0;        // r
	materialDiffuse[2][1][1] = 0.50980392; // g
	materialDiffuse[2][1][2] = 0.50980392; // b
	materialDiffuse[2][1][3] = 1.0f;       // a

	// specular material
	materialSpecular[2][1][0] = 0.50196078; // r
	materialSpecular[2][1][1] = 0.50196078; // g
	materialSpecular[2][1][2] = 0.50196078; // b
	materialSpecular[2][1][3] = 1.0f;       // a

	// shininess
	materialShininess[2][1][0] = 0.25 * 128;

	// *******************************************************

	// ***** 3rd sphere on 2nd column, green *****
	// ambient material
	materialAmbient[2][2][0] = 0.0;  // r
	materialAmbient[2][2][1] = 0.0;  // g
	materialAmbient[2][2][2] = 0.0;  // b
	materialAmbient[2][2][3] = 1.0f; // a

	// diffuse material
	materialDiffuse[2][2][0] = 0.1;  // r
	materialDiffuse[2][2][1] = 0.35; // g
	materialDiffuse[2][2][2] = 0.1;  // b
	materialDiffuse[2][2][3] = 1.0f; // a

	// specular material
	materialSpecular[2][2][0] = 0.45; // r
	materialSpecular[2][2][1] = 0.55; // g
	materialSpecular[2][2][2] = 0.45; // b
	materialSpecular[2][2][3] = 1.0f; // a

	// shininess
	materialShininess[2][2][0] = 0.25 * 128;

	// *******************************************************

	// ***** 4th sphere on 3rd column, red *****
	// ambient material
	materialAmbient[2][3][0] = 0.0;  // r
	materialAmbient[2][3][1] = 0.0;  // g
	materialAmbient[2][3][2] = 0.0;  // b
	materialAmbient[2][3][3] = 1.0f; // a

	// diffuse material
	materialDiffuse[2][3][0] = 0.5;  // r
	materialDiffuse[2][3][1] = 0.0;  // g
	materialDiffuse[2][3][2] = 0.0;  // b
	materialDiffuse[2][3][3] = 1.0f; // a

	// specular material
	materialSpecular[2][3][0] = 0.7;  // r
	materialSpecular[2][3][1] = 0.6;  // g
	materialSpecular[2][3][2] = 0.6;  // b
	materialSpecular[2][3][3] = 1.0f; // a

	// shininess
	materialShininess[2][3][0] = 0.25 * 128;

	// *******************************************************

	// ***** 5th sphere on 3rd column, white *****
	// ambient material
	materialAmbient[2][4][0] = 0.0;  // r
	materialAmbient[2][4][1] = 0.0;  // g
	materialAmbient[2][4][2] = 0.0;  // b
	materialAmbient[2][4][3] = 1.0f; // a

	// diffuse material
	materialDiffuse[2][4][0] = 0.55; // r
	materialDiffuse[2][4][1] = 0.55; // g
	materialDiffuse[2][4][2] = 0.55; // b
	materialDiffuse[2][4][3] = 1.0f; // a

	// specular material
	materialSpecular[2][4][0] = 0.70; // r
	materialSpecular[2][4][1] = 0.70; // g
	materialSpecular[2][4][2] = 0.70; // b
	materialSpecular[2][4][3] = 1.0f; // a

	// shininess
	materialShininess[2][4][0] = 0.25 * 128;

	// *******************************************************

	// ***** 6th sphere on 3rd column, yellow plastic *****
	// ambient material
	materialAmbient[2][5][0] = 0.0;  // r
	materialAmbient[2][5][1] = 0.0;  // g
	materialAmbient[2][5][2] = 0.0;  // b
	materialAmbient[2][5][3] = 1.0f; // a

	// diffuse material
	materialDiffuse[2][5][0] = 0.5;  // r
	materialDiffuse[2][5][1] = 0.5;  // g
	materialDiffuse[2][5][2] = 0.0;  // b
	materialDiffuse[2][5][3] = 1.0f; // a

	// specular material
	materialSpecular[2][5][0] = 0.60; // r
	materialSpecular[2][5][1] = 0.60; // g
	materialSpecular[2][5][2] = 0.50; // b
	materialSpecular[2][5][3] = 1.0f; // a

	// shininess
	materialShininess[2][5][0] = 0.25 * 128;

	// *******************************************************
	// *******************************************************
	// *******************************************************


	// ***** 1st sphere on 4th column, black *****
	// ambient material
	materialAmbient[3][0][0] = 0.02; // r
	materialAmbient[3][0][1] = 0.02; // g
	materialAmbient[3][0][2] = 0.02; // b
	materialAmbient[3][0][3] = 1.0f; // a

	// diffuse material
	materialDiffuse[3][0][0] = 0.01; // r
	materialDiffuse[3][0][1] = 0.01; // g
	materialDiffuse[3][0][2] = 0.01; // b
	materialDiffuse[3][0][3] = 1.0f; // a

	// specular material
	materialSpecular[3][0][0] = 0.4;  // r
	materialSpecular[3][0][1] = 0.4;  // g
	materialSpecular[3][0][2] = 0.4;  // b
	materialSpecular[3][0][3] = 1.0f; // a

	// shininess
	materialShininess[3][0][0] = 0.078125 * 128;

	// *******************************************************

	// ***** 2nd sphere on 4th column, cyan *****
	// ambient material
	materialAmbient[3][1][0] = 0.0;  // r
	materialAmbient[3][1][1] = 0.05; // g
	materialAmbient[3][1][2] = 0.05; // b
	materialAmbient[3][1][3] = 1.0f; // a

	// diffuse material
	materialDiffuse[3][1][0] = 0.4;  // r
	materialDiffuse[3][1][1] = 0.5;  // g
	materialDiffuse[3][1][2] = 0.5;  // b
	materialDiffuse[3][1][3] = 1.0f; // a

	// specular material
	materialSpecular[3][1][0] = 0.04; // r
	materialSpecular[3][1][1] = 0.7;  // g
	materialSpecular[3][1][2] = 0.7;  // b
	materialSpecular[3][1][3] = 1.0f; // a

	// shininess
	materialShininess[3][1][0] = 0.078125 * 128;

	// *******************************************************

	// ***** 3rd sphere on 4th column, green *****
	// ambient material
	materialAmbient[3][2][0] = 0.0;  // r
	materialAmbient[3][2][1] = 0.05; // g
	materialAmbient[3][2][2] = 0.0;  // b
	materialAmbient[3][2][3] = 1.0f; // a

	// diffuse material
	materialDiffuse[3][2][0] = 0.4;  // r
	materialDiffuse[3][2][1] = 0.5;  // g
	materialDiffuse[3][2][2] = 0.4;  // b
	materialDiffuse[3][2][3] = 1.0f; // a

	// specular material
	materialSpecular[3][2][0] = 0.04; // r
	materialSpecular[3][2][1] = 0.7;  // g
	materialSpecular[3][2][2] = 0.04; // b
	materialSpecular[3][2][3] = 1.0f; // a

	// shininess
	materialShininess[3][2][0] = 0.078125 * 128;

	// *******************************************************

	// ***** 4th sphere on 4th column, red *****
	// ambient material
	materialAmbient[3][3][0] = 0.05; // r
	materialAmbient[3][3][1] = 0.0;  // g
	materialAmbient[3][3][2] = 0.0;  // b
	materialAmbient[3][3][3] = 1.0f; // a

	// diffuse material
	materialDiffuse[3][3][0] = 0.5;  // r
	materialDiffuse[3][3][1] = 0.4;  // g
	materialDiffuse[3][3][2] = 0.4;  // b
	materialDiffuse[3][3][3] = 1.0f; // a

	// specular material
	materialSpecular[3][3][0] = 0.7;  // r
	materialSpecular[3][3][1] = 0.04; // g
	materialSpecular[3][3][2] = 0.04; // b
	materialSpecular[3][3][3] = 1.0f; // a

	// shininess
	materialShininess[3][3][0] = 0.078125 * 128;

	// *******************************************************

	// ***** 5th sphere on 4th column, white *****
	// ambient material
	materialAmbient[3][4][0] = 0.05; // r
	materialAmbient[3][4][1] = 0.05; // g
	materialAmbient[3][4][2] = 0.05; // b
	materialAmbient[3][4][3] = 1.0f; // a

	// diffuse material
	materialDiffuse[3][4][0] = 0.5;  // r
	materialDiffuse[3][4][1] = 0.5;  // g
	materialDiffuse[3][4][2] = 0.5;  // b
	materialDiffuse[3][4][3] = 1.0f; // a

	// specular material
	materialSpecular[3][4][0] = 0.7;  // r
	materialSpecular[3][4][1] = 0.7;  // g
	materialSpecular[3][4][2] = 0.7;  // b
	materialSpecular[3][4][3] = 1.0f; // a

	// shininess
	materialShininess[3][4][0] = 0.078125 * 128;
	
	// *******************************************************

	// ***** 6th sphere on 4th column, yellow rubber *****
	// ambient material
	materialAmbient[3][5][0] = 0.05; // r
	materialAmbient[3][5][1] = 0.05; // g
	materialAmbient[3][5][2] = 0.0;  // b
	materialAmbient[3][5][3] = 1.0f; // a

	// diffuse material
	materialDiffuse[3][5][0] = 0.5;  // r
	materialDiffuse[3][5][1] = 0.5;  // g
	materialDiffuse[3][5][2] = 0.4;  // b
	materialDiffuse[3][5][3] = 1.0f; // a

	// specular material
	materialSpecular[3][5][0] = 0.7;  // r
	materialSpecular[3][5][1] = 0.7;  // g
	materialSpecular[3][5][2] = 0.04; // b
	materialSpecular[3][5][3] = 1.0f; // a


	// shininess
	materialShininess[3][5][0] = 0.078125 * 128;

}

