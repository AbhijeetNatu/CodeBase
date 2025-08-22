// Global Variables
var canvas = null;
var gl = null;
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;

const webGLMacros = 
{
    AAN_ATTRIBUTE_POSITION: 0,
    AAN_ATTRIBUTE_COLOR: 1,
    AAN_ATTRIBUTE_NORMAL: 2,
    AAN_ATTRIBUTE_TEXTURE0: 3
};

const FBO_WIDTH = 512;
const FBO_HEIGHT = 512;

var shaderProgramObject;

var vao_cube;
var vbo_cube_position;
var vbo_cube_texcoord;

var mvpMatrixUniform;
var textureSamplerUniform;

var perspectiveProjectionMatrix;

var angleCube = 0.0;

// FBO Related Variables
var fbo = [];
var rbo = [];
var fbo_texture = [];
var bFBOResult = false;

// Texture Scene Global Variables

// Per-Vertex
// ----------------------------------------------
var shaderProgramObject_sphere_pv;
var modelMatrixUniform_sphere_pv;
var viewMatrixUniform_sphere_pv;
var projectionMatrixUniform_sphere_pv;

var laUniform_sphere_pv;
var ldUniform_sphere_pv;
var lsUniform_sphere_pv;
var lightPositionUniform_sphere_pv;

var kaUniform_sphere_pv;
var kdUniform_sphere_pv;
var ksUniform_sphere_pv;
var materialShininessUniform_sphere_pv;

var lightingEnabledUniform_sphere_pv;
// ----------------------------------------------

// Per-Fragment
// ----------------------------------------------
var shaderProgramObject_sphere_pf;
var modelMatrixUniform_sphere_pf;
var viewMatrixUniform_sphere_pf;
var projectionMatrixUniform_sphere_pf;

var laUniform_sphere_pf;
var ldUniform_sphere_pf;
var lsUniform_sphere_pf;
var lightPositionUniform_sphere_pf;

var kaUniform_sphere_pf;
var kdUniform_sphere_pf;
var ksUniform_sphere_pf;
var materialShininessUniform_sphere_pf;

var lightingEnabledUniform_sphere_pf;
// ----------------------------------------------

var perspectiveProjectionMatrix_sphere;

var sphere = null;

var bLight = false;

var chosenShader = 'v';

var lightAmbient_sphere = new Float32Array();
var lightDiffuse_sphere = new Float32Array();
var lightSpecular_sphere = new Float32Array();
var lightPosition_sphere = new Float32Array();

var materialAmbient_sphere = [0.0, 0.0, 0.0];
var materialDiffuse_sphere = [1.0, 1.0, 1.0];
var materialSpecular_sphere = [1.0, 1.0, 1.0];
var materialShininess_sphere = 50.0;

var radius_sphere = 30.0;

var lightAngleZero_sphere = 0.0;
var lightAngleOne_sphere = 0.0;
var lightAngleTwo_sphere = 0.0;

// Like SwapBuffers(), glxSwapBuffers(), requestRender()
var requestAnimationFrame = window.requestAnimationFrame || 
                            window.mozRequestAnimationFrame ||
                            window.webkitRequestAnimationFrame || 
                            window.oRequestAnimationFrame ||
                            window.msRequestAnimationFrame;

function main()
{
    // Code

    // Get Canvas
    canvas = document.getElementById("AAN");
    if (!canvas)
        console.log("Could Not Obtain Canvas !!!\n");
    else 
        console.log("Canvas Obtained !!!\n");

    // Backup Canvas Dimensions
    canvas_original_width = canvas.width;
    canvas_original_height = canvas.height;

    // Initialize
    initialize();

    // Warmup Resize
    resize();

    // Display
    display();

    // Adding Event Listeners
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mousedown, false);
    window.addEventListener("resize", resize, false);
}

function toggleFullscreen()
{
    // Code
    var fullscreen_Element = document.fullscreenElement ||       
                             document.mozFullScreenElement ||    
                             document.webkitFullscreenElement || 
                             document.msFullscreenElement ||     
                             null;                               
    
    // If Not Fullscreen
    if (fullscreen_Element == null)
    {
        if (canvas.requestFullscreen)
            canvas.requestFullscreen();
        else if (canvas.mozRequestFullScreen)
            canvas.mozRequestFullScreen();
        else if (canvas.webkitRequestFullscreen)
            canvas.webkitRequestFullscreen();
        else if (canvas.msRequestFullscreen)  
            canvas.msRequestFullScreen();
        
        bFullscreen = true;
    }
    else
    {
        if (document.exitFullscreen)
            document.exitFullscreen();
        else if (document.mozExitFullScreen)
            document.mozExitFullscreen();
        else if (document.webkitExitFullscreen)
            document.webkitExitFullscreen();
        else if (document.msExitFullscreen)  
            document.msExitFullscreen();
        
        bFullscreen = false;
    }                     
}

