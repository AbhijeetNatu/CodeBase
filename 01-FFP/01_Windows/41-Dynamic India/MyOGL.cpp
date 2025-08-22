// Header Files 
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>
#include "MyOGL.h"

#define _USE_MATH_DEFINES 1
#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "GLU32.lib")
#pragma comment(lib,"winmm.lib")

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HWND ghwnd = NULL;      //g for global
HDC ghdc = NULL;
HGLRC ghrc = NULL;          //Handle OpenGL Rendering Context
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE; //By default, our window is not active
FILE* gpFile = NULL;    //File is type *pointer, 
//gp = Global Pointer gpFile is Pointer Variable of type FILE

BOOL bXPositive = TRUE;


float radius = 5.2f;
float radian = 0.0f;
float angle = 180.0f;
float trans = -2.5f;

float fade1 = 0.0f;
float fade2 = 0.0f;
float fade3 = 0.0f;
float fade4 = 0.0f;

float i_flag = 0.0f;
float n_flag = 0.0f;
float d_flag = 0.0f;
float a_flag = 0.0f;

float plane_flag = 0.0f;

float I1_movement = -8.0f;
float N_movement = 8.0f;
float D_movement = 0.0f;
float I2_movement = 8.0f;
float A_movement = 11.0f;

float plane1_movement = -8.0f;
float plane2_movement = -8.0f;
float plane3_movement = -8.0f;

float plane1_X = -8.0f;
float plane1_Y = 8.0f;

float plane3_X = -18.0f;
float plane3_Y = 1.4f;

BOOL plane1_flag = FALSE;
BOOL plane2_flag = FALSE;
BOOL plane3_flag = TRUE;

BOOL plane1_rotate_left_flag = TRUE;
BOOL plane2_rotate_flag = FALSE;
BOOL plane3_rotate_flag = FALSE;

BOOL plane1_rotate_right_flag = FALSE;

//GLOBAL FUNCTION declaration
void plane(void);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//function declarations
	int initialize(void);
	void display(void);
	void update(void);
	void uninitialize(void);
	void ToggleFullScreen(void);

	//variable declarations

	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("DYNAMIC INDIA");
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

	// Get Screen Co-ordinates
	int ScreenX = GetSystemMetrics(SM_CXSCREEN);
	int ScreenY = GetSystemMetrics(SM_CYSCREEN);

	//create the window

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("DYNAMIC INDIA"),
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

	else
	{
		fprintf(gpFile, "Making OpenGL context as current context Successful\n");
	}

	//show window

	ShowWindow(hwnd, iCmdShow);
	
	ToggleFullScreen();

	// Playing the music
	PlaySound(MAKEINTRESOURCE(MYSOUND), NULL, SND_RESOURCE | SND_ASYNC);

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
	
	void resize(int, int);
	void ToggleFullScreen(void);

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
		resize(LOWORD(lParam), HIWORD(lParam));
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
	void resize(int, int);


	//Variable Declarations
	PIXELFORMATDESCRIPTOR pfd;
	int	iPixelFormatIndex;

	//code

	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	//initialization of PIXELFORMATDESCRIPTOR 
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
		return (-1);
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
		return (-3);
	}

	//make the rendering context as the current context
	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
	{
		return (-4);
	}

	//Here starts OpenGL code
	//clear the screen using Black color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	
	//Warmup  resize call
	resize(WIN_WIDTH, WIN_HEIGHT);
	return 0;

}

void resize(int width, int height)
{
	//code
	if (height == 0)
		height = 1;      //To avoid Divided by 0 illegal statement for future code

	glViewport(0, 0, GLsizei(width), GLsizei(height));

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void I1(void)
{
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, 0.5f, 0.0f);
	glVertex3f(1.0f, 0.5f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glEnd();
	
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(-0.2f, 0.5f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.2f, 0.0f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.2f, 0.5f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.2f, 0.0f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-0.2f, -0.5f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.2f, -0.5f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -0.5f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);
	glVertex3f(1.0f, -0.5f, 0.0f);
	glEnd();
}

void N(void)
{
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, 0.5f, 0.0f);
	glVertex3f(-0.6f, 0.5f, 0.0f);
	glVertex3f(-0.6f, 1.0f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(-1.0f, 0.5f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 0.0f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.6f, 0.0f, 0.0f);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(-0.6f, 0.5f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 0.0f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -0.5f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-0.6f, -0.5f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.6f, 0.0f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -0.5f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glVertex3f(-0.6f, -1.0f, 0.0f);
	glVertex3f(-0.6f, -0.5f, 0.0f);
	glEnd();


	//For the diagonal
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(-0.6f, 1.0f, 0.0f);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(-0.6f, 0.5f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.0f, -0.2f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.0f, 0.3f, 0.0f);
	glEnd();


	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.0f, 0.3f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.0f, -0.2f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.6f, -1.0f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.6f, -0.5f, 0.0f);
	glEnd();

	//Right Side of N

	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.6f, 0.5f, 0.0f);
	glVertex3f(0.6f, 1.0f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(1.0f, 0.5f, 0.0f);
	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.6f, 0.0f, 0.0f);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.6f, 0.5f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, -0.5f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.6f, -0.5f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.6f, 0.0f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, -0.5f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);
	glVertex3f(0.6f, -1.0f, 0.0f);
	glVertex3f(0.6f, -0.5f, 0.0f);
	glEnd();
}


