// global variables
var canvas=null;
var gl=null; 
var bFullscreen=false;
var canvas_original_width;
var canvas_original_height;

const WebGLMacros = 
{
	AAN_ATTRIBUTE_POSITION:0,
	AAN_ATTRIBUTE_COLOR:1,
	AAN_ATTRIBUTE_NORMAL:2,
	AAN_ATTRIBUTE_TEXTURE0:3,
};

var vertexShaderObject;
var fragmentShaderObject;
var shaderProgramObject;

var vao_cube;
var vbo_cube;

var lightAmbient=[0.0,0.0,0.0];
var lightDiffuse=[1.0,1.0,1.0];
var lightSpecular=[1.0,1.0,1.0];
var lightPosition=[100.0,100.0,100.0,1.0];

var materialAmbient= [0.0,0.0,0.0];
var materialDiffuse= [1.0,1.0,1.0];
var materialSpecular= [1.0,1.0,1.0];
var materialShininess= 50.0; 

var modelMatrixUniform;
var viewMatrixUniform;
var projectionMatrixUniform;

var laUniform = 0;
var ldUniform = 0;
var lsUniform = 0;

var kaUniform = 0;
var kdUniform = 0;
var ksUniform = 0;

var materialShininessUniform = 0;

var lightPositionUniform = 0;

var lightingEnabledUniform = 0;

var angleCube = 0.0;

var textureSamplerUniform;

var texture_marble=0;

var bLight = false;

var perspectiveProjectionMatrix;

var requestAnimationFrame = 
	window.requestAnimationFrame || 
	window.webkitRequestAnimationFrame || 
	window.mozRequestAnimationFrame || 
	window.oRequestAnimationFrame || 
	window.msRequestAnimationFrame;

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

function main()
{
	canvas = document.getElementById("AAN");
	if(!canvas)
		console.log("Obtaining Canvas Failed\n");
	else
		console.log("Obtaining Canvas Succeeded\n");

	//backup canvas dimension	
	canvas_original_width=canvas.width;
	canvas_original_height=canvas.height;
	
	// initialize
	initialize();
	
	//warmup resize
	resize();

	//display
	draw();
	
	window.addEventListener("keydown", keyDown, false);
	window.addEventListener("click", mouseDown, false);
	window.addEventListener("resize", resize, false);
	
}

//Event Listener

function keyDown(event)
{
	// code
	switch(event.keyCode)
	{
		case 27: 
			uninitialize();
			window.close(); 
			break;

		case 70: //'F' or ' f '
			toggleFullScreen();
			break;

		case 76:
		case 108:
			if(bLight == false)
			{
				bLight = true;
			}
			else
			{
				bLight = false;
			}
			break;

	}
}