function initialize()
{
    // Code

    // Get WebGL 2.0 Context From Canvas
    gl = canvas.getContext("webgl2");
    if (!gl)
        console.log("Could Not Obtain WebGL 2.0 Context !!!\n");
    else
        console.log("WebGL 2.0 Context Obtained !!!\n");
    
    // Set Viewport Width & Height Of Context
    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;

    // Vertex Shader
    var vertexShaderSourceCode = 
    "#version 300 es" +
    "\n" +
    "in vec4 a_position;" +
    "in vec2 a_texcoord;" +
    "uniform mat4 u_mvpMatrix;" +
    "out vec2 a_texcoord_out;" +
    "void main(void)" +
    "{" +
    "gl_Position = u_mvpMatrix * a_position;" +
    "a_texcoord_out = a_texcoord;" +
    "}";

    var vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);

    gl.shaderSource(vertexShaderObject, vertexShaderSourceCode);

    gl.compileShader(vertexShaderObject);

    if (gl.getShaderParameter(vertexShaderObject, gl.COMPILE_STATUS))
    {
        var error = gl.getShaderInfoLog(vertexShaderObject);

        if (error.length > 0)
        {
            alert("Vertex Shader Compilation Error : \n" + error);
            uninitialize();
        }
    }
    
    // Fragment Shader
    var fragmentShaderSourceCode = 
    "#version 300 es" +
    "\n" +
    "precision highp float;" +
    "precision highp sampler2D;" +
    "in vec2 a_texcoord_out;" +
    "uniform sampler2D u_textureSampler;" +
    "out vec4 FragColor;" +
    "void main(void)" +
    "{" +
    "FragColor = texture(u_textureSampler, a_texcoord_out);" +
    "}";

    var fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);

    gl.shaderSource(fragmentShaderObject, fragmentShaderSourceCode);

    gl.compileShader(fragmentShaderObject);

    if (gl.getShaderParameter(fragmentShaderObject, gl.COMPILE_STATUS))
    {
        var error = gl.getShaderInfoLog(fragmentShaderObject);

        if (error.length > 0)
        {
            alert("Fragment Shader Compilation Error : \n" + error);
            uninitialize();
        }
    }

    // Shader Program Object
    shaderProgramObject = gl.createProgram();

    gl.attachShader(shaderProgramObject, vertexShaderObject);
    gl.attachShader(shaderProgramObject, fragmentShaderObject);

    // Pre-Linking
    gl.bindAttribLocation(shaderProgramObject, webGLMacros.AAN_ATTRIBUTE_POSITION, "a_position");
    gl.bindAttribLocation(shaderProgramObject, webGLMacros.AAN_ATTRIBUTE_TEXTURE0, "a_texcoord");

    // Shader Program Linking
    gl.linkProgram(shaderProgramObject);

    // Error Checking
    if (gl.getProgramParameter(shaderProgramObject, gl.LINK_STATUS) == false)
    {
        var error = gl.getProgramInfoLog(shaderProgramObject);

        if (error.length > 0)
        {
            alert("Shader Program Link Error : \n" + error);
            uninitialize();
        }
    }

    // Post-linking
    mvpMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_mvpMatrix");
    textureSamplerUniform = gl.getUniformLocation(shaderProgramObject, "u_textureSampler");

    // Declaration & Initialization Of Geometry
    var cubeVertices = new Float32Array([
        // top
        1.0, 1.0, -1.0,
        -1.0, 1.0, -1.0,
        -1.0, 1.0, 1.0,
        1.0, 1.0, 1.0,

        // bottom
        1.0, -1.0, -1.0,
       -1.0, -1.0, -1.0,
       -1.0, -1.0,  1.0,
        1.0, -1.0,  1.0,

        // front
        1.0, 1.0, 1.0,
       -1.0, 1.0, 1.0,
       -1.0, -1.0, 1.0,
        1.0, -1.0, 1.0,

        // back
        1.0, 1.0, -1.0,
       -1.0, 1.0, -1.0,
       -1.0, -1.0, -1.0,
        1.0, -1.0, -1.0,

        // right
        1.0, 1.0, -1.0,
        1.0, 1.0, 1.0,
        1.0, -1.0, 1.0,
        1.0, -1.0, -1.0,

        // left
        -1.0, 1.0, 1.0,
        -1.0, 1.0, -1.0,
        -1.0, -1.0, -1.0,
        -1.0, -1.0, 1.0
    ]);

    var cubeTexcoords = new Float32Array([
        // Top
        0.0, 1.0,
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
    
        //Bottom
        1.0, 1.0,
        0.0, 1.0,
        0.0, 0.0,
        1.0, 0.0,
    
        // Front
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0,
    
        // Back
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0,
        0.0, 0.0,
    
        // Right
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0,
        0.0, 0.0,
    
        // Left
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0,
    ]);

    // VAO and VBO Related Code

    // Cube
    // VAO
    vao_cube = gl.createVertexArray();
    gl.bindVertexArray(vao_cube);

    // VBO For Position
    vbo_cube_position = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_cube_position);
    gl.bufferData(gl.ARRAY_BUFFER, cubeVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    // VBO For Color
    vbo_cube_texcoord = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_cube_texcoord);
    gl.bufferData(gl.ARRAY_BUFFER, cubeTexcoords, gl.STATIC_DRAW);
    gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_TEXTURE0, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_TEXTURE0);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    
    gl.bindVertexArray(null);

    // Depth Related Code
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    // Clear The Screen Using White Color
    gl.clearColor(0.5, 0.5, 0.5, 1.0);

    // FBO Code
    bFBOResult = createFBO(FBO_WIDTH, FBO_HEIGHT);

    if (bFBOResult)
        initialize_sphere();
    else
        return;

    perspectiveProjectionMatrix = mat4.create();
}

