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

var vao_grid_vertical;
var vbo_grid_vertical;
var vao_grid_horizontal;
var vbo_grid_horizontal;

var vao_axes;
var vbo_axes_position;
var vbo_axes_colors;

var vao_square;
var vbo_square;

var vao_outer_triangle;
var vbo_outer_triangle;

var vao_inner_triangle;
var vbo_inner_triangle;

var vao_circle;
var vbo_circle;

var numVertices = 1080;
var circleVertices = [];

var drawGrid = false;
var drawSquare = false;
var drawOuterTriangle = false;
var drawInnerTriangle = false;
var drawCircle = false;

var mvpMatrixUniform;

var perspectiveProjectionMatrix;

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
    "uniform mat4 u_mvpMatrix;" +
    "out vec4 a_color_out;" +
    "void main(void)" +
    "{" +
    "gl_Position = u_mvpMatrix * a_position;" +
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
    mvpMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_mvpMatrix");

    // Declaration & Initialization Of Geometry
    var gridLinesVertical = new Float32Array([
        0.0, 1.0, 0.0,
        0.0, -1.0, 0.0
    ]);

    var gridLinesHorizontal = new Float32Array([
        1.0, 0.0, 0.0,
        -1.0, 0.0, 0.0
    ]);

    var axesVertices = new Float32Array([
        -1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        0.0, -1.0, 0.0,
        0.0, 1.0, 0.0
    ]);

    var axesColors = new Float32Array([
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0
    ]);

    var squareVertices = new Float32Array([
        0.5, 0.5, 0.0,
        -0.5, 0.5, 0.0,
        -0.5, -0.5, 0.0,
        0.5, -0.5, 0.0
    ]);

    var outerTriangleVertices = new Float32Array([
        0.0, 0.5, 0.0,
        -0.5, -0.5, 0.0,
        0.5, -0.5, 0.0
    ]);

    var innerTriangleVertices = new Float32Array([
        0.0, 0.5, 0.0,
        -0.4, -0.3, 0.0,
        0.4, -0.3, 0.0
    ]);

    // VAO and VBO Related Code

    // Grid - Vertical
    // ------------------------------------------------------------------------------------
    // VAO
    vao_grid_vertical = gl.createVertexArray();
    gl.bindVertexArray(vao_grid_vertical);

    // VBO For Position
    vbo_grid_vertical = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_grid_vertical);
    gl.bufferData(gl.ARRAY_BUFFER, gridLinesVertical, gl.STATIC_DRAW);
    gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);
    // ------------------------------------------------------------------------------------

    // Grid - Horizontal
    // ------------------------------------------------------------------------------------
    // VAO
    vao_grid_horizontal = gl.createVertexArray();
    gl.bindVertexArray(vao_grid_horizontal);

    // VBO For Position
    vbo_grid_horizontal = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_grid_horizontal);
    gl.bufferData(gl.ARRAY_BUFFER, gridLinesHorizontal, gl.STATIC_DRAW);
    gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);
    // ------------------------------------------------------------------------------------

    // Axes
    // ------------------------------------------------------------------------------------
    // VAO
    vao_axes = gl.createVertexArray();
    gl.bindVertexArray(vao_axes);

    // VBO For Position
    vbo_axes_position = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_axes_position);
    gl.bufferData(gl.ARRAY_BUFFER, axesVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    // VBO For Color
    vbo_axes_colors = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_axes_colors);
    gl.bufferData(gl.ARRAY_BUFFER, axesColors, gl.STATIC_DRAW);
    gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);
    // ------------------------------------------------------------------------------------

    // Square
    // ------------------------------------------------------------------------------------
    // VAO
    vao_square = gl.createVertexArray();
    gl.bindVertexArray(vao_square);

    // VBO For Position
    vbo_square = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_square);
    gl.bufferData(gl.ARRAY_BUFFER, squareVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);
    // ------------------------------------------------------------------------------------

    // Outer Triangle
    // ------------------------------------------------------------------------------------
    // VAO
    vao_outer_triangle = gl.createVertexArray();
    gl.bindVertexArray(vao_outer_triangle);

    // VBO For Position
    vbo_outer_triangle = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_outer_triangle);
    gl.bufferData(gl.ARRAY_BUFFER, outerTriangleVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);
    // ------------------------------------------------------------------------------------
    
    // Inner Triangle
    // ------------------------------------------------------------------------------------
    // VAO
    vao_inner_triangle = gl.createVertexArray();
    gl.bindVertexArray(vao_inner_triangle);

    // VBO For Position
    vbo_inner_triangle = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_inner_triangle);
    gl.bufferData(gl.ARRAY_BUFFER, innerTriangleVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);
    // ------------------------------------------------------------------------------------
    
    // Circle
    // ------------------------------------------------------------------------------------
    getCircleVertices();

    // VAO
    vao_circle = gl.createVertexArray();
    gl.bindVertexArray(vao_circle);

    // VBO For Position
    vbo_circle = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_circle);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(circleVertices), gl.STATIC_DRAW);
    gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);
    // ------------------------------------------------------------------------------------

    // Depth Related Code
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    // Clear The Screen Using BLack Color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    perspectiveProjectionMatrix = mat4.create();
}

