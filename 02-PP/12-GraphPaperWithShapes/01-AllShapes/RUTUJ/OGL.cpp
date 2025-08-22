// header files
#include<windows.h>
#include<stdio.h> // for File I/O functions
#include<stdlib.h> // for exit()
#include"OGL.h"

//OpenGL HeaderFiles
#include<GL/glew.h> //THIS MUST BE ABOVE/INCLUDED BEFORE gl.h
#include<GL/gl.h>
#include"vmath.h"
using namespace vmath;


// Defining Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//OpenGL Libraries
#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"OpenGL32.lib")


// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variable declarations
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC hglrc = NULL;
BOOL gbFullScreen = FALSE;
FILE* gpFile = NULL;
BOOL gbActiveWindow = FALSE;

// Programmable Pipeline Related Variables
GLuint shaderProgramObject;
enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXTURE0
};
GLuint vao_GraphLines;
GLuint vbo_Lines_position;
GLuint vbo_Lines_color;
GLuint mvpMatrixUniform;
mat4 perspectiveProjectionMatrix;

// Graph Related
BOOL gbGraph = FALSE;
BOOL gbSquare = FALSE;
BOOL gbCircle = FALSE;
BOOL gbTriangle = FALSE;
GLfloat linesColor[6];
GLfloat squareColor[] =
{
	1.0f,1.0f,0.0f,
	1.0f,1.0f,0.0f,
	1.0f,1.0f,0.0f,
	1.0f,1.0f,0.0f,
};
GLfloat triangleColor[] =
{
	1.0f,1.0f,0.0f,
	1.0f,1.0f,0.0f,
	1.0f,1.0f,0.0f,
};
GLfloat circleColor[] = { 1.0f,1.0f,0.0f };
float x3 = (-1.0f) + 2.0f * cosf((M_PI / 180.0f) * 60.0f), y3 = (-1.0f) + 2.0f * sinf((M_PI / 180.0f) * 60.0f);
float x_pos = 0.0f, y_pos = 0.0f;
float gRad = 0.0f;

// entry-Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpSZCmdLine, int iCmdShow) //lpSZCmdLine is like char *argv[] in main() of CUI programming
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
	TCHAR szAppName[] = TEXT("MyWindow");
	BOOL bDone = FALSE;
	MONITORINFO mi_Centre;
	int iRetVal = 0;

	// code
	if (fopen_s(&gpFile,"Log.txt","w") != 0)
	{
		MessageBox(NULL, TEXT("Creation of Log File Failed. Exitting !!!"), TEXT("File I/O ERROR"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log File Successfully created !!!\n");
	}


	// initialization of WNDCLASSEX structure
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

	mi_Centre.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(MonitorFromWindow(NULL, MONITORINFOF_PRIMARY), &mi_Centre);
	int x = (int)mi_Centre.rcMonitor.right - mi_Centre.rcMonitor.left;
	int y = (int)mi_Centre.rcMonitor.bottom - mi_Centre.rcMonitor.top;
	

	// registering above WNDCLASSEX
	RegisterClassEx(&wndclass);


	// create the window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName,
		TEXT("Rutuj Vijay Dhimate OGL Window"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		(x / 2) - (WIN_WIDTH / 2),
		(y / 2) - (WIN_HEIGHT / 2),
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd; // assigning our hwnd to the globally ghwnd.

	// Initialize
	iRetVal = initialize();
	if (iRetVal == -1)
	{
		fprintf(gpFile, "ChoosePixelFormat() Failed!!!\n\n");
		uninitialize();
	}

	else if (iRetVal == -2)
	{
		fprintf(gpFile, "SetPixelFormat() Failed!!!\n\n");
		uninitialize();
	}

	else if (iRetVal == -3)
	{
		fprintf(gpFile, "CreateOpenGLContext() Failed!!!\n\n");
		uninitialize();
	}

	else if(iRetVal == -4)
	{
		fprintf(gpFile, "Making OpenGL context as current Context Failed!!!\n\n");
		uninitialize();
	}
	else if (iRetVal == -5)
	{
		fprintf(gpFile, "glewInit() Failed!!!\n\n");
		uninitialize();
	}
	else
	{
		fprintf(gpFile, "Making OpenGL context as current Context.successFull!!!\n\n");
	}

	// show the window
	ShowWindow(hwnd, iCmdShow);

	// Foregrounding and Focusing the window
	SetForegroundWindow(hwnd);  // Win32 API
	SetFocus(hwnd);  // // Win32 API


	//Game Loop
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
				// Render the scene
				display();

				// Update the scene
				update();
				
			}
		}
	}

	uninitialize();
	
	return (int)msg.wParam;
}

// callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// function declarations
	void ToggleFullScreen(void);
	void resize(int, int);
	

	// code
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
		case 'F':
		case 'f':
			ToggleFullScreen();
			break;

		case 'G':
		case 'g':
			if (gbGraph == FALSE)
				gbGraph = TRUE;
			else
				gbGraph = FALSE;
			break;

		case 'S':
		case 's':
			if (gbSquare == FALSE)
				gbSquare = TRUE;
			else
				gbSquare = FALSE;
			break;

		case 'C':
		case 'c':
			if (gbCircle == FALSE)
				gbCircle = TRUE;
			else
				gbCircle = FALSE;
			break;

		case 'T':
		case 't':
			if (gbTriangle == FALSE)
				gbTriangle = TRUE;
			else
				gbTriangle = FALSE;
			break;

		default:
			break;
		}
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

	return DefWindowProc(hwnd, iMsg, wParam, lParam); // Default Window Procedure()
}

void ToggleFullScreen(void)
{
	// variable declarations
	static DWORD dwStyle;
	static WINDOWPLACEMENT wp;
	MONITORINFO mi;
	
	
	// code
	wp.length = sizeof(WINDOWPLACEMENT);

	if (gbFullScreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);  //Algorithm Step 1.a)
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(MONITORINFO);


			if (GetWindowPlacement(ghwnd, &wp) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi)) // Step 1.b),1.c)
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW); // Step1.d)
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED); // Step1.e)
			}

			ShowCursor(FALSE); // // Step1.f)
			gbFullScreen = TRUE;
		}
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW); // Step2.a)
		SetWindowPlacement(ghwnd, &wp);// Step2.b)
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);// Step2.c)

		ShowCursor(TRUE);// Step2.d)
		gbFullScreen = FALSE;// Step2.e)
	}
}