function createFBO(textureWidth, textureHeight)
{
    var maxRenderbufferSize = null;

    maxRenderbufferSize = gl.getParameter(gl.MAX_RENDERBUFFER_SIZE);
    if (maxRenderbufferSize < textureWidth || maxRenderbufferSize < textureHeight)
	{
        console.log("Insufficient Render Buffer Size !!!\n");
		return false;
	}

    // Create Framebuffer Object
    fbo = gl.createFramebuffer();
    gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);

    // Create Renderbuffer Object
    rbo = gl.createRenderbuffer();
    gl.bindRenderbuffer(gl.RENDERBUFFER, rbo);

    // Storage and Format of Renderbuffer
    gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, textureWidth, textureHeight);

    // Create Empty Texture for upcoming target scene
    fbo_texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, fbo_texture);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGB, textureWidth, textureHeight, 0, gl.RGB, gl.UNSIGNED_SHORT_5_6_5, null);
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, fbo_texture, 0);

    // Give Renderbuffer to FBO
    gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, rbo);

    // Check Framebuffer creation status
    var result = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
    if (result != gl.FRAMEBUFFER_COMPLETE)
    {
        console.log("AAN: Framebuffer Incomplete !!!");
        return false;
    }

    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    return true;
}

function initialize_sphere()
{
    // **PER-VERTEX**
    
    // Vertex Shader
    var vertexShaderSourceCode_pv = 
    "#version 300 es" +
    "\n" +
    "in vec4 a_position;" +
    "in vec3 a_normal;" +
    "uniform mat4 u_modelMatrix;" +
    "uniform mat4 u_viewMatrix;" +
    "uniform mat4 u_projectionMatrix;" +
    "uniform vec3 u_la[3];" +
    "uniform vec3 u_ld[3];" +
    "uniform vec3 u_ls[3];" +
    "uniform vec4 u_lightPosition[3];" +
    "uniform vec3 u_ka;" +
    "uniform vec3 u_kd;" +
    "uniform vec3 u_ks;" +
    "uniform float u_materialShininess;" +
    "uniform mediump int u_lightingEnabled;" +
    "out vec3 phong_ads_light;" +
    "void main(void)" +
    "{" +
    "if (u_lightingEnabled == 1)" +
    "{" +
    "vec4 eyeCoordinates = u_viewMatrix * u_modelMatrix * a_position;" +
    "mat3 normalMatrix = mat3(u_viewMatrix * u_modelMatrix);" +
    "vec3 transformedNormals = normalize(normalMatrix * a_normal);" +
    "vec3 viewerVector = normalize(-eyeCoordinates.xyz);" +
    "vec3 ambient[3];" +
    "vec3 lightDirection[3];" +
    "vec3 diffuse[3];" +
    "vec3 reflectionVector[3];" +
    "vec3 specular[3];" +
    "for (int i = 0; i < 3; i++)" +
    "{" +
    "ambient[i] = u_la[i] * u_ka;" +
    "lightDirection[i] = normalize(vec3(u_lightPosition[i]) - eyeCoordinates.xyz);" +
    "diffuse[i] = u_ld[i] * u_kd * max(dot(lightDirection[i], transformedNormals), 0.0);" +
    "reflectionVector[i] = reflect(-lightDirection[i], transformedNormals);" +
    "specular[i] = u_ls[i] * u_ks * pow(max(dot(reflectionVector[i], viewerVector), 0.0), u_materialShininess);" +
    "phong_ads_light = phong_ads_light + ambient[i] + diffuse[i] + specular[i];" +
    "}" +
    "}" +
    "else" +
    "{" +
    "phong_ads_light = vec3(1.0, 1.0, 1.0);" +
    "}" +
    "gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" +
    "}";

    var vertexShaderObject_pv = gl.createShader(gl.VERTEX_SHADER);

    gl.shaderSource(vertexShaderObject_pv, vertexShaderSourceCode_pv);

    gl.compileShader(vertexShaderObject_pv);

    if (gl.getShaderParameter(vertexShaderObject_pv, gl.COMPILE_STATUS))
    {
        var error = gl.getShaderInfoLog(vertexShaderObject_pv);

        if (error.length > 0)
        {
            alert("Per-Vertex : Vertex Shader Compilation Error : \n" + error);
            uninitialize();
        }
    }
    
    // Fragment Shader
    var fragmentShaderSourceCode_pv = 
    "#version 300 es" +
    "\n" +
    "precision highp float;" +
    "in vec3 phong_ads_light;" +
    "out vec4 FragColor;" +
    "void main(void)" +
	"{" +
    "FragColor = vec4(phong_ads_light, 1.0);" +
    "}";

    var fragmentShaderObject_pv = gl.createShader(gl.FRAGMENT_SHADER);

    gl.shaderSource(fragmentShaderObject_pv, fragmentShaderSourceCode_pv);

    gl.compileShader(fragmentShaderObject_pv);

    if (gl.getShaderParameter(fragmentShaderObject_pv, gl.COMPILE_STATUS))
    {
        var error = gl.getShaderInfoLog(fragmentShaderObject_pv);

        if (error.length > 0)
        {
            alert("Per-Vertex : Fragment Shader Compilation Error : \n" + error);
            uninitialize();
        }
    }

    // Shader Program Object
    shaderProgramObject_sphere_pv = gl.createProgram();

    gl.attachShader(shaderProgramObject_sphere_pv, vertexShaderObject_pv);
    gl.attachShader(shaderProgramObject_sphere_pv, fragmentShaderObject_pv);

    // Pre-Linking
    gl.bindAttribLocation(shaderProgramObject_sphere_pv, webGLMacros.AAN_ATTRIBUTE_POSITION, "a_position");
    gl.bindAttribLocation(shaderProgramObject_sphere_pv, webGLMacros.AAN_ATTRIBUTE_NORMAL, "a_normal");

    // Shader Program Linking
    gl.linkProgram(shaderProgramObject_sphere_pv);

    // Error Checking
    if (gl.getProgramParameter(shaderProgramObject_sphere_pv, gl.LINK_STATUS) == false)
    {
        var error = gl.getProgramInfoLog(shaderProgramObject_sphere_pv);

        if (error.length > 0)
        {
            alert("Per-Vertex Shader Program Link Error : \n" + error);
            uninitialize();
        }
    }

    // Post-linking
    modelMatrixUniform_sphere_pv = gl.getUniformLocation(shaderProgramObject_sphere_pv, "u_modelMatrix");
    viewMatrixUniform_sphere_pv = gl.getUniformLocation(shaderProgramObject_sphere_pv, "u_viewMatrix");
    projectionMatrixUniform_sphere_pv = gl.getUniformLocation(shaderProgramObject_sphere_pv, "u_projectionMatrix");

    laUniform_sphere_pv = gl.getUniformLocation(shaderProgramObject_sphere_pv, "u_la");
    ldUniform_sphere_pv = gl.getUniformLocation(shaderProgramObject_sphere_pv, "u_ld");
    lsUniform_sphere_pv = gl.getUniformLocation(shaderProgramObject_sphere_pv, "u_ls");
    lightPositionUniform_sphere_pv = gl.getUniformLocation(shaderProgramObject_sphere_pv, "u_lightPosition");

    kaUniform_sphere_pv = gl.getUniformLocation(shaderProgramObject_sphere_pv, "u_ka");
    kdUniform_sphere_pv = gl.getUniformLocation(shaderProgramObject_sphere_pv, "u_kd");
    ksUniform_sphere_pv = gl.getUniformLocation(shaderProgramObject_sphere_pv, "u_ks");

    materialShininessUniform_sphere_pv = gl.getUniformLocation(shaderProgramObject_sphere_pv, "u_materialShininess");
    lightingEnabledUniform_sphere_pv = gl.getUniformLocation(shaderProgramObject_sphere_pv, "u_lightingEnabled");


    // **PER-FRAGMENT**

    // Vertex Shader
    var vertexShaderSourceCode_pf = 
    "#version 300 es" +
    "\n" +
    "in vec4 a_position;" +
    "in vec3 a_normal;" +
    "uniform mat4 u_modelMatrix;" +
    "uniform mat4 u_viewMatrix;" +
    "uniform mat4 u_projectionMatrix;" +
    "uniform vec4 u_lightPosition[3];" +
    "uniform mediump int u_lightingEnabled;" +
    "out vec3 transformedNormals;" +
    "out vec3 lightDirection[3];" +
    "out vec3 viewerVector;" +
    "void main(void)" +
    "{" +
    "if (u_lightingEnabled == 1)" +
    "{" +
    "vec4 eyeCoordinates = u_viewMatrix * u_modelMatrix * a_position;" +
    "mat3 normalMatrix = mat3(u_viewMatrix * u_modelMatrix);" +
    "transformedNormals = normalMatrix * a_normal;" +
    "viewerVector = -eyeCoordinates.xyz;" +
    "for (int i = 0; i < 3; i++)" +
    "{" +
    "lightDirection[i] = vec3(u_lightPosition[i]) - eyeCoordinates.xyz;" +
    "}" +
    "}" +
    "gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" +
    "}";

    var vertexShaderObject_pf = gl.createShader(gl.VERTEX_SHADER);

    gl.shaderSource(vertexShaderObject_pf, vertexShaderSourceCode_pf);

    gl.compileShader(vertexShaderObject_pf);

    if (gl.getShaderParameter(vertexShaderObject_pf, gl.COMPILE_STATUS))
    {
        var error = gl.getShaderInfoLog(vertexShaderObject_pf);

        if (error.length > 0)
        {
            alert("Per-Fragment: Vertex Shader Compilation Error : \n" + error);
            uninitialize();
        }
    }
    
    // Fragment Shader
    var fragmentShaderSourceCode_pf = 
    "#version 300 es" +
    "\n" +
    "precision highp float;" +
    "in vec3 transformedNormals;" +
    "in vec3 lightDirection[3];" +
    "in vec3 viewerVector;" +
    "uniform vec3 u_la[3];" +
    "uniform vec3 u_ld[3];" +
    "uniform vec3 u_ls[3];" +
    "uniform vec3 u_ka;" +
    "uniform vec3 u_kd;" +
    "uniform vec3 u_ks;" +
    "uniform float u_materialShininess;" +
    "uniform mediump int u_lightingEnabled;" +
    "out vec4 FragColor;" +
    "void main(void)" +
    "{" +
    "vec3 phong_ads_light;" +
    "if (u_lightingEnabled == 1)" +
    "{" +
    "vec3 ambient[3];" +
    "vec3 diffuse[3];" +
    "vec3 specular[3];" +
    "vec3 reflectionVector[3];" +
    "vec3 normalized_light_direction[3];" +
    "vec3 normalized_transformed_normals = normalize(transformedNormals);" +
    "vec3 normalized_viewer_vector = normalize(viewerVector);" +
    "for (int i = 0; i < 3; i++)" +
    "{" +
    "ambient[i] = u_la[i] * u_ka;" +
    "normalized_light_direction[i] = normalize(lightDirection[i]);" +
    "diffuse[i] = u_ld[i] * u_kd * max(dot(normalized_light_direction[i], normalized_transformed_normals), 0.0);" +
    "reflectionVector[i] = reflect(-normalized_light_direction[i], normalized_transformed_normals);" +      
    "specular[i] = u_ls[i] * u_ks * pow(max(dot(reflectionVector[i], normalized_viewer_vector), 0.0), u_materialShininess);" +
    "phong_ads_light = phong_ads_light + ambient[i] + diffuse[i] + specular[i];" +
    "}" +
    "}" +
    "else" +
    "{" +
    "phong_ads_light = vec3(1.0, 1.0, 1.0);" +
    "}" +
    "FragColor = vec4(phong_ads_light, 1.0);" +
    "}";

    var fragmentShaderObject_pf = gl.createShader(gl.FRAGMENT_SHADER);

    gl.shaderSource(fragmentShaderObject_pf, fragmentShaderSourceCode_pf);

    gl.compileShader(fragmentShaderObject_pf);

    if (gl.getShaderParameter(fragmentShaderObject_pf, gl.COMPILE_STATUS))
    {
        var error = gl.getShaderInfoLog(fragmentShaderObject_pf);

        if (error.length > 0)
        {
            alert("Per-Fragment : Fragment Shader Compilation Error : \n" + error);
            uninitialize();
        }
    }

    // Shader Program Object
    shaderProgramObject_sphere_pf = gl.createProgram();

    gl.attachShader(shaderProgramObject_sphere_pf, vertexShaderObject_pf);
    gl.attachShader(shaderProgramObject_sphere_pf, fragmentShaderObject_pf);

    // Pre-Linking
    gl.bindAttribLocation(shaderProgramObject_sphere_pf, webGLMacros.AAN_ATTRIBUTE_POSITION, "a_position");
    gl.bindAttribLocation(shaderProgramObject_sphere_pf, webGLMacros.AAN_ATTRIBUTE_NORMAL, "a_normal");

    // Shader Program Linking
    gl.linkProgram(shaderProgramObject_sphere_pf);

    // Error Checking
    if (gl.getProgramParameter(shaderProgramObject_sphere_pf, gl.LINK_STATUS) == false)
    {
        var error = gl.getProgramInfoLog(shaderProgramObject_sphere_pf);

        if (error.length > 0)
        {
            alert("Per-Fragment : Shader Program Link Error : \n" + error);
            uninitialize();
        }
    }

    // Post-linking
    modelMatrixUniform_sphere_pf = gl.getUniformLocation(shaderProgramObject_sphere_pf, "u_modelMatrix");
    viewMatrixUniform_sphere_pf = gl.getUniformLocation(shaderProgramObject_sphere_pf, "u_viewMatrix");
    projectionMatrixUniform_sphere_pf = gl.getUniformLocation(shaderProgramObject_sphere_pf, "u_projectionMatrix");

    laUniform_sphere_pf = gl.getUniformLocation(shaderProgramObject_sphere_pf, "u_la");
    ldUniform_sphere_pf = gl.getUniformLocation(shaderProgramObject_sphere_pf, "u_ld");
    lsUniform_sphere_pf = gl.getUniformLocation(shaderProgramObject_sphere_pf, "u_ls");
    lightPositionUniform_sphere_pf = gl.getUniformLocation(shaderProgramObject_sphere_pf, "u_lightPosition");

    kaUniform_sphere_pf = gl.getUniformLocation(shaderProgramObject_sphere_pf, "u_ka");
    kdUniform_sphere_pf = gl.getUniformLocation(shaderProgramObject_sphere_pf, "u_kd");
    ksUniform_sphere_pf = gl.getUniformLocation(shaderProgramObject_sphere_pf, "u_ks");

    materialShininessUniform_sphere_pf = gl.getUniformLocation(shaderProgramObject_sphere_pf, "u_materialShininess");
    lightingEnabledUniform_sphere_pf = gl.getUniformLocation(shaderProgramObject_sphere_pf, "u_lightingEnabled");


    sphere = new Mesh();
    makeSphere(sphere, 2.0, 50, 30);

    // Depth Related Code
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    // Clear The Screen Using Black Color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    lightAmbient_sphere = [
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0
    ];

    lightDiffuse_sphere = [
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0
    ];

    lightSpecular_sphere = [
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0
    ];

    lightPosition_sphere = [
        0.0, 0.0, 0.0, 1.0,
        0.0, 0.0, 0.0, 1.0,
        0.0, 0.0, 0.0, 1.0
    ];

    perspectiveProjectionMatrix_sphere = mat4.create();

    resize_sphere(FBO_WIDTH, FBO_HEIGHT);
}

