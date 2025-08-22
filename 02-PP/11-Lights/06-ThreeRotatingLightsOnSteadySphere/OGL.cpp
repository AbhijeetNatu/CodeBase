// Header Files
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "OGL.h"
#include "Sphere.h"
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
#pragma comment(lib, "Sphere.lib")

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
GLuint shaderProgramObject_pv;
GLuint shaderProgramObject_pf;

enum
{
    AAN_ATTRIBUTE_POSITION = 0,
    AAN_ATTRIBUTE_COLOR,
    AAN_ATTRIBUTE_NORMAL,
    AAN_ATTRIBUTE_TEXTURE0
};

GLuint vao_sphere;
GLuint vbo_sphere_position;
GLuint vbo_sphere_normal;
GLuint vbo_sphere_element;

// PER VERTEX
GLuint modelMatrixUniform_pv;
GLuint viewMatrixUniform_pv;
GLuint projectionMatrixUniform_pv;

GLuint laUniform_pv[3];   
GLuint ldUniform_pv[3];   
GLuint lsUniform_pv[3];   
GLuint lightPositionUniform_pv[3];

GLuint kaUniform_pv;   
GLuint kdUniform_pv;   
GLuint ksUniform_pv;   
GLuint materialShininessUniform_pv;

GLuint lightingEnabledUniform_pv;

// PER FRAGMENT
GLuint modelMatrixUniform_pf;
GLuint viewMatrixUniform_pf;
GLuint projectionMatrixUniform_pf;

GLuint laUniform_pf[3];   
GLuint ldUniform_pf[3];   
GLuint lsUniform_pf[3];   
GLuint lightPositionUniform_pf[3];

GLuint kaUniform_pf;   
GLuint kdUniform_pf;   
GLuint ksUniform_pf;   
GLuint materialShininessUniform_pf;

GLuint lightingEnabledUniform_pf;

mat4 perspectiveProjectionMatrix;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_texCoords[764];
unsigned short sphere_elements[2280];
unsigned int gNumVertices, gNumElements;

BOOL bLight = FALSE;

struct Light
{
    vec4 lightAmbient;
    vec4 lightDiffuse;
    vec4 lightSpecular;
    vec4 lightPosition;
};

Light lights[3];

GLfloat materialAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat materialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialShininess = 50.0f;

GLfloat lightAngleZero = 0.0f;
GLfloat lightAngleOne = 0.0f;
GLfloat lightAngleTwo = 0.0f;

float radius = 30.0f;

