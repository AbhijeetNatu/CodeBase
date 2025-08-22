// Header files
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "OGL.h"

//OpenGL Header files
#include <GL/glew.h>		//This MUST be above gl.h
#include <GL/gl.h>

#include "vmath.h"
using namespace vmath;		

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//OpenGL Libraries
#pragma comment(lib,"glew32.lib")
#pragma comment(lib, "OpenGL32.lib")

//Global Function declarations
LRESULT CALLBACK WndProc(HWND , UINT , WPARAM , LPARAM);


//Global Variables Declarations
HWND ghwnd = NULL;      //g for global
HDC ghdc = NULL;
HGLRC ghrc = NULL;          //Handle OpenGL Rendering Context
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE; //By default, our window is not active
FILE* gpFile = NULL;    //File is type *pointer, 
//gp = Global Pointer gpFile is Pointer Variable of type FILE

//Programmable Pipeline related global variables
GLuint shaderProgramObject;


enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXTURE0
};

GLuint vao_pyramid;
GLuint vbo_pyramid_position;
GLuint vbo_normal;

GLfloat anglePyramid = 0.0f;

GLuint mvpMatrixUniform;

mat4 perspectiveProjectionMatrix;		

GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint projectionMatrixUniform;

GLuint laUniform[2];
GLuint ldUniform[2];
GLuint lsUniform[2];
GLuint lightPositionUniform[2];

GLuint kaUniform;
GLuint kdUniform;
GLuint ksUniform;
GLuint materialShininessUniform;

GLuint lightingEnabledUniform;

BOOL bLight = FALSE;

struct Light 
{
	vec4 lightAmbient;
	vec4 lightDiffuse;
	vec4 lightSpecular;
	vec4 lightPosition;
};

Light lights[2];

GLfloat materialAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat materialDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat materialSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat materialShininess = 50.0f;


//Entry Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//function declarations
	int initialize(void);
	void display(void);
	void update(void);
	void uninitialize(void);

	//variable declarations

	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyWindow");
	BOOL bDone = FALSE;
	int iRetVal = 0;

	//code

	if (fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Creation of Log File Failed. Exitting...!!\n"), TEXT("File I/O Error"), MB_OK);
		exit(0);
	}

	else          //File opened Successfully
	{
		fprintf(gpFile, "Log File Successfully Created !!\n");
	}

	//initialization of WNDCLASSEX structure
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	//Registering above wndClass
	
	//Register WNDCLASSEX

	RegisterClassEx(&wndclass);

	//Get Screen Coordinates
	int ScreenX = GetSystemMetrics(SM_CXSCREEN);
	int ScreenY = GetSystemMetrics(SM_CYSCREEN);

	//create the window

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("Abhijeet Natu : TWO LIGHTS ON SPINNING PYRAMID"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		(ScreenX / 2) - (WIN_WIDTH / 2),
		(ScreenY / 2) - (WIN_HEIGHT / 2),
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	//initialize

	iRetVal = initialize();

	if (iRetVal == -1)
	{
		fprintf(gpFile, "Choose Pixel Format Failed\n");
		uninitialize();
	}

	else if (iRetVal == -2)
	{
		fprintf(gpFile, "Set Pixel Format Failed\n");
		uninitialize();
	}

	else if (iRetVal == -3)
	{
		fprintf(gpFile, "Create OpenGL context Failed\n");
		uninitialize();
	}

	else if (iRetVal == -4)
	{
		fprintf(gpFile, "Making OpenGL context as current context Failed\n");
		uninitialize();
	}

	else if (iRetVal == -5)
	{
		fprintf(gpFile, "GLEW Initialization Failed\n");
		uninitialize();
	}

	else
	{
		fprintf(gpFile, "Making OpenGL context as current context Successful\n");
	}

	//show window

	ShowWindow(hwnd, iCmdShow);

	//Foregrounding and Focusing the Window
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//Message Loop or Game Loop

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
				//Render the scene
				display();

				//Update the scene
				update();
			}
		}
	}
	uninitialize();
	return((int)msg.wParam);
}

	//CallBack function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//Local Function Declaration
	void ToggleFullScreen(void);
	void resize(int, int);
	

	//code
	switch (iMsg)
	{
	case WM_CREATE: 
					break;

	case WM_KEYDOWN:
		switch (wParam)
		{
			case 27:				
				DestroyWindow(hwnd);
				break;

			default:
				break;
		}
		break;
	
	case WM_SIZE:
		resize(LOWORD(lParam),HIWORD(lParam));
		break;

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

		default:
			break;
		}
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

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}