function resize()
{
    // Code
    if (bFullscreen)
    {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }
    else
    {
        canvas.width = canvas_original_width;
        canvas.height = canvas_original_height;
    }

    if (canvas.height == 0)
        canvas.height = 1;

    gl.viewport(0, 0, canvas.width, canvas.height);

    resize_sphere(canvas.width, canvas.height);

    mat4.perspective(perspectiveProjectionMatrix, 45.0, parseFloat(canvas.width) / parseFloat(canvas.height), 0.1, 100.0);
}

function resize_sphere(width, height)
{
    if (height == 0)
        height = 1;
    
    gl.viewport(0, 0, width, height);

    mat4.perspective(perspectiveProjectionMatrix_sphere, 45.0, parseFloat(width) / parseFloat(height), 0.1, 100.0);
}

function display()
{
    // Code
    if (bFBOResult)
    {
        display_sphere(FBO_WIDTH, FBO_HEIGHT);
        update_sphere();
    }

    gl.clearColor(0.5, 0.5, 0.5, 1.0);

    resize();

    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(shaderProgramObject);

    // Cube

    // Transformations
    modelViewMatrix = mat4.create();
    modelViewProjectionMatrix = mat4.create();
    translationMatrix = mat4.create();
    rotationMatrix = mat4.create();

    mat4.translate(translationMatrix, translationMatrix, [0.0, 0.0, -6.0]);
    mat4.rotateX(rotationMatrix, rotationMatrix, degreeToRadians(angleCube));
    mat4.rotateY(rotationMatrix, rotationMatrix, degreeToRadians(angleCube));
    mat4.rotateZ(rotationMatrix, rotationMatrix, degreeToRadians(angleCube));

    mat4.multiply(modelViewMatrix, translationMatrix, rotationMatrix);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);

    gl.activeTexture(gl.TEXTURE0);
    gl.uniform1i(textureSamplerUniform, 0);
	gl.bindTexture(gl.TEXTURE_2D, fbo_texture);
	
    gl.bindVertexArray(vao_cube);
    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);
    gl.bindVertexArray(null);

    gl.useProgram(null);

    update();

    // Double Buffering Emulation
    requestAnimationFrame(display, canvas);
}

