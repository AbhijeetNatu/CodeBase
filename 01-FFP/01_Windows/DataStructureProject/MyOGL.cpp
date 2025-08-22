// Header Files 
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


#define SECONDS_TO_MICROSECONDS(s) (s*1000*1000)
#define MINUTES_TO_MICROSECONDS(m) (m*1000*1000*60)

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "GLU32.lib")
#pragma comment(lib,"winmm.lib")

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HWND ghwnd = NULL;      
HDC ghdc = NULL;
HGLRC ghrc = NULL;          
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE; 
FILE* gpFile = NULL;    

BOOL bXPositive = TRUE;

////////////////////////////////////////

// OPENING SCENE VARIABLES 
float angleCube = 0.0f;
float transY = -2.5f;

BOOL openingCredits_flag = TRUE;
BOOL scene1_flag = FALSE;
BOOL scene2_flag = FALSE;
BOOL scene3_flag = FALSE;

float transBackground = 0.0f;

////////////////////////////////////////

// ENDING SCENE VARIABLES 
float transPhoto1 = -1.0f;
float transPhoto2 = -3.2f;
float transPhoto3 = -5.0f;
float transPhoto4 = -6.7f;

BOOL flag1 = FALSE;
BOOL flag2 = FALSE;
BOOL flag3 = FALSE;
BOOL flag4 = FALSE;


/////////////////////////////////////////


// SCENE 1 VARIABLES
float transSun = -6.0f;
float fade1 = 0.0f;
float fade2 = 0.0f;
float radius = 20.0f;

BOOL sunFlag = TRUE;

/////////////////////////////////////////
 
// SCENE 2 VARIABLES
float translateBirdFood = -2.3f;
float birdX = 15.5f;
float birdY = 1.9f;

BOOL singleBirdFlag = FALSE;

BOOL birdFlag = FALSE;
BOOL birdYFlag = FALSE;
BOOL FoodFlag = FALSE;
BOOL birdExitXFlag = FALSE;
BOOL birdExitYFlag = FALSE;
BOOL birdXnewFlag = FALSE;
int birdStopCounter = 1;

float newbirdX = -2.0f;
float newbirdY = -0.9f;

///////////////////////////////////////

// VARIABLES FOR BIRDFOOD
float translateZForBirdFood = -30.0f;
float movex_birdfood = 0.0f;
float movey_birdfood = 0.0f;
float movez_birdfood = 5.0f;
float eyex_birdfood = 0.0f;
float eyey_birdfood = 0.0f;
float eyez_birdfood = -5.0f;


//////////////////////////////////////
//VARIABLES FOR TREE (STACK) 
float x, y, z;

GLUquadric* quadric = NULL;

//////////////////////////////////////

//TEXTURES USED

GLuint texture_OpeningScene;
GLuint texture_mountain;
GLuint texture_background;
GLuint texture_sun;
GLuint texture_tree;
GLuint texture_branch;
GLuint texture_cloud;
GLuint texture_grass;
GLuint texture_home;
GLuint texture_door;
GLuint texture_window;
GLuint texture_birdfood;

//End Credits
GLuint texture_Photo1;
GLuint texture_Photo2;
GLuint texture_Photo3;
GLuint texture_Photo4;


/////////////////////////////////////////

int counter = 0;
BOOL counterFlag = FALSE;
BOOL birdCounterFlag = FALSE;
int birdcounter = 0;

////////////////////////////////////////

LARGE_INTEGER gliInitialTimestamp;
LARGE_INTEGER gliCurrentTimestamp;
LARGE_INTEGER gliEventTime;
LARGE_INTEGER frequency;
LARGE_INTEGER liFirstEventTime;

enum {
	SCENE0,
	SCENE1,
	SCENE2,
	SCENE3,
	SCENE4
};

int giScene = SCENE0;


