// Header files
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "OGL.h"

//OpenGL Header files
#include <GL/glew.h>		//This MUST be above gl.h
#include <GL/gl.h>

#include "vmath.h"
using namespace vmath;		

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//OpenGL Libraries
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "OpenGL32.lib")

//Global Function declarations
LRESULT CALLBACK WndProc(HWND , UINT , WPARAM , LPARAM);


//Global Variables Declarations
HWND ghwnd = NULL;      //g for global
HDC ghdc = NULL;
HGLRC ghrc = NULL;          //Handle OpenGL Rendering Context
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE; //By default, our window is not active
FILE* gpFile = NULL;    //File is type *pointer

float ModelRotationAngle = 0.0f;

//Programmable Pipeline related global variables
GLuint shaderProgramObject;

enum
{
	AAN_ATTRIBUTE_VERTEX = 0,
};

GLuint gVao;
GLuint gVbo;
GLuint gElementBuffer;
GLuint vbo_color;
GLuint mvpMatrixUniform;

mat4 perspectiveProjectionMatrix;		//4x4 matrix

struct vec_int
{
	int *p;
	int size;
};

struct vec_float
{
	float *pf;
	int size;
};

#define BUFFER_SIZE 1024
char buffer[BUFFER_SIZE];

FILE *gp_mesh_file;

struct vec_float *gp_vertex, *gp_texture, *gp_normal;
struct vec_float *gp_vertex_sorted, *gp_texture_sorted, *gp_normal_sorted;
struct vec_int *gp_vertex_indices, *gp_texture_indices, *gp_normal_indices;


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

	if (fopen_s(&gpFile, "PP_OBJ_PARSER_Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Creation of Log File Failed. Exitting...!!\n"), TEXT("File I/O Error"), MB_OK);
		fflush(gpFile);
		exit(0);
	}

	else          //File opened Successfully
	{
		fprintf(gpFile, "Log File Successfully Created !!\n");
		fflush(gpFile);
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
		TEXT("Abhijeet Natu : MODEL LOADING"),
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
	fflush(gpFile);
	fprintf(gpFile, " BEFORE INITIALIZE()");
	iRetVal = initialize();
	fflush(gpFile);
	fprintf(gpFile, " AFTER INITIALIZE()");
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

	void load_mesh(void);


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
		//printGLInfo();


		//VERTEX SHADER

		//Step 1
		const GLchar* vertexShaderSourceCode =
			"#version 460 core" \
			"\n" \
			"in vec4 a_position;" \
			"uniform mat4 u_mvpMatrix;" \
			"void main(void)" \
			"{" \
			"gl_Position = u_mvpMatrix * a_position;" \
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
		const GLchar* fragmentShaderSourceCode=
			"#version 460 core" \
			"\n" \
			"out vec4 FragColor;" \
			"void main(void)" \
			"{" \
			"FragColor =  vec4(1.0,1.0,1.0,1.0);" \
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

		glBindAttribLocation(shaderProgramObject, AAN_ATTRIBUTE_VERTEX,"a_position");		//chukla tar andhaar FIX !!!

		glLinkProgram(shaderProgramObject);


		glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);

		status = 0;
		infoLogLength = 0;
		log = NULL;


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


		load_mesh();

		//gVao and gVbo related code
		glGenVertexArrays(1, &gVao);
		glBindVertexArray(gVao);
		glGenBuffers(1, &gVbo);
		glBindBuffer(GL_ARRAY_BUFFER, gVbo);
		glBufferData(GL_ARRAY_BUFFER, (gp_vertex_sorted->size) * sizeof(GLfloat), gp_vertex_sorted->pf, GL_STATIC_DRAW);
		glVertexAttribPointer(AAN_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);		//x,y,z
		glEnableVertexAttribArray(AAN_ATTRIBUTE_VERTEX);
		glBindBuffer(GL_ARRAY_BUFFER, 0);		//UNBIND

		glGenBuffers(1, &gElementBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gElementBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, gp_vertex_indices->size * sizeof(int), gp_vertex_indices->p, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);		//UNBIND (gVbo)
		glBindVertexArray(0);					//UNBIND
		
		//Depth Related code

		glShadeModel(GL_SMOOTH);
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glFrontFace(GL_CCW);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		

		//clear the screen using Black color
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);


		perspectiveProjectionMatrix = mat4::identity();

		resize(WIN_WIDTH, WIN_HEIGHT);

	return 0 ;

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

	mat4 translationMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();
	mat4 modelViewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix = mat4::identity();


	translationMatrix = translate(0.0f, 0.0f, -6.0f);
	rotationMatrix = rotate(ModelRotationAngle, 0.0f, 1.0f, 0.0f);	//Spinning

	modelViewMatrix = translationMatrix * rotationMatrix;

	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	//Here is the drawing code
	glBindVertexArray(gVao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gElementBuffer);

	glDrawElements(GL_TRIANGLES, (gp_vertex_indices->size), GL_UNSIGNED_INT, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	//Unuse the shader program object
	glUseProgram(0);

	SwapBuffers(ghdc);
}