function update()
{
    // Code
    angleCube = angleCube + 0.8;
    if (angleCube > 360.0)
    {
        angleCube = angleCube - 360.0;
    }
    
}

function display_sphere(textureWidth, textureHeight)
{
    gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);

    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    resize_sphere(textureWidth, textureHeight);

    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    if (chosenShader == 'v')
        gl.useProgram(shaderProgramObject_sphere_pv);
    else
        gl.useProgram(shaderProgramObject_sphere_pf);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var translationMatrix = mat4.create();

    mat4.translate(translationMatrix, translationMatrix, [0.0, 0.0, -6.0]);
    modelMatrix = translationMatrix;

    // Light 0
    lightPosition_sphere[1] = radius_sphere * Math.sin(degreeToRadians(lightAngleZero_sphere));
    lightPosition_sphere[2] = radius_sphere * Math.cos(degreeToRadians(lightAngleZero_sphere));
    
    // Light 1
    lightPosition_sphere[4] = radius_sphere * Math.cos(degreeToRadians(lightAngleOne_sphere));
    lightPosition_sphere[6] = radius_sphere * Math.sin(degreeToRadians(lightAngleOne_sphere));
    
    // Light 2
    lightPosition_sphere[8] = radius_sphere * Math.cos(degreeToRadians(lightAngleTwo_sphere));
    lightPosition_sphere[9] = radius_sphere * Math.sin(degreeToRadians(lightAngleTwo_sphere));

    if (chosenShader == 'v')
    {
        gl.uniformMatrix4fv(modelMatrixUniform_sphere_pv, false, modelMatrix);
        gl.uniformMatrix4fv(viewMatrixUniform_sphere_pv, false, viewMatrix);
        gl.uniformMatrix4fv(projectionMatrixUniform_sphere_pv, false, perspectiveProjectionMatrix_sphere);
    
        if (bLight)
        {
            gl.uniform1i(lightingEnabledUniform_sphere_pv, 1);
    
            gl.uniform3fv(laUniform_sphere_pv, lightAmbient_sphere);
            gl.uniform3fv(ldUniform_sphere_pv, lightDiffuse_sphere);
            gl.uniform3fv(lsUniform_sphere_pv, lightSpecular_sphere);
            gl.uniform4fv(lightPositionUniform_sphere_pv, lightPosition_sphere);
    
            gl.uniform3fv(kaUniform_sphere_pv, materialAmbient_sphere);
            gl.uniform3fv(kdUniform_sphere_pv, materialDiffuse_sphere);
            gl.uniform3fv(ksUniform_sphere_pv, materialSpecular_sphere);
            gl.uniform1f(materialShininessUniform_sphere_pv, materialShininess_sphere);
            
        }
        else
            gl.uniform1i(lightingEnabledUniform_sphere_pv, 0);
    }

    else if (chosenShader == 'f')
    {
        gl.uniformMatrix4fv(modelMatrixUniform_sphere_pf, false, modelMatrix);
        gl.uniformMatrix4fv(viewMatrixUniform_sphere_pf, false, viewMatrix);
        gl.uniformMatrix4fv(projectionMatrixUniform_sphere_pf, false, perspectiveProjectionMatrix_sphere);
    
        if (bLight)
        {
            gl.uniform1i(lightingEnabledUniform_sphere_pf, 1);
    
            gl.uniform3fv(laUniform_sphere_pf, lightAmbient_sphere);
            gl.uniform3fv(ldUniform_sphere_pf, lightDiffuse_sphere);
            gl.uniform3fv(lsUniform_sphere_pf, lightSpecular_sphere);
            gl.uniform4fv(lightPositionUniform_sphere_pf, lightPosition_sphere);
    
            gl.uniform3fv(kaUniform_sphere_pf, materialAmbient_sphere);
            gl.uniform3fv(kdUniform_sphere_pf, materialDiffuse_sphere);
            gl.uniform3fv(ksUniform_sphere_pf, materialSpecular_sphere);
            gl.uniform1f(materialShininessUniform_sphere_pf, materialShininess_sphere);
            
        }
        else
            gl.uniform1i(lightingEnabledUniform_sphere_pf, 0);
    }

    sphere.draw();

    gl.useProgram(null);

    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
}

