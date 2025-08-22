//global variables
var canvas=null;
var gl=null; //for webgl context
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;

const webGLMacros=
{
	AAN_ATTRIBUTE_VERTEX:0,
	AAN_ATTRIBUTE_COLOR:1,
	AAN_ATTRIBUTE_NORMAL:2,
	AAN_ATTRIBUTE_TEXTURE0:3
};

var materialProperties;
var vertexShaderObject;
var fragmentShaderObject;
var shaderProgramObject;

var rotationAxis; 

var vbo_position;
var vbo_normal;
var vao;
var angleSphere=0.0;

var modelMatrixUniform;
var viewMatrixUniform;
var projectionMatrixUniform;
var lightingEnabledUniform;
var laUniform;
var ldUniform;
var lsUniform;
var kaUniform;
var kdUniform;
var ksUniform;
var materialShininessUniform;
var bLight=false;

var perspectiveProjectionMatrix;

var RADIUS = 100.0;

var light_ambient = [0.0, 0.0, 0.0];	
var light_diffuse = [1.0, 1.0, 1.0];
var light_specular = [1.0, 1.0, 1.0];

var lightPosition = [0.0,0.0,0.0,1.0];	


var winWidth,winHeight;

var material_ambient = [0.0, 0.0, 0.0];
var material_diffuse = [1.0, 1.0, 1.0];
var material_specular = [1.0, 1.0, 1.0];
var material_shininess = 128.0;

var sphere=null;


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
			
//main

function main()
{
	 //code 
     canvas = document.getElementById("AAN");
     if(!canvas)
     {
         console.log("Obtaining Canvas Failed \n");
     }
     else
     {
         console.log("Obtaining Canvas Succeded \n");
     }
 
     //backup canvas dimension
     canvas_original_width = canvas.width;
     canvas_original_height = canvas.height;
 
 
     //initialize
     initialize();
 
     // warmup resize 
     resize();
 
     //display
     draw();
 
     window.addEventListener("keydown",keyDown,false); 
     window.addEventListener("click",mouseDown,false);
     window.addEventListener("resize",resize,false); 
}

//Event Listener

function keyDown(event)
{
	switch(event.keyCode)
	{
		case 27:            //Esc
			uninitialize();
			window.close();
			break;

		case 120:            //x/X
		case 88:
			rotationAxis='x';
			break;

		case 121:             //y/Y
		case 89: 
			rotationAxis='y';
			break;

		case 122:            //z/Z
		case 90:
			rotationAxis='z';
			break;

		case 76:              //l/L
		case 108:
            if(bLight)
                bLight = false;
            else
                bLight = true;
			break;

		case 70: //for 'F' or 'f'	
		    toggleFullScreen();			
			break;
	}
}

// mouse event
function mouseDown()
{
	//code
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
		
	//if not full screen
	if(fullScreen_element == null)
	{
		if(canvas.requestFullscreen)
			canvas.requestFullscreen();
		else if(canvas.mozRequestFullScreen)
			canvas.mozRequestFullScreen();
		else if(canvas.webkitRequestFullscreen)
			canvas.webkitRequestFullscreen();
		else if(canvas.msRequestFullscreen)
			canvas.msRequestFullscreen();

            bFullscreen = true;
	}
	else //restore from fullscreen
	{
        if(document.exitFullscreen)
            document.exitFullscreen();
        else if(document.mozCancelFullScreen)
            document.mozCancelFullScreen();
        else if(document.webkitExitFullscreen)
            document.webkitExitFullscreen();
        else if(document.msExitFullscreen)
            document.msExitFullscreen();

            bFullscreen = false;
	}
}