void D(void)
{
	//fade1 = 1.0f, fade2 =0.50f in color3f

	glBegin(GL_QUADS);
	glColor3f(fade1, fade2, 0.0f);
	glVertex3f(-1.2f, 1.0f, 0.0f);
	glVertex3f(-1.2f, 0.5f, 0.0f);
	glVertex3f(1.0f, 0.5f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(fade1, fade2, 0.0f);
	glVertex3f(-1.0f, 0.5f, 0.0f);
	glColor3f(fade1, fade1, fade1);
	glVertex3f(-1.0f, 0.0f, 0.0f);
	glColor3f(fade1, fade1, fade1);
	glVertex3f(-0.6f, 0.0f, 0.0f);
	glColor3f(fade1, fade2, 0.0f);
	glVertex3f(-0.6f, 0.5f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(fade1, fade1, fade1);
	glVertex3f(-1.0f, 0.0f, 0.0f);
	glColor3f(0.0f, fade1, 0.0f);
	glVertex3f(-1.0f, -0.5f, 0.0f);
	glColor3f(0.0f, fade1, 0.0f);
	glVertex3f(-0.6f, -0.5f, 0.0f);
	glColor3f(fade1, fade1, fade1);
	glVertex3f(-0.6f, 0.0f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(fade1, fade2, 0.0f);
	glVertex3f(1.0f, 0.5f, 0.0f);
	glColor3f(fade1, fade1, fade1);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glColor3f(fade1, fade1, fade1);
	glVertex3f(0.6f, 0.0f, 0.0f);
	glColor3f(fade1, fade2, 0.0f);
	glVertex3f(0.6f, 0.5f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(fade1, fade1, fade1);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glColor3f(0.0f, fade1, 0.0f);
	glVertex3f(1.0f, -0.5f, 0.0f);
	glColor3f(0.0f, fade1, 0.0f);
	glVertex3f(0.6f, -0.5f, 0.0f);
	glColor3f(fade1, fade1, fade1);
	glVertex3f(0.6f, 0.0f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.0f, fade1, 0.0f);
	glVertex3f(-1.2f, -0.5f, 0.0f);
	glVertex3f(-1.2f, -1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);
	glVertex3f(1.0f, -0.5f, 0.0f);
	glEnd();
}

void I2(void)
{
	glLineWidth(5.0f);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, 0.5f, 0.0f);
	glVertex3f(1.0f, 0.5f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(-0.2f, 0.5f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.2f, 0.0f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.2f, 0.5f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.2f, 0.0f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-0.2f, -0.5f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.2f, -0.5f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -0.5f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);
	glVertex3f(1.0f, -0.5f, 0.0f);
	glEnd();
}

void A(void)
{
	//TOP
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.8f, 0.0f);
	glVertex3f(-0.2f, 0.5f, 0.0f);
	glVertex3f(-0.4f, 0.5f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.8f, 0.0f);
	glVertex3f(0.2f, 0.5f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.4f, 0.5f, 0.0f);
	glEnd();

	//MIDDLE
	glBegin(GL_QUADS);
	
	glVertex3f(-0.4f, 0.5f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.2f, 0.5f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.4f, 0.15f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.6f, 0.15f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.4f, 0.5f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.2f, 0.5f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.4f, 0.15f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.6f, 0.15f, 0.0f);
	glEnd();

	//BOTTOM
	glBegin(GL_QUADS);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-0.6f, 0.15f, 0.0f);
	glVertex3f(-1.3f, -1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glVertex3f(-0.4f, 0.15f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.6f, 0.15f, 0.0f);
	glVertex3f(1.3f, -1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);
	glVertex3f(0.4f, 0.15f, 0.0f);
	glEnd();


	//Center Line of A
	//fade3: 1.0f, fade4: 0.5f
	glBegin(GL_QUADS);
	glColor3f(fade3, fade3, fade3);
	glVertex3f(-0.5f, 0.10f, 0.0f);
	glColor3f(fade3, fade4, 0.0f);	//Orange
	glVertex3f(-0.5f, -0.15f, 0.0f);
	glColor3f(0.0f, fade3, 0.0f);	//Orange
	glVertex3f(0.5f, -0.15f, 0.0f);
	glColor3f(fade3, fade4, 0.0f);
	glVertex3f(0.5f, 0.10f, 0.0f);
	glEnd();


}

void I_for_flag(void)
{
	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.10f, 0.10f, 0.0f);
	glVertex3f(-0.10f, 0.025f, 0.0f);
	glVertex3f(0.10f, 0.025f, 0.0f);
	glVertex3f(0.10f, 0.10f, 0.0f);
	glEnd();
	
	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.01f, 0.025f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.01f, 0.0f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.01f, 0.0f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.01f, 0.025f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.01f, 0.0f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.01f, -0.025f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.01f, -0.025f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.01f, 0.0f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.10f, -0.025f, 0.0f);
	glVertex3f(-0.10f, -0.10f, 0.0f);
	glVertex3f(0.10f, -0.10f, 0.0f);
	glVertex3f(0.10f, -0.025f, 0.0f);
	glEnd();

}

void A_for_flag(void)
{
	glBegin(GL_QUADS);
	glVertex3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, 0.4f, 0.0f);
	glVertex3f(-0.1f, 0.25f, 0.0f);
	glVertex3f(-0.1f, 0.25f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, 0.4f, 0.0f);
	glVertex3f(0.1f, 0.25f, 0.0f);
	glVertex3f(0.1f, 0.25f, 0.0f);
	glEnd();
}