function update_sphere()
{
    // Code
    lightAngleZero_sphere = lightAngleZero_sphere + 1.0;
    if (lightAngleZero_sphere > 360.0)
        lightAngleZero_sphere = lightAngleZero_sphere - 360.0;

    lightAngleOne_sphere = lightAngleOne_sphere + 1.0;
    if (lightAngleOne_sphere > 360.0)
        lightAngleOne_sphere = lightAngleOne_sphere - 360.0;

    lightAngleTwo_sphere = lightAngleTwo_sphere + 1.0;
    if (lightAngleTwo_sphere > 360.0)
        lightAngleTwo_sphere = lightAngleTwo_sphere - 360.0;
}

function degreeToRadians(degrees)
{
    return (degrees * (Math.PI / 180.0));
}

// Keyboard Event Listener
function keyDown(event)
{
    // Code
    switch(event.keyCode)
    {
        case 84: //T
            toggleFullscreen();
        break;

        case 70: // F
            chosenShader = 'f';
        break;

        case 76: // L
            if (bLight == false)
                bLight = true;
            else
                bLight = false;
        break;

        case 86: // V
            chosenShader = 'v';
        break;

        case 81: // Q
            uninitialize();
            window.close(); // Not Applicable For All Browsers
        break;
            
    }
}

