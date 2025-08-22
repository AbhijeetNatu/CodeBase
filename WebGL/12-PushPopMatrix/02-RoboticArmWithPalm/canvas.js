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

var MODELVIEW_MATRIX_STACK_SIZE  = 32; 
var matrixStack = new Float32Array(MODELVIEW_MATRIX_STACK_SIZE * 16); 
var matrixStackTop = -1;

var shoulder = 0.0;
var elbow = 0.0;
var palm = 0.0;

var clockWiseRotation = false;

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
    "in vec4 a_color;" +
    "uniform mat4 u_modelMatrix;" +
    "uniform mat4 u_viewMatrix;" +
    "uniform mat4 u_projectionMatrix;" +
    "out vec4 a_color_out;"+
    "void main(void)" +
    "{" +
    "gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" +
    "a_color_out = a_color;" +
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
    "in vec4 a_color_out;" +
    "out vec4 FragColor;" +
    "void main(void)" +
    "{" +
    "FragColor = a_color_out;" +
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
    gl.bindAttribLocation(shaderProgramObject, webGLMacros.AAN_ATTRIBUTE_COLOR, "a_color");

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

    sphere = new Mesh();
    makeSphere(sphere, 2.0, 50, 30);
    gl.vertexAttrib3f(webGLMacros.AAN_ATTRIBUTE_COLOR, 0.5, 0.35, 0.05);

    // Depth Related Code
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    // Clear The Screen Using Black Color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    perspectiveProjectionMatrix = mat4.create();

    initializeMatrixStack();
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
    var tempModelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var translationMatrix = mat4.create();
    var rotationMatrix = mat4.create();
    var scaleMatrix = mat4.create();

    mat4.translate(translationMatrix, translationMatrix, [0.0, 0.0, -12.0]);
    mat4.multiply(modelMatrix, modelMatrix, translationMatrix);

    pushMatrix(modelMatrix);
    {
        mat4.rotateZ(rotationMatrix, rotationMatrix, shoulder);
        
        mat4.identity(translationMatrix);
        mat4.translate(translationMatrix, translationMatrix, [2.0, 0.0, 0.0]);
        
        mat4.identity(tempModelMatrix);

        mat4.multiply(tempModelMatrix, tempModelMatrix, rotationMatrix);
        mat4.multiply(tempModelMatrix, tempModelMatrix, translationMatrix);
        mat4.multiply(modelMatrix, modelMatrix, tempModelMatrix);

        pushMatrix(modelMatrix);
        {
            mat4.scale(scaleMatrix, scaleMatrix, [1.0, 0.2, 1.0]);
            
            mat4.multiply(modelMatrix, modelMatrix, scaleMatrix);

            gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
            gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
            gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);
            
            sphere.draw();
        }
        modelMatrix = popMatrix();

        mat4.identity(translationMatrix);
        mat4.identity(rotationMatrix);
        mat4.identity(tempModelMatrix);

        mat4.translate(translationMatrix, translationMatrix, [2.0, 0.0, 0.0]);
        mat4.rotateZ(rotationMatrix, rotationMatrix, elbow);
        
        mat4.multiply(tempModelMatrix, tempModelMatrix, translationMatrix);
        mat4.multiply(tempModelMatrix, tempModelMatrix, rotationMatrix);
        mat4.multiply(modelMatrix, modelMatrix, tempModelMatrix);

        mat4.identity(translationMatrix);
        mat4.translate(translationMatrix, translationMatrix, [1.5, 0.0, 0.0]);
        mat4.multiply(modelMatrix, modelMatrix, translationMatrix);

        pushMatrix(modelMatrix);
        {
            mat4.identity(scaleMatrix);
            mat4.scale(scaleMatrix, scaleMatrix, [0.8, 0.2, 0.2]);
            mat4.multiply(modelMatrix, modelMatrix, scaleMatrix);

            gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
            gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
            gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);
            
            sphere.draw();
        }
        modelMatrix = popMatrix();

        mat4.identity(translationMatrix);
        mat4.identity(rotationMatrix);
        mat4.identity(tempModelMatrix);

        mat4.translate(translationMatrix, translationMatrix, [1.2, 0.0, 0.0]);
        mat4.rotateZ(rotationMatrix, rotationMatrix, palm);

        mat4.multiply(tempModelMatrix, tempModelMatrix, translationMatrix);
        mat4.multiply(tempModelMatrix, tempModelMatrix, rotationMatrix);
        mat4.multiply(modelMatrix, modelMatrix, tempModelMatrix);

        mat4.identity(translationMatrix);
        mat4.translate(translationMatrix, translationMatrix, [0.8, 0.0, 0.0]);
        mat4.multiply(modelMatrix, modelMatrix, translationMatrix);
        
        pushMatrix(modelMatrix);
        {
            mat4.identity(scaleMatrix);
            mat4.scale(scaleMatrix, scaleMatrix, [0.2, 0.2, 0.2]);
            mat4.multiply(modelMatrix, modelMatrix, scaleMatrix);

            gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
            gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
            gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);
            
            sphere.draw();
        }
        modelMatrix = popMatrix();
    }
    modelMatrix = popMatrix();

    gl.useProgram(null);

    // Double Buffering Emulation
    requestAnimationFrame(display, canvas);
}

function initializeMatrixStack()
{
    matrixStackTop = 0;
    for (let i = matrixStackTop; i < MODELVIEW_MATRIX_STACK_SIZE; i++)
    {
        for (let j = 0; j < 16; j++)
        {
            matrixStack[i * 16 * j] = 0.0;
        }
    }
}

function pushMatrix(matrix)
{
    if (matrixStackTop > (MODELVIEW_MATRIX_STACK_SIZE - 1))
    {
        console.log("Stack Overflow !!!");
        uninitialize();
    }

    for (let i = 0; i < 16; i++)
    {
        matrixStack[matrixStackTop * 16 + i] = matrix[i];
    }

    matrixStackTop++;
}

function popMatrix()
{
    let matrix = new Float32Array(16);

    if (matrixStackTop < 0)
    {
        console.log("Stack Empty !!!");
        uninitialize();
    }

    matrixStackTop--;

    for (let i = 0; i < 16; i++)
    {
        matrix[i] = matrixStack[matrixStackTop * 16 + i];
    }

    return matrix;
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

        case 83: // S
            shoulder = (shoulder + 0.05) % 360;
        break;

        case 69: // E
            elbow = (elbow + 0.05) % 360;
        break;

        case 80: // P
            if (palm < 0.5)   
                palm = (palm + 0.05) % 60;
            else
                palm = 0.0;     
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