//MAIN 

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
	TCHAR szAppName[] = TEXT("FFP DEMO");
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
		TEXT("UTHA UTHA CHIU TAAI : ABHIJEET NATU"),
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
		fprintf(gpFile, "Loading Texture for Mountain Failed\n");
		uninitialize();
	}

	else if (iRetVal == -6)
	{
		fprintf(gpFile, "Loading Texture for Background Failed\n");
		uninitialize();
	}

	else if (iRetVal == -7)
	{
		fprintf(gpFile, "Loading Texture for Sun Failed\n");
		uninitialize();
	}

	else if (iRetVal == -8)
	{
		fprintf(gpFile, "Loading Texture for Tree Failed\n");
		uninitialize();
	}

	else if (iRetVal == -9)
	{
		fprintf(gpFile, "Loading Texture for Cloud Failed\n");
		uninitialize();
	}

	else if (iRetVal == -10)
	{
		fprintf(gpFile, "Loading Texture for Grass Failed\n");
		uninitialize();
	}

	else if (iRetVal == -11)
	{
		fprintf(gpFile, "Loading Texture for Home Failed\n");
		uninitialize();
	}

	else if (iRetVal == -12)
	{
		fprintf(gpFile, "Loading Texture for Door Failed\n");
		uninitialize();
	}

	else if (iRetVal == -13)
	{
		fprintf(gpFile, "Loading Texture for window Failed\n");
		uninitialize();
	}

	else if (iRetVal == -14)
	{
		fprintf(gpFile, "Loading Texture for branch Failed\n");
		uninitialize();
	}

	else if (iRetVal == -15)
	{
		fprintf(gpFile, "Loading Texture for Birdfood Failed\n");
		uninitialize();
	}

	else if (iRetVal == -16)
	{
		fprintf(gpFile, "Loading Texture for Opening Scene Failed\n");
		uninitialize();
	}

	else if (iRetVal == -17)
	{
		fprintf(gpFile, "Loading Texture for Ending Scene Failed\n");
		uninitialize();
	}

	else
	{
		fprintf(gpFile, "Making OpenGL context as current context Successful\n");
	}

	//show window

	ShowWindow(hwnd, iCmdShow);

	ToggleFullScreen();

	//PLAYSOUND FUNCTION 

	PlaySound(TEXT("D:\\rtr4_0\\01-Windows\\01-OpenGL\\01-FFP\\01_Windows\\DataStructureProject\\UthaUthaChiutaai.wav"), NULL, SND_ASYNC);
	
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
	BOOL LoadGLTexture(GLuint*, TCHAR[]);
	void uninitialize(void);


	void InitializeCountdownTimer(void);


	//Variable Declarations
	PIXELFORMATDESCRIPTOR pfd;
	int	iPixelFormatIndex = 0;

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
	
	//clear the screen 
	glClearColor(0.0f,0.0f, 0.0f, 1.0f);


	//Depth Related changes
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	quadric = gluNewQuadric();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	//TEXTURE RELATED CHANGES

	if (LoadGLTexture(&texture_mountain, MAKEINTRESOURCE(IDBITMAP_MOUNTAIN)) == FALSE)
	{
		uninitialize();
		return(-5);
	}


	if (LoadGLTexture(&texture_background, MAKEINTRESOURCE(IDBITMAP_BACKGROUND)) == FALSE)
	{
		uninitialize();
		return(-6);
	}

	if (LoadGLTexture(&texture_sun, MAKEINTRESOURCE(IDBITMAP_SUN)) == FALSE)
	{
		uninitialize();
		return(-7);
	}

	if (LoadGLTexture(&texture_tree, MAKEINTRESOURCE(IDBITMAP_TREE)) == FALSE)
	{
		uninitialize();
		return(-8);
	}

	if (LoadGLTexture(&texture_cloud, MAKEINTRESOURCE(IDBITMAP_CLOUD)) == FALSE)
	{
		uninitialize();
		return(-9);
	}

	if (LoadGLTexture(&texture_grass, MAKEINTRESOURCE(IDBITMAP_GRASS)) == FALSE)
	{
		uninitialize();
		return(-10);
	}


	if (LoadGLTexture(&texture_home, MAKEINTRESOURCE(IDBITMAP_HOME)) == FALSE)
	{
		uninitialize();
		return(-11);
	}

	if (LoadGLTexture(&texture_door, MAKEINTRESOURCE(IDBITMAP_DOOR)) == FALSE)
	{
		uninitialize();
		return(-12);
	}

	if (LoadGLTexture(&texture_window, MAKEINTRESOURCE(IDBITMAP_WINDOW)) == FALSE)
	{
		uninitialize();
		return(-13);
	}

	if (LoadGLTexture(&texture_branch, MAKEINTRESOURCE(IDBITMAP_BRANCH)) == FALSE)
	{
		uninitialize();
		return(-14);
	}

	if (LoadGLTexture(&texture_birdfood, MAKEINTRESOURCE(IDBITMAP_BIRDFOOD)) == FALSE)
	{
		uninitialize();
		return(-15);
	}

	if (LoadGLTexture(&texture_OpeningScene, MAKEINTRESOURCE(IDBITMAP_OPENINGSCENE)) == FALSE)
	{
		return(-16);
	}

	if (LoadGLTexture(&texture_Photo1, MAKEINTRESOURCE(IDBITMAP_PHOTO1)) == FALSE)
	{
		return(-17);
	}
	if (LoadGLTexture(&texture_Photo2, MAKEINTRESOURCE(IDBITMAP_PHOTO2)) == FALSE)
	{
		return(-17);
	}
	if (LoadGLTexture(&texture_Photo3, MAKEINTRESOURCE(IDBITMAP_PHOTO3)) == FALSE)
	{
		return(-17);
	}
	if (LoadGLTexture(&texture_Photo4, MAKEINTRESOURCE(IDBITMAP_PHOTO4)) == FALSE)
	{
		return(-17);
	}

	//Enabling the texture
	glEnable(GL_TEXTURE_2D);

	//COUNTDOWN TIMER
	InitializeCountdownTimer();

	DisplayQueue();

	gliEventTime = Dequeue();

	DisplayQueue();


	if (!QueryPerformanceCounter(&gliInitialTimestamp))
	{
		//error check
	}

	QueryPerformanceFrequency(&frequency);
	frequency.QuadPart /= 1000;



	//Warmup  resize call
	resize(WIN_WIDTH, WIN_HEIGHT);
	return 0;

}

void InitializeCountdownTimer(void)
{
	Enqueue(SECONDS_TO_MICROSECONDS(0));
	Enqueue(SECONDS_TO_MICROSECONDS(0));
	Enqueue(SECONDS_TO_MICROSECONDS(45));
	Enqueue(SECONDS_TO_MICROSECONDS(56));

	liFirstEventTime = Dequeue();
}


BOOL LoadGLTexture(GLuint* texture, TCHAR imageresourceID[])		
{
	//variable declarations
	HBITMAP hbitmap = NULL;
	BITMAP bmp;
	BOOL bResult = FALSE;

	//code

	hbitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), imageresourceID, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

	if (hbitmap)
	{
		bResult = TRUE;
		GetObject(hbitmap, sizeof(BITMAP), &bmp);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//Create Texture
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);

		glBindTexture(GL_TEXTURE_2D, 0);
		DeleteObject(hbitmap);
	}
	return bResult;
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

