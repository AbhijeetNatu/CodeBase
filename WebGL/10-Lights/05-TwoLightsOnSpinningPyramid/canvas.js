
var canvas=null;
var gl=null; 
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;

const webGLMacros=
{
	AAN_ATTRIBUTE_POSITION:0,
	AAN_ATTRIBUTE_COLOR:1,
	AAN_ATTRIBUTE_NORMAL:2,
	AAN_ATTRIBUTE_TEXTURE0:3
};

var shaderProgramObject;

var vao_pyramid;
var vbo_pyramid_position;
var vbo_pyramid_normal;

var anglePyramid=0.0;

var modelMatrixUniform;
var viewMatrixUniform;
var projectionMatrixUniform;


// RELATED TO LIGHTS

var lightPosition0Uniform;
var lightPosition1Uniform;
var keyPressedUniform;
var la0Uniform;
var ld0Uniform;
var ls0Uniform;
var la1Uniform;
var ld1Uniform;
var ls1Uniform;
var kaUniform;
var kdUniform;
var ksUniform;
var materialShininessUniform;
var bLight=false;
var perspectiveProjectionMatrix;

var light0_ambient = [0.0, 0.0, 0.0];
var light0_diffuse = [1.0, 0.0, 0.0];
var light0_specular = [1.0, 0.0, 0.0];
var light0_position = [2.0, 2.0, 0.0, 1.0];
	
var light1_ambient = [0.0, 0.0, 0.0];
var light1_diffuse = [0.0, 0.0, 1.0];
var light1_specular = [0.0, 0.0, 1.0];
var light1_position = [-2.0, 2.0, 0.0, 1.0];

var material_ambient = [0.0, 0.0, 0.0];
var material_diffuse = [1.0, 1.0, 1.0];
var material_specular = [1.0, 1.0, 1.0];
	
var material_shininess = 128.0;

var requestAnimationFrame = window.requestAnimationFrame ||
							window.webkitRequestAnimationFrame||
							window.mozRequestAnimationFrame||
							window.oRequestAnimationFrame||
							window.msRequestAnimationFrame;

var cancelAnimationFrame = 
			window.cancelAnimationFrame||
			window.webkitCancelRequestAnimationFrame||
			window.webkitCancelAnimationFrame||
			window.mozCancelRequestAnimationFrame||
			window.mozCancelAnimationFrame||
			window.oCancelRequestAnimationFrame||
			window.oCancelAnimationFrame||
			window.msCancelRequestAnimationFrame||
			window.msCancelAnimationFrame;

function main()
{
	
	canvas = document.getElementById("AAN");
	if(!canvas)
		console.log("Obtaining canvas from main document failed\n");
	else
		console.log("Obtaining canvas from main document succeeded\n");
	
	console.log("Canvas width:" + canvas.width +" height:" +canvas.height);
	canvas_original_width = canvas.width;
	canvas_original_height = canvas.height;
	
	window.addEventListener("keydown", keydown, false);
	window.addEventListener("click", mouseDown, false);
	window.addEventListener("resize", resize, false);
	
	init();
	resize();
	draw();
}