int initialize(void)
{
	// function declarations
	void resize(int, int);
	void printGLInfo(void);
	void uninitialize(void);

	// variable declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;
	GLint status;
	GLint infoLogLength;
	char* log = NULL;

	// code

	//Initialization of PIXELFORMATDESCRIPTOR STRUCTURE
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
	pfd.cDepthBits = 32; // 24 also can be done.
	
	// GetDC()
	ghdc = GetDC(ghwnd);

	// ChoosePixelFormat()
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
	{
		return -1;
	}

	// Set the Chosen Pixel Format
	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		return -2;
	}

	//Create OpenGL Rendering Context
	hglrc = wglCreateContext(ghdc);
	if (hglrc == NULL)
	{
		return -3;
	}
	// Make the rendering context as current context
	
	if (wglMakeCurrent(ghdc, hglrc) == FALSE)
	{
		return -4;
	}

	// Here starts OpenGL code
	//GLEW Initialization
	if (glewInit() != GLEW_OK)
	{
		return -5;
	}

	// printf OpenGl Info
	printGLInfo();

	//---------------VERTEX SHADER----------------------//
	//Vertex Shader Source Code
	const GLchar* vertexShaderSourceCode =
		"#version 460 core" \
		"\n" \
		"in vec4 a_position;" \
		"in vec4 a_color;" \
		"out vec4 a_color_out;" \
		"uniform mat4  u_mvpMatrix;" \
		"void main(void)" \
		"{" \
		"gl_Position = u_mvpMatrix * a_position;" \
		"a_color_out = a_color;" \
		"}";

	//Creating Shader object
	GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	//Giving Shader Code to Shader Object
	glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);

	//Compiling the Shader
	glCompileShader(vertexShaderObject);

	//Error Checking for Vertex Shader
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
				fprintf(gpFile, "\nVertex Shader Compilation Log: %s\n", log);
				free(log);
				uninitialize();
			}
		}
	}

	//---------------FRAGMENT SHADER----------------------//
	// Fragment Shader Source Code
	const GLchar* fragmentShaderSourceCode =
		"#version 460 core" \
		"\n" \
		"in vec4 a_color_out;" \
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"FragColor = a_color_out;" \
		"}";

	//Creating Shader object
	GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	//Giving Shader Code to Shader Object
	glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);

	//Compiling the Shader
	glCompileShader(fragmentShaderObject);

	//Error Checking for Fragment Shader
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
				fprintf(gpFile, "Fragment Shader Compilation Log: %s\n", log);
				free(log);
				uninitialize();
			}
		}

	}

	// Shader Program Object
	shaderProgramObject = glCreateProgram();

	glAttachShader(shaderProgramObject, vertexShaderObject);
	glAttachShader(shaderProgramObject, fragmentShaderObject);

	//Pre-Link Binding
	glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "a_position");
	glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_COLOR, "a_color");

	glLinkProgram(shaderProgramObject);

	//Error Checking for Shader Program
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
				fprintf(gpFile, "Shader Program Link Log: %s\n", log);
				free(log);
				uninitialize();
			}
		}
	}

	//Post Link
	mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_mvpMatrix");

	// Graph related initialization
	gRad = ((2.0f) * (2.0f) * (2.0f)) / (4.0f * pow(3.0f, 0.5f)); //radius of circumcenter
	y_pos = y3 - gRad;

	//Declaration/Initialization of Vertex Data Arrays
	GLfloat linesColor[] =
	{
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,	
	};
	// LINES
	// VAO
	glGenVertexArrays(1, &vao_GraphLines);
	glBindVertexArray(vao_GraphLines);

	// VBO for Position
	glGenBuffers(1, &vbo_Lines_position);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_position);
	glBufferData(GL_ARRAY_BUFFER, 2 * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// VBO for Color
	glGenBuffers(1, &vbo_Lines_color);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(linesColor), linesColor, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// Unbind VAO(for LINES)
	glBindVertexArray(0);

	// Depth Related changes
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//Clear the screen using blue color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	perspectiveProjectionMatrix = mat4::identity();

	// warm up resize call
	resize(WIN_WIDTH, WIN_HEIGHT);

	return 0;
}