void OpeningCredits(void)
{
	glTranslatef(0.0f, transY, -3.0f);
	glBindTexture(GL_TEXTURE_2D, texture_OpeningScene);
	glBegin(GL_QUADS);
	
	glColor3f(1.0f, 1.0f, 1.0f);
	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.1f, 1.0f, 0.008f);    

	glTexCoord3f(0.0f, 1.0f, 1.0f);
	glVertex3f(-1.1f, 1.0f, 0.008f);	 

	glTexCoord3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-1.1f, -1.0f, 0.008f);		

	glTexCoord3f(1.0f, 0.0f, 1.0f);
	glVertex3f(1.1f, -1.0f, 0.008f);		

	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Scene1(void)
{
	void DrawHome(void);
	void DrawSun(void);
	void DrawBackground(void);
	void DrawMountains(void);
	void DrawGrass(void);
	void DrawTree(int, int, float, float);
	void DrawLeaf(void);
	void DrawLeaf1(void);
	void treeInit(int);

	//HOME
	glLoadIdentity();
	glTranslatef(-1.1f, -1.5f, -2.0f);
	DrawHome();

	glLoadIdentity();
	glTranslatef(4.2f, 2.5f, -15.0f);
	DrawSun();

	glLoadIdentity();
	glTranslatef(7.5f, -0.36f, -22.0f);
	glScalef(2.15f, 2.15f, 2.15f);
	DrawMountains();

	glLoadIdentity();
	glTranslatef(12.0f, -0.36f, -24.0f);
	glScalef(2.15f, 2.15f, 2.15f);
	DrawMountains();

	glLoadIdentity();
	glTranslatef(-1.5f, -0.36f, -24.0f);
	glScalef(2.15f, 2.15f, 2.15f);
	DrawMountains();

	glLoadIdentity();
	glTranslatef(2.0f, -0.36f, -22.0f);
	glScalef(2.15f, 2.15f, 2.15f);
	DrawMountains();

	glLoadIdentity();
	glTranslatef(-12.0f, -0.36f, -23.0f);
	glScalef(2.15f, 2.15f, 2.15f);
	glColor3f(0.16f, 0.081f, 0.0f);
	DrawMountains();
	
	glLoadIdentity();
	glTranslatef(0.0f, -2.5f, -17.0f);
	glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
	DrawTree(5, 0, 0.45, 6.2);

	glLoadIdentity();
	DrawLeaf();

	glLoadIdentity();
	glTranslatef(-11.0f, -2.4f, -17.0f);
	glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
	DrawTree(5, 0, 0.35, 6.2);

	glTranslatef(-11.0f, -2.4f, -20.0f);
	glLoadIdentity();
	DrawLeaf1();

	glLoadIdentity();
	glTranslatef(transBackground, 0.0f, -28.0f);
	DrawBackground();

	glLoadIdentity();
	glTranslatef(-3.0f, 0.0f, -16.0f);
	DrawGrass();

	glLoadIdentity();
	gluLookAt(x, y+2, z + 6, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glTranslatef(-9.5f, -5.8f, -10.0f);
	
	treeInit(2);																			

	glLoadIdentity();
	gluLookAt(x, y + 2, z+ 6, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glTranslatef(-5.5f, -5.8f, -10.0f);
	
	treeInit(2);																			

	glLoadIdentity();
	gluLookAt(x, y + 2, z+ 6, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glTranslatef(2.8f, -5.8f, -10.0f);
	
	treeInit(2);																			
	
	glLoadIdentity();
	gluLookAt(x, y + 2, z+ 6, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glTranslatef(6.5f, -5.8f, -10.0f);
	
	treeInit(2);																			

	glLoadIdentity();
	gluLookAt(x, y+2, z + 6, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glTranslatef(10.5f, -5.8f, -10.0f);
	
	treeInit(2);	
}

void Scene2(void)
{
	void DrawBirdFood(void);
	void DrawBird(int,int);

	glLoadIdentity();

	DrawBirdFood();

	glLoadIdentity();
	glTranslatef(birdX, birdY, -10.0f);
	glScalef(0.12f, 0.12f, 0.12f);
	DrawBird(1, 1);

	glLoadIdentity();
	glTranslatef(birdX + 0.99f, birdY + 0.35f, -10.0f);
	glScalef(0.12f, 0.12f, 0.12f);
	DrawBird(1, 1);

	glLoadIdentity();
	glTranslatef(birdX - 0.95f, birdY + 0.45f, -10.0f);
	glScalef(0.12f, 0.12f, 0.12f);
	DrawBird(1, 1);

	glLoadIdentity();
	if(!singleBirdFlag)
	{ 
		glTranslatef(-5.0f , -1.0f, -10.0f);
	}
	if (singleBirdFlag)
	{
		glTranslatef(newbirdX,newbirdY, -10.0f);
	}
	glScalef(0.12f, 0.12f, 0.12f);
	DrawBird(1, 1);
}

void EndCredits(GLuint texture)
{
	
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, 1.1f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, 1.1f, 1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.1f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.1f, 1.0f);
	glEnd();

	glBindTexture(0, texture);

}

void display(void)
{
	void OpeningCredits(void);
	void Scene1(void);
	void Scene2(void);
	void EndCredits(GLuint);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	
	if (openingCredits_flag)
	{
		OpeningCredits();
	}

	if(scene1_flag)
	{
		Scene1();
	}

	if (scene2_flag)
	{
		Scene2();
	}


	//END CREDITS 

	if (flag1)
	{
		glLoadIdentity();
		glClearColor(0.0f, 0.0f, 0.0f,1.0f);
		glTranslatef(0.0f, transPhoto1, -3.0f);
		EndCredits(texture_Photo1);
	}

	if (flag2)
	{
		glLoadIdentity();
		glTranslatef(0.0f, transPhoto2, -3.0f);
		EndCredits(texture_Photo2);
	}

	if (flag3)
	{
		glLoadIdentity();
		glTranslatef(0.0f, transPhoto3, -3.0f);
		EndCredits(texture_Photo3);
	}

	if (flag4)
	{
		glLoadIdentity();
		glTranslatef(0.0f, transPhoto4, -3.0f);
		EndCredits(texture_Photo4);
	}

	SwapBuffers(ghdc);
}

// RELATED TO TREE
void DrawTree(int n, int arg, float mult, float v)
{
	/*
		Function DrawTree:
		Arguments:
			n - number of sides
			arg - starting angle (not important)
			mult - multiplying sides to increase their length
			v - cylinder height*/
	

	// Cylinder Bottom
	glBegin(GL_POLYGON);
	glColor3f(0.0f, 0.0f, 0.0f);

	for (int i = arg; i <= (360 + arg); i += (360 / n))
	{
		float a = i * M_PI / 180; // degrees to radians

		glVertex3f(mult * cos(a), mult * sin(a), 0.0);

	}

	glEnd();
	// Cylinder Top
	glBegin(GL_POLYGON);
	glColor3f(0.0f, 0.0f, 0.0f);
	for (int i = arg; i <= (360 + arg); i += (360 / n))
	{
		float a = i * M_PI / 180; // degrees to radians

		glVertex3f(mult * cos(a), mult * sin(a), v);
	}
	glEnd();
	// Cylinder "Cover"
	glColor3f(0.25f, 0.17f, 0.141f);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, texture_branch);

	glBegin(GL_QUAD_STRIP);
	for (int i = arg; i < 480; i += (360 / n))
	{
		float a = i * M_PI / 180; // degrees to radians
		glTexCoord3f(mult * cos(a), mult * sin(a), 0.0);
		glVertex3f(mult * cos(a), mult * sin(a), 0.0);
		glTexCoord3f(mult * cos(a), mult * sin(a), v);
		glVertex3f(mult * cos(a), mult * sin(a), v);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void DrawLeaf(void)
{
	glLoadIdentity();

	glTranslatef(0.0f, 1.45f, -10.0f);

	//Beautification 1
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

	//Beautification 2
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Beautification 3
	glColor3f(0.0f, 1.0f, 0.0f);		//Green
	glBindTexture(GL_TEXTURE_2D, texture_tree);

	//Draw Sphere
	gluSphere(quadric, 0.45f, 20, 20);
	glBindTexture(GL_TEXTURE_2D, 0);

	glLoadIdentity();
	glTranslatef(0.45f, 1.45f, -10.0f);

	//Beautification 3
	glColor3f(0.0f, 1.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);
	glLoadIdentity();
	glTranslatef(-0.10f, 1.55f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 10, 10);

	glLoadIdentity();
	glTranslatef(-0.40f, 1.45f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);

	glLoadIdentity();
	glTranslatef(0.25f, 2.5f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);
	glLoadIdentity();

	glTranslatef(-0.30f, 2.5f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);
	glLoadIdentity();
	glTranslatef(-0.45f, 1.95f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);
	glLoadIdentity();
	glTranslatef(-0.35f, 2.05f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);
	glLoadIdentity();
	glTranslatef(-0.40f, 2.05f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);
	glLoadIdentity();
	glTranslatef(-0.43f, 1.95f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);
	glLoadIdentity();
	glTranslatef(0.43f, 1.95f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);
	glLoadIdentity();
	glTranslatef(0.40, 1.95f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);
	glLoadIdentity();
	glTranslatef(-0.40, 1.95f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);

	//CHANGE
	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);
	glLoadIdentity();
	glTranslatef(-0.60, 2.15f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);


	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);
	glLoadIdentity();
	glTranslatef(-0.70, 2.35f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);


	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);
	glLoadIdentity();
	glTranslatef(-0.70, 2.0f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);
	glLoadIdentity();
	glTranslatef(0.70, 2.0f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);
	glLoadIdentity();
	glTranslatef(-0.40, 2.0f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);


	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);
	glLoadIdentity();
	glTranslatef(0.40, 2.0f, -10.0f);
	glBindTexture(GL_TEXTURE_2D, 0);


	glColor3f(0.0f, 1.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 20, 20);
	glBindTexture(GL_TEXTURE_2D, 0);
	glLoadIdentity();
}

void DrawLeaf1(void)
{

	glLoadIdentity();

	glTranslatef(-6.5f, 1.45f, -10.0f);

	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glColor3f(0.0f, 1.0f, 0.0f);		//Green

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	//Draw Sphere
	gluSphere(quadric, 0.45f, 30, 30);
	glBindTexture(GL_TEXTURE_2D, 0);

	glLoadIdentity();
	glTranslatef(-6.45f, 1.45f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glColor3f(0.0f, 1.0f, 0.0f);		//Green
	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);
	glLoadIdentity();
	glTranslatef(-6.10f, 1.55f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);

	glLoadIdentity();
	glTranslatef(-6.40f, 1.45f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);

	glLoadIdentity();
	glTranslatef(-5.95f, 2.5f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);
	glLoadIdentity();

	glTranslatef(-6.30f, 2.5f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);
	glLoadIdentity();
	glTranslatef(-6.45f, 1.95f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);
	glLoadIdentity();
	glTranslatef(-6.35f, 2.05f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);
	glLoadIdentity();
	glTranslatef(-6.40f, 2.05f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);
	glLoadIdentity();
	glTranslatef(-6.43f, 1.95f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);
	glLoadIdentity();
	glTranslatef(-6.13f, 1.95f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);
	glLoadIdentity();
	glTranslatef(-6.10, 1.95f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);
	glLoadIdentity();
	glTranslatef(-6.40, 1.95f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);
	glLoadIdentity();
	glTranslatef(-6.60, 2.15f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);
	glLoadIdentity();
	glTranslatef(-6.70, 2.35f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);
	glLoadIdentity();
	glTranslatef(-6.70, 2.0f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);
	glLoadIdentity();
	glTranslatef(-6.10, 2.0f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);
	glLoadIdentity();
	glTranslatef(-6.40, 2.0f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);


	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);
	glLoadIdentity();
	glTranslatef(-6.70, 2.0f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, 0);


	glColor3f(0.0f, 1.0f, 0.0f);

	glBindTexture(GL_TEXTURE_2D, texture_tree);
	gluSphere(quadric, 0.45f, 30, 30);
	glBindTexture(GL_TEXTURE_2D, 0);
	glLoadIdentity();
}

