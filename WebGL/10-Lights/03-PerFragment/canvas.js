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

var vao;
var vbo;

var modelMatrixUniform;
var viewMatrixUniform;
var projectionMatrixUniform;

var perspectiveProjectionMatrix;

var sphere = null;

//RELATED TO LIGHT
var laUniform;
var ldUniform;
var lsUniform;
var lightPositionUniform;

var kaUniform;
var kdUniform;
var ksUniform;
var materialShininessUniform;

var lightingEnabledUniform;

var bLight = false;

var lightAmbient = [0.1, 0.1, 0.1];
var lightDiffuse = [1.0, 1.0, 1.0];
var lightSpecular = [1.0, 1.0, 1.0];
var lightPosition = [100.0, 100.0, 100.0, 1.0];

var materialAmbient = [0.0, 0.0, 0.0];
var materialDiffuse = [0.5, 0.2, 0.7];
var materialSpecular = [0.7, 0.7, 0.7];
var materialShininess = 128.0;

/* WHITE  

var materialAmbient = [0.0, 0.0, 0.0];
var materialDiffuse = [1.0, 1.0, 1.0];
var materialSpecular = [1.0, 1.0, 1.0];
var materialShininess = 50.0;
*/


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
    "uniform vec4 u_lightPosition;" +
    "uniform mediump int u_lightingEnabled;" +
    "out vec3 transformedNormals;" +
    "out vec3 lightDirection;" +
    "out vec3 viewerVector;" +
    "void main(void)" +
    "{" +
    "if (u_lightingEnabled == 1)" +
    "{" +
    "vec4 eyeCoordinates = u_viewMatrix * u_modelMatrix * a_position;" +
    "mat3 normalMatrix = mat3(u_viewMatrix * u_modelMatrix);" +
    "transformedNormals = normalMatrix * a_normal;" +
    "lightDirection = vec3(u_lightPosition - eyeCoordinates);" +
    "viewerVector = -eyeCoordinates.xyz;" +
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
    "in vec3 transformedNormals;" +
    "in vec3 lightDirection;" +
    "in vec3 viewerVector;" +
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
    "vec3 normalized_transformed_normals = normalize(transformedNormals);" +
    "vec3 normalized_light_direction = normalize(lightDirection);" +
    "vec3 diffuse = u_ld * u_kd * max(dot(normalized_light_direction, normalized_transformed_normals), 0.0);" +
    "vec3 reflectionVector = reflect(-normalized_light_direction, normalized_transformed_normals);" +
    "vec3 normalized_viewer_vector = normalize(viewerVector);" +
    "vec3 specular = u_ls * u_ks * pow(max(dot(reflectionVector, normalized_viewer_vector), 0.0), u_materialShininess);" +
    "phong_ads_light = ambient + diffuse + specular;" +
    "}" +
    "else" +
    "{" +
    "phong_ads_light = vec3(1.0, 1.0, 1.0);" +
    "}" + 
    "FragColor = vec4(phong_ads_light, 1.0);" +
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
    gl.bindAttribLocation(shaderProgramObject, webGLMacros.AAN_ATTRIBUTE_NORMAL, "a_normal");

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

    laUniform = gl.getUniformLocation(shaderProgramObject, "u_la");
    ldUniform = gl.getUniformLocation(shaderProgramObject, "u_ld");
    lsUniform = gl.getUniformLocation(shaderProgramObject, "u_ls");
    lightPositionUniform = gl.getUniformLocation(shaderProgramObject, "u_lightPosition");

    kaUniform = gl.getUniformLocation(shaderProgramObject, "u_ka");
    kdUniform = gl.getUniformLocation(shaderProgramObject, "u_kd");
    ksUniform = gl.getUniformLocation(shaderProgramObject, "u_ks");

    materialShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_materialShininess");
    lightingEnabledUniform = gl.getUniformLocation(shaderProgramObject, "u_lightingEnabled");

    //Declaration & Initialization of geometry

    sphere = new Mesh();
    makeSphere(sphere, 2.0, 30, 30);

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
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var translationMatrix = mat4.create();

    mat4.translate(translationMatrix, translationMatrix, [0.0, 0.0, -6.0]);
    modelMatrix = translationMatrix;

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if (bLight)
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
        gl.uniform1i(lightingEnabledUniform, 0);


    //Here is the drawing code

    sphere.draw();
    
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

    case 76: // L
        if (bLight == false)
            bLight = true;
        else
            bLight = false;
        break;  
    
    case 81: // Q
        uninitialize();
        window.close(); // Not Applicable For All Browsers
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