function init()
{
	gl=canvas.getContext("webgl2");
	if(gl == null)
		console.log("Obtaining 2D webgl2 failed\n");
	else
		console.log("Obtaining 2D webgl2 succeeded\n");
	
	gl.viewportWidth  = canvas.width;
	gl.viewportHeight  = canvas.height;
	
	var vertexShaderSourceCode =
	    "#version 300 es" +
		"\n" +
		"in vec4 a_position;" +
		"in vec3 a_normal;" +
		"uniform mat4 u_model_matrix;" +
		"uniform mat4 u_view_matrix;" +
		"uniform mat4 u_projection_matrix;" +
		"uniform int u_lighting_enabled;" +
		"uniform vec3 u_la0;" +
		"uniform vec3 u_ld0;" +
		"uniform vec3 u_ls0;" +
		"uniform vec4 u_light0_position;" +
		"uniform vec3 u_la1;" +
		"uniform vec3 u_ld1;" +
		"uniform vec3 u_ls1;" +
		"uniform vec4 u_light1_position;" +
		"uniform vec3 u_ka;" +
		"uniform vec3 u_kd;" +
		"uniform vec3 u_ks;" +
		"uniform float u_material_shininess;" +
		"out vec3 phong_ads_color;" +
		"void main(void)" +
		"{" +
		"if(u_lighting_enabled == 1)" +
		"{"+
		"vec4 eye_coordinates = u_view_matrix* u_model_matrix * a_position;" +
		"vec3 transformed_normals = normalize(mat3(u_view_matrix*u_model_matrix) * a_normal);" +
		"vec3 light0_direction = normalize(vec3(u_light0_position) - eye_coordinates.xyz);" +
		"vec3 light1_direction = normalize(vec3(u_light1_position) - eye_coordinates.xyz);" +
		"float tn_dot_ld0 = max(dot(transformed_normals, light0_direction), 0.0);" +
		"float tn_dot_ld1 = max(dot(transformed_normals, light1_direction), 0.0);" +
		"vec3 ambient0 = u_la0 * u_ka;" +
		"vec3 ambient1 = u_la1 * u_ka;" +
		"vec3 diffuse0 = u_ld0 * u_kd * tn_dot_ld0;" +
		"vec3 diffuse1 = u_ld1 * u_kd * tn_dot_ld1;" +
		"vec3 reflection_vector0 = reflect(-light0_direction, transformed_normals);" +
		"vec3 reflection_vector1 = reflect(-light1_direction, transformed_normals);" +
		"vec3 viewer_vector = normalize(-eye_coordinates.xyz);" +
		"vec3 specular0 = u_ls0 * u_ks * pow(max(dot(reflection_vector0, viewer_vector),0.0), u_material_shininess);" +
		"vec3 specular1 = u_ls1 * u_ks * pow(max(dot(reflection_vector1, viewer_vector),0.0), u_material_shininess);" +
		"phong_ads_color = ambient0 + ambient1 + diffuse0 + diffuse1 + specular0 + specular1;" +
		"}" +
		"else" +
		"{" +
		"phong_ads_color = vec3(1.0, 1.0, 1.0);" +
		"}"+
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * a_position;" +
		"}";

	vertexShaderObject=gl.createShader(gl.VERTEX_SHADER);

	gl.shaderSource(vertexShaderObject, vertexShaderSourceCode);

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
	
	//fragmentShader
	var fragmentShaderSource =
	"#version 300 es"+
	"\n"+
	"precision highp float;"+
	"in vec3 phong_ads_color;" +
	"out vec4 FragColor;" +
	"void main(void)" +
	"{" +
	"FragColor = vec4(phong_ads_color, 1.0);" +
	"}";

	fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);

	gl.shaderSource(fragmentShaderObject,fragmentShaderSource);

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
	shaderProgramObject=gl.createProgram();

	gl.attachShader(shaderProgramObject, vertexShaderObject);
	gl.attachShader(shaderProgramObject, fragmentShaderObject);
	
	//pre-linking
	gl.bindAttribLocation(shaderProgramObject, webGLMacros.AAN_ATTRIBUTE_POSITION, "a_position");
	gl.bindAttribLocation(shaderProgramObject, webGLMacros.AAN_ATTRIBUTE_NORMAL, "a_normal");

	//linking
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
	
	modelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_model_matrix");
	viewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_view_matrix");
	projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projection_matrix");
	keyPressedUniform = gl.getUniformLocation(shaderProgramObject, "u_lighting_enabled");
	la0Uniform = gl.getUniformLocation(shaderProgramObject, "u_la0");		
	ld0Uniform = gl.getUniformLocation(shaderProgramObject, "u_ld0");
	ls0Uniform = gl.getUniformLocation(shaderProgramObject, "u_ls0");
	
	la1Uniform = gl.getUniformLocation(shaderProgramObject, "u_la1");		
	ld1Uniform = gl.getUniformLocation(shaderProgramObject, "u_ld1");
	ls1Uniform = gl.getUniformLocation(shaderProgramObject, "u_ls1");
	
	kaUniform = gl.getUniformLocation(shaderProgramObject, "u_ka");
	kdUniform = gl.getUniformLocation(shaderProgramObject, "u_kd");
	ksUniform = gl.getUniformLocation(shaderProgramObject, "u_ks");
	
	lightPosition0Uniform = gl.getUniformLocation(shaderProgramObject, "u_light0_position");
	lightPosition1Uniform = gl.getUniformLocation(shaderProgramObject, "u_light1_position");
	materialShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_material_shininess");
	

    //Declaration & Initialization of geometry

	var pyramidVertices = new Float32Array(
                [
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
                ]
    );

    
	var pyramidNormals = new Float32Array(
        [
            0.0, 0.447214, 0.894427,// front-top
            0.0, 0.447214, 0.894427,// front-left
            0.0, 0.447214, 0.894427,// front-right

            0.894427, 0.447214, 0.0, // right-top
            0.894427, 0.447214, 0.0, // right-left
            0.894427, 0.447214, 0.0, // right-right

            0.0, 0.447214, -0.894427, // back-top
            0.0, 0.447214, -0.894427, // back-left
            0.0, 0.447214, -0.894427, // back-right

            -0.894427, 0.447214, 0.0, // left-top
            -0.894427, 0.447214, 0.0, // left-left
            -0.894427, 0.447214, 0.0 // left-right
        ]
        );

    //vao_pyramid and vbo related code
	vao_pyramid=gl.createVertexArray();
	gl.bindVertexArray(vao_pyramid);

	//vbo for position
	vbo_pyramid_position =  gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_pyramid_position);
	gl.bufferData(gl.ARRAY_BUFFER, pyramidVertices, gl.STATIC_DRAW);
	gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_POSITION,3,gl.FLOAT,false, 0, 0);
	gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_POSITION);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	//vbo for normal
	vbo_pyramid_normal =  gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_pyramid_normal);
	gl.bufferData(gl.ARRAY_BUFFER, pyramidNormals, gl.STATIC_DRAW);
	gl.vertexAttribPointer(webGLMacros.AAN_ATTRIBUTE_NORMAL,3,gl.FLOAT,false, 0, 0);
	gl.enableVertexAttribArray(webGLMacros.AAN_ATTRIBUTE_NORMAL);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
		
	//Depth Related code
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    //Clear screen with black color
    gl.clearColor(0.0,0.0,0.0,1.0);

	perspectiveProjectionMatrix =  mat4.create();
}

