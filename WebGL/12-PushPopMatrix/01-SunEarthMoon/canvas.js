// global variables
var canvas=null;
var gl=null; // webgl context
var bFullscreen=false;
var canvas_original_width;
var canvas_original_height;

const webGLMacros = 
// when whole 'webGLMacros' is 'const', all inside it are automatically 'const'
{
	AAN_ATTRIBUTE_VERTEX:0,
	AAN_ATTRIBUTE_COLOR:1,
	AAN_ATTRIBUTE_NORMAL:2,
	AAN_ATTRIBUTE_TEXTURE0:3,
};

var vertexShaderObject;
var fragmentShaderObject;
var shaderProgramObject;

var mvpUniform;

var angleDay;
var angleYear;

var year = 0;
var day = 0;

var sphere=null;

var perspectiveProjectionMatrix;

// To start animation : To have requestAnimationFrame() to be called "cross-browser" compatible
var requestAnimationFrame = 
	window.requestAnimationFrame || 
	window.webkitRequestAnimationFrame || 
	window.mozRequestAnimationFrame || 
	window.oRequestAnimationFrame || 
	window.msRequestAnimationFrame;

// To stop animation : To have cancelAnimationFrame() to be called "cross-browser" compatible
var cancelAnimationFrame = 
	window.cancelAnimationFrame || 
	window.webkitCancelRequestAnimationFrame || 
	window.webkitCancelAnimationFrame || 
	window.mozCancelRequestAnimationFrame || 
	window.mozCancelAnimationFrame || 
	window.oCancelRequestAnimationFrame || 
	window.oCancelAnimationFrame || 
	window.msCancelRequestAnimationFrame || 
	window.msCancelAnimationFrame;

// onload function
function main()
{
	// get <canvas> element
	canvas = document.getElementById("AAN");
	if(!canvas)
		console.log("Obtaining Canvas Failed\n");
	else
		console.log("Obtaining Canvas Succeeded\n");
	canvas_original_width=canvas.width;
	canvas_original_height=canvas.height;
	
	// register keyboard's keydown event handler
	window.addEventListener("keydown", keyDown, false);
	window.addEventListener("click", mouseDown, false);
	window.addEventListener("resize", resize, false);
	
	// initialize WebGL
	init();
	
	// start drawing here as warming-up
	resize();

	draw();
}

function toggleFullScreen()
{
	// code
	var fullscreen_element = 
		document.fullscreenElement || 
		document.webkitFullscreenElement || 
		document.mozFullScreenElement || 
		document.msFullscreenElement || 
		null;
		
	// if not fullscreen
	if(fullscreen_element == null)
	{
		if(canvas.requestFullscreen)
			canvas.requestFullscreen();
		else if(canvas.mozRequestFullScreen)
			canvas.mozRequestFullScreen();
		else if(canvas.webkitRequestFullscreen)
			canvas.webkitRequestFullscreen();
		else if(canvas.msRequestFullscreen)
			canvas.msRequestFullscreen();
		bFullscreen=true;
	}
	else // if already fullscreen
	{
		if(document.exitFullscreen)
			document.exitFullscreen();
		else if(document.mozCancelFullScreen)
			document.mozCancelFullScreen();
		else if(document.webkitExitFullscreen)
			document.webkitExitFullscreen();
		else if(document.msExitFullscreen)
			document.msExitFullscreen();
		bFullscreen=false;
	}
}

function init()
{
	// code
	// get WebGL 2.0 context
	gl=canvas.getContext("webgl2");
	if(gl==null) // failed to get context
	{
		console.log("Failed to get the rendering context for WebGL");
		return;
	}
	gl.viewportwidth = canvas.width;
	gl.viewportHeight = canvas.height;
	
	// vertex shader
	var vertexShaderSourceCode =
		"#version 300 es"+
		"\n"+
		"in vec4 a_position;" +
		"in vec4 a_color;" +
		"uniform mat4 u_mvpMatrix;" +
		"out vec4 a_color_out;" +
		"void main(void)" +
		"{" +
		"gl_Position = u_mvpMatrix * a_position;" +
		"a_color_out = a_color;" +
		"}";

	vertexShaderObject=gl.createShader(gl.VERTEX_SHADER);
	gl.shaderSource(vertexShaderObject,vertexShaderSourceCode);
	gl.compileShader(vertexShaderObject);
	if(gl.getShaderParameter(vertexShaderObject,gl.COMPILE_STATUS)==false)
	{
		var error=gl.getShaderInfoLog(vertexShaderObject);
		if(error.length > 0)
		{
			alert(error);
			uninitialize( );
		}
	}
	
	// fragment shader
	var fragmentShaderSourceCode =
		"#version 300 es"+
		"\n"+
		"precision highp float;"+
		"in vec4 a_color_out;" +
		"out vec4 FragColor;" +
		"void main(void)" +
		"{" +
		"FragColor =a_color_out;" +
		"}";

	fragmentShaderObject=gl.createShader(gl.FRAGMENT_SHADER);
	gl.shaderSource(fragmentShaderObject,fragmentShaderSourceCode);
	gl.compileShader(fragmentShaderObject);
	if(gl.getShaderParameter(fragmentShaderObject,gl.COMPILE_STATUS)==false)
	{
		var error=gl.getShaderInfoLog(fragmentShaderObject);
		if(error.length > 0)
		{
			alert(error);
			uninitialize();
		}
	}
	
	// shader program
	shaderProgramObject=gl.createProgram();
	gl.attachShader(shaderProgramObject,vertexShaderObject);
	gl.attachShader(shaderProgramObject,fragmentShaderObject);
	
	// pre-link binding of shader program object with vertex shader attributes
	gl.bindAttribLocation(shaderProgramObject,webGLMacros.AAN_ATTRIBUTE_VERTEX,"a_position");
	gl.bindAttribLocation(shaderProgramObject,webGLMacros.AAN_ATTRIBUTE_COLOR, "a_color");
	
	// linking
	gl.linkProgram(shaderProgramObject);
	if (!gl.getProgramParameter(shaderProgramObject, gl.LINK_STATUS))
	{
		var error=gl.getProgramInfoLog(shaderProgramObject); 
		if(error.length > 0) 
		{
			alert(error); 
			uninitialize();
		}
	}
	
	// get MVP uniform location
	mvpUniform=gl.getUniformLocation(shaderProgramObject,"u_mvpMatrix");
	
	sphere=new Mesh();
	makeSphere(sphere,3.0,60,60);
	
	// set clear color
	gl.clearColor(0.0, 0.0, 0.0, 1.0);

	// Depth test will always be enabled
	gl.enable(gl.DEPTH_TEST);
	
	// depth test to do
	gl.depthFunc(gl.LEQUAL);
	
	// initialize projection matrix
	perspectiveProjectionMatrix=mat4.create();
}

