var canvas = null;
var gl = null;          // this is Context internally
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;

const webGLMacros = 
    {
        AAN_ATTRIBUTE_POSITION:0,
        AAN_ATTRIBUTE_COLOR:1,
        AAN_ATTRIBUTE_NORMAL:2,
        AAN_ATTRIBUTE_TEXTURE0:3,
    };

var shaderProgramObject;

var vao_cube;
var vbo_cube_position;
var vbo_cube_normal;

var angleCube = 0.0;


var modelMatrixUniform;
var viewMatrixUniform;
var projectionMatrixUniform;


//LIGHTS
var ldUniform;
var kdUniform;
var lightPositionUniform;
var lightingEnabledUniform;

var bLight = false;

var lightPosition = [0.0,0.0,2.0];
var materialDiffuse = [0.5,0.5,0.5];
var lightDiffuse = [1.0,1.0,1.0];



var perspectiveProjectionMatrix;

var requestAnimationFrame = window.requestAnimationFrame || //initialized variable
                            window.mozRequestAnimationFrame ||
                            window.webkitRequestAnimationFrame ||
                            window.oRequestAnimationFrame ||      //Opera : O            
                            window.msRequestAnimationFrame;

function main()
{
    //code
    //Get Canvas
    canvas=document.getElementById("AAN");
    if(!canvas)
        console.log("Obtaining Canvas Failed..");
    else
        console.log("Obtaining Canvas Succeeded..");

    //Backup Canvas Dimensions
    canvas_original_width=canvas.width;
    canvas_original_height=canvas.height;
    
    //Initialize
    initialize();
    
    //Warmup resize
    resize();

    //Display
    display();
    
    //Adding Keyboard and Mouse Event Listeners
    window.addEventListener("keydown",keyDown,false);
    window.addEventListener("click",mouseDown,false);
    window.addEventListener("resize",resize,false);
}


function toggleFullscreen()
{
    var fullscreen_element = document.fullscreenElement || 
    document.mozFullScreenElement ||
    document.webkitFullscreenElement ||
    document.msFullScreenElement ||
    null;

    if(fullscreen_element == null)      //If Not Fullscreen
    {
        if(canvas.requestFullscreen)        //Function Pointer
        {
            canvas.requestFullscreen();
        }
        else if(canvas.mozRequestFullScreen)
        {
            canvas.mozRequestFullScreen();
        }
        else if(canvas.webkitRequestFullscreen)
        {
            canvas.webkitRequestFullscreen();
        }
        else if(canvas.msRequestFullScreen)
        {
            canvas.msRequestFullScreen();
        }
        
        bFullscreen = true;
    }
    else
    {
        if(document.exitFullscreen)
        {
            document.exitFullscreen();
        }
        else if(document.mozExitFullScreen)
        {
            document.mozExitFullScreen();
        }
        else if(document.webkitFullscreen)
        {
            document.webkitExitFullscreen();
        }
        else if(document.msExitFullScreen)
        {
            document.msExitFullScreen();
        }
        bFullscreen = false;
    }

}


