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

var shaderProgramObject;

var vao;
var vbo;

var modelMatrixUniform;
var viewMatrixUniform;
var projectionMatrixUniform;

var perspectiveProjectionMatrix;

var sphere = null;

var ldUniform;
var kdUniform;
var lightPositionUniform;
var lightingEnabledUniform;

var bLight = false;

var lightDiffuse = [1.0, 1.0, 1.0];
var lightPosition = [0.0, 0.0, 2.0, 1.0];
var materialDiffuse = [0.5, 0.5, 0.5];

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
    var fullscreen_Element = document.fullscreenElement ||       // Generic
                             document.mozFullScreenElement ||    // Mozilla
                             document.webkitFullscreenElement || // Apple
                             document.msFullscreenElement ||     // Edge
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
    //-------------------------------------------------------------------------------
    var vertexShaderSourceCode = 
    "#version 300 es" +
    "\n" +
    "in vec4 a_position;" +
    "in vec3 a_normal;" +
    "uniform mat4 u_modelMatrix;" +
    "uniform mat4 u_viewMatrix;" +
    "uniform mat4 u_projectionMatrix;" +
    "uniform vec3 u_ld;" +
    "uniform vec3 u_kd;" +
    "uniform vec4 u_lightPosition;" +
    "uniform mediump int u_lightingEnabled;" +
    "out vec3 diffused_light_color;" +
    "void main(void)" +
    "{" +
    "if(u_lightingEnabled == 1)" +
    "{" +
    "vec4 eyeCoordinates = u_viewMatrix * u_modelMatrix * a_position;" +
    "mat3 normalMatrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix)));" +
    "vec3 transformedNormals = normalize(normalMatrix * a_normal);" +
    "vec3 lightDirection = normalize(vec3(u_lightPosition - eyeCoordinates));" +
    "diffused_light_color = u_ld * u_kd * max(dot(lightDirection, transformedNormals), 0.0);" +
    "}" +
    "gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" +
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
    //-------------------------------------------------------------------------------
    
    // Fragment Shader
    //-------------------------------------------------------------------------------
    var fragmentShaderSourceCode = 
    "#version 300 es" +
    "\n" +
    "precision highp float;" +
    "in vec3 diffused_light_color;" +
    "uniform mediump int u_lightingEnabled;" +
    "out vec4 FragColor;" +
    "void main(void)" +
    "{" +
    "if(u_lightingEnabled == 1)" +
    "{" +
    "FragColor = vec4(diffused_light_color, 1.0);" +
    "}" +
    "else" +
    "{" +
    "FragColor = vec4(1.0, 1.0, 1.0, 1.0);" +
    "}" +
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
    //-------------------------------------------------------------------------------

    // Shader Program Object
    shaderProgramObject = gl.createProgram();

    gl.attachShader(shaderProgramObject, vertexShaderObject);
    gl.attachShader(shaderProgramObject, fragmentShaderObject);

    // Pre-Linking
    gl.bindAttribLocation(shaderProgramObject, webGLMacros.AAN_ATTRIBUTE_POSITION, "a_position");
    gl.bindAttribLocation(shaderProgramObject, webGLMacros.AAN_ATTRIBUTE_NORMAL, "a_normal");

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
    modelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_modelMatrix");
    viewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_viewMatrix");
    projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projectionMatrix");

    ldUniform = gl.getUniformLocation(shaderProgramObject, "u_ld");
    kdUniform = gl.getUniformLocation(shaderProgramObject, "u_kd");

    lightPositionUniform = gl.getUniformLocation(shaderProgramObject, "u_lightPosition");
    lightingEnabledUniform = gl.getUniformLocation(shaderProgramObject, "u_lightingEnabled");

    sphere = new Mesh();
    makeSphere(sphere, 2.0, 50, 30);

    // Depth Related Code
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    // Clear The Screen Using Black Color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    perspectiveProjectionMatrix = mat4.create();
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

    gl.viewport(0, 0, canvas.width, canvas.height);

    mat4.perspective(perspectiveProjectionMatrix, 45.0, parseFloat(canvas.width) / parseFloat(canvas.height), 0.1, 100.0);
}

function display()
{
    // Code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUUFER_BIT);

    gl.useProgram(shaderProgramObject);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var translationMatrix = mat4.create();

    mat4.translate(translationMatrix, translationMatrix, [0.0, 0.0, -6.0]);
    modelMatrix = translationMatrix;

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if (bLight)
    {
        gl.uniform1i(lightingEnabledUniform, 1);

        gl.uniform3fv(ldUniform, lightDiffuse);
        gl.uniform3fv(kdUniform, materialDiffuse);
        gl.uniform4fv(lightPositionUniform, lightPosition);
    }
    else
        gl.uniform1i(lightingEnabledUniform, 0);

    sphere.draw();

    gl.useProgram(null);

    // Double Buffering Emulation
    requestAnimationFrame(display, canvas);
}

function update()
{
    // Code
}

// Keyboard Event Listener
function keyDown(event)
{
    // Code
    switch(event.keyCode)
    {
        case 70: // F
            toggleFullscreen();
        break;

        case 76: // L
            if (bLight == false)
                bLight = true;
            else
                bLight = false;
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
    if(vao)
    {
        gl.deleteVertexArray(vao);
        vao = null;
    }

    if(vbo)
    {
        gl.deleteBuffer(vbo);
        vbo = null;
    }

    if (sphere)
    {
        sphere.deallocate();
        sphere = null;
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

