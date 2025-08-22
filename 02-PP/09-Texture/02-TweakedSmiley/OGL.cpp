// Header Files
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "OGL.h"
#include "vmath.h"
using namespace vmath;

// OpenGL Header Files
// THIS MUST BE INCLUDED BEFORE gl.h
#include <GL/glew.h>
#include <GL/gl.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// OpenGL Libraries
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "OpenGL32.lib")

// Global Function Declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global Variable Declarations
HWND ghwnd = FALSE;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE;
FILE *gpFile = NULL;

// PP Related Global Variables
GLuint shaderProgramObject;

enum
{
    ADN_ATTRIBUTE_POSITION = 0,
    ADN_ATTRIBUTE_COLOR,
    ADN_ATTRIBUTE_NORMAL,
    ADN_ATTRIBUTE_TEXTURE0
};

// For Pyramid
GLuint vao;
GLuint vbo_position;
GLuint vbo_texcoord;

GLuint mvpMatrixUniform;
GLuint textureSamplerUniform;
GLuint keyPressedUniform;

mat4 perspectiveProjectionMatrix;

GLuint texture_smiley;

int keyPressed = -1;

// Entry Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    // Function Declarations
    int initialize(void);
    void display(void);
    void uninitialize(void);

    // Variable Declarations
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("MyWindow");
    BOOL bDone = FALSE;
    int iRetVal = 0;

    // Code
    if (fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Creation Of Log File Failed ... Exiting Now !!!"), TEXT("File I/O Error"), MB_OK);
		exit(0);
	}

	else
	{
		fprintf(gpFile, "Log File Created Successfully ...\n");
	}

    // Initialization Of WNDCLASSEX Structure
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbWndExtra = 0;
    wndclass.cbClsExtra = 0;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;

    // Register Our Class
    RegisterClassEx(&wndclass);

    // Get Screen Co-ordinates
	int ScreenX = GetSystemMetrics(SM_CXSCREEN);
	int ScreenY = GetSystemMetrics(SM_CYSCREEN);

    // Create Window
    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
        szAppName,
        TEXT("Abhijeet Natu : Smiley Texture"),
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

    // Initialize
    iRetVal = initialize();

    if (iRetVal == -1)
    {
        fprintf(gpFile, "ChoosePixelFormat() Failed !!!\n");
        uninitialize();
    }

    else if (iRetVal == -2)
    {
        fprintf(gpFile, "SetPixelFormat() Failed !!!\n");
        uninitialize();
    }

    else if (iRetVal == -3)
    {
        fprintf(gpFile, "Failed to create OpenGL context!!!\n");
        uninitialize();
    }

    else if (iRetVal == -4)
    {
        fprintf(gpFile, "Failed to make rendering context as current context !!!\n");
        uninitialize();
    }

    else if (iRetVal == -5)
    {
        fprintf(gpFile, "GLEW Initialization Failed !!!\n");
        uninitialize();
    }

    else if (iRetVal == -6)
    {
        fprintf(gpFile, "Failed To Load Smiley Texture !!!\n");
        uninitialize();
    }

    else
    {
        fprintf(gpFile, "initialize() Succeeded !!!\n");
    }

    // Show Window
    ShowWindow(hwnd, iCmdShow);

    // Foregrounding And Focusing The Window
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    // Game Loop
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
            if (gbActiveWindow)
            {
                // Render The Scene
                display();
            }
        }
    }

    uninitialize();

    return (int)msg.wParam;

}

// Callback Function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    // Function Declarations
	void ToggleFullScreen(void);
    void resize(int, int);   

    // Code
    switch(iMsg)
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
        
            switch(wParam)
		    {
			    case 'F':
			    case 'f':
				    ToggleFullScreen();
			    break;

			    default:
			    break;
		    }

        break;

        case WM_KEYDOWN:

            switch(wParam)
		    {
			    case 27:
				    DestroyWindow(hwnd);
			    break;

                case 49:
                    keyPressed = 1;
                    break;

                case 50:
                    keyPressed = 2;
                    break;

                case 51:
                    keyPressed = 3;
                    break;

                case 52:
                    keyPressed = 4;
                    break;

                default:
                    keyPressed = 0;
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

    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void ToggleFullScreen(void)
{
	// Variable Declarations
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
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_NOZORDER);		

		ShowCursor(TRUE);
		gbFullScreen = FALSE;
	
	}

}

