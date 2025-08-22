// Header Files
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "OGL.h"
#include "Sphere.h"
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
GLuint shaderProgramObject;

enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXTURE0
};

GLuint vao_sphere;
GLuint vbo_sphere_position;
GLuint vbo_sphere_normal;
GLuint vbo_sphere_element;

mat4 perspectiveProjectionMatrix;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_texCoords[764];

unsigned short sphere_elements[2280];
unsigned int gNumVertices, gNumElements;

GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint projectionMatrixUniform;

GLuint laUniform;
GLuint ldUniform;
GLuint lsUniform;
GLuint lightPositionUniform;

GLuint kaUniform;
GLuint kdUniform;
GLuint ksUniform;
GLuint materialShininessUniform;

GLuint lightingEnabledUniform;

/*
GLfloat lightAmbient[] = {0.0f,0.0f,0.0f,1.0f};
GLfloat lightDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightPosition[] = { 100.0f,100.0f,100.0f,1.0f };

GLfloat materialAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat materialDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat materialSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat materialShininess = 50.0f;
*/

GLfloat lightAmbient[] = { 0.1f,0.1f,0.1f,1.0f };
GLfloat lightDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightPosition[] = { 100.0f,100.0f,100.0f,1.0f };

GLfloat materialAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat materialDiffuse[] = { 0.5f,0.2f,0.7f,1.0f };
GLfloat materialSpecular[] = { 0.7f,0.7f,0.7f,1.0f };
GLfloat materialShininess = 128.0f;

BOOL bLight = FALSE;

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
        TEXT("Abhijeet Natu : Per Fragment "),
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
                // update();
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

        case 'L':
        case 'l':
            if (bLight == FALSE)
            {
                bLight = TRUE;
            }
            else
            {
                bLight = FALSE;
            }
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
    
    const GLchar* vertexShaderSourceCode =
        "#version 460 core" \
        "\n" \
        "in vec4 a_position;" \
        "in vec3 a_normal;" \
        "uniform mat4 u_modelMatrix;" \
        "uniform mat4 u_viewMatrix;" \
        "uniform mat4 u_projectionMatrix;" \
        "uniform vec4 u_lightPosition;" \
        "out vec3 lightDirection;" \
        "uniform int u_LightingEnabled;" \
        "out vec3 transformedNormals;" \
        "out vec3 viewerVector;" \
        "void main(void)" \
        "{" \
        "if(u_LightingEnabled == 1)" \
        "{" \
        "vec4 eyeCoordinates = u_viewMatrix * u_modelMatrix * a_position;" \
        "mat3 normalMatrix = mat3(u_viewMatrix * u_modelMatrix);" \
        "transformedNormals = normalMatrix * a_normal;" \
        "lightDirection = vec3(u_lightPosition) - eyeCoordinates.xyz;" \
        "viewerVector = -eyeCoordinates.xyz;" \
        "}" \
        "gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" \
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
    

    // Fragment Shader
    
    const GLchar* fragmentShaderSourceCode =
        "#version 460 core" \
        "\n" \
        "uniform vec3 u_la;" \
        "uniform vec3 u_ld;" \
        "uniform vec3 u_ls;" \
        "uniform vec3 u_ka;" \
        "uniform vec3 u_kd;" \
        "uniform vec3 u_ks;" \
        "uniform float u_materialShininess;" \
        "out vec4 FragColor;" \
        "in vec3 transformedNormals;" \
        "in vec3 viewerVector;" \
        "in vec3 lightDirection;" \
        "uniform int u_LightingEnabled;" \
        "void main(void)" \
        "{" \
        "vec3 phong_ads_color;" \
        "if(u_LightingEnabled == 1)" \
        "{" \
        "vec3 ambient = u_la * u_ka;" \
        "vec3 normalised_transformed_normal = normalize(transformedNormals);" \
        "vec3 normalised_light_direction = normalize(lightDirection);" \
        "vec3 diffuse = u_ld * u_kd * max(dot(normalised_light_direction, normalised_transformed_normal),0.0);" \
        "vec3 reflectionVector = reflect(-normalised_light_direction,normalised_transformed_normal);" \
        "vec3 normalised_viewerVector = normalize(viewerVector);" \
        "vec3 specular = u_ls * u_ks * pow(max(dot(reflectionVector,normalised_viewerVector),0.0),u_materialShininess);" \
        "phong_ads_color = ambient + diffuse + specular;" \
        "}" \
        "else" \
        "{" \
        "phong_ads_color = vec3(1.0f,1.0f,1.0f);" \
        "}" \
        "FragColor = vec4(phong_ads_color, 1.0);" \
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
    

    // Shader Program Object
    shaderProgramObject = glCreateProgram();

    glAttachShader(shaderProgramObject, vertexShaderObject);
    glAttachShader(shaderProgramObject, fragmentShaderObject);

    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "a_position");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "a_normal");
    glLinkProgram(shaderProgramObject);


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
    lightingEnabledUniform = glGetUniformLocation(shaderProgramObject, "u_LightingEnabled");


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
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // VBO For Normal
    glGenBuffers(1, &vbo_sphere_normal);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_normal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // VBO For Element
    glGenBuffers(1, &vbo_sphere_element);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // Here Starts OpenGL Code

    // Depth Related Changes
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Clear The Screen Using Black Color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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

    // Use The Shader Program Object
    glUseProgram(shaderProgramObject);

    // Transformations
    mat4 translationMatrix = mat4::identity();
    mat4 modelMatrix = mat4::identity();
    mat4 viewMatrix = mat4::identity();

    translationMatrix = translate(0.0f, 0.0f, -2.0f);
    modelMatrix = translationMatrix;

    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    if (bLight == TRUE)
    {
        glUniform1i(lightingEnabledUniform, 1);
        glUniform3fv(laUniform,1, lightAmbient);
        glUniform3fv(ldUniform,1, lightDiffuse);
        glUniform3fv(lsUniform,1, lightSpecular);
        glUniform4fv(lightPositionUniform, 1, lightPosition);

        glUniform3fv(kaUniform, 1, materialAmbient);
        glUniform3fv(kdUniform, 1, materialDiffuse);
        glUniform3fv(ksUniform, 1, materialSpecular);
        glUniform1f(materialShininessUniform,materialShininess);
    }

    else
    {
        glUniform1i(lightingEnabledUniform, 0);
    }

    glBindVertexArray(vao_sphere);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);

    // Unuse The Shader Program Object
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