void printGLInfo(void)
{
	//local variable declarations
	GLint numExtensions = 0;
	
	// code
	fprintf(gpFile, "OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
	fprintf(gpFile, "OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(gpFile, "OpenGL Version: %s\n", glGetString(GL_VERSION));
	fprintf(gpFile, "GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

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
	if (height == 0) // To avoid Divided by zero condition(Illegal statement) which may arive in future code.
	{
		height = 1;
	}

	glViewport(0, 0, GLsizei(width), GLsizei(height));
	perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void display(void)
{
	// code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the Shader Program Object
	glUseProgram(shaderProgramObject);

	//Transformations
	mat4 translationMatrix = mat4::identity();
	mat4 modelViewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix = mat4::identity();

	translationMatrix = vmath::translate(0.0f, 0.0f, -10.5f);
	modelViewMatrix = translationMatrix;
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	glBindVertexArray(vao_GraphLines);
	GLfloat linesVertices[6];
	GLfloat squareVertices[12];
	GLfloat triangleVertices[9];
	//Here there should be the drawing/Animation/graphics Code
	if (gbGraph == TRUE)
	{
		glLineWidth(0.01f);
		for (float i = 0.15f; i <= 3.15f; i += 0.15f)
		{
			linesColor[0] = 0.0f;
			linesColor[1] = 0.0f;
			linesColor[2] = 1.0f;
			linesColor[3] = 0.0f;
			linesColor[4] = 0.0f;
			linesColor[5] = 1.0f;
			linesVertices[0] = -4.0f;
			linesVertices[1] = y_pos + i;
			linesVertices[2] = 0.0f;
			linesVertices[3] = 4.0f;
			linesVertices[4] = y_pos + i;
			linesVertices[5] = 0.0f;
			glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_color);
			glBufferData(GL_ARRAY_BUFFER, sizeof(linesColor), linesColor, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_position);
			glBufferData(GL_ARRAY_BUFFER, sizeof(linesVertices), linesVertices, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_LINE_LOOP, 0, 2);

			linesVertices[1] = y_pos - i;
			linesVertices[4] = y_pos - i;
			glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_position);
			glBufferData(GL_ARRAY_BUFFER, sizeof(linesVertices), linesVertices, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_LINE_LOOP, 0, 2);
		}

		for (float i = 0.2f; i <= 4.2f; i += 0.2f)
		{
			linesVertices[0] = i;
			linesVertices[1] = y_pos - 3.0f;
			linesVertices[2] = 0.0f;
			linesVertices[3] = i;
			linesVertices[4] = y_pos + 3.0f;
			linesVertices[5] = 0.0f;
			glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_position);
			glBufferData(GL_ARRAY_BUFFER, sizeof(linesVertices), linesVertices, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_LINE_LOOP, 0, 2);

			linesVertices[0] = -i;
			linesVertices[3] = -i;
			glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_position);
			glBufferData(GL_ARRAY_BUFFER, sizeof(linesVertices), linesVertices, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_LINE_LOOP, 0, 2);
		}

		//Red Line
		glLineWidth(3.0f);
		linesColor[0] = 1.0f;
		linesColor[1] = 0.0f;
		linesColor[2] = 0.0f;
		linesColor[3] = 1.0f;
		linesColor[4] = 0.0f;
		linesColor[5] = 0.0f;
		linesVertices[0] = -4.0f;
		linesVertices[1] = y_pos;
		linesVertices[2] = 0.0f;
		linesVertices[3] = 4.0f;
		linesVertices[4] = y_pos;
		linesVertices[5] = 0.0f;
		glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_position);
		glBufferData(GL_ARRAY_BUFFER, sizeof(linesVertices), linesVertices, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_color);
		glBufferData(GL_ARRAY_BUFFER, sizeof(linesColor), linesColor, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_LINE_LOOP, 0, 2);

		// Green Line
		linesColor[0] = 0.0f;
		linesColor[1] = 1.0f;
		linesColor[2] = 0.0f;
		linesColor[3] = 0.0f;
		linesColor[4] = 1.0f;
		linesColor[5] = 0.0f;
		linesVertices[0] = 0.0f;
		linesVertices[1] = y_pos + 3.0f;
		linesVertices[2] = 0.0f;
		linesVertices[3] = 0.0f;
		linesVertices[4] = y_pos - 3.0f;
		linesVertices[5] = 0.0f;
		glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_position);
		glBufferData(GL_ARRAY_BUFFER, sizeof(linesVertices), linesVertices, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_color);
		glBufferData(GL_ARRAY_BUFFER, sizeof(linesColor), linesColor, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_LINE_LOOP, 0, 2);

		//Center Point
		glPointSize(6.0f);
		linesColor[0] = 1.0f;
		linesColor[1] = 1.0f;
		linesColor[2] = 0.0f;
		linesVertices[0] = x_pos;
		linesVertices[1] = y_pos;
		linesVertices[2] = 0.0f;
		glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_position);
		glBufferData(GL_ARRAY_BUFFER, sizeof(linesVertices), linesVertices, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_color);
		glBufferData(GL_ARRAY_BUFFER, sizeof(linesColor), linesColor, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_POINTS, 0, 1);
	}

	if (gbSquare == TRUE)
	{
		glLineWidth(3.0f);
		squareVertices[0] = gRad;
		squareVertices[1] = y_pos + gRad;
		squareVertices[2] = 0.0f;
		squareVertices[3] = -gRad;
		squareVertices[4] = y_pos + gRad;
		squareVertices[5] = 0.0f;
		squareVertices[6] = -gRad;
		squareVertices[7] = y_pos - gRad;
		squareVertices[8] = 0.0f;
		squareVertices[9] = gRad;
		squareVertices[10] = y_pos - gRad;
		squareVertices[11] = 0.0f;
		glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_position);
		glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_color);
		glBufferData(GL_ARRAY_BUFFER, sizeof(squareColor), squareColor, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_LINE_LOOP, 0, 4);
	}

	if (gbCircle == TRUE)
	{
		float angleCircle = 0;
		glPointSize(3.0f);
		for (float deg = 0; deg <= 360; deg += 1.0f)
		{
			angleCircle = (M_PI / 180.0f) * deg;
			linesVertices[0] = x_pos + gRad * cosf(angleCircle);
			linesVertices[1] = y_pos + gRad * sinf(angleCircle);
			linesVertices[2] = 0.0f;
			glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_position);
			glBufferData(GL_ARRAY_BUFFER, sizeof(linesVertices), linesVertices, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_color);
			glBufferData(GL_ARRAY_BUFFER, sizeof(circleColor), circleColor, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_POINTS, 0, 1);
		}
	}

	if (gbTriangle == TRUE)
	{
		glLineWidth(3.0f);
//		glVertex3f((-1.0f) + 2.0f * cosf((M_PI / 180.0f) * 60.0f), (-1.0f) + 2.0f * sinf((M_PI / 180.0f) * 60.0f), 0.0f);
	//	glVertex3f(-1.0f, -1.0f, 0.0f);
		//glVertex3f(1.0f, -1.0f, 0.0f);
		triangleVertices[0] = (-1.0f) + 2.0f * cosf((M_PI / 180.0f) * 60.0f);
		triangleVertices[1] = (-1.0f) + 2.0f * sinf((M_PI / 180.0f) * 60.0f);
		triangleVertices[2] = 0.0f;
		triangleVertices[3] = -1.0f;
		triangleVertices[4] = -1.0f;
		triangleVertices[5] = 0.0f;
		triangleVertices[6] = 1.0f;
		triangleVertices[7] = -1.0f;
		triangleVertices[8] = 0.0f;
		glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_position);
		glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_Lines_color);
		glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColor), triangleColor, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_LINE_LOOP, 0, 3);
	}

	glBindVertexArray(0);

	//Unuse the Shader Program Object
	glUseProgram(0);

	SwapBuffers(ghdc);
}