void ToggleFullScreen(void)
{
	//variable declarations
	static DWORD dwStyle;
	static WINDOWPLACEMENT wp;
	MONITORINFO	mi;

	//code
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
	//Function Declarations
	void resize(int,int);
	void printGLInfo(void);
	void uninitialize(void);


	//Variable Declarations
	PIXELFORMATDESCRIPTOR pfd;
	int	iPixelFormatIndex;

	//code
	
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	//initialization of PIXELFORMATDESCRIPTOR structure 
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

	//GetDC
	ghdc = GetDC(ghwnd);

	//Choose Pixel Format
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);

		if (iPixelFormatIndex == 0)
		{
			return(-1);
		}
		//Set the chosen pixel format
		if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
		{
			return(-2);
		}
		
		//Create OpenGL Rendering Context
		ghrc = wglCreateContext(ghdc);

		if (ghrc == NULL)
		{
			return(-3);
		}

		//make the rendering context as the current context
		if (wglMakeCurrent(ghdc, ghrc) == FALSE)
		{
			return(-4);
		}


		//GLEW Initialization

		if (glewInit() != GLEW_OK)
		{
			return(-5);
		}

		//Print OpenGL Info
		printGLInfo();


		//VERTEX SHADER

		//Step 1
		const GLchar* vertexShaderSourceCode =
			"#version 460 core" \
			"\n" \
			"in vec4 a_position;" \
			"in vec3 a_normal;" \
			"uniform mat4 u_modelMatrix;" \
			"uniform mat4 u_viewMatrix;" \
			"uniform mat4 u_projectionMatrix;" \
			"uniform vec3 u_la[2];" \
			"uniform vec3 u_ld[2];" \
			"uniform vec3 u_ls[2];" \
			"uniform vec4 u_lightPosition[2];" \
			"uniform vec3 u_ka;" \
			"uniform vec3 u_kd;" \
			"uniform vec3 u_ks;" \
			"uniform float u_materialShininess;" \
			"uniform int u_lightingEnabled;" \
			"out vec3 phong_ads_light;" \
			"void main(void)" \
			"{" \
			"if(u_lightingEnabled == 1)" \
			"{" \
			"vec4 eyeCoordinates = u_viewMatrix * u_modelMatrix * a_position;" \
			"mat3 normalMatrix = mat3(u_viewMatrix * u_modelMatrix);" \
			"vec3 transformedNormals = normalize(normalMatrix * a_normal);" \
			"vec3 viewerVector = normalize(-eyeCoordinates.xyz);" \
			"vec3 ambient[2];" \
			"vec3 lightDirection[2];" \
			"vec3 diffuse[2];" \
			"vec3 reflectionVector[2];" \
			"vec3 specular[2];" \
			"for(int i =0;i<2;i++)" \
			"{" \
			"ambient[i] = u_la[i] * u_ka;" \
			"lightDirection[i] = normalize(vec3(u_lightPosition[i]) - eyeCoordinates.xyz);" \
			"diffuse[i] = u_ld[i] * u_kd * max(dot(lightDirection[i], transformedNormals),0.0);" \
			"reflectionVector[i] = reflect(-lightDirection[i],transformedNormals);" \
			"specular[i] = u_ls[i] * u_ks * pow(max(dot(reflectionVector[i],viewerVector),0.0),u_materialShininess);" \
			"phong_ads_light = phong_ads_light + ambient[i] + diffuse[i] + specular[i];" \
			"};" \
			"}" \
			"else" \
			"{" \
			"phong_ads_light = vec3(1.0f,1.0f,1.0f);" \
			"}" \
			"gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" \
			"}";

		//Step 2
		GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

		//Step 3
		glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);

		//Step 4
		glCompileShader(vertexShaderObject);

		//Step 5 : Only if error
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
					glGetShaderInfoLog(vertexShaderObject, GL_INFO_LOG_LENGTH, &written,log);
					fprintf(gpFile, "Vertex Shader Compilation Log:  %s\n", log);
					free(log);
					uninitialize();
				}
			}	
		}


		//FRAGMENT SHADER

		//Step 1
		const GLchar* fragmentShaderSourceCode =
			"#version 460 core" \
			"\n" \
			"in vec3 phong_ads_light;" \
			"out vec4 FragColor;" \
			"void main(void)" \
			"{" \
			"FragColor = vec4(phong_ads_light, 1.0);" \
			"}";

		//Step 2
		GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

		//Step 3
		glShaderSource(fragmentShaderObject,1, (const GLchar**)&fragmentShaderSourceCode, NULL);

		//Step 4
		glCompileShader(fragmentShaderObject);

		//Step 5
		status = 0;
		infoLogLength = 0;
		log = NULL;				//Re initialize all 3 variables

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
					glGetShaderInfoLog(fragmentShaderObject, GL_INFO_LOG_LENGTH, &written, log);
					fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", log);
					free(log);
					uninitialize();
				}
			}
		}

		//Shader Program Object
		shaderProgramObject = glCreateProgram();

		glAttachShader(shaderProgramObject,vertexShaderObject);
		glAttachShader(shaderProgramObject, fragmentShaderObject);

		glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION,"a_position");
		glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "a_normal");		//chukla tar andhaar FIX !!!

		glLinkProgram(shaderProgramObject);

		modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_modelMatrix");
		viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_viewMatrix");
		projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");

		laUniform[0] = glGetUniformLocation(shaderProgramObject, "u_la[0]");
		ldUniform[0] = glGetUniformLocation(shaderProgramObject, "u_ld[0]");
		lsUniform[0] = glGetUniformLocation(shaderProgramObject, "u_ls[0]");
		lightPositionUniform[0] = glGetUniformLocation(shaderProgramObject, "u_lightPosition[0]");

		laUniform[1] = glGetUniformLocation(shaderProgramObject, "u_la[1]");
		ldUniform[1] = glGetUniformLocation(shaderProgramObject, "u_ld[1]");
		lsUniform[1] = glGetUniformLocation(shaderProgramObject, "u_ls[1]");
		lightPositionUniform[1] = glGetUniformLocation(shaderProgramObject, "u_lightPosition[1]");

		kaUniform = glGetUniformLocation(shaderProgramObject, "u_ka");
		kdUniform = glGetUniformLocation(shaderProgramObject, "u_kd");
		ksUniform = glGetUniformLocation(shaderProgramObject, "u_ks");
		materialShininessUniform = glGetUniformLocation(shaderProgramObject, "u_materialShininess");
		
		lightingEnabledUniform = glGetUniformLocation(shaderProgramObject, "u_lightingEnabled");


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
					glGetProgramInfoLog(shaderProgramObject, GL_INFO_LOG_LENGTH ,& written, log);
					fprintf(gpFile,"Shader Program Link Log : %s\n", log);
					free(log);
					uninitialize();
				}
			}
		}
		
		//Declaration &initialization of Vertex Data Arrays
		const GLfloat pyramidPosition[] =
		{
			// front
			0.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, 1.0f,

				// right
			0.0f, 1.0f, 0.0f,
			1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, -1.0f,

			// back
			0.0f, 1.0f, 0.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,

			// left
			0.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, 1.0f

		};

		const GLfloat pyramidNormals[] =
		{
			0.0f, 0.447214f, 0.894427f,// front-top
			0.0f, 0.447214f, 0.894427f,// front-left
			0.0f, 0.447214f, 0.894427f,// front-right

			0.894427f, 0.447214f, 0.0f, // right-top
			0.894427f, 0.447214f, 0.0f, // right-left
			0.894427f, 0.447214f, 0.0f, // right-right

			0.0f, 0.447214f, -0.894427f, // back-top
			0.0f, 0.447214f, -0.894427f, // back-left
			0.0f, 0.447214f, -0.894427f, // back-right

			-0.894427f, 0.447214f, 0.0f, // left-top
			-0.894427f, 0.447214f, 0.0f, // left-left
			-0.894427f, 0.447214f, 0.0f // left-right

		};

		const GLfloat pyramidColor[] =
		{ 
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,

			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,

			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,

			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f

		};

	
		//vao and vbo related code
		
		//pyramid

		//vao
		glGenVertexArrays(1, &vao_pyramid);
		glBindVertexArray(vao_pyramid);

		//vbo for position
		glGenBuffers(1, &vbo_pyramid_position);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid_position);
		glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidPosition), pyramidPosition, GL_STATIC_DRAW);
		glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);		//x,y,z
		glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
		glBindBuffer(GL_ARRAY_BUFFER, 0);	

		//vbo for normals
		glGenBuffers(1, &vbo_normal);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
		glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidNormals), pyramidNormals, GL_STATIC_DRAW);
		glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);		//x,y,z
		glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);					//UNBIND (vao)

		//Depth Related code
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		//clear the screen using Black color
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		lights[0].lightAmbient = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		lights[0].lightDiffuse = vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f);	//RED
		lights[0].lightSpecular = vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		lights[0].lightPosition = vmath::vec4(-2.0f, 0.0f, 0.0f, 1.0f);

		lights[1].lightAmbient = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		lights[1].lightDiffuse = vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f);	//BLUE
		lights[1].lightSpecular = vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		lights[1].lightPosition = vmath::vec4(2.0f, 0.0f, 0.0f, 1.0f);


		perspectiveProjectionMatrix = mat4::identity();

		resize(WIN_WIDTH, WIN_HEIGHT);

	return 0;

}