function initialize()
{
    //code

    //Obtain webgl2 Context from Canvas
    gl=canvas.getContext("webgl2");
    if(!gl)
        console.log("Obtaining WebGL2.0 Context Failed..");
    else
        console.log("Obtaining WebGL2.0 Context Succeeded..");

    //Set viewport width and height of context
    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;

    //Vertex Shader
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
    "diffused_light_color = u_ld * u_kd * max(dot(lightDirection, transformedNormals),0.0);" +
    "}" +
    "gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" +
    "}";

    var vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);

    gl.shaderSource(vertexShaderObject,vertexShaderSourceCode);

    gl.compileShader(vertexShaderObject);

    if(gl.getShaderParameter(vertexShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(vertexShaderObject);
    
        if(error.length>0)
        {
            alert("Vertex Shader Compilation Error : \n" + error);
            uninitialize();
        }
    }

    //Fragment Shader
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

    gl.shaderSource(fragmentShaderObject,fragmentShaderSourceCode);

    gl.compileShader(fragmentShaderObject);

    if(gl.getShaderParameter(fragmentShaderObject,gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(fragmentShaderObject);

        if(error.length>0)
        {
            alert("Fragment Shader Compilation Error : \n" + error);
            uninitialize();
        }
    }

    //Shader Program Object
    shaderProgramObject = gl.createProgram();

    gl.attachShader(shaderProgramObject,vertexShaderObject);
    gl.attachShader(shaderProgramObject,fragmentShaderObject);

    //Prelinking

    //Binding attribute location
    gl.bindAttribLocation(shaderProgramObject,webGLMacros.AAN_ATTRIBUTE_POSITION,"a_position");
    gl.bindAttribLocation(shaderProgramObject,webGLMacros.AAN_ATTRIBUTE_NORMAL,"a_normal");

    // Shader Program Linking
    gl.linkProgram(shaderProgramObject);

    //Link Error Check
    if(gl.getProgramParameter(shaderProgramObject,gl.LINK_STATUS) == false)
    {
        var log = gl.getProgramInfoLog(shaderProgramObject);

        if(error.length>0)
        {
            alert("Shader Program Link Error : \n" + error);
            uninitialize();
        }
    }

    //Post Linking (Get Uniform Location)

    modelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_modelMatrix");
    viewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_viewMatrix");
    projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projectionMatrix");


    ldUniform = gl.getUniformLocation(shaderProgramObject, "u_ld");
    kdUniform = gl.getUniformLocation(shaderProgramObject,"u_kd");
    lightPositionUniform = gl.getUniformLocation(shaderProgramObject, "u_lightPosition");
    lightingEnabledUniform = gl.getUniformLocation(shaderProgramObject, "u_lightingEnabled");

    //Declaration & Initialization of geometry

    var cubePosition = new Float32Array(
        [
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
			-1.0, -1.0, 1.0,
        ]
    );

    var cubeNormals = new Float32Array(
        [
            0.0, 1.0, 0.0,  // top-right of top
			0.0, 1.0, 0.0, // top-left of top
			0.0, 1.0, 0.0, // bottom-left of top
			0.0, 1.0, 0.0,  // bottom-right of top

			// bottom surface
			0.0, -1.0, 0.0,  // top-right of bottom
			0.0, -1.0, 0.0,  // top-left of bottom
			0.0, -1.0, 0.0,  // bottom-left of bottom
			0.0, -1.0, 0.0,   // bottom-right of bottom

			// front surface
			0.0, 0.0, 1.0,  // top-right of front
			0.0, 0.0, 1.0, // top-left of front
			0.0, 0.0, 1.0, // bottom-left of front
			0.0, 0.0, 1.0,  // bottom-right of front

			// back surface
			0.0, 0.0, -1.0,  // top-right of back
			0.0, 0.0, -1.0, // top-left of back
			0.0, 0.0, -1.0, // bottom-left of back
			0.0, 0.0, -1.0,  // bottom-right of back

			// right surface
			1.0, 0.0, 0.0,  // top-right of right
			1.0, 0.0, 0.0,  // top-left of right
			1.0, 0.0, 0.0,  // bottom-left of right
			1.0, 0.0, 0.0  // bottom-right of right

			// left surface
			-1.0, 0.0, 0.0, // top-right of left
			-1.0, 0.0, 0.0, // top-left of left
			-1.0, 0.0, 0.0, // bottom-left of left
			-1.0, 0.0, 0.0, // bottom-right of left
        ]
    );
    

    //vao_cube and vbo related code

    //vao_cube
    vao_cube = gl.createVertexArray();
    gl.bindVertexArray(vao_cube);
    
    //vbo
    vbo_cube_position = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,vbo_cube_position);
    gl.bufferData(gl.ARRAY_BUFFER,cubePosition,gl.STATIC_DRAW);
    gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER,null);        //UNBIND


    vbo_cube_normal = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,vbo_cube_normal);
    gl.bufferData(gl.ARRAY_BUFFER,cubeNormals,gl.STATIC_DRAW);
    gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_NORMAL);
    gl.bindBuffer(gl.ARRAY_BUFFER,null);        //UNBIND

    gl.bindVertexArray(null);                   //UNBIND


    //Depth Related code
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    //Clear screen with black color
    gl.clearColor(0.0,0.0,0.0,1.0);

    perspectiveProjectionMatrix = mat4.create();       //Taken from gl-matrix-min.js 

}

