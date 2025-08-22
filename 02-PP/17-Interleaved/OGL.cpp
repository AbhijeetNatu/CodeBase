// Header Files
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "OGL.h"
#include "vmath.h"
using namespace vmath;

// OpenGL Header Files
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
FILE* gpFile = NULL;

// PP Related Global Variables
GLuint shaderProgramObject;

enum
{
    AAN_ATTRIBUTE_POSITION = 0,
    AAN_ATTRIBUTE_COLOR,
    AAN_ATTRIBUTE_NORMAL,
    AAN_ATTRIBUTE_TEXTURE0
};

GLuint vao_cube;
GLuint vbo;

GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint projectionMatrixUniform;

GLuint laUniform;   // Light Ambient
GLuint ldUniform;   // Light Diffuse
GLuint lsUniform;   // Light Specular
GLuint lightPositionUniform;

GLuint kaUniform;   // Material Ambient
GLuint kdUniform;   // Material Diffuse
GLuint ksUniform;   // Material Specular
GLuint materialShininessUniform;

GLuint textureSamplerUniform;

GLuint lightingEnabledUniform;

mat4 perspectiveProjectionMatrix;

BOOL bLight = FALSE;

float angleCube = 0.0f;

GLfloat lightAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightPosition[] = { 100.0f, 100.0f, 100.0f, 1.0f };

GLfloat materialAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat materialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialShininess = 50.0f;

GLuint texture_marble;