void F_for_flag(void)
{
	glLineWidth(4.0f);
	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.6f, 0.3f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.6f, 0.1f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.6f, 0.22f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.45f, 0.22f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.6f, 0.3f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.45f, 0.3f, 0.0f);
	glEnd();

}

void plane(void)		
{
	glLineWidth(8.5f);
	
	glBegin(GL_TRIANGLES);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.22f, 0.0f);
	glVertex3f(-0.34f, -0.34f, 0.0f);
	glVertex3f(0.34f, -0.34f, 0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.34f, -0.34f, 0.0f);
	glVertex3f(-0.34f, -0.34f, 0.0f);
	glVertex3f(-0.22f, -1.3f, 0.0f);
	glVertex3f(0.22f, -1.3f, 0.0f);
	glEnd();


	//Right wing
	glBegin(GL_TRIANGLES);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.7f, -0.45f, 0.0f);
	glVertex3f(-0.30f, -0.60f, 0.0f);
	glVertex3f(-0.30f, -0.80f, 0.0f);
	glEnd();

	//Left Wing
	glBegin(GL_TRIANGLES);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-0.7f, -0.45f, 0.0f);
	glVertex3f(0.30f, -0.60f, 0.0f);
	glVertex3f(0.30f, -0.80f, 0.0f);
	glEnd();

	//INDIA FLAG WINGS

	glBegin(GL_LINES);
	glLineWidth(20.0f);

	//Center Line of A
	//Orange line
	glColor3f(1.0f, 0.5f, 0.0f);	//Orange 
	glVertex3f(0.88f, -0.45f, 0.0f);
	glVertex3f(-0.88f, -0.43f, 0.0f);

	//White line
	glColor3f(1.0f, 1.0f, 1.0f);	//White
	glVertex3f(0.88f, -0.46f, 0.0f);
	glVertex3f(-0.88f, -0.46f, 0.0f);

	//Green line
	glColor3f(0.0f, 1.0f, 0.0f);	//Green 
	glVertex3f(0.88f, -0.48f, 0.0f);
	glVertex3f(-0.88f, -0.48f, 0.0f);

	glEnd();

	// FLAG IN CENTER OF PLANE
	glBegin(GL_LINES);
	glLineWidth(20.0f);

	//Center Line of A
	//Orange line
	glColor3f(1.0f, 0.5f, 0.0f);	//Orange 
	glVertex3f(0.35f, -0.93f, 0.0f);
	glVertex3f(-0.35f, -0.93f, 0.0f);

	//White line
	glColor3f(1.0f, 1.0f, 1.0f);	//White
	glVertex3f(0.35f, -0.96f, 0.0f);
	glVertex3f(-0.35f, -0.96f, 0.0f);

	//Green line
	glColor3f(0.0f, 1.0f, 0.0f);	//Green 
	glVertex3f(0.35f, -0.98f, 0.0f);
	glVertex3f(-0.35f, -0.98f, 0.0f);

	glEnd();

	//Flag adjustments
	glTranslatef(-0.22f, -0.68f, 0.0f);
	I_for_flag();
	
	glTranslatef(0.25f, -0.38f, 0.0f);
	A_for_flag();

	glTranslatef(0.75f, 0.15f, 0.0f);
	F_for_flag();
}