void printGLInfo(void)
{
	//Local Variable declarations
	GLint numExtensions = 0;

	//code
	fprintf(gpFile, "OpenGL Vendor   : %s\n", glGetString(GL_VENDOR));
	fprintf(gpFile, "OpenGL Renderer : %s\n", glGetString(GL_RENDERER));
	fprintf(gpFile, "OpenGL Version : %s\n", glGetString(GL_VERSION));
	fprintf(gpFile, "GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

	fprintf(gpFile, "Number of Supported Extensions : %d\n", numExtensions);

	for (int i = 0; i < numExtensions; i++)
	{
		fprintf(gpFile, "%s\n", glGetStringi(GL_EXTENSIONS,i));
	}

}

void resize(int width, int height)
{
	//code
	if (height == 0)
		height = 1;      //To avoid Divided by 0 illegal statement for future code

	glViewport(0, 0, GLsizei(width), GLsizei(height));
	
	perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Use the shader program object
	glUseProgram(shaderProgramObject);

	// " PYRAMID "
	
	//Transformations
	mat4 translationMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();
	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();

	mat4 modelViewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix = mat4::identity();

	translationMatrix = translate(0.0f, 0.0f, -5.0f);
	rotationMatrix = rotate(anglePyramid, 0.0f, 1.0f, 0.0f);	//Spinning

	modelMatrix = translationMatrix * rotationMatrix;

	glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	if (bLight == TRUE)
	{
		glUniform1i(lightingEnabledUniform, 1);

		for (int i = 0; i < 2; i++)
		{
			glUniform3fv(laUniform[i], 1, lights[i].lightAmbient);
			glUniform3fv(ldUniform[i], 1, lights[i].lightDiffuse);
			glUniform3fv(lsUniform[i], 1, lights[i].lightSpecular);
			glUniform4fv(lightPositionUniform[i], 1, lights[i].lightPosition);
		}

		glUniform3fv(kaUniform, 1, materialAmbient);
		glUniform3fv(kdUniform, 1, materialDiffuse);
		glUniform3fv(ksUniform, 1, materialSpecular);
		glUniform1f(materialShininessUniform, materialShininess);

	}

	else
	{
		glUniform1i(lightingEnabledUniform, 0);
	}

	glBindVertexArray(vao_pyramid);

	//Here is the drawing code
	glDrawArrays(GL_TRIANGLES, 0, 12);

	glBindVertexArray(0);

	//Unuse the shader program object
	glUseProgram(0);

	SwapBuffers(ghdc);
}

void update(void)
{
	//code
	anglePyramid = anglePyramid + 1.0f;
	if (anglePyramid >= 360.0f)
		anglePyramid = anglePyramid - 360.0f;

}

void uninitialize(void)
{
	//function declarations
	void ToggleFullScreen(void);

	//code
	if (gbFullScreen)
	{
		ToggleFullScreen();
	}

	//Deletion and uninitialization of vbo

	if (vbo_pyramid_position)
	{
		glDeleteBuffers(1, &vbo_pyramid_position);
		vbo_pyramid_position = 0;
	}

	//Deletion and uninitialization of vao
	if (vao_pyramid)
	{
		glDeleteVertexArrays(1, &vao_pyramid);
		vao_pyramid = 0;
	}

	if (vbo_normal)
	{
		glDeleteBuffers(1, &vbo_normal);
		vbo_normal = 0;
	}

	//Shader Uninitialization
	if (shaderProgramObject)
	{
		glUseProgram(shaderProgramObject);

		GLsizei numAttachedShaders;			//Number of attached shaders
		glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numAttachedShaders);
		GLuint* Shader_Objects = NULL;

		Shader_Objects = (GLuint*)malloc(numAttachedShaders * sizeof(GLuint));

		glGetAttachedShaders(shaderProgramObject, numAttachedShaders, &numAttachedShaders,Shader_Objects);

		for (GLsizei i = 0;i<=numAttachedShaders; i++)
		{
			glDetachShader(shaderProgramObject, Shader_Objects[i]);
			glDeleteShader(Shader_Objects[i]);
			Shader_Objects[i] = 0;
		}

		free(Shader_Objects);
		Shader_Objects = NULL;
		glUseProgram(0);
		glDeleteProgram(shaderProgramObject);
		shaderProgramObject = 0;
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
		fprintf(gpFile, "Log File Successfully Closed !!\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}