void update(void)
{
	//code
	ModelRotationAngle = ModelRotationAngle + 1.0f;
	if (ModelRotationAngle >= 360.0f)
		ModelRotationAngle = ModelRotationAngle - 360.0f;
}

void uninitialize(void)
{
	//function declarations
	void ToggleFullScreen(void);
	
	int destroy_vec_int(struct vec_int *p_vec_int);
	int destroy_vec_float(struct vec_float *p_vec_float);
	
	//code
	if (gbFullScreen)
	{
		ToggleFullScreen();
	}

	//Deletion and uninitialization of gVbo
	if (vbo_color)
	{
		glDeleteBuffers(1, &vbo_color);
		vbo_color = 0;
	}

	if (gVbo)
	{
		glDeleteBuffers(1, &gVbo);
		gVbo = 0;
	}

	//Deletion and uninitialization of gVao
	if (gVao)
	{
		glDeleteVertexArrays(1, &gVao);
		gVao = 0;
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

	destroy_vec_float(gp_vertex);
	gp_vertex = NULL;
	
	destroy_vec_float(gp_texture);
	gp_texture = NULL;
	
	destroy_vec_float(gp_normal);
	gp_normal = NULL;

	destroy_vec_float(gp_vertex_sorted);
	gp_vertex_sorted = NULL;

	destroy_vec_float(gp_texture_sorted);
	gp_texture_sorted = NULL;

	destroy_vec_float(gp_normal_sorted);
	gp_normal_sorted = NULL;

	destroy_vec_int(gp_vertex_indices);
	gp_vertex_indices = NULL;

	destroy_vec_int(gp_texture_indices);
	gp_texture_indices = NULL;

	destroy_vec_int(gp_normal_indices);
	gp_normal_indices = NULL;

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

void load_mesh(void)
{
	void uninitialize(void);
	struct vec_int *create_vec_int();
	struct vec_float *create_vec_float();
	int push_back_vec_int(struct vec_int *p_vec_int, int data);
	int push_back_vec_float(struct vec_float *p_vec_float, float data);
	void show_vec_float(struct vec_float *p_vec_float);
	void show_vec_int(struct vec_int *p_vec_int);
	int destroy_vec_float(struct vec_float *p_vec_float);
	int destroy_vec_int(struct vec_int *p_vec_int);


	char *space = " ", *slash = "/", *first_token = NULL, *token;
	char *f_entries[3] = { NULL, NULL, NULL };
	int nr_pos_cords = 0, nr_tex_cords = 0, nr_normal_cords = 0, nr_faces = 0;
	int i, vi;

	gp_mesh_file = fopen("MonkeyHead.obj", "r");
	if (gp_mesh_file == NULL)
	{
		fprintf(stderr, "error in opening file\n");
		uninitialize();
		exit(EXIT_FAILURE);
	}

	gp_vertex = create_vec_float();
	gp_texture = create_vec_float();
	gp_normal = create_vec_float();

	gp_vertex_indices = create_vec_int();
	gp_texture_indices = create_vec_int();
	gp_normal_indices = create_vec_int();

	while (fgets(buffer, BUFFER_SIZE, gp_mesh_file) != NULL)
	{
		first_token = strtok(buffer, space);

		if (strcmp(first_token, "v") == 0)
		{
			nr_pos_cords++;
			while ((token = strtok(NULL, space)) != NULL)
				push_back_vec_float(gp_vertex, atof(token));

		}
		else if (strcmp(first_token, "vt") == 0)
		{
			nr_tex_cords++;
			while ((token = strtok(NULL, space)) != NULL)
				push_back_vec_float(gp_texture, atof(token));
		}
		else if (strcmp(first_token, "vn") == 0)
		{
			nr_normal_cords++;
			while ((token = strtok(NULL, space)) != NULL)
				push_back_vec_float(gp_normal, atof(token));
		}
		else if (strcmp(first_token, "f") == 0)
		{
			nr_faces++;
			for (i = 0; i < 3; i++)
				f_entries[i] = strtok(NULL, space);

			for (i = 0; i < 3; i++)
			{
				token = strtok(f_entries[i], slash);
				push_back_vec_int(gp_vertex_indices, atoi(token)-1);
				token = strtok(NULL, slash);
				push_back_vec_int(gp_texture_indices, atoi(token)-1);
				token = strtok(NULL, slash);
				push_back_vec_int(gp_normal_indices, atoi(token)-1);
			}
		}
	}

	gp_vertex_sorted = create_vec_float();
	for(int i = 0; i < gp_vertex_indices->size;i++)
		push_back_vec_float(gp_vertex_sorted,gp_vertex->pf[i]);


	gp_texture_sorted = create_vec_float();
	for(int i = 0; i < gp_texture_indices->size;i++)
		push_back_vec_float(gp_texture_sorted,gp_texture->pf[i]);

	gp_normal_sorted = create_vec_float();
	for(int i = 0; i < gp_normal_indices->size;i++)
		push_back_vec_float(gp_normal_sorted,gp_normal->pf[i]);



	fclose(gp_mesh_file);
	gp_mesh_file = NULL;
}


struct vec_int* create_vec_int()
{
	struct vec_int* p = (struct vec_int*)malloc(sizeof(struct vec_int));
	assert(p);
	memset(p, 0, sizeof(struct vec_int));
	return p;
}

struct vec_float* create_vec_float()
{
	struct vec_float* p = (struct vec_float*)malloc(sizeof(struct vec_float));
	memset(p, 0, sizeof(struct vec_float));
	return p;
}

int push_back_vec_int(struct vec_int *p_vec_int, int data)
{
	p_vec_int->p = (int*)realloc(p_vec_int->p, (p_vec_int->size + 1) * sizeof(int));
	p_vec_int->size = p_vec_int->size + 1;
	p_vec_int->p[p_vec_int->size - 1] = data;
	return 0;
}

int push_back_vec_float(struct vec_float *p_vec_float, float data)
{
	p_vec_float->pf = (float*)realloc(p_vec_float->pf, (p_vec_float->size + 1) * sizeof(float));
	p_vec_float->size = p_vec_float->size + 1;
	p_vec_float->pf[p_vec_float->size - 1] = data;
	return 0;
}


int destroy_vec_int(struct vec_int *p_vec_int)
{
	free(p_vec_int->p);
	free(p_vec_int);
	return 0;
}

int destroy_vec_float(struct vec_float *p_vec_float)
{
	free(p_vec_float->pf);
	free(p_vec_float);
	return 0;
}