function resize()
{
    //code
    if(bFullscreen == true)
    {
        canvas.width=window.innerWidth;
        canvas.height=window.innerHeight;
    }
    else
    {
        canvas.width=canvas_original_width;
        canvas.height=canvas_original_height;
    }

    if(canvas.height == 0)
    {
        canvas.height = 1;          //To avoid Divided by 0 illegal statement for future code
    }

    gl.viewport(0,0,canvas.width,canvas.height);

   mat4.perspective(perspectiveProjectionMatrix,45.0,parseFloat(canvas.width)/parseFloat(canvas.height),1.0,100.0);
}

function display()
{
    //code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    //Use Shader Program Object
    gl.useProgram(shaderProgramObject);

    //Transformations
    var translationMatrix = mat4.create();
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var rotationMatrix = mat4.create();
    
    var angleInRadian = degreeToRadian(angleCube);


    mat4.translate(translationMatrix, translationMatrix, [0.0, 0.0, -4.0]);
    

    mat4.rotateX(rotationMatrix,rotationMatrix,angleInRadian);
    mat4.rotateY(rotationMatrix,rotationMatrix,angleInRadian);
    mat4.rotateZ(rotationMatrix,rotationMatrix,angleInRadian);


    mat4.multiply(modelMatrix,translationMatrix,rotationMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform,false, modelMatrix);
    
    gl.uniformMatrix4fv(viewMatrixUniform,false, viewMatrix);

    gl.uniformMatrix4fv(projectionMatrixUniform,false, perspectiveProjectionMatrix);

    if(bLight == true)
    {
        gl.uniform1i(lightingEnabledUniform, 1);
        gl.uniform3fv(ldUniform,lightDiffuse);
        gl.uniform3fv(kdUniform,materialDiffuse);
        gl.uniform4fv(lightPositionUniform,lightPosition);
    }
    else
    {
        gl.uniform1i(lightingEnabledUniform, 0);
    }

    gl.bindVertexArray(vao_cube);

    //Here is the drawing code

    gl.drawArrays(gl.TRIANGLE_FAN,0,4);
    gl.drawArrays(gl.TRIANGLE_FAN,4,4);
    gl.drawArrays(gl.TRIANGLE_FAN,8,4);
    gl.drawArrays(gl.TRIANGLE_FAN,12,4);
    gl.drawArrays(gl.TRIANGLE_FAN,16,4);
    gl.drawArrays(gl.TRIANGLE_FAN,20,4);

    gl.bindVertexArray(null);
    
    gl.useProgram(null);

    //FUNCTION CALL for update() 
    update();

    //Double Buffering Emulation Call
    requestAnimationFrame(display,canvas);
}

function degreeToRadian(angleInDegree)
{
    return (angleInDegree * Math.PI/180);
}

function update()
{
    //code
    angleCube = angleCube + 1.0;
    if(angleCube >= 360.0)
    {
        angleCube = angleCube - 360.0;
    }
}

//Keyboard Event Listener
function keyDown(event)
{
   //code
   switch(event.keyCode)
   {
    case 27:                
        uninitialize();
        window.close();     //Not all browsers will follow this
        break;

    case 70:
        toggleFullscreen();
        break;

    case 76:        //L
    bLight = !bLight;
        break;  
   }



}

//Mouse Event Listener
function mouseDown()
{
    //code
    
}


function uninitialize()
{
    //code
    if(vao_cube)
    {
        gl.deleteVertexArray(vao_cube);
        vao_cube = null;
    }

    if(vbo_cube_position)
    {
        gl.deleteBuffer(vbo_cube_position);
        vbo_cube_position = null;
    }

    if(vbo_cube_normal)
    {
        gl.deleteBuffer(vbo_cube_normal);
        vbo_cube_normal = null;
    }

    //Shader uninitialization
    if(shaderProgramObject)
    {
        gl.useProgram(shaderProgramObject);

        var shader_Objects = gl.getAttachedShaders(shaderProgramObject);
    }

    for(let i = 0; i < shader_Objects.length; i++)
    {
        gl.detachShader(shaderProgramObject,shader_Objects[i]);
        gl.deleteShader(shader_Objects[i]);
        shader_Objects[i] = 0;
    }

    gl.useProgram(null);
    gl.deleteProgram(shaderProgramObject);
    shaderProgramObject = null;
}