function getCircleVertices()
{
    let fx = 0.0, fy = 0.0;
    let fRadius = 0.5;
    let numberOfVertices = 1080;

    let angle = 0.0;
    let i = 0;
    for (i = 0, angle = 0.0; i < numberOfVertices && angle < 360.0; i = i + 3, angle++)
    {
        let fAngle = (2 * Math.PI * angle)/360.0;

        fx = fRadius * Math.cos(fAngle);
        fy = fRadius * Math.sin(fAngle);

        circleVertices[i] = fx;
        circleVertices[i + 1] = fy;
        circleVertices[i + 2] = 0.0;   
    }

    circleVertices[numberOfVertices - 3] = circleVertices[0];
    circleVertices[numberOfVertices - 2] = circleVertices[1];
    circleVertices[numberOfVertices - 1] = circleVertices[2];
    
    numVertices = numVertices / 3;
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
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();
    var translationMatrix = mat4.create();
    var rotationMatrix = mat4.create();

    if (drawGrid)
    {
        // Grid
        for (let count = -1.0; count <= 1.01; count = count + 0.05)
        {
            // Vertical Lines
            mat4.identity(translationMatrix);
            mat4.translate(translationMatrix, translationMatrix, [count, 0.0, -3.0]);
            modelViewMatrix = translationMatrix;
            mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
            gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
            gl.bindVertexArray(vao_grid_vertical);
            gl.lineWidth(1.5);
            gl.vertexAttrib3f(webGLMacros.AAN_ATTRIBUTE_COLOR, 0.0, 0.0, 1.0);
            gl.drawArrays(gl.LINES, 0, 2);
            gl.bindVertexArray(null);

            // Horizontal Lines
            mat4.identity(translationMatrix);
            mat4.translate(translationMatrix, translationMatrix, [0.0, count, -3.0]);
            modelViewMatrix = translationMatrix;
            mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
            gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
            gl.bindVertexArray(vao_grid_horizontal);
            gl.lineWidth(1.5);
            gl.vertexAttrib3f(webGLMacros.AAN_ATTRIBUTE_COLOR, 0.0, 0.0, 1.0);    
            gl.drawArrays(gl.LINES, 0, 2);
            gl.bindVertexArray(null);
        }

        // Axes
        mat4.identity(translationMatrix);
        mat4.translate(translationMatrix, translationMatrix, [0.0, 0.0, -3.0]);
        modelViewMatrix = translationMatrix;
        mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
        gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
        gl.bindVertexArray(vao_axes);
        gl.lineWidth(3.0);
        gl.drawArrays(gl.LINES, 0, 4);
        gl.bindVertexArray(null);    
    }

    if (drawSquare)
    {
        gl.bindVertexArray(vao_square);
        gl.vertexAttrib3f(webGLMacros.AAN_ATTRIBUTE_COLOR, 1.0, 1.0, 0.0);
        gl.drawArrays(gl.LINE_LOOP, 0, 4);
        gl.bindVertexArray(null);    
    }

    if (drawOuterTriangle)
    {
        gl.bindVertexArray(vao_outer_triangle);
        gl.vertexAttrib3f(webGLMacros.AAN_ATTRIBUTE_COLOR, 1.0, 1.0, 0.0);
        gl.drawArrays(gl.LINE_LOOP, 0, 3);
        gl.bindVertexArray(null); 
    }

    if (drawInnerTriangle)
    {
        gl.bindVertexArray(vao_inner_triangle);
        gl.vertexAttrib3f(webGLMacros.AAN_ATTRIBUTE_COLOR, 1.0, 1.0, 0.0);
        gl.drawArrays(gl.LINE_LOOP, 0, 3);
        gl.bindVertexArray(null); 
    }

    if (drawCircle)
    {
        gl.bindVertexArray(vao_circle);
        gl.vertexAttrib3f(webGLMacros.AAN_ATTRIBUTE_COLOR, 1.0, 1.0, 0.0);
        gl.drawArrays(gl.LINE_LOOP, 0, numVertices);
        gl.bindVertexArray(null);
    }
    
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

        case 67: // C
            if (drawCircle == false)
                drawCircle = true;
            else
                drawCircle = false;
        break;


        case 71: // G
            if (drawGrid == false)
                drawGrid = true;
            else
                drawGrid = false;
        break;

        case 73: // I
            if (drawInnerTriangle == false)
                drawInnerTriangle = true;
            else
                drawInnerTriangle = false;
        break;

        case 79: // T
            if (drawOuterTriangle == false)
                drawOuterTriangle = true;
            else
                drawOuterTriangle = false;
        break;

        case 83: // S
            if (drawSquare == false)
                drawSquare = true;
            else
                drawSquare = false;
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
    if (vbo_circle)
    {
        gl.deleteBuffer(vbo_circle);
        vbo_circle = null;
    }

    if (vao_circle)
    {
        gl.deleteVertexArray(vao_circle);
        vao_circle = null;
    }

    if (vbo_inner_triangle)
    {
        gl.deleteBuffer(vbo_inner_triangle);
        vbo_inner_triangle = null;
    }

    if (vao_inner_triangle)
    {
        gl.deleteVertexArray(vao_inner_triangle);
        vao_inner_triangle = null;
    }

    if (vbo_outer_triangle)
    {
        gl.deleteBuffer(vbo_outer_triangle);
        vbo_outer_triangle = null;
    }

    if (vao_outer_triangle)
    {
        gl.deleteVertexArray(vao_outer_triangle);
        vao_outer_triangle = null;
    }

    if (vbo_square)
    {
        gl.deleteBuffer(vbo_square);
        vbo_square = null;
    }

    if (vao_square)
    {
        gl.deleteVertexArray(vao_square);
        vao_square = null;
    }

    if (vbo_axes_colors)
    {
        gl.deleteBuffer(vbo_axes_colors);
        vbo_axes_colors = null;
    }

    if (vbo_axes_position)
    {
        gl.deleteBuffer(vbo_axes_position);
        vbo_axes_position = null;
    }

    if (vao_axes)
    {
        gl.deleteVertexArray(vao_axes);
        vao_axes = null;
    }

    if (vbo_grid_horizontal)
    {
        gl.deleteBuffer(vbo_grid_horizontal);
        vbo_grid_horizontal = null;
    }

    if (vao_grid_horizontal)
    {
        gl.deleteVertexArray(vao_grid_horizontal);
        vao_grid_horizontal = null;
    }

    if (vbo_grid_vertical)
    {
        gl.deleteBuffer(vbo_grid_vertical);
        vbo_grid_vertical = null;
    }

    if (vao_grid_vertical)
    {
        gl.deleteVertexArray(vao_grid_vertical);
        vao_grid_vertical = null;
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