//mouse event
function mouseDown()
{
	// code
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

function initialize()
{
	// code
	// Obtain Context 
    gl = canvas.getContext("webgl2");
    if (!gl)
        console.log("Could Not Obtain WebGL 2.0 Context !!!\n");
    else
        console.log("WebGL 2.0 Context Obtained !!!\n");
    
    // Set Viewport Width & Height
	gl.viewportwidth = canvas.width;
	gl.viewportHeight = canvas.height;
	
	// vertex shader
	var vertexShaderSourceCode =
		"#version 300 es" +
		"\n" +
		"in vec4 a_position;" +
		"in vec4 a_color;" +
		"in vec2 a_texcoord;" +
		"in vec3 a_normal;" +
		"uniform mat4 u_modelMatrix;" +
		"uniform mat4 u_viewMatrix;" +
		"uniform mat4 u_projectionMatrix;" +
		"uniform vec4 u_lightPosition;" +
		"uniform mediump int u_lightingEnabled;" +
		"out vec3 transformedNormals;" +
		"out vec3 lightDirection;" +
		"out vec3 viewerVector;" +
		"out vec2 a_texcoord_out;" +
		"out vec4 a_color_out;" +
		"void main(void)" +
		"{" +
		"if (u_lightingEnabled == 1)" +
		"{" +
		"vec4 eyeCoordinates = u_viewMatrix * u_modelMatrix * a_position;" +
		"mat3 normalMatrix = mat3(u_viewMatrix * u_modelMatrix);" +
		"transformedNormals = normalMatrix * a_normal;" +
		"lightDirection = vec3(u_lightPosition) - eyeCoordinates.xyz;" +
		"viewerVector = -eyeCoordinates.xyz;" +
		"}" +
		"gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" +
		"a_texcoord_out = a_texcoord;" +
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
		"#version 300 es" +
		"\n" +
		"precision highp float;" +
		"in vec3 transformedNormals;" +
		"in vec3 lightDirection;" +
		"in vec3 viewerVector;" +
		"in vec2 a_texcoord_out;" +
		"in vec4 a_color_out;" +
		"uniform highp sampler2D u_textureSampler;" +
		"uniform vec3 u_la;" +
		"uniform vec3 u_ld;" +
		"uniform vec3 u_ls;" +
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
		"vec3 ambient = u_la * u_ka;" +
		"vec3 normalized_tranformed_normals = normalize(transformedNormals);" +
		"vec3 normalized_light_direction = normalize(lightDirection);" +
		"vec3 diffuse = u_ld * u_kd * max(dot(normalized_light_direction, normalized_tranformed_normals), 0.0);" +
		"vec3 reflectionVector = reflect(-normalized_light_direction, normalized_tranformed_normals);" +
		"vec3 normalized_viewer_vector = normalize(viewerVector);" +
		"vec3 specular = u_ls * u_ks * pow(max(dot(reflectionVector, normalized_viewer_vector), 0.0), u_materialShininess);" +
		"phong_ads_light = ambient + diffuse + specular;" +
		"}" +
		"else" +
		"{" +
		"phong_ads_light = vec3(1.0, 1.0, 1.0);" +
		"}" +
		"FragColor = vec4(phong_ads_light * vec3(texture(u_textureSampler, a_texcoord_out)) * vec3(a_color_out), 1.0);" +
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
	shaderProgramObject = gl.createProgram();

	gl.attachShader(shaderProgramObject,vertexShaderObject);
	gl.attachShader(shaderProgramObject,fragmentShaderObject);
	
	// pre-link
	gl.bindAttribLocation(shaderProgramObject,WebGLMacros.AAN_ATTRIBUTE_POSITION,"a_position");
	gl.bindAttribLocation(shaderProgramObject,WebGLMacros.AAN_ATTRIBUTE_COLOR, "a_color");
	gl.bindAttribLocation(shaderProgramObject,WebGLMacros.AAN_ATTRIBUTE_NORMAL,"a_normal");
	gl.bindAttribLocation(shaderProgramObject,WebGLMacros.AAN_ATTRIBUTE_TEXTURE0, "a_texcoord");
	
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
	
	// Load Texture
	texture_marble = gl.createTexture();
	texture_marble.image = new Image();
	texture_marble.image.src = "marble.png";
	texture_marble.image.onload = function ()
	{
		gl.bindTexture(gl.TEXTURE_2D, texture_marble);
		gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
		gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, texture_marble.image);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
		gl.bindTexture(gl.TEXTURE_2D, null);
	}
	
	// Post Linking
	modelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_modelMatrix");
	viewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_viewMatrix");
	projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projectionMatrix");
	
	laUniform = gl.getUniformLocation(shaderProgramObject, "u_la");
	ldUniform = gl.getUniformLocation(shaderProgramObject, "u_ld");
	lsUniform = gl.getUniformLocation(shaderProgramObject, "u_ls");
	lightPositionUniform = gl.getUniformLocation(shaderProgramObject, "u_light_position");

	kaUniform = gl.getUniformLocation(shaderProgramObject, "u_ka");
	kdUniform = gl.getUniformLocation(shaderProgramObject, "u_kd");
	ksUniform = gl.getUniformLocation(shaderProgramObject, "u_ks");
	materialShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_materialShininess");
	
	
	lightingEnabledUniform = gl.getUniformLocation(shaderProgramObject, "u_lightingEnabled");
	
	textureSamplerUniform = gl.getUniformLocation(shaderProgramObject,"u_textureSampler");
	

	//Vertex Data Array
	var cubePCNT = new Float32Array([				//Position, colors, normals, texcoords
		// Top face
		1.0, 1.0, -1.0, 	1.0, 0.0, 0.0, 		0.0, 1.0, 0.0, 		0.0, 1.0,
		-1.0, 1.0, -1.0, 	1.0, 0.0, 0.0, 		0.0, 1.0, 0.0, 		0.0, 0.0,
		-1.0, 1.0, 1.0, 	1.0, 0.0, 0.0, 		0.0, 1.0, 0.0, 		1.0, 0.0,
		1.0, 1.0, 1.0, 		1.0, 0.0, 0.0, 		0.0, 1.0, 0.0, 		1.0, 1.0,

		// Bottom face
		1.0, -1.0, -1.0,	0.0, 1.0, 0.0, 		0.0,-1.0, 0.0, 		1.0, 1.0,
		-1.0, -1.0, -1.0, 	0.0, 1.0, 0.0, 		0.0,-1.0, 0.0, 		0.0, 1.0,
		-1.0, -1.0, 1.0, 	0.0, 1.0, 0.0, 		0.0, -1.0, 0.0, 	0.0, 0.0,
		1.0, -1.0, 1.0, 	0.0, 1.0, 0.0, 		0.0, -1.0, 0.0, 	1.0, 0.0,

		// Front face
		1.0, 1.0, 1.0, 		0.0, 0.0, 1.0, 		0.0, 0.0, 1.0, 		0.0, 0.0,
		-1.0, 1.0, 1.0, 	0.0, 0.0, 1.0, 		0.0, 0.0, 1.0, 		1.0, 0.0,
		-1.0, -1.0, 1.0,	 0.0, 0.0, 1.0, 	0.0, 0.0, 1.0, 		1.0, 1.0,
		1.0, -1.0, 1.0,		 0.0, 0.0, 1.0, 	0.0, 0.0, 1.0, 		0.0, 1.0,

		// Back face
		1.0, 1.0, -1.0, 	0.0, 1.0, 1.0, 		0.0, 0.0, -1.0, 	1.0, 0.0,
		-1.0, 1.0, -1.0, 	0.0, 1.0, 1.0, 		0.0, 0.0, -1.0, 	1.0, 1.0,
		-1.0, -1.0, -1.0, 	0.0, 1.0, 1.0, 		0.0, 0.0, -1.0, 	0.0, 1.0,
		1.0, -1.0, -1.0, 	0.0, 1.0, 1.0, 		0.0, 0.0, -1.0, 	0.0, 0.0,

		// Right face
		1.0, 1.0, -1.0, 	1.0, 0.0, 1.0, 		1.0, 0.0, 0.0, 		1.0, 0.0,
		1.0, 1.0, 1.0, 		1.0, 0.0, 1.0, 		1.0, 0.0, 0.0, 		1.0, 1.0,
		1.0, -1.0, 1.0, 	1.0, 0.0, 1.0, 		1.0, 0.0, 0.0,		0.0, 1.0,
		1.0, -1.0, -1.0, 	1.0, 0.0, 1.0, 		1.0, 0.0, 0.0,		0.0, 0.0,

		// Left face
		-1.0, 1.0, 1.0, 	1.0, 1.0, 0.0, 		-1.0, 0.0, 0.0, 	0.0, 0.0,
		-1.0, 1.0, -1.0, 	1.0, 1.0, 0.0, 		-1.0, 0.0, 0.0, 	1.0, 0.0,
		-1.0, -1.0, -1.0, 	1.0, 1.0, 0.0, 		-1.0, 0.0, 0.0, 	1.0, 1.0,
		-1.0, -1.0, 1.0, 	1.0, 1.0, 0.0, 		-1.0, 0.0, 0.0, 	0.0, 1.0
	]);
	
	
	vao_cube = gl.createVertexArray();
	gl.bindVertexArray(vao_cube);
	
	vbo_cube = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_cube);
	
	gl.bufferData(gl.ARRAY_BUFFER, cubePCNT, gl.STATIC_DRAW);
	
	//Position 
	gl.vertexAttribPointer(WebGLMacros.AAN_ATTRIBUTE_POSITION,3, gl.FLOAT, false, 11 * 4, 0 * 4);
	gl.enableVertexAttribArray(WebGLMacros.AAN_ATTRIBUTE_POSITION);
	
	//Color 
	gl.vertexAttribPointer(WebGLMacros.AAN_ATTRIBUTE_COLOR,3,gl.FLOAT, false, 11 * 4, 3 * 4);
	gl.enableVertexAttribArray(WebGLMacros.AAN_ATTRIBUTE_COLOR);

	//Normal 
	gl.vertexAttribPointer(WebGLMacros.AAN_ATTRIBUTE_NORMAL,3,gl.FLOAT, false, 11 * 4, 6 * 4);
	gl.enableVertexAttribArray(WebGLMacros.AAN_ATTRIBUTE_NORMAL);

	//Texture 
	gl.vertexAttribPointer(WebGLMacros.AAN_ATTRIBUTE_TEXTURE0,2,gl.FLOAT, false, 11 * 4, 9 * 4);
	gl.enableVertexAttribArray(WebGLMacros.AAN_ATTRIBUTE_TEXTURE0);
	
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	gl.bindVertexArray(null);
	
	//clear color
	gl.clearColor(0.0, 0.0, 0.0, 1.0);

	gl.clearDepth(1.0);
	gl.enable(gl.DEPTH_TEST);
	gl.depthFunc(gl.LEQUAL);
	
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

	winWidth = canvas.width;
	winHeight = canvas.height;
	
	mat4.perspective(perspectiveProjectionMatrix, 45.0, parseFloat(canvas.width)/ parseFloat(canvas.height), 0.1, 100.0);
	
	gl.viewport(0, 0, canvas.width, canvas.height);
	
	
}