void display(void)
{
	void I1(void);
	void N(void);
	void D(void);
	void I2(void);
	void A(void);

	void I_for_flag(void);
	void A_for_flag(void);
	void F_for_flag(void);
	
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	
	glLoadIdentity();
	glTranslatef(I1_movement, 0.0f, -12.0f);
	I1();
	
	glLoadIdentity();
	glTranslatef(-2.95f, N_movement, -12.0f);
	N();

	glLoadIdentity();
	glTranslatef(-0.3f, 0.0f, -12.0f);
	D();

	glLoadIdentity();
	glTranslatef(2.05f, -I2_movement, -12.0f);
	I2();

	glLoadIdentity();
	glTranslatef(A_movement+1.5f, 0.0f, -12.0f);
	A();
	
	glLoadIdentity();
	glTranslatef(plane1_X, plane1_Y, -15.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	plane();

	glLoadIdentity();
	glTranslatef(plane1_X, -plane1_Y, -15.0f);
	glRotatef(-angle+180.0f, 0.0f, 0.0f, 1.0f);
	plane();

	glLoadIdentity();
	glTranslatef(plane3_X+0.1f, 0.0f, -15.0f);
	glRotatef(270.0f, 0.0f, 0.0f, 1.0f);
	plane();
	
	glLoadIdentity();

	SwapBuffers(ghdc);
}


void update(void)
{
	//code	
	I1_movement = I1_movement + 0.009f;
	if (I1_movement >= -5.5f)
	{
		I1_movement = -5.5f;
		n_flag = TRUE;
	}

	if (n_flag == TRUE)
	{
		N_movement = N_movement - 0.009f;
		if (N_movement <= 0.0f)
		{
			N_movement = 0.0f;
			d_flag = TRUE;
		}
	}

	if (d_flag == TRUE)
	{
		fade1 = fade1 + 0.001f;
		if (fade1 >= 1.0f)
			fade1 = 1.0f;

		fade2 = fade2 + 0.001f;
		if (fade2 >= 0.5f)
			fade2 = 0.5f;

		i_flag = TRUE;
	}

	if (i_flag == TRUE)
	{
		I2_movement = I2_movement - 0.009f;
		if (I2_movement <= -0.02f)
		{
			I2_movement = -0.02f;
			a_flag = TRUE;
		}
	}

	if (a_flag == TRUE)
	{
		A_movement = A_movement - 0.009f;
		if (A_movement <= 3.0f)
		{
			A_movement = 3.0f;
			plane_flag = TRUE;
			if(plane1_X >= 5.9f)
			{
				fade3 = fade3 + 0.001f;
				if (fade3 >= 1.0f)
					fade3 = 1.0f;

				fade4 = fade4 + 0.001f;
				if (fade4 >= 0.5f)
					fade4 = 0.5f;
			}
		}
	}

	if (plane_flag == TRUE)
	{
		if(plane1_rotate_left_flag)
		{
			angle = angle + 0.1f;
			if (angle >= 270.0f)
			{
				angle = 270.0f;
				plane1_flag = TRUE;
				plane1_rotate_left_flag = FALSE;
			}
			radian = (M_PI / 180) * angle;

			plane1_X = cos(radian) * radius - 5.3f;
			plane1_Y = sin(radian) * radius + 6.2f;
			
		}
		if (plane1_flag)
		{
			//fprintf(gpFile, "Plane 1 X = %f\n", plane1_X);
			plane1_X = plane1_X + 0.02f;
			if (plane1_X >= 6.8f)
			{
				plane1_X = 6.8f;
				plane1_rotate_right_flag = TRUE;
				plane1_flag = FALSE;
			}
		}

		if (plane1_rotate_right_flag)
		{
			angle = angle + 0.1f;
			if (angle >= 360.0f)
			{
				angle = 360.0f;
				plane_flag = FALSE;
				plane1_rotate_right_flag = FALSE;
			}
			radian = (M_PI / 180) * angle;

			plane1_X = cos(radian) * radius + 6.8f;
			plane1_Y = sin(radian) * radius + 6.2f;
		}
		
		plane3_X = plane3_X + 0.018f;
		
	}
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


