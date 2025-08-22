// Header files
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "OGL.h"

#include <GL/gl.h>
#include <GL/glu.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "GLU32.lib")

//Global Function declarations
LRESULT CALLBACK WndProc(HWND , UINT , WPARAM , LPARAM);

//Global Variables Declarations
HWND ghwnd = NULL;      //g for global
HDC ghdc = NULL;
HGLRC ghrc = NULL;          //Handle OpenGL Rendering COntext
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE; //By default, our window is not active
FILE* gpFile = NULL;    //File is type *pointer, 
//gp = Global Pointer gpFile is Pointer Variable of type FILE


// For shape points
float gfPoints[4][4][2];

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

	// Get Screen Co-ordinates
	int ScreenX = GetSystemMetrics(SM_CXSCREEN);
	int ScreenY = GetSystemMetrics(SM_CYSCREEN);

	//create the window

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("All Shapes : Abhijeet Natu"),
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
	void resize(int, int);
	void FillPoints(void);
	
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
		//clear the screen using Blue color
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glShadeModel(GL_SMOOTH);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		FillPoints();

	//Warmup  resize call
	resize(WIN_WIDTH,WIN_HEIGHT);
	return 0 ;

}

void resize(int width, int height)
{
	//code
	if (height == 0)
		height = 1;      //To avoid Divided by 0 illegal statement for future code

	glViewport(0, 0, GLsizei(width), GLsizei(height));
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

}

void display(void)
{
	//function declarations
	void DrawShape1(void);
	void DrawShape2(void);
	void DrawShape3(void);
	void DrawShape4(void);
	void DrawShape5(void);
	void DrawShape6(void);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);		//Model View Matrix
	
	DrawShape1();

	DrawShape3();

	DrawShape2();

	DrawShape5();

	DrawShape4();

	DrawShape6();

	SwapBuffers(ghdc);
}

void FillPoints(void)
{
	float y = 1.0f;
	for (int i = 0; i < 4; i++)
	{
		float x = -1.0f;
		for (int j = 0; j < 4; j++)
		{
			gfPoints[i][j][0] = x;
			gfPoints[i][j][1] = y;

			x = x + 0.15f;
		}
		y = y - 0.15f;
	}
}

