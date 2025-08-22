var canvas=null;
var gl=null;
var bFullScreen=false;
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

var vao;
var vbo;

var modelMatrixUniform;
var viewMatrixUniform;
var projectionMatrixUniform;

var perspectiveProjectionMatrix;

var sphere=null;

// Light

var enableShader = 0;

var bLight=false;

var laUniform=[3];
var ldUniform=[3];
var lsUniform=[3];
var lightPositionUniform=[3];

var kaUniform;
var kdUniform;
var ksUniform;
var materialShinenessUniform;

var lightingEnabledUniform;
var shaderEnabledUniform;

var materialAmbient = new Float32Array([ 0.0,0.0 ,0.0 ,1.0 ]);
var materialDiffuse = new Float32Array([ 1.0,1.0 ,1.0 ,1.0 ]);
var materialSpecular = new Float32Array([ 1.0 ,1.0 ,1.0 ,1.0 ]);
var materialShininess = 128.0;

var lightAngleZero = 0.0;
var lightAngleOne = 0.0;
var lightAngleTwo = 0.0;
var radius = 20.0;

var lights=[
    [
        [0.0, 0.0, 0.0, 1.0],   //lightAmbient
        [1.0, 0.0, 0.0, 1.0],   //lightDiffuse
        [1.0, 0.0, 0.0, 1.0],   //lightSpecular
        [0.0, 0.0, 0.0, 1.0]    //lightPosition  
    ],
    [
        [0.0, 0.0, 0.0, 1.0],   //lightAmbient
        [0.0, 1.0, 0.0, 1.0],   //lightDiffuse
        [0.0, 1.0, 0.0, 1.0],    //lightSpecular
        [0.0, 0.0, 0.0, 1.0]    //lightPosition  
    ],
    [
        [0.0, 0.0, 0.0, 1.0],   //lightAmbient
        [0.0, 0.0, 1.0, 1.0],    //lightDiffuse
        [0.0, 0.0, 1.0, 1.0],   //lightSpecular
        [0.0, 0.0, 0.0, 1.0]    //lightPosition  
    ]
];


var requestAnimationFrame=window.requestAnimationFrame ||
window.mozRequestAnimationFrame ||
window.webkitRequestAnimationFrame ||
window.oRequestAnimationFrame ||
window.msRequestAnimationFrame;

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
    display();

    window.addEventListener("keydown",keyDown,false); 
    window.addEventListener("click",mouseDown,false);
    window.addEventListener("resize",resize,false); 

}

// Event Listener