void update(void)
{
	// code
}

void uninitialize(void)
{
	// function declarations
	void ToggleFullScreen(void);

	// code
	if (gbFullScreen)  // if the window already is in fullscreen mode,then make it a normal window (i.e not a fullscreen) before closing/destroying window.(this occurs when ESC is pressed as fullscreen window can be terminated only by pressing ESC)  
	{
		ToggleFullScreen();
	}

	// Deletion and Uninitialization of vbo_Lines_position
	if (vbo_Lines_color)
	{
		glDeleteBuffers(1, &vbo_Lines_color);
		vbo_Lines_color = 0;
	}

	if (vbo_Lines_position)
	{
		glDeleteBuffers(1, &vbo_Lines_position);
		vbo_Lines_position = 0;
	}

	// Deletion and Uninitialization of vao_GraphLines
	if (vao_GraphLines)
	{
		glDeleteVertexArrays(1, &vao_GraphLines);
		vao_GraphLines = 0;
	}

	// Shader Uninitialization
	if (shaderProgramObject)
	{
		glUseProgram(shaderProgramObject);

		GLsizei numAttachedShaders;
		glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numAttachedShaders);

		GLuint* shaderObjects = NULL;
		shaderObjects = (GLuint*)malloc(numAttachedShaders);
		glGetAttachedShaders(shaderProgramObject, numAttachedShaders, &numAttachedShaders, shaderObjects);

		for (GLsizei i = 0; i < numAttachedShaders; i++)
		{
			glDetachShader(shaderProgramObject, shaderObjects[i]);
			glDeleteShader(shaderObjects[i]);
			shaderObjects[i] = 0;
		}

		free(shaderObjects);
		glUseProgram(0);
		glDeleteProgram(shaderProgramObject);
		shaderProgramObject = 0;
	}

	if (wglGetCurrentContext() == hglrc)
	{
		wglMakeCurrent(NULL, NULL);
	}

	if (hglrc)
	{
		wglDeleteContext(hglrc);
		hglrc = NULL;
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
		fprintf(gpFile, "Log File Successfully closed !!!\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}