char chosenShader = 'v';

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
        TEXT("AAN : Three Rotating Lights On Steady Sphere"),
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
        case 'V':
        case 'v':
            chosenShader = 'v';
            break;

        case 'F':
        case 'f':
            chosenShader = 'f';
            break;

        case 'L':
        case 'l':
            if (bLight == FALSE)
                bLight = TRUE;
            else
                bLight = FALSE;
            break;

        case 'q':
        case 'Q':
            DestroyWindow(hwnd);
            break;

        default:
            break;
        }

        break;

    case WM_KEYDOWN:

        switch (wParam)
        {
        case 27:
            ToggleFullScreen();
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

    // PER-VERTEX

    // Vertex Shader
    const GLchar* vertexShaderSourceCode_pv =
        "#version 460 core" \
        "\n" \
        "in vec4 a_position;" \
        "in vec3 a_normal;" \
        "uniform mat4 u_modelMatrix;" \
        "uniform mat4 u_viewMatrix;" \
        "uniform mat4 u_projectionMatrix;" \
        "uniform vec3 u_la[3];" \
        "uniform vec3 u_ld[3];" \
        "uniform vec3 u_ls[3];" \
        "uniform vec4 u_lightPosition[3];" \
        "uniform vec3 u_ka;" \
        "uniform vec3 u_kd;" \
        "uniform vec3 u_ks;" \
        "uniform float u_materialShininess;" \
        "uniform int u_lightingEnabled;" \
        "out vec3 phong_ads_light;" \
        "void main(void)" \
        "{" \
        "if (u_lightingEnabled == 1)" \
        "{" \
        "vec4 eyeCoordinates = u_viewMatrix * u_modelMatrix * a_position;" \
        "mat3 normalMatrix = mat3(u_viewMatrix * u_modelMatrix);" \
        "vec3 transformedNormals = normalize(normalMatrix * a_normal);" \
        "vec3 viewerVector = normalize(-eyeCoordinates.xyz);" \
        "vec3 ambient[3];" \
        "vec3 lightDirection[3];" \
        "vec3 diffuse[3];" \
        "vec3 reflectionVector[3];" \
        "vec3 specular[3];" \
        "for (int i = 0; i < 3; i++)" \
        "{" \
        "ambient[i] = u_la[i] * u_ka;" \
        "lightDirection[i] = normalize(vec3(u_lightPosition[i]) - eyeCoordinates.xyz);" \
        "diffuse[i] = u_ld[i] * u_kd * max(dot(lightDirection[i], transformedNormals), 0.0);" \
        "reflectionVector[i] = reflect(-lightDirection[i], transformedNormals);" \
        "specular[i] = u_ls[i] * u_ks * pow(max(dot(reflectionVector[i], viewerVector), 0.0), u_materialShininess);" \
        "phong_ads_light = phong_ads_light + ambient[i] + diffuse[i] + specular[i];" \
        "}" \
        "}" \
        "else" \
        "{" \
        "phong_ads_light = vec3(1.0, 1.0, 1.0);" \
        "}" \
        "gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" \
        "}";

    GLuint vertexShaderObject_pv = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShaderObject_pv, 1, (const GLchar**)&vertexShaderSourceCode_pv, NULL);

    glCompileShader(vertexShaderObject_pv);

    GLint status;
    GLint infoLogLength;
    char* log = NULL;

    glGetShaderiv(vertexShaderObject_pv, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetShaderiv(vertexShaderObject_pv, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            log = (char*)malloc(infoLogLength);

            if (log != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(vertexShaderObject_pv, GL_INFO_LOG_LENGTH, &written, log);
                fprintf(gpFile, "Per-Vertex : Vertex Shader Compilation Log : %s\n", log);
                free(log);
                uninitialize();
            }
        }
    }

    // Fragment Shader
    const GLchar* fragmentShaderSourceCode_pv =
        "#version 460 core" \
        "\n" \
        "in vec3 phong_ads_light;" \
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
        "FragColor = vec4(phong_ads_light, 1.0);" \
        "}";

    GLuint fragmentShaderObject_pv = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShaderObject_pv, 1, (const GLchar**)&fragmentShaderSourceCode_pv, NULL);

    glCompileShader(fragmentShaderObject_pv);

    status = 0;
    infoLogLength = 0;
    log = NULL;

    glGetShaderiv(fragmentShaderObject_pv, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObject_pv, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            log = (char*)malloc(infoLogLength);

            if (log != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(fragmentShaderObject_pv, GL_INFO_LOG_LENGTH, &written, log);
                fprintf(gpFile, "Per-Vertex : Fragment Shader Compilation Log : %s\n", log);
                free(log);
                uninitialize();
            }
        }
    }

    // Shader Program Object
    shaderProgramObject_pv = glCreateProgram();

    glAttachShader(shaderProgramObject_pv, vertexShaderObject_pv);
    glAttachShader(shaderProgramObject_pv, fragmentShaderObject_pv);

    glBindAttribLocation(shaderProgramObject_pv, AAN_ATTRIBUTE_POSITION, "a_position");
    glBindAttribLocation(shaderProgramObject_pv, AAN_ATTRIBUTE_NORMAL, "a_normal");

    glLinkProgram(shaderProgramObject_pv);

    status = 0;
    infoLogLength = 0;
    log = NULL;

    glGetProgramiv(shaderProgramObject_pv, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetProgramiv(shaderProgramObject_pv, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            log = (char*)malloc(infoLogLength);

            if (log != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(shaderProgramObject_pv, GL_INFO_LOG_LENGTH, &written, log);
                fprintf(gpFile, "Per-Vertex : Shader Program Link Log : %s\n", log);
                free(log);
                uninitialize();
            }
        }
    }

    modelMatrixUniform_pv = glGetUniformLocation(shaderProgramObject_pv, "u_modelMatrix");
    viewMatrixUniform_pv = glGetUniformLocation(shaderProgramObject_pv, "u_viewMatrix");
    projectionMatrixUniform_pv = glGetUniformLocation(shaderProgramObject_pv, "u_projectionMatrix");

    laUniform_pv[0] = glGetUniformLocation(shaderProgramObject_pv, "u_la[0]");
    ldUniform_pv[0] = glGetUniformLocation(shaderProgramObject_pv, "u_ld[0]");
    lsUniform_pv[0] = glGetUniformLocation(shaderProgramObject_pv, "u_ls[0]");
    lightPositionUniform_pv[0] = glGetUniformLocation(shaderProgramObject_pv, "u_lightPosition[0]");

    laUniform_pv[1] = glGetUniformLocation(shaderProgramObject_pv, "u_la[1]");
    ldUniform_pv[1] = glGetUniformLocation(shaderProgramObject_pv, "u_ld[1]");
    lsUniform_pv[1] = glGetUniformLocation(shaderProgramObject_pv, "u_ls[1]");
    lightPositionUniform_pv[1] = glGetUniformLocation(shaderProgramObject_pv, "u_lightPosition[1]");

    laUniform_pv[2] = glGetUniformLocation(shaderProgramObject_pv, "u_la[2]");
    ldUniform_pv[2] = glGetUniformLocation(shaderProgramObject_pv, "u_ld[2]");
    lsUniform_pv[2] = glGetUniformLocation(shaderProgramObject_pv, "u_ls[2]");
    lightPositionUniform_pv[2] = glGetUniformLocation(shaderProgramObject_pv, "u_lightPosition[2]");

    kaUniform_pv = glGetUniformLocation(shaderProgramObject_pv, "u_ka");
    kdUniform_pv = glGetUniformLocation(shaderProgramObject_pv, "u_kd");
    ksUniform_pv = glGetUniformLocation(shaderProgramObject_pv, "u_ks");
    materialShininessUniform_pv = glGetUniformLocation(shaderProgramObject_pv, "u_materialShininess");

    lightingEnabledUniform_pv = glGetUniformLocation(shaderProgramObject_pv, "u_lightingEnabled");

    getSphereVertexData(sphere_vertices, sphere_normals, sphere_texCoords, sphere_elements);
    gNumVertices = getNumberOfSphereVertices();
    gNumElements = getNumberOfSphereElements();

    // vao_sphere and vbo_sphere_position Related Code
    glGenVertexArrays(1, &vao_sphere);
    glBindVertexArray(vao_sphere);

    // VBO For Position
    glGenBuffers(1, &vbo_sphere_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AAN_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AAN_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // VBO For Normal
    glGenBuffers(1, &vbo_sphere_normal);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_normal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);
    glVertexAttribPointer(AAN_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AAN_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // VBO For Element
    glGenBuffers(1, &vbo_sphere_element);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

   //////////////////////////////////////////////////////////////////////////


    // PER-FRAGMENT

    // Vertex Shader
    const GLchar* vertexShaderSourceCode_pf =
        "#version 460 core" \
        "\n" \
        "in vec4 a_position;" \
        "in vec3 a_normal;" \
        "uniform mat4 u_modelMatrix;" \
        "uniform mat4 u_viewMatrix;" \
        "uniform mat4 u_projectionMatrix;" \
        "uniform vec4 u_lightPosition[3];" \
        "uniform int u_lightingEnabled;" \
        "out vec3 transformedNormals;" \
        "out vec3 lightDirection[3];" \
        "out vec3 viewerVector;" \
        "void main(void)" \
        "{" \
        "if (u_lightingEnabled == 1)" \
        "{" \
        "vec4 eyeCoordinates = u_viewMatrix * u_modelMatrix * a_position;" \
        "mat3 normalMatrix = mat3(u_viewMatrix * u_modelMatrix);" \
        "transformedNormals = normalMatrix * a_normal;" \
        "viewerVector = -eyeCoordinates.xyz;" \
        "for (int i = 0; i < 3; i++)" \
        "{" \
        "lightDirection[i] = vec3(u_lightPosition[i]) - eyeCoordinates.xyz;" \
        "}" \
        "}" \
        "gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" \
        "}";

    GLuint vertexShaderObject_pf = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShaderObject_pf, 1, (const GLchar**)&vertexShaderSourceCode_pf, NULL);

    glCompileShader(vertexShaderObject_pf);

    status = 0;
    infoLogLength = 0;
    log = NULL;

    glGetShaderiv(vertexShaderObject_pf, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetShaderiv(vertexShaderObject_pf, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            log = (char*)malloc(infoLogLength);

            if (log != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(vertexShaderObject_pf, GL_INFO_LOG_LENGTH, &written, log);
                fprintf(gpFile, "Per-Fragment : Vertex Shader Compilation Log : %s\n", log);
                free(log);
                uninitialize();
            }
        }
    }
    

    // Fragment Shader
    const GLchar* fragmentShaderSourceCode_pf =
        "#version 460 core" \
        "\n" \
        "in vec3 transformedNormals;" \
        "in vec3 lightDirection[3];" \
        "in vec3 viewerVector;" \
        "uniform vec3 u_la[3];" \
        "uniform vec3 u_ld[3];" \
        "uniform vec3 u_ls[3];" \
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
        "vec3 ambient[3];" \
        "vec3 diffuse[3];" \
        "vec3 reflectionVector[3];" \
        "vec3 specular[3];" \
        "vec3 normalized_light_direction[3];" \
        "vec3 normalized_tranformed_normals = normalize(transformedNormals);" \
        "vec3 normalized_viewer_vector = normalize(viewerVector);"
        "for (int i = 0; i < 3; i++)" \
        "{" \
        "ambient[i] = u_la[i] * u_ka;" \
        "normalized_light_direction[i] = normalize(lightDirection[i]);" \
        "diffuse[i] = u_ld[i] * u_kd * max(dot(normalized_light_direction[i], normalized_tranformed_normals), 0.0);" \
        "reflectionVector[i] = reflect(-normalized_light_direction[i], normalized_tranformed_normals);" \
        "specular[i] = u_ls[i] * u_ks * pow(max(dot(reflectionVector[i], normalized_viewer_vector), 0.0), u_materialShininess);" \
        "phong_ads_light = phong_ads_light + ambient[i] + diffuse[i] + specular[i];" \
        "}" \
        "}" \
        "else" \
        "{" \
        "phong_ads_light = vec3(1.0, 1.0, 1.0);" \
        "}" \
        "FragColor = vec4(phong_ads_light, 1.0);" \
        "}";

    GLuint fragmentShaderObject_pf = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShaderObject_pf, 1, (const GLchar**)&fragmentShaderSourceCode_pf, NULL);

    glCompileShader(fragmentShaderObject_pf);

    status = 0;
    infoLogLength = 0;
    log = NULL;

    glGetShaderiv(fragmentShaderObject_pf, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObject_pf, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            log = (char*)malloc(infoLogLength);

            if (log != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(fragmentShaderObject_pf, GL_INFO_LOG_LENGTH, &written, log);
                fprintf(gpFile, "Per-Fragment : Fragment Shader Compilation Log : %s\n", log);
                free(log);
                uninitialize();
            }
        }
    }

    // Shader Program Object
    shaderProgramObject_pf = glCreateProgram();

    glAttachShader(shaderProgramObject_pf, vertexShaderObject_pf);
    glAttachShader(shaderProgramObject_pf, fragmentShaderObject_pf);

    glBindAttribLocation(shaderProgramObject_pf, AAN_ATTRIBUTE_POSITION, "a_position");
    glBindAttribLocation(shaderProgramObject_pf, AAN_ATTRIBUTE_NORMAL, "a_normal");

    glLinkProgram(shaderProgramObject_pf);

    status = 0;
    infoLogLength = 0;
    log = NULL;

    glGetProgramiv(shaderProgramObject_pf, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetProgramiv(shaderProgramObject_pf, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            log = (char*)malloc(infoLogLength);

            if (log != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(shaderProgramObject_pf, GL_INFO_LOG_LENGTH, &written, log);
                fprintf(gpFile, "Per-Fragment : Shader Program Link Log : %s\n", log);
                free(log);
                uninitialize();
            }
        }
    }

    modelMatrixUniform_pf = glGetUniformLocation(shaderProgramObject_pf, "u_modelMatrix");
    viewMatrixUniform_pf = glGetUniformLocation(shaderProgramObject_pf, "u_viewMatrix");
    projectionMatrixUniform_pf = glGetUniformLocation(shaderProgramObject_pf, "u_projectionMatrix");

    laUniform_pf[0] = glGetUniformLocation(shaderProgramObject_pf, "u_la[0]");
    ldUniform_pf[0] = glGetUniformLocation(shaderProgramObject_pf, "u_ld[0]");
    lsUniform_pf[0] = glGetUniformLocation(shaderProgramObject_pf, "u_ls[0]");
    lightPositionUniform_pf[0] = glGetUniformLocation(shaderProgramObject_pf, "u_lightPosition[0]");

    laUniform_pf[1] = glGetUniformLocation(shaderProgramObject_pf, "u_la[1]");
    ldUniform_pf[1] = glGetUniformLocation(shaderProgramObject_pf, "u_ld[1]");
    lsUniform_pf[1] = glGetUniformLocation(shaderProgramObject_pf, "u_ls[1]");
    lightPositionUniform_pf[1] = glGetUniformLocation(shaderProgramObject_pf, "u_lightPosition[1]");

    laUniform_pf[2] = glGetUniformLocation(shaderProgramObject_pf, "u_la[2]");
    ldUniform_pf[2] = glGetUniformLocation(shaderProgramObject_pf, "u_ld[2]");
    lsUniform_pf[2] = glGetUniformLocation(shaderProgramObject_pf, "u_ls[2]");
    lightPositionUniform_pf[2] = glGetUniformLocation(shaderProgramObject_pf, "u_lightPosition[2]");

    kaUniform_pf = glGetUniformLocation(shaderProgramObject_pf, "u_ka");
    kdUniform_pf = glGetUniformLocation(shaderProgramObject_pf, "u_kd");
    ksUniform_pf = glGetUniformLocation(shaderProgramObject_pf, "u_ks");
    materialShininessUniform_pf = glGetUniformLocation(shaderProgramObject_pf, "u_materialShininess");

    lightingEnabledUniform_pf = glGetUniformLocation(shaderProgramObject_pf, "u_lightingEnabled");

    getSphereVertexData(sphere_vertices, sphere_normals, sphere_texCoords, sphere_elements);
    gNumVertices = getNumberOfSphereVertices();
    gNumElements = getNumberOfSphereElements();

    // vao and vbo Related Code
    glGenVertexArrays(1, &vao_sphere);
    glBindVertexArray(vao_sphere);

    // VBO For Position
    glGenBuffers(1, &vbo_sphere_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AAN_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AAN_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // VBO For Normal
    glGenBuffers(1, &vbo_sphere_normal);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_normal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);
    glVertexAttribPointer(AAN_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AAN_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // VBO For Element
    glGenBuffers(1, &vbo_sphere_element);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
   

    // Depth Related Changes
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Clear The Screen Using Black Color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    lights[0].lightAmbient = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    lights[0].lightDiffuse = vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    lights[0].lightSpecular = vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    lights[0].lightPosition = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    lights[1].lightAmbient = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    lights[1].lightDiffuse = vmath::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    lights[1].lightSpecular = vmath::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    lights[1].lightPosition = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    lights[2].lightAmbient = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    lights[2].lightDiffuse = vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    lights[2].lightSpecular = vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    lights[2].lightPosition = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    perspectiveProjectionMatrix = mat4::identity();

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

    if (chosenShader == 'v')
        glUseProgram(shaderProgramObject_pv);
    else
        glUseProgram(shaderProgramObject_pf);

    // Transformations
    mat4 translationMatrix = mat4::identity();
    mat4 rotationMatrix = mat4::identity();
    mat4 modelMatrix = mat4::identity();
    mat4 viewMatrix = mat4::identity();

    translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
    modelMatrix = translationMatrix;

    // Light 0
    lights[0].lightPosition[2] = radius * cos(radians(lightAngleZero));
    lights[0].lightPosition[1] = radius * sin(radians(lightAngleZero));

    // Light 1
    lights[1].lightPosition[0] = radius * cos(radians(lightAngleOne));
    lights[1].lightPosition[2] = radius * sin(radians(lightAngleOne));

    // Light 2
    lights[2].lightPosition[1] = radius * sin(radians(lightAngleTwo));
    lights[2].lightPosition[0] = radius * cos(radians(lightAngleTwo));

    if (chosenShader == 'v')
    {
        glUniformMatrix4fv(modelMatrixUniform_pv, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(viewMatrixUniform_pv, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projectionMatrixUniform_pv, 1, GL_FALSE, perspectiveProjectionMatrix);

        if (bLight)
        {
            glUniform1i(lightingEnabledUniform_pv, 1);

            for (int i = 0; i < 3; i++)
            {
                glUniform3fv(laUniform_pv[i], 1, lights[i].lightAmbient);
                glUniform3fv(ldUniform_pv[i], 1, lights[i].lightDiffuse);
                glUniform3fv(lsUniform_pv[i], 1, lights[i].lightSpecular);
                glUniform4fv(lightPositionUniform_pv[i], 1, lights[i].lightPosition);
            }

            glUniform3fv(kaUniform_pv, 1, materialAmbient);
            glUniform3fv(kdUniform_pv, 1, materialDiffuse);
            glUniform3fv(ksUniform_pv, 1, materialSpecular);
            glUniform1f(materialShininessUniform_pv, materialShininess);
        }
        else
        {
            glUniform1i(lightingEnabledUniform_pv, 0);
        }
    }

    else if (chosenShader == 'f')
    {
        glUniformMatrix4fv(modelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(viewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projectionMatrixUniform_pf, 1, GL_FALSE, perspectiveProjectionMatrix);

        if (bLight)
        {
            glUniform1i(lightingEnabledUniform_pf, 1);

            for (int i = 0; i < 3; i++)
            {
                glUniform3fv(laUniform_pf[i], 1, lights[i].lightAmbient);
                glUniform3fv(ldUniform_pf[i], 1, lights[i].lightDiffuse);
                glUniform3fv(lsUniform_pf[i], 1, lights[i].lightSpecular);
                glUniform4fv(lightPositionUniform_pf[i], 1, lights[i].lightPosition);
            }

            glUniform3fv(kaUniform_pf, 1, materialAmbient);
            glUniform3fv(kdUniform_pf, 1, materialDiffuse);
            glUniform3fv(ksUniform_pf, 1, materialSpecular);
            glUniform1f(materialShininessUniform_pf, materialShininess);
        }
        else
        {
            glUniform1i(lightingEnabledUniform_pf, 0);
        }
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    SwapBuffers(ghdc);
}

void update(void)
{
    // Code
    lightAngleZero = lightAngleZero + 1.0f;
    if (lightAngleZero > 360.0f)
        lightAngleZero = lightAngleZero - 360.0f;

    lightAngleOne = lightAngleOne + 1.0f;
    if (lightAngleOne > 360.0f)
        lightAngleOne = lightAngleOne - 360.0f;

    lightAngleTwo = lightAngleTwo + 1.0f;
    if (lightAngleTwo > 360.0f)
        lightAngleTwo = lightAngleTwo - 360.0f;
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

    if (vbo_sphere_element)
    {
        glDeleteBuffers(1, &vbo_sphere_element);
        vbo_sphere_element = 0;
    }

    if (vbo_sphere_normal)
    {
        glDeleteBuffers(1, &vbo_sphere_normal);
        vbo_sphere_normal = 0;
    }

    if (vbo_sphere_position)
    {
        glDeleteBuffers(1, &vbo_sphere_position);
        vbo_sphere_position = 0;
    }

    if (vao_sphere)
    {
        glDeleteVertexArrays(1, &vao_sphere);
        vao_sphere = 0;
    }

    // Shader Uninitialization
    if (shaderProgramObject_pf)
    {
        glUseProgram(shaderProgramObject_pf);

        GLsizei numAttachedShaders;

        glGetProgramiv(shaderProgramObject_pf, GL_ATTACHED_SHADERS, &numAttachedShaders);

        GLuint* shaderObjects = NULL;

        shaderObjects = (GLuint*)malloc(numAttachedShaders * sizeof(GLuint));

        glGetAttachedShaders(shaderProgramObject_pf, numAttachedShaders, &numAttachedShaders, shaderObjects);

        for (GLsizei i = 0; i < numAttachedShaders; i++)
        {
            glDetachShader(shaderProgramObject_pf, shaderObjects[i]);
            glDeleteShader(shaderObjects[i]);
            shaderObjects[i] = 0;
        }

        free(shaderObjects);

        shaderObjects = NULL;

        glUseProgram(0);

        glDeleteProgram(shaderProgramObject_pf);

        shaderProgramObject_pf = 0;
    }

    if (shaderProgramObject_pv)
    {
        glUseProgram(shaderProgramObject_pv);

        GLsizei numAttachedShaders;

        glGetProgramiv(shaderProgramObject_pv, GL_ATTACHED_SHADERS, &numAttachedShaders);

        GLuint* shaderObjects = NULL;

        shaderObjects = (GLuint*)malloc(numAttachedShaders * sizeof(GLuint));

        glGetAttachedShaders(shaderProgramObject_pv, numAttachedShaders, &numAttachedShaders, shaderObjects);

        for (GLsizei i = 0; i < numAttachedShaders; i++)
        {
            glDetachShader(shaderProgramObject_pv, shaderObjects[i]);
            glDeleteShader(shaderObjects[i]);
            shaderObjects[i] = 0;
        }

        free(shaderObjects);

        shaderObjects = NULL;

        glUseProgram(0);

        glDeleteProgram(shaderProgramObject_pv);

        shaderProgramObject_pv = 0;
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