// Entry Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    // Function Declarations
    int initialize(void);
    void display(void);
    void update(void);
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
        TEXT("AAN : OpenGL Interleaved"),
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
        fprintf(gpFile, "Failed To Load Marble Texture !!!\n");
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

                // Update The Scene
                update();
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

        case 'l':
        case 'L':
            if (bLight == FALSE)
                bLight = TRUE;
            else
                bLight = FALSE;
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
        "in vec4 a_color;" \
        "in vec2 a_texcoord;" \
        "in vec3 a_normal;" \
        "uniform mat4 u_modelMatrix;" \
        "uniform mat4 u_viewMatrix;" \
        "uniform mat4 u_projectionMatrix;" \
        "uniform vec4 u_lightPosition;" \
        "uniform int u_lightingEnabled;" \
        "out vec3 transformedNormals;" \
        "out vec3 lightDirection;" \
        "out vec3 viewerVector;" \
        "out vec2 a_texcoord_out;" \
        "out vec4 a_color_out;" \
        "void main(void)" \
        "{" \
        "if (u_lightingEnabled == 1)" \
        "{" \
        "vec4 eyeCoordinates = u_viewMatrix * u_modelMatrix * a_position;" \
        "mat3 normalMatrix = mat3(u_viewMatrix * u_modelMatrix);" \
        "transformedNormals = normalMatrix * a_normal;" \
        "lightDirection = vec3(u_lightPosition) - eyeCoordinates.xyz;" \
        "viewerVector = -eyeCoordinates.xyz;" \
        "}" \
        "gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" \
        "a_texcoord_out = a_texcoord;" \
        "a_color_out = a_color;" \
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
        "in vec3 transformedNormals;" \
        "in vec3 lightDirection;" \
        "in vec3 viewerVector;" \
        "in vec2 a_texcoord_out;" \
        "in vec4 a_color_out;" \
        "uniform sampler2D u_textureSampler;" \
        "uniform vec3 u_la;" \
        "uniform vec3 u_ld;" \
        "uniform vec3 u_ls;" \
        "uniform vec3 u_ka;" \
        "uniform vec3 u_kd;" \
        "uniform vec3 u_ks;" \
        "uniform float u_materialShininess;" \
        "uniform int u_lightingEnabled;" \
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
        "vec3 phong_ads_light;" \
        "if (u_lightingEnabled == 1)" \
        "{" \
        "vec3 ambient = u_la * u_ka;" \
        "vec3 normalized_tranformed_normals = normalize(transformedNormals);" \
        "vec3 normalized_light_direction = normalize(lightDirection);" \
        "vec3 diffuse = u_ld * u_kd * max(dot(normalized_light_direction, normalized_tranformed_normals), 0.0);" \
        "vec3 reflectionVector = reflect(-normalized_light_direction, normalized_tranformed_normals);" \
        "vec3 normalized_viewer_vector = normalize(viewerVector);"
        "vec3 specular = u_ls * u_ks * pow(max(dot(reflectionVector, normalized_viewer_vector), 0.0), u_materialShininess);" \
        "phong_ads_light = ambient + diffuse + specular;" \
        "}" \
        "else" \
        "{" \
        "phong_ads_light = vec3(1.0, 1.0, 1.0);" \
        "}" \
        "FragColor = vec4(phong_ads_light * vec3(texture(u_textureSampler, a_texcoord_out)) * vec3(a_color_out), 1.0);" \
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

    glBindAttribLocation(shaderProgramObject, AAN_ATTRIBUTE_POSITION, "a_position");
    glBindAttribLocation(shaderProgramObject, AAN_ATTRIBUTE_TEXTURE0, "a_texcoord");
    glBindAttribLocation(shaderProgramObject, AAN_ATTRIBUTE_NORMAL, "a_normal");
    glBindAttribLocation(shaderProgramObject, AAN_ATTRIBUTE_COLOR, "a_color");

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

    modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_modelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_viewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");
    laUniform = glGetUniformLocation(shaderProgramObject, "u_la");
    ldUniform = glGetUniformLocation(shaderProgramObject, "u_ld");
    lsUniform = glGetUniformLocation(shaderProgramObject, "u_ls");
    lightPositionUniform = glGetUniformLocation(shaderProgramObject, "u_lightPosition");
    kaUniform = glGetUniformLocation(shaderProgramObject, "u_ka");
    kdUniform = glGetUniformLocation(shaderProgramObject, "u_kd");
    ksUniform = glGetUniformLocation(shaderProgramObject, "u_ks");
    materialShininessUniform = glGetUniformLocation(shaderProgramObject, "u_materialShininess");
    lightingEnabledUniform = glGetUniformLocation(shaderProgramObject, "u_lightingEnabled");

    // Declaration of Vertex Data Arrays
    const GLfloat cube_pcnt[] =                 //position color normal texcoord
    {
        //front             //color-red         //normal-front      //texture-front
        1.0f, 1.0f, 1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
        -1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
        -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
        1.0f, -1.0f, 1.0f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,

        //right             //color-green       //normal-right      //texture-right
        1.0f, 1.0f, -1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
        1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
        1.0f, -1.0f, 1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,

        //back              //color-blue        //normal-back       //texture-back
        -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
        1.0f, 1.0f, -1.0f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,0.0f, 0.0f, 1.0f,   0.0f, 0.0f, -1.0f,  1.0f, 1.0f,

        //left              //color-cyan        //normal-left       //texture-back
        -1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,0.0f, 1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,

        //top               //color-magenta     //normal-top        //texture-top
        1.0f, 1.0f, -1.0f,  1.0f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
        -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
        -1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
        1.0f, 1.0f, 1.0f,   1.0f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,

        //bottom            //color-yellow      //normal-bottom     //texture-bottom
        1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f,   0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,1.0f, 1.0f, 0.0f,   0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
        -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f,   0.0f, -1.0f, 0.0f,  0.0f, 1.0f,
        1.0f, -1.0f, 1.0f,  1.0f, 1.0f, 0.0f,   0.0f, -1.0f, 0.0f,  1.0f, 1.0f
    };

    // VAO and VBO Related Code

    // vao
    glGenVertexArrays(1, &vao_cube);
    glBindVertexArray(vao_cube);

    //CODE RELATED TO "INTERLEAVED"
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_pcnt), cube_pcnt, GL_STATIC_DRAW);    
    //sizeof(cube_pcnt) is nothing but 11 x 24 x sizeof(float) ie. 264 x sizeof(float). 
    //11 x 24 is not for columns and rows but it is done for better readability


    //position
    glVertexAttribPointer(AAN_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)(0));
    glEnableVertexAttribArray(AAN_ATTRIBUTE_POSITION);

    //color
    glVertexAttribPointer(AAN_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(AAN_ATTRIBUTE_COLOR);

    //normal
    glVertexAttribPointer(AAN_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)(6 * sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(AAN_ATTRIBUTE_NORMAL);

    //texcoord
    glVertexAttribPointer(AAN_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)(9 * sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(AAN_ATTRIBUTE_TEXTURE0);


    // Unbinding of vao_cube
    glBindVertexArray(0);

    // Here Starts OpenGL Code

    // Depth Related Changes
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Clear The Screen Using Black Color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    if (loadGLTexture(&texture_marble, MAKEINTRESOURCE(IDBITMAP_MARBLE)) == FALSE)
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
    mat4 rotationMatrix = mat4::identity();
    mat4 rotationMatrix_x = mat4::identity();
    mat4 rotationMatrix_y = mat4::identity();
    mat4 rotationMatrix_z = mat4::identity();
    mat4 modelMatrix = mat4::identity();
    mat4 viewMatrix = mat4::identity();

    translationMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
    rotationMatrix_x = vmath::rotate(angleCube, 1.0f, 0.0f, 0.0f);
    rotationMatrix_y = vmath::rotate(angleCube, 0.0f, 1.0f, 0.0f);
    rotationMatrix_z = vmath::rotate(angleCube, 0.0f, 0.0f, 1.0f);
    rotationMatrix = rotationMatrix_x * rotationMatrix_y * rotationMatrix_z;
    modelMatrix = translationMatrix * rotationMatrix;

    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, texture_marble);

    glUniform1i(textureSamplerUniform, 0);

    if (bLight)
    {
        glUniform1i(lightingEnabledUniform, 1);

        glUniform3fv(laUniform, 1, lightAmbient);
        glUniform3fv(ldUniform, 1, lightDiffuse);
        glUniform3fv(lsUniform, 1, lightSpecular);
        glUniform4fv(lightPositionUniform, 1, lightPosition);

        glUniform3fv(kaUniform, 1, materialAmbient);
        glUniform3fv(kdUniform, 1, materialDiffuse);
        glUniform3fv(ksUniform, 1, materialSpecular);
        glUniform1f(materialShininessUniform, materialShininess);
    }
    else
    {
        glUniform1i(lightingEnabledUniform, 0);
    }

    glBindVertexArray(vao_cube);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Unuse The shaderProgramObject
    glUseProgram(0);

    SwapBuffers(ghdc);
}

void update(void)
{
    // Code
    angleCube = angleCube + 0.5f;
    if (angleCube >= 360.0f)
        angleCube = 0.0f;
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

    if (texture_marble)
    {
        glDeleteTextures(1, &texture_marble);
        texture_marble = 0;
    }

    if (vbo)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    if (vao_cube)
    {
        glDeleteVertexArrays(1, &vao_cube);
        vao_cube = 0;
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
