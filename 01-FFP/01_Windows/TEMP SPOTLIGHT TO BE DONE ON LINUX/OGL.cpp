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
HGLRC ghrc = NULL;          //Handle OpenGL Rendering Context
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE; //By default, our window is not active
FILE* gpFile = NULL;    //File is type *pointer, 
//gp = Global Pointer gpFile is Pointer Variable of type FILE

float x_pos=0.0f, y_pos=0.0f;

GLUquadric* quadric = NULL;
BOOL gbLight = FALSE;

GLfloat lightAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat lightDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightPosition[] = { x_pos,y_pos,5.0f,1.0f };

GLfloat materialAmbient[] = { 0.5f,0.5f,0.5f,1.0f };
GLfloat materialDiffuse[] = { 1.0f,0.0f,0.0f,1.0f };
GLfloat materialSpecular[] = { 0.7f,0.7f,0.7f,1.0f };
GLfloat materialShininess = 128.0f;

GLfloat cutoff_angle = 10.0f;
GLfloat spot_exponent = 10.0f;
GLfloat spot_direction[] = { 0.0f,0.0f,-1.0f };

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
		TEXT("OpenGL Window"),
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

			case VK_UP:
				//if (y_pos > -2.0f)
					y_pos += 0.1f;
					break;

			case VK_DOWN:
				//if(y_pos < 2.0f)
					y_pos -= 0.1f;
					break;

			case VK_LEFT:
				//if(x_pos > -5.0f)
					x_pos -= 0.1f;
					break;

			case VK_RIGHT:
				//if(x_pos < 3.0f)
					x_pos += 0.1f;
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
			if (gbLight == FALSE)
			{
				glEnable(GL_LIGHTING);
				gbLight = TRUE;
			}
			else
			{
				glDisable(GL_LIGHTING);
				gbLight = FALSE;
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
	pfd.cDepthBits = 32;		//24 also can be done

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

		//Set property of light
		glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);		//Float Vector array related to light
		glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
		glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
		//glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);
		
		glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
		//glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
		glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

		glEnable(GL_LIGHT1);

		//RELATED TO SPOTLIGHT
		glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, cutoff_angle);
		glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, spot_exponent);
		
		//glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//Depth Related changes
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glShadeModel(GL_SMOOTH);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

		quadric = gluNewQuadric();

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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);		//Model View Matrix

	//Light Position
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);
	
	//glLoadIdentity();

	//View Transformation/Camera Transformation
	//gluLookAt(0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	//Draw Sphere
	glLoadIdentity();
	glTranslatef(-8.0f, 7.0f, -20.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluSphere(quadric, 1.0f, 500, 500);

	glLoadIdentity();
	glTranslatef(-8.0f, 4.5f, -20.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0f, 1.0f, 0.0f);
	gluSphere(quadric, 1.0f, 500, 500);

	glLoadIdentity();
	glTranslatef(-8.0f, 2.0f, -20.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0f, 0.0f, 1.0f);
	gluSphere(quadric, 1.0f, 500, 500);

	glLoadIdentity();
	glTranslatef(-8.0f, -0.5f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0f, 0.0f, 1.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	glLoadIdentity();
	glTranslatef(-8.0f, -3.0f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	glLoadIdentity();
	glTranslatef(-8.0f, -5.5f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0f, 1.0f, 0.0f);
	gluSphere(quadric, 1.0f, 30, 30);


	glLoadIdentity();
	glTranslatef(-4.0f, 7.0f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0f, 0.0f, 1.0f);
	gluSphere(quadric, 1.0f, 30, 30);


	glLoadIdentity();
	glTranslatef(-4.0f, 4.5f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluSphere(quadric, 1.0f, 30, 30);


	glLoadIdentity();
	glTranslatef(-4.0f, 2.0f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0f, 1.0f, 0.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	glLoadIdentity();
	glTranslatef(-4.0f, -0.5f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0f, 0.0f, 1.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	glLoadIdentity();
	glTranslatef(-4.0f, -3.0f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	glLoadIdentity();
	glTranslatef(-4.0f, -5.5f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0f, 1.0f, 0.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	glLoadIdentity();
	glTranslatef(0.0f, 7.0f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0f, 0.0f, 1.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	glLoadIdentity();
	glTranslatef(0.0f, 4.5f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	glLoadIdentity();
	glTranslatef(0.0f, 2.0f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0f, 1.0f, 0.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	glLoadIdentity();
	glTranslatef(0.0f, -0.5f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0f, 0.0f, 1.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	glLoadIdentity();
	glTranslatef(0.0f, -3.0f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	glLoadIdentity();
	glTranslatef(0.0f, -5.5f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0f, 1.0f, 0.0f);
	gluSphere(quadric, 1.0f, 30, 30);


	glLoadIdentity();
	glTranslatef(4.0f, 7.0f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0f, 0.0f, 1.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	glLoadIdentity();
	glTranslatef(4.0f, 4.5f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	glLoadIdentity();
	glTranslatef(4.0f, 2.0f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0f, 1.0f, 0.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	glLoadIdentity();
	glTranslatef(4.0f, -0.5f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0f, 0.0f, 1.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	glLoadIdentity();
	glTranslatef(4.0f, -3.0f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	glLoadIdentity();
	glTranslatef(4.0f, -5.5f, -20.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0f, 1.0f, 0.0f);
	gluSphere(quadric, 1.0f, 30, 30);

	SwapBuffers(ghdc);
}


void update(void)
{

	lightPosition[0] = x_pos;

	lightPosition[1] = y_pos;

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