void drawtree(int n)
{
	if (n > 0)
	{
		//////////////////////////
		//stack of matrices
		glPushMatrix();

		glTranslatef(-0.04, 1.8, 0.0);
		glRotatef(45, 0.0, 0.0, 1.0);
		glRotatef(20, 1.0, 0.0, 0.0);
		glScalef(0.707, 0.707, 0.907);

		//recursion
		drawtree(n - 1.0);

		glPopMatrix();
		////////////////////////////
		glPushMatrix();
		glTranslatef(-0.04, 1.8, 0.0);
		glRotatef(-45, 0.0, 0.0, 1.0);
		glRotatef(20, 1.0, 0.0, 0.0);
		glScalef(0.907, 0.907, 0.907);
		drawtree(n - 1.0);
		glPopMatrix();


		glPushMatrix();
		glTranslatef(-0.04, 1.8, 0.0);
		glRotatef(-45, 1.0, 0.0, 0.0);
		glScalef(0.707, 0.707, 0.707);
		drawtree(n - 1.0);
		glPopMatrix();

		////////////////////////////

		glColor3f(0.57f, 0.39f, 0.1f);
		glRotatef(-90, 1.0, 0.0, 0.0);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, texture_branch);
		gluCylinder(quadric, 0.22f, 0.12f, 1.8f, 30, 30);
		glBindTexture(GL_TEXTURE_2D, 0);

	}


	/// leaves // change as per requirement  
	if (n == 0)
	{
		glColor3f(0.2f, 0.8f, 0.2f);
		glBindTexture(GL_TEXTURE_2D, texture_tree);
		gluSphere(quadric, 1.0, 30, 30);
		glBindTexture(GL_TEXTURE_2D, 0);

		glTranslatef(-1.0, -0.0, 0.0);
		glColor3f(0.4f, 1.0f, 0.3f);
		glBindTexture(GL_TEXTURE_2D, texture_tree);
		gluSphere(quadric, 0.7, 30, 30);

		glTranslatef(0.0, 1.0, 0.0);
		glColor3f(0.1f, 0.9f, 0.1f);
		gluSphere(quadric, 0.7, 30, 30);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

}