// Mouse Event Listener
function mousedown()
{
    // Code
    
}

function uninitialize()
{
    // Code
    uninitialize_sphere();

    if (vbo_cube_texcoord)
    {
        gl.deleteBuffer(vbo_cube_texcoord);
        vbo_cube_texcoord = null;
    }
    
    if (vbo_cube_position)
    {
        gl.deleteBuffer(vbo_cube_position);
        vbo_cube_position = null;
    }

    if (vao_cube)
    {
        gl.deleteVertexArray(vao_cube);
        vao_cube = null;
    }

    // Shader Uninitialization
    if (shaderProgramObject)
    {
        gl.useProgram(shaderProgramObject);

        var shaderObjects = gl.getAttachedShaders(shaderProgramObject);
        
        for (let i = 0; i < shaderObjects; i++)
        {
            gl.detachShader(shaderProgramObject, shaderObjects[i]);
            gl.deleteShader(shaderObjects[i]);
            shaderObjects[i] = 0;
        }

        gl.useProgram(null);
        gl.deleteProgram(shaderProgramObject);
        shaderProgramObject = null;
    }
}

function uninitialize_sphere()
{
    // Code
    if (sphere)
    {
        sphere.deallocate();
        sphere = null;
    }

    // Shader Uninitialization
    if (shaderProgramObject_sphere_pf)
    {
        gl.useProgram(shaderProgramObject_sphere_pf);

        var shaderObjects = gl.getAttachedShaders(shaderProgramObject_sphere_pf);
        
        for (let i = 0; i < shaderObjects; i++)
        {
            gl.detachShader(shaderProgramObject_sphere_pf, shaderObjects[i]);
            gl.deleteShader(shaderObjects[i]);
            shaderObjects[i] = 0;
        }

        gl.useProgram(null);
        gl.deleteProgram(shaderProgramObject_sphere_pf);
        shaderProgramObject_sphere_pf = null;
    }

    if (shaderProgramObject_sphere_pv)
    {
        gl.useProgram(shaderProgramObject_sphere_pv);

        var shaderObjects = gl.getAttachedShaders(shaderProgramObject_sphere_pv);
        
        for (let i = 0; i < shaderObjects; i++)
        {
            gl.detachShader(shaderProgramObject_sphere_pv, shaderObjects[i]);
            gl.deleteShader(shaderObjects[i]);
            shaderObjects[i] = 0;
        }

        gl.useProgram(null);
        gl.deleteProgram(shaderProgramObject_sphere_pv);
        shaderProgramObject_sphere_pv = null;
    }
}