function resize()
{
	if(bFullscreen == true)
	{
		canvas.width = window.innerWidth;
		canvas.height = window.innerHeight;		
	}else
	{
		canvas.width = canvas_original_width;
		canvas.height = canvas_original_height;
	}
	
    if(canvas.height == 0)
    {
        canvas.height = 1;          //To avoid Divided by 0 illegal statement for future code
    }

    gl.viewport(0,0,canvas.width,canvas.height);


	mat4.perspective(perspectiveProjectionMatrix, 45.0, parseFloat(canvas.width)/parseFloat(canvas.height),1.0, 100.0);		
	
}


function draw()
{
    //code

	gl.clear(gl.COLOR_BUFFER_BIT|gl.DEPTH_BUFFER_BIT);
	
    //Use Shader Program Object
    gl.useProgram(shaderProgramObject);
	
	//Related to light
	if(bLight)
    {
			gl.uniform1i(keyPressedUniform, 1);
			gl.uniform3fv(la0Uniform, light0_ambient);
			gl.uniform3fv(ld0Uniform, light0_diffuse);
			gl.uniform3fv(ls0Uniform, light0_specular);
			
			gl.uniform4fv(lightPosition0Uniform, light0_position);
			
			gl.uniform3fv(la1Uniform, light1_ambient);
			gl.uniform3fv(ld1Uniform, light1_diffuse);
			gl.uniform3fv(ls1Uniform, light1_specular);
			
			gl.uniform4fv(lightPosition1Uniform, light1_position);
			
			gl.uniform3fv(kaUniform, material_ambient);
			gl.uniform3fv(kdUniform, material_diffuse);
			gl.uniform3fv(ksUniform, material_specular);
			gl.uniform1f(materialShininessUniform,material_shininess);
			
	}
    
    else
    {
			gl.uniform1i(keyPressedUniform, 0);
	}


    //Transformations
	var modelMatrix = mat4.create();
	var viewMatrix = mat4.create();
	
	var angleInRadian = degreeToRadian(anglePyramid);
	mat4.translate(modelMatrix, modelMatrix, [0.0,0.0,-4.0]);
	mat4.rotateY(modelMatrix, modelMatrix,angleInRadian);

	gl.uniformMatrix4fv(modelMatrixUniform,false,modelMatrix);
	gl.uniformMatrix4fv(viewMatrixUniform,false,viewMatrix);
	gl.uniformMatrix4fv(projectionMatrixUniform,false,perspectiveProjectionMatrix);
	
	//draw pyramid
	gl.bindVertexArray(vao_pyramid);
	gl.drawArrays(gl.TRIANGLES, 0,12);
	gl.bindVertexArray(null);
	
	gl.useProgram(null);

    //FUNCTION CALL for update() 
	update();

	requestAnimationFrame(draw,canvas);
}


function toggleFullScreen()
{
	//code
	var fullScreen_element = 
		document.fullscreenElement||
		document.webkitFullscreenElement||
		document.mozFullScreenElement||
		document.msFullscreenElement||
		null;
		
        if(fullScreen_element == null)      //If Not Fullscreen
        {
            if(canvas.requestFullscreen)        
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
    
	resize();
}

function keydown(event)
{
	switch(event.keyCode)
	{
		case 27:                                        //ESCAPE
			uninitialize();
			window.close();
			break;

		case 76:
		case 108:
				if(bLight)
					bLight = false;
				else
					bLight = true;
			break;

		case 70:                                    //'F' or 'f'		
		    toggleFullScreen();			
			break;
	}
}
function update()
{
		if( anglePyramid >= 360.0)
			anglePyramid = 0.0;
		else
			anglePyramid = anglePyramid + 1.0;
}

function degreeToRadian(angleInDegree)
{
	return (angleInDegree *  Math.PI/ 180);
}

function mouseDown()
{
	//code
}

function uninitialize()
{
	if(vao_pyramid)
	{
		gl.deleteVertexArray(vao_pyramid);
		vao_pyramid = null;
	}
	if(vbo_pyramid_position)
	{
		gl.deleteBuffer(vbo_pyramid_position);
		vbo_pyramid_position=null;
	}
	if(vbo_pyramid_normal)
	{
		gl.deleteBuffer(vbo_pyramid_normal);
		vbo_pyramid_normal=null;
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