int initialize(void)
{
    // Function Declarations
    void resize(int, int);
    void uninitialize(void);
    void printGLInfo(void);
    BOOL loadGLTexture(GLuint*, TCHAR[]);

    // Variable Declarations
    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex;

    // Code
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    // Initialization Of PIXELFORMATDESCRIPTOR Structure
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

    // Get DC
    ghdc = GetDC(ghwnd);

    // Choose Pixel Format
    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if (iPixelFormatIndex == 0)
    {
        return -1;
    }

    // Set The Chosen Pixel Format
    if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        return -2;
    }

    // Create OpenGL Rendering Context
    ghrc = wglCreateContext(ghdc);
    if (ghrc == NULL)
    {
        return -3;
    }

    // Make The Rendering Context As The Current Context
    if (wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        return -4;
    }

    // GLEW Initialization
    if (glewInit() != GLEW_OK)
    {
        return -5;
    }

    // Print OpenGL Information
    printGLInfo();

    // Vertex Shader
    // --------------------------------------------------------------------------------------------------- 
    const GLchar* vertexShaderSourceCode =
        "#version 460 core" \
        "\n" \
        "in vec4 a_position;" \
        "in vec2 a_texcoord;" \
        "uniform mat4 u_mvpMatrix;"	\
        "out vec2 a_texcoord_out;" \
        "void main(void)" \
        "{" \
        "gl_Position = u_mvpMatrix * a_position;" \
        "a_texcoord_out = a_texcoord;" \
        "}";

    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);

    glCompileShader(vertexShaderObject);

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
                glGetShaderInfoLog(vertexShaderObject, GL_INFO_LOG_LENGTH, &written, log);
                fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", log);
                free(log);
                uninitialize();
            }
        }
    }
    // ----------------------------------------------------------------------------------------------------

    // Fragment Shader
    // ----------------------------------------------------------------------------------------------------
    const GLchar* fragmentShaderSourceCode =
        "#version 460 core" \
        "\n" \
        "in vec2 a_texcoord_out;" \
        "uniform sampler2D u_textureSampler;" \
        "uniform int u_keyPressed;" \
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
        "if(u_keyPressed == 1)" \
        "{" \
        "FragColor = texture(u_textureSampler, a_texcoord_out);" \
        "}" \
        "else" \
        "{" \
        "FragColor = vec4(1.0, 1.0, 1.0, 1.0);" \
        "}" \
        "}";

    GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);

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
                glGetShaderInfoLog(fragmentShaderObject, GL_INFO_LOG_LENGTH, &written, log);
                fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", log);
                free(log);
                uninitialize();
            }
        }
    }
    // ----------------------------------------------------------------------------------------------------

    // Shader Program Object
    shaderProgramObject = glCreateProgram();

    glAttachShader(shaderProgramObject, vertexShaderObject);
    glAttachShader(shaderProgramObject, fragmentShaderObject);

    // For Position
    glBindAttribLocation(shaderProgramObject, ADN_ATTRIBUTE_POSITION, "a_position");
    // For Color
    glBindAttribLocation(shaderProgramObject, ADN_ATTRIBUTE_TEXTURE0, "a_texcoord");

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
                glGetProgramInfoLog(shaderProgramObject, GL_INFO_LOG_LENGTH, &written, log);
                fprintf(gpFile, "Shader Program Link Log : %s\n", log);
                free(log);
                uninitialize();
            }
        }
    }

    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_mvpMatrix");
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "u_textureSampler");
    keyPressedUniform = glGetUniformLocation(shaderProgramObject, "u_keyPressed");

    // Declaration of Vertex Data Arrays
    const GLfloat position[] =
    {
        1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f
    };

    // VAO and VBO Related Code
    // -------------------------------------------------------------------------------------------------- //
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // VBO For Position
    glGenBuffers(1, &vbo_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
    glVertexAttribPointer(ADN_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(ADN_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // VBO For Texcoord
    glGenBuffers(1, &vbo_texcoord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord);
    glBufferData(GL_ARRAY_BUFFER, 4*2*sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(ADN_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(ADN_ATTRIBUTE_TEXTURE0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Unbinding of vao
    glBindVertexArray(0); 
    // -------------------------------------------------------------------------------------------------- //
    

    // Here Starts OpenGL Code

    // Depth Related Changes
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Clear The Screen Using Black Color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    if (loadGLTexture(&texture_smiley, MAKEINTRESOURCE(IDBITMAP_SMILEY)) == FALSE)
    {
        return -6;
    }

    // Enabling the texture
    glEnable(GL_TEXTURE_2D);

    // Warmup Resize Call
    resize(WIN_WIDTH, WIN_HEIGHT);
 
    return 0;
}

void printGLInfo(void)
{
    // Local Variable Declarations
    GLint numExtensions = 0;

    // Code
    fprintf(gpFile, "OpenGL Vendor : %s\n", glGetString(GL_VENDOR));
    fprintf(gpFile, "OpenGL Renderer : %s\n", glGetString(GL_RENDERER));
    fprintf(gpFile, "OpenGL Version : %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    
    fprintf(gpFile, "Number Of Supported Extensions : %d\n", numExtensions);

    for (int i = 0; i < numExtensions; i++)
    {
        fprintf(gpFile, "%s\n", glGetStringi(GL_EXTENSIONS, i));
    }
}

BOOL loadGLTexture(GLuint* texture, TCHAR imageResourceID[])
{
    // Variable Declarations
    HBITMAP hBitmap = NULL;
    BITMAP bmp;
    BOOL bResult = FALSE;

    // Code
    hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), imageResourceID, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

    // Error Checking
    if (hBitmap)
    {
        bResult = TRUE;
        GetObject(hBitmap, sizeof(bmp), &bmp);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // Step By 1 For Better Performance In Shaders
        glGenTextures(1, texture);
        glBindTexture(GL_TEXTURE_2D, *texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        // Create the texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, bmp.bmBits);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Unbind Texture
        glBindTexture(GL_TEXTURE_2D, 0);
        DeleteObject(hBitmap);
    }

    return bResult;
}

void resize(int width, int height)
{
    // Code
    if (height == 0)
    {
        height = 1; // To Avoid 'Divide By 0' Illegal Code In Future Code
    }

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    perspectiveProjectionMatrix = vmath::perspective(45.0f, GLfloat(width) / GLfloat(height), 0.1f, 100.0f);
}

void display(void)
{
    // Code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use The shaderProgramObject
    glUseProgram(shaderProgramObject);
    
    // Transformations
    mat4 translationMatrix = mat4::identity();
    mat4 modelViewMatrix = mat4::identity();
    mat4 modelViewProjectionMatrix = mat4::identity();

    translationMatrix = vmath::translate(0.0f, 0.0f, -4.0f);
    modelViewMatrix = translationMatrix; 
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);
    
   

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_smiley);
    glUniform1i(textureSamplerUniform, 0);

    glBindVertexArray(vao);

    GLfloat texCoord[8];

    if (keyPressed == 1)
    {
        texCoord[0] = 0.5f;
        texCoord[1] = 0.5f;
        texCoord[2] = 0.0f;
        texCoord[3] = 0.5f;
        texCoord[4] = 0.0f;
        texCoord[5] = 0.0f;
        texCoord[6] = 0.5f;
        texCoord[7] = 0.0f;
        glUniform1i(keyPressedUniform, 1);
    }

    else if (keyPressed == 2)
    {
        texCoord[0] = 1.0f;
        texCoord[1] = 1.0f;
        texCoord[2] = 0.0f;
        texCoord[3] = 1.0f;
        texCoord[4] = 0.0f;
        texCoord[5] = 0.0f;
        texCoord[6] = 1.0f;
        texCoord[7] = 0.0f;
        glUniform1i(keyPressedUniform, 1);
    }

    else if (keyPressed == 3)
    {
        texCoord[0] = 2.0f;
        texCoord[1] = 2.0f;
        texCoord[2] = 0.0f;
        texCoord[3] = 2.0f;
        texCoord[4] = 0.0f;
        texCoord[5] = 0.0f;
        texCoord[6] = 2.0f;
        texCoord[7] = 0.0f;
        glUniform1i(keyPressedUniform, 1);
    }

    else if (keyPressed == 4)
    {
        texCoord[0] = 0.5f;
        texCoord[1] = 0.5f;
        texCoord[2] = 0.5f;
        texCoord[3] = 0.5f;
        texCoord[4] = 0.5f;
        texCoord[5] = 0.5f;
        texCoord[6] = 0.5f;
        texCoord[7] = 0.5f;
        glUniform1i(keyPressedUniform, 1);
    }

    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoord), texCoord, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Unuse The shaderProgramObject
    glUseProgram(0);
    
    SwapBuffers(ghdc);
}

void update(void)
{
    // Code
}

void uninitialize(void)
{
    // Function Declarations
    void ToggleFullScreen(void);

    // Code
    if (gbFullScreen)
    {
        ToggleFullScreen();
    }

    if (texture_smiley)
    {
        glDeleteTextures(1, &texture_smiley);
        texture_smiley = 0;
    }

    // Deletion and uninitialization of vbo_texcoord
    if (vbo_texcoord)
    {
        glDeleteBuffers(1, &vbo_texcoord);
        vbo_texcoord = 0;
    }
    
    // Deletion and uninitialization of vbo_position
    if (vbo_position)
    {
        glDeleteBuffers(1, &vbo_position);
        vbo_position = 0;
    }

    // Deletion and uninitialization of vao
    if (vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }

    // Shader Uninitialization
    if (shaderProgramObject)
    {
        glUseProgram(shaderProgramObject);

        GLsizei numAttachedShaders;

        glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numAttachedShaders);

        GLuint* shaderObjects = NULL;

        shaderObjects = (GLuint*)malloc(numAttachedShaders * sizeof(GLuint));

        glGetAttachedShaders(shaderProgramObject, numAttachedShaders, &numAttachedShaders, shaderObjects);

        for (GLsizei i = 0; i < numAttachedShaders; i++)
        {
            glDetachShader(shaderProgramObject, shaderObjects[i]);
            glDeleteShader(shaderObjects[i]);
            shaderObjects[i] = 0;
        }

        free(shaderObjects);

        shaderObjects = NULL;

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
		fprintf(gpFile, "Log File Closed Successfully ...");
		fclose(gpFile);
		gpFile = NULL;
	}
    
}