void treeInit(int n)
{
	drawtree(n);
}

void DrawHome(void)
{
	glBindTexture(GL_TEXTURE_2D, texture_home);

	glTranslatef(1.0f, 1.0f, 0.0f);
	glRotatef(180, 0.0f, 1.0f, 0.0f);
	glScalef(1.0f, 1.0f, 1.0f);
	glColor3f(1.0, 0.25, 1.0);
	glColor3f(.5, 0.5, .25);

	//main rectangle
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_POLYGON);
	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.2f, 0.1f, 0.0f);
	glTexCoord3f(0.0f, 1.0f, 1.0f);
	glVertex3f(0.9f, 0.1f, 0.0f);
	glTexCoord3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.9f, 0.575f, 0.0f);
	glTexCoord3f(1.0f, 0.0f, 1.0f);
	glVertex3f(0.2f, 0.575f, 0.0f);
	glEnd();

	//back rectangle
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_POLYGON);
	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.2f, 0.1f, 0.5f);
	glTexCoord3f(0.0f, 1.0f, 1.0f);
	glVertex3f(0.9f, 0.1f, 0.5f);
	glTexCoord3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.9f, 0.575f, 0.5f);
	glTexCoord3f(1.0f, 0.0f, 1.0f);
	glVertex3f(0.2f, 0.575f, 0.5f);
	glEnd();

	//left rectangle
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_POLYGON);
	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.2f, 0.1f, 0.5f);
	glTexCoord3f(0.0f, 1.0f, 1.0f);
	glVertex3f(0.2f, 0.1f, 0.0f);
	glTexCoord3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.2f, 0.575f, 0.0f);
	glTexCoord3f(1.0f, 0.0f, 1.0f);
	glVertex3f(0.2f, 0.575f, 0.5f);
	glEnd();

	//right rectangle
	glColor3f(0.75f, 0.75f, 0.25f);
	glBegin(GL_POLYGON);
	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.9f, 0.1f, 0.5f);
	glTexCoord3f(0.0f, 1.0f, 1.0f);
	glVertex3f(0.9f, 0.1f, 0.0f);
	glTexCoord3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.9f, 0.575f, 0.0f);
	glTexCoord3f(1.0f, 0.0f, 1.0f);
	glVertex3f(0.9f, 0.575f, 0.5f);
	glEnd();

	//left triangle
	glColor3f(0.5f, 0.5f, 0.25f);
	glBegin(GL_TRIANGLES);
	glTexCoord3f(0.5f, 1.0f, 0.0f);
	glVertex3f(0.9f, 0.575f, 0.0f);
	glTexCoord3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.9f, 0.575f, 0.5f);
	glTexCoord3f(1.0f, 0.0f, 1.0f);
	glVertex3f(0.9f, 0.8f, 0.25f);
	glEnd();

	//right triangle
	glColor3f(0.5f, 0.5f, 0.25f);
	glBegin(GL_TRIANGLES);
	glTexCoord3f(0.5f, 1.0f, 0.0f);
	glVertex3f(0.2f, 0.575f, 0.0f);
	glTexCoord3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.2f, 0.575f, 0.5f);
	glTexCoord3f(1.0f, 0.0f, 1.0f);
	glVertex3f(0.2f, 0.8f, 0.25f);
	glEnd();

	//roof
	glColor3f(0.55f, 0.35f, 0.2f);
	glBegin(GL_POLYGON);
	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.2f, 0.575f, 0.0f);
	glTexCoord3f(0.0f, 1.0f, 1.0f);
	glVertex3f(0.9f, 0.575f, 0.0f);
	glTexCoord3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.9f, 0.8f, 0.25f);
	glTexCoord3f(1.0f, 0.0f, 1.0f);
	glVertex3f(0.2f, 0.8f, 0.25f);
	glEnd();

	//back roof
	glColor3f(0.55f, 0.35f, 0.2f);
	glBegin(GL_POLYGON);
	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.2f, 0.575f, 0.5f);
	glTexCoord3f(0.0f, 1.0f, 1.0f);
	glVertex3f(0.9f, 0.575f, 0.5f);
	glTexCoord3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.9f, 0.8f, 0.25f);
	glTexCoord3f(1.0f, 0.0f, 1.0f);
	glVertex3f(0.2f, 0.8f, 0.25f);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	//door 
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture_door);
	glBegin(GL_POLYGON);
	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.47f, 0.105f, 0.0f);
	glTexCoord3f(0.0f, 1.0f, 1.0f);
	glVertex3f(0.65f, 0.105f, 0.0f);
	glTexCoord3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.65f, 0.46f, 0.0f);
	glTexCoord3f(1.0f, 0.0f, 1.0f);
	glVertex3f(0.47f, 0.46f, 0.0f);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	//window 1 
	glColor3f(0.3f, 0.2f, 0.1f);
	glBindTexture(GL_TEXTURE_2D, texture_window);
	glBegin(GL_POLYGON);
	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.72f, 0.25f, 0.0f);
	glTexCoord3f(0.0f, 1.0f, 1.0f);
	glVertex3f(0.83f, 0.25f, 0.0f);
	glTexCoord3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.83f, 0.4f, 0.0f);
	glTexCoord3f(1.0f, 0.0f, 1.0f);
	glVertex3f(0.72f, 0.4f, 0.0f);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	//window 2
	glColor3f(0.3f, 0.2f, 0.1f);
	glBindTexture(GL_TEXTURE_2D, texture_window);
	glBegin(GL_POLYGON);
	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.27f, 0.25f, 0.0f);
	glTexCoord3f(0.0f, 1.0f, 1.0f);
	glVertex3f(0.38f, 0.25f, 0.0f);
	glTexCoord3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.38f, 0.4f, 0.0f);
	glTexCoord3f(1.0f, 0.0f, 1.0f);
	glVertex3f(0.27f, 0.4f, 0.0f);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void DrawSun(void)
{
	//View Transformation/Camera Transformation
	gluLookAt(0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	glTranslatef(12.5f, transSun, -13.0f);
	//Beautification 1
	glRotatef(180.0f, 1.0f, 0.0f, 0.0f);

	//Beautification 2
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Beautification 3
	glColor3f(1.0f, 1.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, texture_sun);
	gluQuadricTexture(quadric, 1);
	//Draw Sphere
	gluSphere(quadric, 1.68f, 30, 30);
	glBindTexture(GL_TEXTURE_2D, 0);

}


void DrawMountains(void)
{
	glBindTexture(GL_TEXTURE_2D, texture_mountain);
	glBegin(GL_TRIANGLES);
	//Front Face
	glColor3f(0.54f, 0.45f, 0.36f);
	glTexCoord3f(0.5f, 1.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glTexCoord3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-1.5f, -1.0f, 1.0f);

	glTexCoord3f(1.0f, 0.0f, 1.0f);
	glVertex3f(1.5f, -1.0f, 1.0f);


	//Right Face
	glColor3f(0.54f, 0.45f, 0.36f);
	glTexCoord3f(0.5f, 1.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glTexCoord3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.5f, -1.0f, 1.0f);

	glTexCoord3f(0.0f, -1.0f, 0.0f);
	glVertex3f(1.5f, -1.0f, -1.0f);


	//Back Face
	glColor3f(0.54f, 0.45f, 0.36f);
	glTexCoord3f(0.5f, 1.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);


	glTexCoord3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.5f, -1.0f, -1.0f);

	glTexCoord3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-1.5f, -1.0f, -1.0f);

	//Left Face
	glColor3f(0.54f, 0.45f, 0.36f);
	glTexCoord3f(0.5f, 1.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glTexCoord3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-1.5f, -1.0f, -1.0f);

	glTexCoord3f(1.0f, 0.0f, 0.0f);
	glVertex3f(-1.5f, -1.0f, 1.0f);

	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);

}