void DrawShape1(void)
{
	glLoadIdentity();
	glTranslatef(-0.40f, -0.2f, -2.5f);
	glPointSize(3.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	for (int i = 0; i < 4; i = i + 1)
	{
		for (int j = 0; j < 4; j = j + 1)
		{
			glBegin(GL_POINTS);
			glVertex3f(gfPoints[i][j][0], gfPoints[i][j][1], 0.0f);
			glEnd();
		}
	}

}

void DrawShape3(void)
{
	glLoadIdentity();
	glTranslatef(1.8f, -0.2f, -2.5f);
	glPointSize(3.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	for (int i = 0; i < 4; i = i + 1)
	{
		for (int j = 0; j < 4; j = j + 1)
		{
			glBegin(GL_LINES);
			glVertex3f(gfPoints[i][j][0], gfPoints[i][j][1], 0.0f);
			if (i != 3)
			{
				glVertex3f(gfPoints[i + 1][j][0], gfPoints[i + 1][j][1], 0.0f);
			}
			glEnd();

			glBegin(GL_LINES);
			glVertex3f(gfPoints[i][j][0], gfPoints[i][j][1], 0.0f);
			if (j != 3)
			{
				glVertex3f(gfPoints[i][j + 1][0], gfPoints[i][j + 1][1], 0.0f);
			}
			glEnd();

		}
	}

}

void DrawShape2(void)
{

	glLoadIdentity();
	glTranslatef(0.75f, -0.2f, -2.5f);
	glPointSize(3.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	for (int i = 0; i < 4; i = i + 1)
	{
		for (int j = 0; j < 4; j = j + 1)
		{
			glBegin(GL_LINES);
			glVertex3f(gfPoints[i][j][0], gfPoints[i][j][1], 0.0f);
			
			if (i != 3)
			{
				if (j == 3)
				{

				}
				else {
					glVertex3f(gfPoints[i + 1][j][0], gfPoints[i + 1][j][1], 0.0f);
				}
			}
			glEnd();

			glBegin(GL_LINES);
			glVertex3f(gfPoints[i][j][0], gfPoints[i][j][1], 0.0f);
			if (j != 3)
			{
				if (i == 3)
				{

				}
				else
				{
					glVertex3f(gfPoints[i][j + 1][0], gfPoints[i][j + 1][1], 0.0f);
				}

			}
			glEnd();

			glBegin(GL_LINES);
			if (i <= 3)
			{
				if (j <= 3)
				{
					if (j == 3)
					{

					}
					else {
						glVertex3f(gfPoints[i + 1][j][0], gfPoints[i + 1][j][1], 0.0f);
					}


					if (i == 3)
					{

					}
					else
					{
						glVertex3f(gfPoints[i][j + 1][0], gfPoints[i][j + 1][1], 0.0f);
					}

				}
			}

			glEnd();

		}
	}
}

void DrawShape4(void)
{
	glLoadIdentity();
	glTranslatef(-0.40f, -1.0f, -2.5f);
	glPointSize(3.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	for (int i = 0; i < 4; i = i + 1)
	{
		for (int j = 0; j < 4; j = j + 1)
		{
			//Vertical lines
			glBegin(GL_LINES);
			glVertex3f(gfPoints[i][j][0], gfPoints[i][j][1], 0.0f);
			if (i != 3)
			{
				glVertex3f(gfPoints[i + 1][j][0], gfPoints[i + 1][j][1], 0.0f);
			}
			glEnd();

			//Horizontal line
			glBegin(GL_LINES);
			glVertex3f(gfPoints[i][j][0], gfPoints[i][j][1], 0.0f);
			if (j != 3)
			{
				glVertex3f(gfPoints[i][j + 1][0], gfPoints[i][j + 1][1], 0.0f);
			}
			glEnd();

			//Diagonal line

			glBegin(GL_LINES);
			if (i <= 3)
			{

				if (j == 3)
				{

				}
				else {
					glVertex3f(gfPoints[i + 1][j][0], gfPoints[i + 1][j][1], 0.0f);
				}


				if (i == 3)
				{

				}
				else
				{
					glVertex3f(gfPoints[i][j + 1][0], gfPoints[i][j + 1][1], 0.0f);
				}
			}

			glEnd();
		}
	}

	
}

void DrawShape5(void)
{
	glLoadIdentity();
	glTranslatef(0.75f, -1.0f, -2.5f);
	glPointSize(3.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	for (int i = 0; i < 4; i = i + 1)
	{
		for (int j = 0; j < 4; j = j + 1)
		{
			//Vertical lines
			glBegin(GL_LINES);
			if (j == 0 || j == 3)
			{
				glVertex3f(gfPoints[i][j][0], gfPoints[i][j][1], 0.0f);
				if (i != 3)
				{
					glVertex3f(gfPoints[i + 1][j][0], gfPoints[i + 1][j][1], 0.0f);
				}
			}

			glEnd();

			//Horizontal line
			glBegin(GL_LINES);
			if (i == 0 || i == 3)
			{
				glVertex3f(gfPoints[i][j][0], gfPoints[i][j][1], 0.0f);
				if (j != 3)
				{
					glVertex3f(gfPoints[i][j + 1][0], gfPoints[i][j + 1][1], 0.0f);
				}
			}

			glEnd();

			glBegin(GL_LINES);

			glVertex3f(gfPoints[0][0][0], gfPoints[0][0][1], 0.0f);
			if (i == 3 || j == 3)
			{
				glVertex3f(gfPoints[i][j][0], gfPoints[i][j][1], 0.0f);
			}
			glEnd();

		}
	}

}

void DrawShape6(void)
{
	glLoadIdentity();
	glTranslatef(1.8f, -1.0f, -2.5f);
	glPointSize(3.0f);
	glColor3f(1.0f, 0.0f, 1.0f);
	

	for (int i = 0; i < 4; i = i + 1)
	{
		for (int j = 0; j < 4; j = j + 1)
		{

			if (j == 0)
			{
				glColor3f(1.0f, 0.0f, 0.0f);
			}
			else if (j == 1)
			{
				glColor3f(0.0f, 1.0f, 0.0f);
			}
			else if (j == 2)
			{
				glColor3f(0.0f, 0.0f, 1.0f);
			}

			glBegin(GL_QUADS);
			if (j != 3)
			{
				glVertex3f(gfPoints[i][j + 1][0], gfPoints[i][j + 1][1], 0.0f);
			}
			glVertex3f(gfPoints[i][j][0], gfPoints[i][j][1], 0.0f);
			if (i != 3)
			{
				glVertex3f(gfPoints[i + 1][j][0], gfPoints[i + 1][j][1], 0.0f);
			}

			if (j != 3)
			{
				glVertex3f(gfPoints[i + 1][j + 1][0], gfPoints[i + 1][j + 1][1], 0.0f);
			}
			glEnd();
		

			glColor3f(1.0f, 1.0f, 1.0f);
			glBegin(GL_LINES);
			if (j == 1 || j == 2)
			{
				glVertex3f(gfPoints[i][j][0], gfPoints[i][j][1], 0.0f);
				if (i != 3)
				{
					glVertex3f(gfPoints[i + 1][j][0], gfPoints[i + 1][j][1], 0.0f);
				}
			}

			glEnd();

			glBegin(GL_LINES);
			if (i == 1 || i == 2)
			{
				glVertex3f(gfPoints[i][j][0], gfPoints[i][j][1], 0.0f);
				if (j != 3)
				{
					glVertex3f(gfPoints[i][j + 1][0], gfPoints[i][j + 1][1], 0.0f);
				}
			}

			glEnd();
		}
	}

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