function initialize()
{
    //code

	//Obtain OpenGL context
	gl=canvas.getContext("webgl2");
	if(!gl)
    {
        console.log("Obtaining WebGL 2.0 failed\n");
    }
		
	else
    {
        console.log("Obtaining WebGL 2.0 succeeded\n");
    }
		
    // set viewport width and height 
	gl.viewportWidth  = canvas.width;
	gl.viewportHeight  = canvas.height;
	
	//vertex shadert
	var vertexShaderSourceCode =
	     "#version 300 es"+
         "\n"+
         "in vec4 a_position;"+
         "in vec3 a_normal;"+
		 "uniform vec4 u_light_position;"+
         "uniform mat4 u_model_matrix;"+
         "uniform mat4 u_view_matrix;"+
         "uniform mat4 u_projection_matrix;"+
         "uniform mediump int u_lightingEnabled;"+
		 "out vec3 transformed_normals;" +
		 "out vec3 light_direction;" +
		 "out vec3 viewer_vector;" +		 
         "void main(void)"+
         "{"+
         "if (u_lightingEnabled == 1)"+
         "{"+
         "vec4 eye_coordinates=u_view_matrix * u_model_matrix * a_position;"+
         "transformed_normals=mat3(u_view_matrix * u_model_matrix) * a_normal;"+
         "light_direction = vec3(u_light_position) - eye_coordinates.xyz;"+
		 "viewer_vector = -eye_coordinates.xyz;"+         
         "}"+
         "gl_Position=u_projection_matrix * u_view_matrix * u_model_matrix * a_position;"+
         "}" ;

	vertexShaderObject=gl.createShader(gl.VERTEX_SHADER);

	gl.shaderSource(vertexShaderObject, vertexShaderSourceCode);

	gl.compileShader(vertexShaderObject);

	if(gl.getShaderParameter(vertexShaderObject,gl.COMPILE_STATUS) == false)
	{
		var error=gl.getShaderInfoLog(vertexShaderObject);
		if(error.length > 0)
		{
			alert(error);
			uninitialize();
		}
	}
	
	//fragmentShader
	var fragmentShaderSource =
	    "#version 300 es"+
        "\n"+
        "precision highp float;"+
		"uniform mediump int u_lightingEnabled;"+
		"in vec3 transformed_normals;" +
		"in vec3 light_direction;" +
		"in vec3 viewer_vector;" +
		"uniform vec3 u_la;" +
		"uniform vec3 u_ld;" +
		"uniform vec3 u_ls;" +
		"uniform vec3 u_ka;" +
		"uniform vec3 u_kd;" +
		"uniform vec3 u_ks;" +
		"uniform float u_materialShininess;" +
		"out vec4 FragColor;" +
		"void main(void)" +
		"{" +
		"vec3 phong_ads_light;"+
		"if(u_lightingEnabled == 1)" +
		"{" +
        "vec3 ambient = u_la * u_ka;" +
		"vec3 normalized_transformed_normals = normalize(transformed_normals);" +
		"vec3 normalized_light_direction = normalize(light_direction);" +
        "vec3 diffuse = u_ld * u_kd * max(dot(normalized_transformed_normals, normalized_light_direction), 0.0);" +
		"vec3 reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normals);" +
        "vec3 normalized_viewer_vector = normalize(viewer_vector);" +
		"vec3 specular = u_ls * u_ks * pow(max(dot(reflection_vector, normalized_viewer_vector),0.0), u_materialShininess);" +
		"phong_ads_light = ambient + diffuse + specular;" +
		"}" +
		"else" +
		"{" +
		"phong_ads_light = vec3(1.0, 1.0, 1.0);" +
		"}" +
		"FragColor = vec4(phong_ads_light, 1.0);" +
		"}";

	fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);

	gl.shaderSource(fragmentShaderObject,fragmentShaderSource);

	gl.compileShader(fragmentShaderObject);

	if(gl.getShaderParameter(fragmentShaderObject,gl.COMPILE_STATUS) == false)
	{
		var error=gl.getShaderInfoLog(fragmentShaderObject);
		if(error.length > 0)
		{
			alert(error);
			uninitialize();
		}
	}

	//shader program
	shaderProgramObject=gl.createProgram();

	gl.attachShader(shaderProgramObject, vertexShaderObject);
	gl.attachShader(shaderProgramObject, fragmentShaderObject);
	
	//pre-link 
	gl.bindAttribLocation(shaderProgramObject, webGLMacros.AAN_ATTRIBUTE_VERTEX, "a_position");
	gl.bindAttribLocation(shaderProgramObject, webGLMacros.AAN_ATTRIBUTE_NORMAL, "a_normal");
	
    //linking
	gl.linkProgram(shaderProgramObject);

	if(!gl.getProgramParameter(shaderProgramObject, gl.LINK_STATUS))
	{
		var error = gl.getProgramInfoLog(shaderProgramObject);
		if(error.length > 0)
		{
			alert(error);
			uninitialize();
		}
	}

	//Post Linking

	modelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_model_matrix");
	viewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_view_matrix");
	projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projection_matrix");
	
    laUniform = gl.getUniformLocation(shaderProgramObject, "u_la");		
	ldUniform = gl.getUniformLocation(shaderProgramObject, "u_ld");
	lsUniform = gl.getUniformLocation(shaderProgramObject, "u_ls");
	lightingPositionUniform = gl.getUniformLocation(shaderProgramObject, "u_light_position");
	
	
	kaUniform = gl.getUniformLocation(shaderProgramObject, "u_ka");
	kdUniform = gl.getUniformLocation(shaderProgramObject, "u_kd");
	ksUniform = gl.getUniformLocation(shaderProgramObject, "u_ks");

    
    lightingEnabledUniform = gl.getUniformLocation(shaderProgramObject, "u_lightingEnabled");
	materialShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_materialShininess");
	
	
	//Sphere Initialization

	sphere = new Mesh();
	makeSphere(sphere,2.0,30,30);

	materialProperties = initializeMaterials();

	gl.clearColor(0.3,0.3,0.3,1.0);

	gl.clearDepth(1.0);
	gl.enable(gl.DEPTH_TEST);
	gl.depthFunc(gl.LEQUAL);
	gl.enable(gl.CULL_FACE);

	perspectiveProjectionMatrix =  mat4.create();
}


