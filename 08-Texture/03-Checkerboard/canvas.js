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

var vao_quad;
var vbo_position;
var vbo_texture;
var mvpMatrixUniform;

var perspectiveProjectionMatrix;

var smiley_texture = 0;

//White
var checkImageHeight = 64;
var checkImageWidth = 64;
var coloredBoardTexture=new  Uint8Array(checkImageHeight*checkImageWidth*4);
var colored_board_texture = 0;

var textureSamplerUniform;


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
    "in vec2 a_texcoord;" +
    "uniform mat4 u_mvpMatrix;" +
    "out vec2 a_texcoord_out;" +
    "void main(void)" +
    "{" +
    "gl_Position = u_mvpMatrix * a_position;" +
    "a_texcoord_out = a_texcoord;" +
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
    "in vec2 a_texcoord_out;" +
    "uniform sampler2D u_texturesampler;" +
    "out vec4 FragColor;" +
    "void main(void)" +
    "{" +
    "FragColor = texture(u_texturesampler,a_texcoord_out);" +
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
    gl.bindAttribLocation(shaderProgramObject,webGLMacros.AAN_ATTRIBUTE_TEXTURE0,"a_texcoord");
    

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

    GenerateTexture();      //Generate checkerboard texture

    mvpMatrixUniform = gl.getUniformLocation(shaderProgramObject,"u_mvpMatrix");

    textureSamplerUniform = gl.getUniformLocation(shaderProgramObject,"u_texturesampler");

    //Declaration & Initialization of geometry
    var checkerboard_texcoords = new Float32Array([
        0.0, 1.0,
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0
    ]); 

    //vao and vbo related code

    //vao
    vao_quad=gl.createVertexArray();
	gl.bindVertexArray(vao_quad);
	//vbo for positions
	vbo_position =  gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position);
	gl.bufferData(gl.ARRAY_BUFFER, 8*4, gl.DYNAMIC_DRAW);
	gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_VERTEX, 3, gl.FLOAT,false, 0, 0);
	gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_VERTEX);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
    
    //vbo for texture
    vbo_texcoord = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,vbo_texcoord);
    gl.bufferData(gl.ARRAY_BUFFER,checkerboard_texcoords,gl.STATIC_DRAW);
    gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_TEXTURE0, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_TEXTURE0);
    gl.bindBuffer(gl.ARRAY_BUFFER,null);        //UNBIND

    gl.bindVertexArray(null);                   //UNBIND


    //Depth Related code
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    //Clear screen with grey color
    gl.clearColor(0.5,0.5,0.5,1.0);

  
    perspectiveProjectionMatrix = mat4.create();       //Taken from gl-matrix-min.js 

}

function GenerateTexture()
{
    MakeCheckerBoard();
	
	gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);
	colored_board_texture = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, colored_board_texture);
	
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
	gl.texImage2D(gl.TEXTURE_2D, 0 ,gl.RGBA, 64, 64, 0,  gl.RGBA, gl.UNSIGNED_BYTE, coloredBoardTexture);


}

function MakeCheckerBoard()
{
    var i, j, index= 0, k = 0;
	
	for (i = 0; i<checkImageHeight; i++)
    {
        for (j = 0; j<checkImageWidth; j++)
        {
			
			c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0)) * 255;
			coloredBoardTexture[index]=c;
			coloredBoardTexture[index+1]=c;
			coloredBoardTexture[index+2]=c;
			coloredBoardTexture[index+3]=255;
			index=index+4;
	
        }
    }
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
    var quadVertices = new Float32Array([
        0.0, 1.0, 0.0,
        -2.0, 1.0, 0.0,
        -2.0, -1.0, 0.0,
        0.0, -1.0, 0.0																
            ]);        

    var tiltedQuadVertices = new Float32Array([
        2.41421, 1.0, -1.41421,
        1.0, 1.0, 0.0,
        1.0, -1.0,  0.0,
        2.41421, -1.0, -1.41421
    ]);				

    gl.clear(gl.COLOR_BUFFER_BIT|gl.DEPTH_BUFFER_BIT);
    gl.useProgram(shaderProgramObject);
    

    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix =  mat4.create();
    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0,0.0,-3.0]);

    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(mvpMatrixUniform,false,modelViewProjectionMatrix);

    gl.bindTexture(gl.TEXTURE_2D, colored_board_texture);

    gl.uniform1i(textureSamplerUniform, 0);

    //draw normal quad
    gl.bindVertexArray(vao_quad);
    gl.bindBuffer(gl.ARRAY_BUFFER,vbo_position);
    gl.bufferData(gl.ARRAY_BUFFER, quadVertices, gl.DYNAMIC_DRAW); 
    gl.drawArrays(gl.TRIANGLE_FAN, 0,4);
    gl.bindVertexArray(null);

    //draw tilted quad
    gl.bindVertexArray(vao_quad);
    gl.bindBuffer(gl.ARRAY_BUFFER,vbo_position);
    gl.bufferData(gl.ARRAY_BUFFER, tiltedQuadVertices, gl.DYNAMIC_DRAW); 
    gl.drawArrays(gl.TRIANGLE_FAN, 0,4);
    gl.bindVertexArray(null);
    
    gl.useProgram(null);

    //Double Buffering Emulation Call
    requestAnimationFrame(display,canvas);
}

function update()
{
    //code
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
    if(vao_quad)
    {
        gl.deleteVertexArray(vao_quad);
        vao_quad = null;
    }

    if(vbo_position)
    {
        gl.deleteBuffer(vbo_position);
        vbo_position = null;
    }

    if(vbo_texcoord)
    {
        gl.deleteBuffer(vbo_texcoord);
        vbo_texcoord = null;
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