function keyDown(event)
{
    
    //code 
    switch (event.keyCode) {

        case 27:
            toggleFullScreen();            
            break;

        case 69:
           uninitialize();
           window.close();  // not all browsers follow this
           break;

        case 70: // f
            enableShader = 1;
			break;

        case 86:  // v
            enableShader = 0;
			break;

        case 76: // l
            if (bLight == false)
            {
                bLight = true;
            }
            else
            {
                bLight = false;
            }
            break;
    
        default:
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
     // Code
     var fullscreen_Element = document.fullscreenElement ||       
     document.mozFullScreenElement ||    
     document.webkitFullscreenElement || 
     document.msFullscreenElement ||     
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
    //code
    //obtain webgl context
    gl=canvas.getContext("webgl2");
    if(!gl)
    {
        console.log("Obtaining WebGL 2.0 Failed");
    }
    else
    {
        console.log("Obtaining WebGL 2.0 Succeded");
    }

    // set viewport width and height 
    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;

    // Vertex Shader
    var vertexShaderSourceCode =
        "#version 300 es" +
        "\n" +
        "in vec4 a_position;" +
        "in vec3 a_normal;" +
        "uniform vec3 u_la[3];" +
        "uniform vec3 u_ld[3];" +
        "uniform vec3 u_ls[3];" +
        "uniform vec4 u_lightPosition[3];" +
        "uniform vec3 u_ka;" +
        "uniform vec3 u_kd;" +
        "uniform vec3 u_ks;" +
        "uniform float u_materialShininess;" +
        "uniform mat4 u_modelMatrix;" +
        "uniform mat4 u_viewMatrix;" +
        "uniform mat4 u_projectionMatrix;" +
        "uniform mediump int u_lightingEnabled;" +
        "uniform mediump int u_shaderEnabled;" +
        "out vec3 transformedNormal;" +
        "out vec3 lightDirection[3];" +
        "out vec3 viewerVector;" +
        "out vec3 phong_ads_light;" +
        "void main(void)" +
        "{" +
        "if (u_lightingEnabled == 1)" +
        "{" +
        "if (u_shaderEnabled == 0)" +
        "{" +
        "vec4 eyecordinate=u_viewMatrix * u_modelMatrix * a_position;" +
        "mat3 normalMatrix=mat3(u_viewMatrix * u_modelMatrix);" +
        "vec3 transformedNormal=normalize(normalMatrix * a_normal);" +
        "vec3 viewerVector=normalize(-eyecordinate.xyz);" +

        "vec3 ambient[3];" +
        "vec3 lightDirection[3];" +
        "vec3 diffuse[3];" +
        "vec3 reflectionVector[3];" +
        "vec3 specular[3];" +
        "for(int i=0; i<3; i++)" +
        "{" +
        "ambient[i] = u_la[i] * u_ka;" +
        "lightDirection[i]=normalize(vec3(u_lightPosition[i]) - eyecordinate.xyz);" +
        "diffuse[i] = u_ld[i] * u_kd * max(dot(lightDirection[i],transformedNormal),0.0);" +
        "reflectionVector[i]=reflect(-(lightDirection[i]),transformedNormal);" +
        "specular[i] = u_ls[i] * u_ks * pow(max(dot(reflectionVector[i],viewerVector),0.0),u_materialShininess);" +
        "phong_ads_light=phong_ads_light + ambient[i] + diffuse[i] + specular[i];" +
        "}" +
        "}" +
        "if (u_shaderEnabled == 1)" +
        "{" +
        "vec4 eyecordinate=u_viewMatrix * u_modelMatrix * a_position;" +
        "mat3 normalMatrix=mat3(u_viewMatrix * u_modelMatrix);" +
        "transformedNormal=normalMatrix * a_normal;" +
        "for(int i=0; i<3; i++)" +
        "{" +
        "lightDirection[i]=vec3(u_lightPosition[i])-eyecordinate.xyz;" +
        "}" +
        "viewerVector=-eyecordinate.xyz;" +
        "}" +
        "}" +
        "gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" +
        "}";

    var vertexShaderObject=gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShaderObject,vertexShaderSourceCode);
    gl.compileShader(vertexShaderObject);

    //error check

    if(gl.getShaderParameter(vertexShaderObject,gl.COMPILE_STATUS)==false)
    {
        var error=gl.getShaderInfoLog(vertexShaderObject);
        if(error.length>0)
        {
            alert("Vertex shader :"+error);
            uninitialize();
        }
    }

    // Fragment Shader
    var fragmentShaderSourceCode = 
    "#version 300 es" +
    "\n" +
    "precision highp float;" +
    "in vec3 transformedNormal;" +
    "in vec3 lightDirection[3];" +
    "in vec3 viewerVector;" +
    "in vec3 phong_ads_light;" +
    "uniform vec3 u_la[3];" +
    "uniform vec3 u_ld[3];" +
    "uniform vec3 u_ls[3];" +
    "uniform vec3 u_ka;" +
    "uniform vec3 u_kd;" +
    "uniform vec3 u_ks;" +
    "uniform float u_materialShininess;" +
    "uniform int u_lightingEnabled;" +
    "uniform int u_shaderEnabled;" +
    "out vec4 FragColor;" +
    "void main(void)" +
    "{" +
    "vec3 phong_ads_color;" +
    "if (u_lightingEnabled == 1)" +
    "{" +
    "if (u_shaderEnabled == 0)" +
    "{" +
    "phong_ads_color=phong_ads_light;" +
    "}" +
    "if (u_shaderEnabled == 1)" +
    "{" +
    "vec3 ambient[3];" +
    "vec3 diffuse[3];" +
    "vec3 reflectionVector[3];" +
    "vec3 specular[3];" +
    "vec3 normalizedTransformedNormal=normalize(transformedNormal);" +
    "vec3 normalizedviewerVector=normalize(viewerVector);" +
    "for(int i=0; i<3; i++)" +
    "{" +
    "ambient[i] = u_la[i] * u_ka;" +
    "vec3 normalizedLightDirection=normalize(lightDirection[i]);" +
    "diffuse[i] = u_ld[i] * u_kd *max(dot(normalizedLightDirection,normalizedTransformedNormal),0.0);" +
    "reflectionVector[i]=reflect(-normalizedLightDirection,normalizedTransformedNormal);" +
    "specular[i] = u_ls[i] * u_ks * pow(max(dot(reflectionVector[i],normalizedviewerVector),0.0),u_materialShininess);" +
    "phong_ads_color=phong_ads_color+ambient[i] + diffuse[i]+specular[i] ;" +
    "}" +
    "}" +
    "}" +
    "else" +
    "{" +
    "phong_ads_color=vec3(1.0,1.0,1.0);" +
    "}" +
    "FragColor=vec4(phong_ads_color,1.0);" +
    "}";

    var fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObject,fragmentShaderSourceCode);
    gl.compileShader(fragmentShaderObject);
    if(gl.getShaderParameter(fragmentShaderObject,gl.COMPILE_STATUS)==false)
    {
        var error=gl.getShaderInfoLog(fragmentShaderObject);
        if(error.length>0)
        {
            alert("Fragment shader :"+error);
            uninitialize();
        }
    }

    shaderProgramObject=gl.createProgram();
    gl.attachShader(shaderProgramObject,vertexShaderObject);
    gl.attachShader(shaderProgramObject,fragmentShaderObject);

    // prelinking binding
    gl.bindAttribLocation(shaderProgramObject,webGLMacros.AAN_ATTRIBUTE_POSITION,"a_position");
    gl.bindAttribLocation(shaderProgramObject,webGLMacros.AAN_ATTRIBUTE_NORMAL,"a_normal");
    gl.linkProgram(shaderProgramObject);

    if(gl.getProgramParameter(shaderProgramObject,gl.LINK_STATUS)==false)
    {
        var error=gl.getProgramInfoLog(shaderProgramObject);
        if(error.length>0)
        {
            alert(error);
            uninitialize();
        }
    }

    modelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_modelMatrix");
	viewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_viewMatrix");
	projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projectionMatrix");

    laUniform[0] = gl.getUniformLocation(shaderProgramObject, "u_la[0]");
	laUniform[1] = gl.getUniformLocation(shaderProgramObject, "u_la[1]");
	laUniform[2] = gl.getUniformLocation(shaderProgramObject, "u_la[2]");
	
	ldUniform[0] = gl.getUniformLocation(shaderProgramObject, "u_ld[0]");
	ldUniform[1] = gl.getUniformLocation(shaderProgramObject, "u_ld[1]");
	ldUniform[2] = gl.getUniformLocation(shaderProgramObject, "u_ld[2]");
	
	lsUniform[0] = gl.getUniformLocation(shaderProgramObject, "u_ls[0]");
	lsUniform[1] = gl.getUniformLocation(shaderProgramObject, "u_ls[1]");
	lsUniform[2] = gl.getUniformLocation(shaderProgramObject, "u_ls[2]");
	
	lightPositionUniform[0] = gl.getUniformLocation(shaderProgramObject, "u_lightPosition[0]");
	lightPositionUniform[1] = gl.getUniformLocation(shaderProgramObject, "u_lightPosition[1]");
	lightPositionUniform[2] = gl.getUniformLocation(shaderProgramObject, "u_lightPosition[2]");

	kaUniform = gl.getUniformLocation(shaderProgramObject, "u_ka");
	kdUniform = gl.getUniformLocation(shaderProgramObject, "u_kd");
	ksUniform = gl.getUniformLocation(shaderProgramObject, "u_ks");
	materialShinenessUniform = gl.getUniformLocation(shaderProgramObject, "u_materialShininess");
	
	lightingEnabledUniform = gl.getUniformLocation(shaderProgramObject, "u_lightingEnabled");

    shaderEnabledUniform = gl.getUniformLocation(shaderProgramObject, "u_shaderEnabled");


    // Sphere initialization
    sphere=new Mesh();
    makeSphere(sphere,1.0,60,60);
 

    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
    
    // clear the screen by blue color
    gl.clearColor(0.0,0.0,0.0,1.0);

    perspectiveProjectionMatrix = mat4.create();


}