void DrawBackground(void)
{
	glBindTexture(GL_TEXTURE_2D, texture_background);
	glBegin(GL_QUADS);
	glColor3f(0.0f, fade2, fade1);
	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(100.2f, 18.0f, -8.0f);
	glTexCoord3f(0.0f, 1.0f, 1.0f);
	glVertex3f(-100.2f, 18.0f, -8.0f);
	glTexCoord3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-100.2f, -6.0f, -8.0f);
	glTexCoord3f(1.0f, 0.0f, 1.0f);
	glVertex3f(100.32f, -6.0f, -8.0f);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void DrawGrass(void)
{
	glBindTexture(GL_TEXTURE_2D, texture_grass);
	glBegin(GL_QUADS);
	glColor3f(0.0f, fade2, fade1);
	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(25.2f, -14.0f, -10.0f);
	glTexCoord3f(0.0f, 1.0f, 1.0f);
	glVertex3f(-20.52f, -14.0f, -10.0f);
	glTexCoord3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-20.52f, -3.0f, -10.0f);
	glTexCoord3f(1.0f, 0.0f, 1.0f);
	glVertex3f(25.32f, -3.0f, -10.0f);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void DrawBird(int x, int y)
{
	void circle(GLfloat rx, GLfloat ry, GLfloat x, GLfloat y);

	//feathers
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glColor3f(0.2f, 0.26f, 0.95f);
	glVertex2f(x + 5.0f, y);
	glColor3f(0.45f, 0.11f, 0.11f);
	glVertex2f(x + 3.5f, y - 2.5f);
	glColor3f(0.45f, 0.11f, 0.11f);
	glVertex2f(x + 1.5f, y - 2.5f);
	glEnd();

	//body
	glColor3f(0.9f, 0.06f, 0.20f);
	circle(2.5, 2, x + 2.55, y - 4.55);

	glBegin(GL_TRIANGLES);
	//lips
	glColor3ub(55, 220, 60);
	glVertex2f(x - 1.2, y - 5);
	glVertex2f(x + .7, y - 3.1);
	glColor3f(1.0f, 0.20f, 0.160f);
	glVertex2f(x + .7, y - 6);

	//tail
	glColor3f(0.55f, 0.55f, 0.55f);
	glVertex2f(x + 4.8, y - 5);
	glColor3f(0.55f, 0.01f, 0.05f);
	glVertex2f(x + 7, y - 5);
	glColor3f(0.95f, 0.51f, 0.19f);
	glVertex2f(x + 7, y - 1);
	glEnd();
	//eye
	glColor3f(0.0f, 0.0f, 0.10f);
	circle(0.3, 0.41, x + 0.1, y - 4.6);

	//legs
	glBegin(GL_QUADS);
	glColor3f(0.0f, 0.0f, 0.12f);
	glVertex2f(x + 1.5, y - 6.5);
	glVertex2f(x + 1.5, y - 8);
	glVertex2f(x + 2, y - 8);
	glVertex2f(x + 2, y - 6.5);

	glColor3f(0.0f, 0.0f, 0.12f);
	glVertex2f(x + 2.5, y - 6.5);
	glVertex2f(x + 2.5, y - 8);
	glVertex2f(x + 3, y - 8);
	glVertex2f(x + 3, y - 6.5);
	glEnd();

	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_TRIANGLES);
	//1 front
	glVertex2f(x + 1.5, y - 8);
	glVertex2f(x + 2.5, y - 8);
	glVertex2f(x + .5, y - 8.5);

	glVertex2f(x + 1.5, y - 8);
	glVertex2f(x + 2, y - 8);
	glVertex2f(x + 1.5, y - 8.9);

	glVertex2f(x + 1.5, y - 8);
	glVertex2f(x + 2, y - 8);
	glVertex2f(x + 2.5, y - 8.9);

	//1 rear leg
	glVertex2d(x + 2.5, y - 8);
	glVertex2d(x + 3, y - 8);
	glVertex2d(x + 2, y - 8.8);

	glVertex2d(x + 2.5, y - 8);
	glVertex2d(x + 3, y - 8);
	glVertex2d(x + 2.9, y - 9.4);

	glVertex2d(x + 2.5, y - 8);
	glVertex2d(x + 3, y - 8);
	glVertex2d(x + 4, y - 9);
	glEnd();

}

