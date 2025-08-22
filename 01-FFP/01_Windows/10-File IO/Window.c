// Header files
#include <windows.h>
#include <stdio.h>		//For File I/O Functions(FILE,fopen_s)
#include <stdlib.h>		//For exit() function

//Global Function Declarations
LRESULT CALLBACK WndProc(HWND , UINT , WPARAM , LPARAM);

//Global Variable Declarations
FILE* gpFile = NULL;   //File is type *pointer, gp = Global Pointer gpFile is Pointer Variable of type FILE 

//Entry Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//variable declarations

	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyWindow");

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
	{
			
	}
	//initialization of WNDCLASSEX structure
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	//Registering above wndClass
	
	//Register WNDCLASSEX

	RegisterClassEx(&wndclass);

	//create the window

	hwnd = CreateWindow(szAppName,
		TEXT("Abhijeet Ajit Natu"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL);

	//show window

	ShowWindow(hwnd, iCmdShow);

	//Update window

	UpdateWindow(hwnd);

	//Message Loop
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return((int)msg.wParam);
}

	//CallBack function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//code
	switch (iMsg)
	{
	case WM_DESTROY:  
		if (gpFile)
		{
			fprintf(gpFile, "Log File Successfully Closed !!\n");
			fclose(gpFile);
			gpFile = NULL;
		}
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}