function resize()
{
	// code
	if(bFullscreen==true)
	{
		canvas.width=window.innerWidth;
		canvas.height=window.innerHeight;
	}
	else
	{
		canvas.width=canvas_original_width;
		canvas.height=canvas_original_height;
	}
	
	// set the viewport to match
	gl.viewport(0, 0, canvas.width, canvas.height);
	
	mat4.perspective(perspectiveProjectionMatrix,45.0,parseFloat(canvas.width)/ parseFloat(canvas.height),1.0, 100.0);
}

function draw()
{
	// code
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	
	gl.useProgram(shaderProgramObject);
	
	var modelViewMatrix=mat4.create(); 
	var modelViewProjectionMatrix = mat4.create();
	var translationMatrix = mat4.create();
	var rotationMatrix = mat4.create();
	var scaleMatrix = mat4.create();
	
	mat4.identity(modelViewMatrix);
	mat4.identity(modelViewProjectionMatrix);
	mat4.identity(translationMatrix);
	mat4.identity(rotationMatrix);
	mat4.identity(scaleMatrix);
	
	mat4.translate(modelViewMatrix, modelViewMatrix, [0.0,0.0,-30.0]); 
	
	mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix); 
	
	gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

	gl.vertexAttrib3f(webGLMacros.AAN_ATTRIBUTE_COLOR, 1.0, 1.0, 0.0);
	
	sphere.draw();
	
	mat4.rotateY(modelViewMatrix ,modelViewMatrix, degToRad(year));
	mat4.translate(modelViewMatrix, modelViewMatrix, [10.0,0.0,0.0]);
	mat4.rotateY(modelViewMatrix ,modelViewMatrix, degToRad(day));
	mat4.scale(modelViewMatrix ,modelViewMatrix, [0.7,0.7,0.7]);
	
	mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
	
	gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);
	
	gl.vertexAttrib3f(webGLMacros.AAN_ATTRIBUTE_COLOR, 0.4, 0.9, 1.0);
	sphere.draw();
	
	mat4.rotateY(modelViewMatrix ,modelViewMatrix, degToRad(year));
	mat4.translate(modelViewMatrix, modelViewMatrix, [6.0,0.0,0.0]);
	mat4.rotateY(modelViewMatrix ,modelViewMatrix, degToRad(day));
	mat4.scale(modelViewMatrix ,modelViewMatrix, [0.4,0.4,0.4]);
	
	mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
	
	gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);
	
	gl.vertexAttrib3f(webGLMacros.AAN_ATTRIBUTE_COLOR, 1.0, 1.0, 1.0);
	sphere.draw();
	
	gl.useProgram(null);
	
	// animation loop
	requestAnimationFrame(draw, canvas);
}

function uninitialize()
{
	// code
	if(sphere)
	{
		sphere.deallocate();
		sphere=null;
	}
	
	if(shaderProgramObject)
	{
		if(fragmentShaderObject)
		{
			gl.detachShader(shaderProgramObject,fragmentShaderObject);
			gl.deleteShader(fragmentShaderObject);
			fragmentShaderObject=null;
		}
		
		if(vertexShaderObject)
		{
			gl.detachShader(shaderProgramObject,vertexShaderObject);
			gl.deleteShader(vertexShaderObject);
			vertexShaderObject=null;
		}
		
		gl.deleteProgram(shaderProgramObject);
		shaderProgramObject=null;
	}
}

function keyDown(event)
{
	// code
	switch(event.keyCode)
	{
		case 27: 
			uninitialize();
			window.close(); 
			break;
		case 70: // for 'F' or ' f '
			toggleFullScreen();
			break;
		
		case 68: //DAY
			if(day <= 360)
			{
				day+=6;
			}
			if(day > 360)
			{
				day = 0;
			}
			break;

		case 89: //YEAR
			if(year <= 360)
			{
				year+=6;
			}
			if(year > 360)
			{
				year = 0;
			}
			break;	
		}
}

function mouseDown()
{
	// code
}

function degToRad(degrees)
{
	// code
	return(degrees * Math.PI / 180);
}