void circle(GLfloat rx, GLfloat ry, GLfloat x, GLfloat y)
{
	int i = 0;
	float angle;
	GLfloat PI = 2.0f * 3.1416;
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x, y);
	for (i = 0; i < 100; i++)
	{
		angle = 2 * PI * i / 100;
		glVertex2f(x + (cos(angle) * rx), y + (sin(angle) * ry));
	}
	glEnd();
}

void DrawBirdFood(void)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture_birdfood);
	gluQuadricTexture(quadric, 1);
	gluSphere(quadric, 0.10f, 30, 30);

	glLoadIdentity();
	glTranslatef(translateBirdFood + 0.8f, -1.7f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture_birdfood);
	gluSphere(quadric, 0.12f, 30, 30);

	glLoadIdentity();
	glTranslatef(translateBirdFood + 0.6f, -1.8f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture_birdfood);
	gluSphere(quadric, 0.11f, 30, 30);

	glLoadIdentity();
	glTranslatef(translateBirdFood + 0.4f, -1.7f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture_birdfood);
	gluSphere(quadric, 0.10f, 30, 30);

	glLoadIdentity();
	glTranslatef(translateBirdFood + 0.2f, -1.8f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture_birdfood);
	gluSphere(quadric, 0.11f, 30, 30);

	glLoadIdentity();
	glTranslatef(translateBirdFood, -1.7f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture_birdfood);
	gluSphere(quadric, 0.12f, 30, 30);

	glLoadIdentity();
	glTranslatef(translateBirdFood - 0.2f, -1.7f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture_birdfood);
	gluSphere(quadric, 0.10f, 30, 30);

	glLoadIdentity();
	glTranslatef(translateBirdFood - 0.4f, -1.8f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture_birdfood);
	gluSphere(quadric, 0.11f, 30, 30);

	glLoadIdentity();
	glTranslatef(translateBirdFood - 0.6f, -1.7f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture_birdfood);
	gluSphere(quadric, 0.10f, 30, 30);

	glLoadIdentity();
	glTranslatef(translateBirdFood - 0.8f, -1.8f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture_birdfood);
	gluSphere(quadric, 0.10f, 30, 30);

	glLoadIdentity();
	glTranslatef(translateBirdFood - 1.0f, -1.7f, -10.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture_birdfood);
	gluSphere(quadric, 0.12f, 30, 30);

	glBindTexture(GL_TEXTURE_2D, 0);
	
}