function drawSpheres(no_of_spheres, no_of_materials) 
{

	var windowCenterX = winWidth / 2;
	var windowCenterY = winHeight / 2;
	var viewPortSizeX = winWidth/4;
	var viewPortSizeY = winHeight/6;
	var newAspectRatio = (parseFloat(viewPortSizeX))/(parseFloat(viewPortSizeY));
	var viewPortSizeAspectRatio = parseFloat(viewPortSizeX / viewPortSizeY);
	
	//update perspective according to new aspect ratio
	mat4.perspective(perspectiveProjectionMatrix, 45.0, newAspectRatio, 0.1, 100.0);		
	
	gl.viewport(windowCenterX, windowCenterY, viewPortSizeX, viewPortSizeY);
	
	var y_trans = viewPortSizeY;
	var x_trans = viewPortSizeX;
	var distanceBetSpheres = 130;
	var currentViewPortX = windowCenterX - x_trans*3; //we have 4 columns, 2-2 from center
	var currentViewPortY = windowCenterY + y_trans*2;  //we have 8 rows but 4 up, 4 down from center
	
	gl.viewport(currentViewPortX, currentViewPortY, viewPortSizeX, viewPortSizeY);
	
	var i = 0, j = 0;
	for (i = 1, j = 0;i <= no_of_spheres;i++, j++) {

		var local_y_trans = 0;
		if (((i - 1) % 4) == 0 && (i - 1 != 0)) {
			currentViewPortY -= y_trans;
			currentViewPortX = windowCenterX - x_trans*3; //reset X
			gl.viewport( currentViewPortX, currentViewPortY, viewPortSizeX, viewPortSizeY);
		}
		currentViewPortX += x_trans;
		gl.viewport(currentViewPortX, currentViewPortY, viewPortSizeX, viewPortSizeY);
		
		//set material properties
		if (j < no_of_materials)
			setMaterialProperties(j);   //j is material number

		//draws sphere 
		sphere.draw();
	}
	
}