function draw()
{
	// code
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	
	gl.useProgram(shaderProgramObject);
	
	var modelMatrix=mat4.create();
	var viewMatrix=mat4.create();
	var translationMatrix = mat4.create();
	var scaleMatrix = mat4.create();
	var rotationMatrix = mat4.create();
	
	mat4.translate(translationMatrix, translationMatrix, [0.0 ,0.0 ,-6.0]);
	mat4.scale(scaleMatrix,scaleMatrix,[0.75,0.75,0.75]);
	
	mat4.multiply(modelMatrix,scaleMatrix,translationMatrix);

	mat4.rotateX(modelMatrix ,modelMatrix, degToRad(angleCube));
	mat4.rotateY(modelMatrix ,modelMatrix, degToRad(angleCube));
	mat4.rotateZ(modelMatrix ,modelMatrix, degToRad(angleCube));

	
	mat4.multiply(modelMatrix,modelMatrix,rotationMatrix);
	
	gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
	gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
	gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);
	
	
	// bind with texture
	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, texture_marble);
	gl.uniform1i(textureSamplerUniform, 0);

	if(bLight ==  true)
	{
		gl.uniform1i(lightingEnabledUniform, 1);
		gl.uniform3fv(laUniform, lightAmbient);
		gl.uniform3fv(ldUniform, lightDiffuse);
		gl.uniform3fv(lsUniform, lightSpecular);
		gl.uniform4fv(lightPositionUniform, lightPosition);

		gl.uniform3fv(kaUniform, materialAmbient);
		gl.uniform3fv(kdUniform, materialDiffuse);
		gl.uniform3fv(ksUniform, materialSpecular);
		gl.uniform1f(materialShininessUniform, materialShininess);
	}
	else
	{
		gl.uniform1i(lightingEnabledUniform, 0);
	}
	
		
	gl.bindVertexArray(vao_cube);
	
	gl.drawArrays(gl.TRIANGLE_FAN,0,4);
	gl.drawArrays(gl.TRIANGLE_FAN,4,4);
	gl.drawArrays(gl.TRIANGLE_FAN,8,4);
	gl.drawArrays(gl.TRIANGLE_FAN,12,4);
	gl.drawArrays(gl.TRIANGLE_FAN,16,4);
	gl.drawArrays(gl.TRIANGLE_FAN,20,4);

	gl.bindTexture(gl.TEXTURE_2D, null);
	
	gl.bindVertexArray(null); 
	
	gl.useProgram(null);
	
	update();

	// animation loop
	requestAnimationFrame(draw, canvas);
}

function update()
{
	angleCube = angleCube+1.0;
	
		if(angleCube >= 360.0)
			angleCube = angleCube - 360.0;
}

function degToRad(degrees)
{
	// code
	return(degrees * Math.PI / 180);
}

function uninitialize()
{
	// code
	if(texture_marble)
	{
		gl.deleteTexture(texture_marble);
		texture_marble=0;
	}
	
	if(vao_cube)
	{
		gl.deleteVertexArray(vao_cube);
		vao_cube=null;
	}
	
	if(vbo_cube)
	{
		gl.deleteBuffer(vbo_cube);
		vbo_cube=null;
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