function resize()
{
    // code
    if(bFullScreen==true)
    {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }
    else
    {
        canvas.width=canvas_original_width;
        canvas.height=canvas_original_height;
    }

    if(canvas.height==0)
    {
        canvas.height=1;
    }

    gl.viewport(0,0,canvas.width,canvas.height);

    mat4.perspective(perspectiveProjectionMatrix,45.0,parseFloat(canvas.width)/parseFloat(canvas.height),0.1,100.0);
    
}
function display()
{
    //code

    gl.clear(gl.COLOR_BUFFER_BIT | gl.COLOR_BUFFER_BIT);
    gl.useProgram(shaderProgramObject);

    var modelMatrix=mat4.create();
    var viewMatrix=mat4.create();

    var translationMatrix=mat4.create();
    mat4.translate(translationMatrix, translationMatrix, [0.0, 0.0, -4.0] ); 
    modelMatrix=translationMatrix;
    gl.uniformMatrix4fv(modelMatrixUniform,false,modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform,false,viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform,false,perspectiveProjectionMatrix);

    if (bLight == true)
	{
		gl.uniform1i(lightingEnabledUniform, 1);
		gl.uniform1i(shaderEnabledUniform, enableShader);
        lights[0][3][0] = radius * Math.sin(lightAngleOne);
        lights[0][3][2] = radius * Math.cos(lightAngleOne);

        lights[1][3][1] = radius * Math.sin(lightAngleOne);
        lights[1][3][2] = radius * Math.cos(lightAngleOne);

        lights[2][3][0] = radius * Math.sin(lightAngleTwo);
        lights[2][3][1] = radius * Math.cos(lightAngleTwo);

        for (let i = 0; i < 3; i++) {

            gl.uniform3fv(laUniform[i], lights[i][0],0,3);              //lightAmbient
			gl.uniform3fv(ldUniform[i], lights[i][1],0,3);              //lightDiffuse
			gl.uniform3fv(lsUniform[i],lights[i][2],0,3);               //lightSpecular
			gl.uniform4fv(lightPositionUniform[i],lights[i][3],0,4);    //lightPosition
        }

        gl.uniform3fv(kaUniform, materialAmbient,0,3);
		gl.uniform3fv(kdUniform, materialDiffuse,0,3);
		gl.uniform3fv(ksUniform, materialSpecular,0,3);
		gl.uniform1f(materialShinenessUniform, materialShininess);

	}
	else
	{
		gl.uniform1i(lightingEnabledUniform, 0);
	}
    
    sphere.draw();
    gl.useProgram(null);

    update();

    // double buffering emulation 
    requestAnimationFrame(display,canvas);
}

function update()
{

    lightAngleZero = lightAngleZero+0.01;
    if(lightAngleZero >= 360.0)
    {
        lightAngleZero=0.0;
    }

    lightAngleOne = lightAngleOne+0.01;
    if(lightAngleOne >= 360.0)
    {
        lightAngleOne = 0.0;
    }

    lightAngleTwo = lightAngleTwo+0.01;
    if(lightAngleTwo >= 360.0)
    {
        lightAngleTwo = 0.0;
    }

}

function uninitialize()
{
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

    if(shaderProgramObject)
    {
        gl.useProgram(shaderProgramObject);
        var shaderObject=gl.getAttachedShaders(shaderProgramObject);
        for (let i = 0; i < shaderObject.length; i++) {
            gl.detachShader(shaderProgramObject,shaderObject[i]);
            gl.deleteShader(shaderObject[i]);
            shaderObject[i]=0;            
        }

        gl.useProgram(null);
        gl.deleteProgram(shaderProgramObject);
        shaderProgramObject=null;
    }


}