function initializeMaterials()
{
			var materialProperties= [
				//0 
                [ 0.0215, 0.1745, 0.0215, 1.0 ],//ambient;
                [ 0.07568, 0.61424, 0.07568, 1.0], //diffuse
                [ 0.633, 0.727811, 0.633, 1.0],//specular
                [0.6 * 128],//shininess,
                 //1 
                [0.135, 0.2225, 0.1575, 1.0],//ambient;
                [0.54, 0.89, 0.63, 1.0], //diffuse
                [0.316228, 0.316228, 0.316228, 1.0],//specular
                [0.1 * 128],//shininess		
                //2 
                [ 0.05375, 0.5, 0.06625, 1.0 ],//ambient;
                [ 0.18275, 0.17, 0.22525, 1.0 ], //diffuse
                [ 0.332741, 0.328634, 0.346435, 1.0 ],//specular
                [0.3 * 128],//shininess						
				//3 
				[ 0.25, 0.20725, 0.20725, 1.0 ],//ambient;
				[ 1.0, 0.829, 0.829, 1.0 ], //diffuse
				[ 0.296648, 0.296648, 0.296648, 1.0 ],//specular
				[0.88 * 128],//shininess
				//4 
				[ 0.1745, 0.01175, 0.01175, 1.0 ],//ambient;
				[ 0.61424, 0.04136, 0.04136, 1.0 ], //diffuse
				[ 0.727811, 0.626959, 0.626959, 1.0 ],//specular
				[0.6 * 128],//shininess
				//5 
				[ 0.1, 0.18725, 0.1745, 1.0 ],//ambient;
				[ 0.396, 0.74151, 0.69102, 1.0 ], //diffuse
				[ 0.297254, 0.30829, 0.306678, 1.0 ],//specular
				[0.1 * 128],//shininess
				//6 
				[ 0.329412, 0.223529, 0.27451, 1.0 ],//ambient;
				[ 0.78392, 0.568627, 0.113725, 1.0 ], //diffuse
				[ 0.992157, 0.941176, 0.807843, 1.0 ],//specular
				[0.21794872 * 128],//shininess
				//7 
				[ 0.2125, 0.1275, 0.054, 1.0 ],//ambient;
				[ 0.714, 0.4284, 0.18144, 1.0 ], //diffuse
				[ 0.393548, 0.271906, 0.166721, 1.0 ],//specular
				[0.2 * 128],//shininess
				//8
				[ 0.25, 0.25, 0.25, 1.0 ],//ambient;
				[ 0.4, 0.4, 0.4, 1.0 ], //diffuse
				[ 0.774597, 0.774597, 0.774597, 1.0 ],//specular
				[0.6 * 128],//shininess
				//9 
				[ 0.19125, 0.0735, 0.0225, 1.0 ],//ambient;
				[ 0.7038, 0.27048, 0.0828, 1.0 ], //diffuse
				[ 0.25677, 0.137622, 0.086014, 1.0 ],//specular
				[0.1 * 128],//shininess
				//10 
				[ 0.24725, 0.1995, 0.0745, 1.0 ],//ambient;
				[ 0.75164, 0.60648, 0.22648, 1.0 ], //diffuse
				[ 0.628281, 0.555802, 0.366065, 1.0 ],//specular
				[0.4 * 128],//shininess
				//11 
				[ 0.19225, 0.19225, 0.19225, 1.0 ],//ambient;
				[ 0.50745, 0.50745, 0.50745, 1.0 ], //diffuse
				[ 0.508273, 0.508273, 0.508273, 1.0 ],//specular
				[0.4 * 128],//shininess
				//12 
				[ 0.0, 0.0, 0.0, 1.0 ],//ambient;
				[ 0.0, 0.0, 0.0, 1.0 ], //diffuse
				[ 0.50, 0.50, 0.50, 1.0 ],//specular
				[0.25 * 128],//shininess
				//13 
				[ 0.0, 0.1, 0.06, 1.0 ],//ambient;
				[ 0.0, 0.50980392, 0.50980392, 1.0 ], //diffuse
				[ 0.50196078, 0.50196078, 0.50196078, 1.0 ],//specular
				[0.25 * 128],//shininess
				//14
				[ 0.0, 0.0, 0.0, 1.0 ],//ambient;
				[ 0.1, 0.35, 0.1, 1.0 ], //diffuse
				[ 0.45, 0.55, 0.45, 1.0 ],//specular
				[0.25 * 128],//shininess
				//15 
				[ 0.0, 0.0, 0.0, 1.0 ],//ambient;
				[ 0.5, 0.0, 0.0, 1.0 ], //diffuse
				[ 0.7, 0.6, 0.6, 1.0 ],//specular
				[0.25 * 128],//shininess
				//16 
				[ 0.0, 0.0, 0.0, 1.0 ],//ambient;
				[ 0.55, 0.55, 0.55, 1.0 ], //diffuse
				[ 0.70, 0.70, 0.70, 1.0 ],//specular
				[0.25 * 128],//shininess
				//17 
				[ 0.0, 0.0, 0.0, 1.0 ],//ambient;
				[ 0.5, 0.5, 0.0, 1.0 ], //diffuse
				[ 0.60, 0.60, 0.50, 1.0 ],//specular
				[0.25 * 128],//shininess
				//18 
				[ 0.02, 0.02, 0.02, 1.0 ],//ambient;
				[ 0.01, 0.01, 0.01, 1.0 ], //diffuse
				[ 0.04, 0.04, 0.04, 1.0 ],//specular
				[0.078125 * 128],//shininess
				//19 
				[ 0.0, 0.05, 0.05, 1.0 ],//ambient;
				[ 0.4, 0.5, 0.5, 1.0 ], //diffuse
				[ 0.04, 0.7, 0.7, 1.0 ],//specular
				[0.078125 * 128],//shininess
				//20 
				[ 0.0, 0.05, 0.00, 1.0 ],//ambient;
				[ 0.4, 0.5, 0.4, 1.0 ], //diffuse
				[ 0.04, 0.7, 0.04, 1.0 ],//specular
				[0.078125 * 128],//shininess
				//21 
				[ 0.05, 0.0, 0.0, 1.0 ],//ambient;
				[ 0.5, 0.4, 0.4, 1.0 ], //diffuse
				[ 0.7, 0.04, 0.04, 1.0 ],//specular
				[0.078125 * 128],//shininess
				//22 
				[ 0.05, 0.05, 0.05, 1.0 ],//ambient;
				[ 0.5, 0.5, 0.5, 1.0 ], //diffuse
				[ 0.7, 0.7, 0.7, 1.0 ],//specular
				[0.078125 * 128],//shininess
				//23 
				[ 0.05, 0.05, 0.0, 1.0 ],//ambient;
				[ 0.5, 0.5, 0.4, 1.0 ], //diffuse
				[ 0.7, 0.7, 0.04, 1.0 ],//specular
				[0.078125 * 128]//shininess	
				
			];
			return materialProperties;
}