void update(void)
{
	LARGE_INTEGER delta;
	delta.QuadPart = 0;

	//code

	if (!QueryPerformanceCounter(&gliCurrentTimestamp))
	{
		//error check
	}

	delta.QuadPart = gliCurrentTimestamp.QuadPart - gliInitialTimestamp.QuadPart;

	delta.QuadPart *= 1000;

	delta.QuadPart /= frequency.QuadPart;

	if (delta.QuadPart >= gliEventTime.QuadPart)
	{
		if (openingCredits_flag)
		{
			transY = transY + 0.006f;
			if (transY >= 3.0f)
			{
				transY = 3.0f;

				openingCredits_flag = FALSE;
				scene1_flag = TRUE;
			}
		}

		if(scene1_flag)
		{
			//FOR FADING BACKGROUND COLOR
			fade1 = fade1 + 0.001f;
			if (fade1 >= 0.85f)
				fade1 = 0.85f;

			fade2 = fade2 + 0.001f;
			if (fade2 >= 0.65f)
				fade2 = 0.65f;

			transBackground = transBackground + 0.01f;

			// FOR SUN ANIMATION 
			if (sunFlag)
			{
				transSun = transSun + 0.03f;

				if (transSun >= 6.0f)
				{
					sunFlag = FALSE;
					birdFlag = TRUE;
					scene2_flag = TRUE;
				}
			}
		}

		if (scene2_flag)
		{
			//FOR BIRD
			if (birdFlag)
			{
				birdX = birdX - 0.015f;
				if (birdX <= 3.0f)
				{
					birdX = 3.0f;
					birdYFlag = TRUE;
				}

				if (birdYFlag)
				{
					birdY = birdY - 0.015f;
					if (birdY <= -1.1f)
					{
						birdY = -1.1f;
						birdYFlag = FALSE;
						FoodFlag = TRUE;
						birdFlag = FALSE;
					}
				}
			}
			if (FoodFlag)
			{
				translateBirdFood += 0.007f;
				if (translateBirdFood >= 0.64f)
				{

					translateBirdFood = 0.64f;
					FoodFlag = FALSE;
					//counter 
					birdcounter++;
					if (birdcounter == 1)
					{
						birdExitXFlag = TRUE;
					}
				}
			}
			if (birdExitXFlag)
			{
				birdY = birdY + 0.010f;
				birdX = birdX - 0.010f;

				if (birdY > 3.3f)
				{
					birdY = 3.3f;
				}

				if (birdX < -15.0f)
				{
					birdX = -15.0f;
				}

				if (birdX <= -4.8f)
				{
					singleBirdFlag = TRUE;
				}

				if (singleBirdFlag)
				{
					newbirdX -= 0.012f;
					newbirdY += 0.012f;

					if (newbirdY > 4.9f)
					{
						newbirdY = 4.9f;
					}

				}
				if (newbirdY == 4.9f)
				{
					for(counter =0 ;counter<=200;counter++)
					{
						scene1_flag = FALSE;
						scene2_flag = FALSE;
						flag1 = TRUE;
					}
				}
			}
		}
		
		if (flag1)
		{
			transPhoto1 = transPhoto1 + 0.004f;
			if (transPhoto1 >= 1.3f)
				flag1 = FALSE;
			flag2 = TRUE;
		}

		if (flag2)
		{
			transPhoto2 = transPhoto2 + 0.004f;
			if (transPhoto2 >= 1.3f)
				flag2 = FALSE;
			flag3 = TRUE;
		}


		if (flag3)
		{
			transPhoto3 = transPhoto3 + 0.004f;
			if (transPhoto3 >= 1.3f)
				flag3 = FALSE;
			flag4 = TRUE;
		}


		if (flag4)
		{
			transPhoto4 = transPhoto4 + 0.004f;
			if (transPhoto3 >= 1.3f)
				flag3 = FALSE;
			flag4 = TRUE;
		}
		

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

	if (texture_OpeningScene)
	{
		glDeleteTextures(1, &texture_OpeningScene);
		texture_OpeningScene = NULL;
	}


	if (quadric)
	{
		gluDeleteQuadric(quadric);
		quadric = NULL;
	}

	if (texture_background)
	{
		glDeleteTextures(1, &texture_background);
	}

	if (texture_mountain)
	{
		glDeleteTextures(1, &texture_mountain);

	}

	if (texture_cloud)
	{
		glDeleteTextures(1, &texture_cloud);

	}

	if (texture_tree)
	{
		glDeleteTextures(1, &texture_tree);

	}

	if (texture_sun)
	{
		glDeleteTextures(1, &texture_sun);

	}

	if (texture_grass)
	{
		glDeleteTextures(1, &texture_grass);

	}

	if (texture_home)
	{
		glDeleteTextures(1, &texture_home);

	}

	if (texture_door)
	{
		glDeleteTextures(1, &texture_door);

	}

	if (texture_window)
	{
		glDeleteTextures(1, &texture_window);

	}

	if (texture_birdfood)
	{
		glDeleteTextures(1, &texture_birdfood);

	}

	if (texture_Photo1)
	{
		glDeleteTextures(1, &texture_Photo1);
		texture_Photo1 = NULL;
	}

	if (texture_Photo2)
	{
		glDeleteTextures(1, &texture_Photo2);
		texture_Photo2 = NULL;
	}

	if (texture_Photo3)
	{
		glDeleteTextures(1, &texture_Photo3);
		texture_Photo3 = NULL;
	}

	if (texture_Photo4)
	{
		glDeleteTextures(1, &texture_Photo4);
		texture_Photo4 = NULL;
	}

	if (gpFile)
	{
		fprintf(gpFile, "Log File Successfully Closed !!\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}


