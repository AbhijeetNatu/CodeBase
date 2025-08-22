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

GLuint vao;
GLuint vbo_position;
GLuint vbo_texcoord;

#define CHECKERBOARD_WIDTH 64
#define CHECKERBOARD_HEIGHT 64

GLuint texture_checkerboard;
GLubyte checkerboard[CHECKERBOARD_WIDTH][CHECKERBOARD_HEIGHT][4];		//3D Array


GLuint mvpMatrixUniform;
GLuint textureSamplerUniform;

mat4 perspectiveProjectionMatrix;		//4x4 matrix


//Entry Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//function declarations
	int initialize(void);
	void display(void);
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
		TEXT("Abhijeet Natu : CHECKERBOARD"),
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

	else if (iRetVal == -6)
	{
		fprintf(gpFile, "Texture Loading for Smiley Failed\n");
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
	void LoadGLTexture(void);

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
			"in vec2 a_texcoord;" \
			"uniform mat4 u_mvpMatrix;" \
			"out vec2 a_texcoord_out;" \
			"void main(void)" \
			"{" \
			"gl_Position = u_mvpMatrix * a_position;" \
			"a_texcoord_out = a_texcoord;" \
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
			"in vec2 a_texcoord_out;" \
			"uniform sampler2D u_texturesampler;" \
			"out vec4 FragColor;" \
			"void main(void)" \
			"{" \
			"FragColor = texture(u_texturesampler,a_texcoord_out);" \
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

		glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION,"a_position");		//chukla tar andhaar FIX !!!
		glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TEXTURE0, "a_texcoord");			//chukla tar andhaar FIX !!!

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
					glGetProgramInfoLog(shaderProgramObject, GL_INFO_LOG_LENGTH ,& written, log);
					fprintf(gpFile,"Shader Program Link Log : %s\n", log);
					free(log);
					uninitialize();
				}
			}
		}
		mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_mvpMatrix");	//chukla tar andhaar

		textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "u_texturesampler");


		
		//Declaration &initialization of Vertex Data Arrays
		
		const GLfloat texcoord[] =
		{ 
			1.0f,1.0f,
			0.0f,1.0f,
			0.0f,0.0f,
			1.0f,0.0f

		};

		//vao and vbo related code
		
		
		//vao
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);


		//vbo for position
		glGenBuffers(1, &vbo_position);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
		glBufferData(GL_ARRAY_BUFFER, 4*3*sizeof(GL_FLOAT), NULL, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);		//x,y,z
		glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
		glBindBuffer(GL_ARRAY_BUFFER, 0);		//UNBIND vbo

		//vbo for texture
		glGenBuffers(1, &vbo_texcoord);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord);
		glBufferData(GL_ARRAY_BUFFER, sizeof(texcoord), texcoord, GL_STATIC_DRAW);
		glVertexAttribPointer(AMC_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL);		//r,g,b
		glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXTURE0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);		//UNBIND (vbo)

		glBindVertexArray(0);					//UNBIND (vao)

		//Depth Related code
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		//clear the screen using Gray color
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

		//Texture Related Changes

		LoadGLTexture();
		

		//Enabling the texture
		glEnable(GL_TEXTURE_2D);

		perspectiveProjectionMatrix = mat4::identity();

		resize(WIN_WIDTH, WIN_HEIGHT);

	return 0;

}


void LoadGLTexture(void)
{
	//Function declarations
	void MakeCheckerboard(void);

	//code
	MakeCheckerboard();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &texture_checkerboard);
	glBindTexture(GL_TEXTURE_2D, texture_checkerboard);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);		//for pattern repeat 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);		//for pattern repeat 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//Create Texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHECKERBOARD_WIDTH, CHECKERBOARD_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkerboard);

	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glBindTexture(GL_TEXTURE_2D, 0);

}

void MakeCheckerboard(void)
{
	//Variable declarations
	int c;

	//code
	for (int i = 0; i < CHECKERBOARD_WIDTH; i++)
	{
		for (int j = 0; j < CHECKERBOARD_HEIGHT; j++)
		{
			c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0)) * 255;
			checkerboard[i][j][0] = (GLubyte)c;
			checkerboard[i][j][1] = (GLubyte)c;
			checkerboard[i][j][2] = (GLubyte)c;
			checkerboard[i][j][3] = (GLubyte)255;
		}
	}
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

	//Transformations
	mat4 translationMatrix = mat4::identity();
	mat4 modelViewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix = mat4::identity();

	translationMatrix = translate(0.0f, 0.0f, -4.0f);

	modelViewMatrix = translationMatrix;		

	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, texture_checkerboard);

	glUniform1i(textureSamplerUniform, 0);

	glBindVertexArray(vao);

	GLfloat position[12];

	position[0] = 0.0f;
	position[1] = 1.0f;
	position[2] = 0.0f;
	position[3] = -2.0f;
	position[4] = 1.0f;
	position[5] = 0.0f;
	position[6] = -2.0f;
	position[7] = -1.0f;
	position[8] = 0.0f;
	position[9] = 0.0f;
	position[10] = -1.0f;
	position[11] = 0.0f;

	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);

	glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Here is the drawing code
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);		//4 as it is quad

	/// no. 2


	position[0] = 2.41421f;
	position[1] = 1.0f;
	position[2] = -1.41421f;
	position[3] = 1.0f;
	position[4] = 1.0f;
	position[5] = 0.0f;
	position[6] = 1.0f;
	position[7] = -1.0f;
	position[8] = 0.0f;
	position[9] = 2.41421f;
	position[10] = -1.0f;
	position[11] = -1.41421f;

	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);

	glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Here is the drawing code
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);		//4 as it is quad



	
	glBindVertexArray(0);			//UNBIND vao

	

	//Unuse the shader program object
	glUseProgram(0);

	SwapBuffers(ghdc);
}

void update(void)
{
	//code

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
	if (vbo_texcoord)
	{
		glDeleteBuffers(1, &vbo_texcoord);
		vbo_texcoord = 0;
	}

	if (vbo_position)
	{
		glDeleteBuffers(1, &vbo_position);
		vbo_position = 0;
	}

	//Deletion and uninitialization of vao
	if (vao)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}

	if (texture_checkerboard)
	{
		glDeleteTextures(1, &texture_checkerboard);
		texture_checkerboard = 0;
	}

	//Shader Uninitialization
	if (shaderProgramObject)
	{
		glUseProgram(shaderProgramObject);

		GLsizei numAttachedShaders;			//Number of attached shaders
		glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numAttachedShaders);
		GLuint* Shader_Objects = NULL;

		Shader_Objects = (GLuint*)malloc(numAttachedShaders * sizeof(GLuint));

		//According to no of  and fill it with the attahched shadow obejcts 

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