function setMaterialProperties(materialNumber)
{
		var materialIndex= materialNumber*4; 
		var material_ambient= materialProperties[materialIndex];
        var material_diffuse = materialProperties[materialIndex + 1];
        var material_specular = materialProperties[materialIndex + 2];
        var material_shininess = materialProperties[materialIndex + 3][0];

		//set material properties
		gl.uniform3fv(kaUniform,material_ambient.slice(0,3));
		gl.uniform3fv(kdUniform, material_diffuse.slice(0,3));
		gl.uniform3fv(ksUniform, material_specular.slice(0,3));
		gl.uniform1f(materialShininessUniform, material_shininess);
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

	winWidth = canvas.width;
	winHeight = canvas.height;
	
	mat4.perspective(perspectiveProjectionMatrix, 45.0, parseFloat(canvas.width/canvas.height),0.1, 100.0);		
	
	gl.viewport(0,0,canvas.width,canvas.height);
}

function draw()
{
	gl.clear(gl.COLOR_BUFFER_BIT|gl.DEPTH_BUFFER_BIT);
	gl.useProgram(shaderProgramObject);

    var modelMatrix = mat4.create();
	var viewMatrix = mat4.create();
	
	mat4.translate(modelMatrix, modelMatrix, [0.0,0.0,-6.0]);

	gl.uniformMatrix4fv(modelMatrixUniform,false,modelMatrix);
	gl.uniformMatrix4fv(viewMatrixUniform,false,viewMatrix);
	gl.uniformMatrix4fv(projectionMatrixUniform,false,perspectiveProjectionMatrix);
	
	
	//lighting details
	if(bLight == true)
    {
        gl.uniform1i(lightingEnabledUniform, 1);
    }
    else
    {
        gl.uniform1i(lightingEnabledUniform, 0);
    }
	
	gl.uniform3fv(laUniform, light_ambient);
	gl.uniform3fv(ldUniform, light_diffuse);
	gl.uniform3fv(lsUniform, light_specular);
	
	switch(rotationAxis)
    {
			case 'x': //x axis
				lightPosition[0] = 0.0;
				lightPosition[1] = RADIUS * Math.cos(angleSphere);
				lightPosition[2] = RADIUS * Math.sin(angleSphere);
				break;
				
			case 'y': //y axis
				lightPosition[0] = RADIUS * Math.cos(angleSphere);
				lightPosition[1] = 0.0;
				lightPosition[2] = RADIUS * Math.sin(angleSphere);
				break;
				
			case 'z': //z axis
				lightPosition[0] = RADIUS * Math.cos(angleSphere);
				lightPosition[1] = RADIUS * Math.sin(angleSphere);
				lightPosition[2] = 0.0;														
				break;

			default:
				break;
	}
		
	gl.uniform4fv(lightingPositionUniform, lightPosition);
	
	//set material properties
	gl.uniform3fv(kaUniform, material_ambient);
	gl.uniform3fv(kdUniform, material_diffuse);
	gl.uniform3fv(ksUniform, material_specular);
	gl.uniform1f(materialShininessUniform,material_shininess);
	
	drawSpheres(24,24);

	gl.useProgram(null);
	update();
	requestAnimationFrame(draw,canvas);
}


function update()
{
		if( angleSphere >= 360.0)
			angleSphere = 0.0;
		else
			angleSphere = angleSphere + 0.08;
}


function degreeToRadian(angleInDegree)
{
	return (angleInDegree *  Math.PI/ 180);
}


function uninitialize()
{
	
	if(sphere)
	{
		sphere.deallocate();
		sphere=null;
	}

	if(vao)
    {
        gl.deleteVertexArray(vao);
        vao = null;
    }

    if(vbo_position)
    {
        gl.deleteBuffer(vbo_position);
        vbo_position = null;
    }

	if(vbo_normal)
    {
        gl.deleteBuffer(vbo_normal);
        vbo_normal = null;
    }
	
	if(shaderProgramObject)
	{
		if(fragmentShaderObject)
		{
			gl.detachShader(shaderProgramObject, fragmentShaderObject);
			fragmentShaderObject = null;
		}
		
		if(vertexShaderObject)
		{
			gl.detachShader(shaderProgramObject, vertexShaderObject);
			vertexShaderObject = null;
		}
		gl.deleteProgram(shaderProgramObject);
		shaderProgramObject = null;
	}

}
