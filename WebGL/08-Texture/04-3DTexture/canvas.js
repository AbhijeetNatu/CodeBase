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

var vao_pyramid;
var vbo_pyramid_position;
var vbo_pyramid_texcoord;

var vao_cube;
var vbo_cube_position;
var vbo_cube_texcoord;

var mvpMatrixUniform;
var textureSamplerUniform;

var perspectiveProjectionMatrix;

var texture_kundali = 0, texture_stone = 0;

var angleCube = 0.0;
var anglePyramid = 0.0;

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
    //-------------------------------------------------------------------------------
    
    // Fragment Shader
    //-------------------------------------------------------------------------------
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
    //-------------------------------------------------------------------------------

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
    var pyramidVertices = new Float32Array([  
        // Front
        0.0, 1.0, 0.0,
        -1.0, -1.0, 1.0,
        1.0, -1.0, 1.0,

        // Right
        0.0, 1.0, 0.0,
        1.0, -1.0, 1.0,
        1.0, -1.0, -1.0,

        // Back
        0.0, 1.0, 0.0,
        1.0, -1.0, -1.0,
        -1.0, -1.0, -1.0,

        // Left
        0.0, 1.0, 0.0,
        -1.0, -1.0, -1.0,
        -1.0, -1.0, 1.0
    ]);

    var pyramidTexcoords = new Float32Array([
        // Front
        0.5, 1.0,
        0.0, 0.0,
        1.0, 0.0,

        // Right
        0.5, 1.0,
        1.0, 0.0,
        0.0, 0.0,

        // Left
        0.5, 1.0,
        0.0, 0.0,
        1.0, 0.0,

        // Back
        0.5, 1.0,
        1.0, 0.0,
        0.0, 0.0,
    ]);

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
        1.0, 0.0,
        0.0, 0.0,
        0.0, 1.0,
    
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

    // Pyramid
    
    // VAO
    vao_pyramid = gl.createVertexArray();
    gl.bindVertexArray(vao_pyramid);

    // VBO For Position
    vbo_pyramid_position = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_pyramid_position);
    gl.bufferData(gl.ARRAY_BUFFER, pyramidVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    // VBO For Texture
    vbo_pyramid_texcoord = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_pyramid_texcoord);
    gl.bufferData(gl.ARRAY_BUFFER, pyramidTexcoords, gl.STATIC_DRAW);
    gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_TEXTURE0, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_TEXTURE0);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    
    gl.bindVertexArray(null);

    

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

    // VBO For Texture
    vbo_cube_texcoord = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_cube_texcoord);
    gl.bufferData(gl.ARRAY_BUFFER, cubeTexcoords, gl.STATIC_DRAW);
    gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_TEXTURE0, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_TEXTURE0);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    
    gl.bindVertexArray(null);
    

    //TEXTURE RELATED CODE
    
    texture_stone = gl.createTexture();
    texture_stone.image = new Image();
    texture_stone.image.src = "stone.png";
    texture_stone.image.onload = function() 
    {
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
        gl.bindTexture(gl.TEXTURE_2D, texture_stone);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);

        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, texture_stone.image);
        gl.generateMipmap(gl.TEXTURE_2D);
        gl.bindTexture(gl.TEXTURE_2D, null);
    };

    texture_kundali = gl.createTexture();
    texture_kundali.image = new Image();
    texture_kundali.image.src = "Vijay_Kundali.png";
    texture_kundali.image.onload = function() 
    {
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
        gl.bindTexture(gl.TEXTURE_2D, texture_kundali);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);

        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, texture_kundali.image);
        gl.generateMipmap(gl.TEXTURE_2D);
        gl.bindTexture(gl.TEXTURE_2D, null);
    };

    gl.enable(gl.TEXTURE_2D);

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

    // Pyramid
    
    // Transformations
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();
    var translationMatrix = mat4.create();
    var rotationMatrix = mat4.create();

    mat4.translate(translationMatrix, translationMatrix, [-1.5, 0.0, -6.0]);
    mat4.rotateY(rotationMatrix, rotationMatrix, degreeToRadians(anglePyramid));

    mat4.multiply(modelViewMatrix, translationMatrix, rotationMatrix);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);

    gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, texture_stone);
	gl.uniform1i(textureSamplerUniform, 0);

    gl.bindVertexArray(vao_pyramid);
    gl.drawArrays(gl.TRIANGLES, 0, 12);
    gl.bindVertexArray(null);

    // Cube
    
    // Transformations
    modelViewMatrix = mat4.create();
    modelViewProjectionMatrix = mat4.create();
    translationMatrix = mat4.create();
    rotationMatrix = mat4.create();
    var scaleMatrix = mat4.create();

    mat4.translate(translationMatrix, translationMatrix, [1.5, 0.0, -6.0]);
    mat4.scale(scaleMatrix, scaleMatrix, [0.75, 0.75, 0.75]);
    mat4.rotateX(rotationMatrix, rotationMatrix, degreeToRadians(angleCube));
    mat4.rotateY(rotationMatrix, rotationMatrix, degreeToRadians(angleCube));
    mat4.rotateZ(rotationMatrix, rotationMatrix, degreeToRadians(angleCube));

    mat4.multiply(scaleMatrix, scaleMatrix, rotationMatrix);
    mat4.multiply(modelViewMatrix, translationMatrix, scaleMatrix);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);

    gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, texture_kundali);
	gl.uniform1i(textureSamplerUniform, 0);

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

function degreeToRadians(degrees)
{
    return (degrees * (Math.PI / 180.0));
}

function update()
{
    // Code
    anglePyramid = anglePyramid + 1.0;
    if (anglePyramid > 360.0)
        anglePyramid = anglePyramid - 360.0

    angleCube = angleCube + 1.0;
    if (angleCube > 360.0)
        angleCube = angleCube - 360.0
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
    if (texture_kundali)
	{
		gl.deleteTexture(texture_kundali);
		texture_kundali = null;
	}

    if (texture_stone)
	{
		gl.deleteTexture(texture_stone);
		texture_stone = null;
	}

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

    if (vbo_pyramid_texcoord)
    {
        gl.deleteBuffer(vbo_pyramid_texcoord);
        vbo_pyramid_texcoord = null;
    }

    if (vbo_pyramid_position)
    {
        gl.deleteBuffer(vbo_pyramid_position);
        vbo_pyramid_position = null;
    }

    if (vao_pyramid)
    {
        gl.deleteVertexArray(vao_pyramid);
        vao_pyramid = null;
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