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

var vao;
var vbo;

var modelMatrixUniform;
var viewMatrixUniform;
var projectionMatrixUniform;

var perspectiveProjectionMatrix;


// Per-Vertex
//--------------------------------
var shaderProgramObject_pv;
var modelMatrixUniform_pv;
var viewMatrixUniform_pv;
var projectionMatrixUniform_pv;

var laUniform_pv;
var ldUniform_pv;
var lsUniform_pv;
var lightPositionUniform_pv;

var kaUniform_pv;
var kdUniform_pv;
var ksUniform_pv;
var materialShininessUniform_pv;

var lightingEnabledUniform_pv;
//---------------------------------

// Per-Fragment
// --------------------------------
var shaderProgramObject_pf;
var modelMatrixUniform_pf;
var viewMatrixUniform_pf;
var projectionMatrixUniform_pf;

var laUniform_pf;
var ldUniform_pf;
var lsUniform_pf;
var lightPositionUniform_pf;

var kaUniform_pf;
var kdUniform_pf;
var ksUniform_pf;
var materialShininessUniform_pf;

var lightingEnabledUniform_pf;
// ----------------------------------------------

var perspectiveProjectionMatrix;

var sphere = null;

var bLight = false;

var chosenShader = 'v';

var lightAmbient = [0.1, 0.1, 0.1];
var lightDiffuse = [1.0, 1.0, 1.0];
var lightSpecular = [1.0, 1.0, 1.0];
var lightPosition = [100.0, 100.0, 100.0, 1.0];

var materialAmbient = [0.0, 0.0, 0.0];
var materialDiffuse = [0.5, 0.2, 0.7];
var materialSpecular = [0.7, 0.7, 0.7];
var materialShininess = 128.0;


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



    // **PER-VERTEX**

    // ****************************************

    // Vertex Shader

    var vertexShaderSourceCode = 
    "#version 300 es" +
    "\n" +
    "in vec4 a_position;" +
    "in vec3 a_normal;" +
    "uniform mat4 u_modelMatrix;" +
    "uniform mat4 u_viewMatrix;" +
    "uniform mat4 u_projectionMatrix;" +
    "uniform vec3 u_la;" +
    "uniform vec3 u_ld;" +
    "uniform vec3 u_ls;" +
    "uniform vec4 u_lightPosition;" +
    "uniform vec3 u_ka;" +
    "uniform vec3 u_kd;" +
    "uniform vec3 u_ks;" +
    "uniform float u_materialShininess;" +
    "uniform mediump int u_lightingEnabled;" +
    "out vec3 phong_ads_light;" +
    "void main(void)" +
    "{" +
    "if (u_lightingEnabled == 1)" +
    "{" +
    "vec3 ambient = u_la * u_ka;" +
    "vec4 eyeCoordinates = u_viewMatrix * u_modelMatrix * a_position;" +
    "mat3 normalMatrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix)));" +
    "vec3 transformedNormals = normalize(normalMatrix * a_normal);" +
    "vec3 lightDirection = normalize(vec3(u_lightPosition - eyeCoordinates));" +
    "vec3 diffuse = u_ld * u_kd * max(dot(lightDirection, transformedNormals), 0.0);" +
    "vec3 reflectionVector = reflect(-lightDirection, transformedNormals);" +
    "vec3 viewerVector = normalize(-eyeCoordinates.xyz);" +
    "vec3 specular = u_ls * u_ks * pow(max(dot(reflectionVector, viewerVector), 0.0), u_materialShininess);" +
    "phong_ads_light = ambient + diffuse + specular;" +
    "}" +
    "else" +
    "{" +
    "phong_ads_light = vec3(1.0, 1.0, 1.0);" +
    "}" +
    "gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" +
    "}";

    var vertexShaderObject_pv = gl.createShader(gl.VERTEX_SHADER);

    gl.shaderSource(vertexShaderObject_pv,vertexShaderSourceCode);

    gl.compileShader(vertexShaderObject_pv);

    if(gl.getShaderParameter(vertexShaderObject_pv, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(vertexShaderObject_pv);
    
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
    "in vec3 phong_ads_light;" +
    "out vec4 FragColor;" +
    "void main(void)" +
	"{" +
    "FragColor = vec4(phong_ads_light, 1.0);" +
    "}";

    var fragmentShaderObject_pv = gl.createShader(gl.FRAGMENT_SHADER);

    gl.shaderSource(fragmentShaderObject_pv,fragmentShaderSourceCode);

    gl.compileShader(fragmentShaderObject_pv);

    if(gl.getShaderParameter(fragmentShaderObject_pv,gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(fragmentShaderObject_pv);

        if(error.length>0)
        {
            alert("Fragment Shader Compilation Error : \n" + error);
            uninitialize();
        }
    }

    //Shader Program Object
    shaderProgramObject_pv = gl.createProgram();
    
    gl.attachShader(shaderProgramObject_pv, vertexShaderObject_pv);
    gl.attachShader(shaderProgramObject_pv, fragmentShaderObject_pv);


    //Prelinking

    // Bind attribute location
    gl.bindAttribLocation(shaderProgramObject_pv,webGLMacros.AAN_ATTRIBUTE_POSITION,"a_position");
    gl.bindAttribLocation(shaderProgramObject_pv, webGLMacros.AAN_ATTRIBUTE_NORMAL, "a_normal");

    // Shader Program Linking
    gl.linkProgram(shaderProgramObject_pv);

    // Link Error Check
    if(gl.getProgramParameter(shaderProgramObject_pv,gl.LINK_STATUS) == false)
    {
        var log = gl.getProgramInfoLog(shaderProgramObject_pv);

        if(error.length>0)
        {
            alert("Per Vertex Shader Program Link Error : \n" + error);
            uninitialize();
        }
    }

    //Post Linking 
    modelMatrixUniform_pv = gl.getUniformLocation(shaderProgramObject_pv, "u_modelMatrix");
    viewMatrixUniform_pv = gl.getUniformLocation(shaderProgramObject_pv, "u_viewMatrix");
    projectionMatrixUniform_pv = gl.getUniformLocation(shaderProgramObject_pv, "u_projectionMatrix");

    laUniform_pv = gl.getUniformLocation(shaderProgramObject_pv, "u_la");
    ldUniform_pv = gl.getUniformLocation(shaderProgramObject_pv, "u_ld");
    lsUniform_pv = gl.getUniformLocation(shaderProgramObject_pv, "u_ls");
    lightPositionUniform_pv = gl.getUniformLocation(shaderProgramObject_pv, "u_lightPosition");

    kaUniform_pv = gl.getUniformLocation(shaderProgramObject_pv, "u_ka");
    kdUniform_pv = gl.getUniformLocation(shaderProgramObject_pv, "u_kd");
    ksUniform_pv = gl.getUniformLocation(shaderProgramObject_pv, "u_ks");

    materialShininessUniform_pv = gl.getUniformLocation(shaderProgramObject_pv, "u_materialShininess");
    lightingEnabledUniform_pv = gl.getUniformLocation(shaderProgramObject_pv, "u_lightingEnabled");


    // ****************************************

    // **PER-FRAGMENT**

    // ****************************************
    // Vertex Shader

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

    var vertexShaderObject_pf = gl.createShader(gl.VERTEX_SHADER);

    gl.shaderSource(vertexShaderObject_pf, vertexShaderSourceCode);

    gl.compileShader(vertexShaderObject_pf);

    if (gl.getShaderParameter(vertexShaderObject_pf, gl.COMPILE_STATUS))
    {
        var error = gl.getShaderInfoLog(vertexShaderObject_pf);

        if (error.length > 0)
        {
            alert("Per-Fragment Vertex Shader Compilation Error : \n" + error);
            uninitialize();
        }
    }
    
    
    // Fragment Shader
    
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

    var fragmentShaderObject_pf = gl.createShader(gl.FRAGMENT_SHADER);

    gl.shaderSource(fragmentShaderObject_pf, fragmentShaderSourceCode);

    gl.compileShader(fragmentShaderObject_pf);

    if (gl.getShaderParameter(fragmentShaderObject_pf, gl.COMPILE_STATUS))
    {
        var error = gl.getShaderInfoLog(fragmentShaderObject_pf);

        if (error.length > 0)
        {
            alert("Per-Fragment : Fragment Shader Compilation Error : \n" + error);
            uninitialize();
        }
    }
    

    // Shader Program Object
    shaderProgramObject_pf = gl.createProgram();

    gl.attachShader(shaderProgramObject_pf, vertexShaderObject_pf);
    gl.attachShader(shaderProgramObject_pf, fragmentShaderObject_pf);

    // Pre-Linking
    gl.bindAttribLocation(shaderProgramObject_pf, webGLMacros.AAN_ATTRIBUTE_POSITION, "a_position");
    gl.bindAttribLocation(shaderProgramObject_pf, webGLMacros.AAN_ATTRIBUTE_NORMAL, "a_normal");

    // Shader Program Linking
    gl.linkProgram(shaderProgramObject_pf);

    // Error Checking
    if (gl.getProgramParameter(shaderProgramObject_pf, gl.LINK_STATUS) == false)
    {
        var error = gl.getProgramInfoLog(shaderProgramObject_pf);

        if (error.length > 0)
        {
            alert("Per-Fragment : Shader Program Link Error : \n" + error);
            uninitialize();
        }
    }

    // Post-linking
    modelMatrixUniform_pf = gl.getUniformLocation(shaderProgramObject_pf, "u_modelMatrix");
    viewMatrixUniform_pf = gl.getUniformLocation(shaderProgramObject_pf, "u_viewMatrix");
    projectionMatrixUniform_pf = gl.getUniformLocation(shaderProgramObject_pf, "u_projectionMatrix");

    laUniform_pf = gl.getUniformLocation(shaderProgramObject_pf, "u_la");
    ldUniform_pf = gl.getUniformLocation(shaderProgramObject_pf, "u_ld");
    lsUniform_pf = gl.getUniformLocation(shaderProgramObject_pf, "u_ls");
    lightPositionUniform_pf = gl.getUniformLocation(shaderProgramObject_pf, "u_lightPosition");

    kaUniform_pf = gl.getUniformLocation(shaderProgramObject_pf, "u_ka");
    kdUniform_pf = gl.getUniformLocation(shaderProgramObject_pf, "u_kd");
    ksUniform_pf = gl.getUniformLocation(shaderProgramObject_pf, "u_ks");

    materialShininessUniform_pf = gl.getUniformLocation(shaderProgramObject_pf, "u_materialShininess");
    lightingEnabledUniform_pf = gl.getUniformLocation(shaderProgramObject_pf, "u_lightingEnabled");



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

    if (chosenShader == 'v')
        gl.useProgram(shaderProgramObject_pv);
    else
        gl.useProgram(shaderProgramObject_pf);

    //Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var translationMatrix = mat4.create();

    mat4.translate(translationMatrix, translationMatrix, [0.0, 0.0, -6.0]);
    modelMatrix = translationMatrix;

    if (chosenShader == 'v')
    {
        gl.uniformMatrix4fv(modelMatrixUniform_pv, false, modelMatrix);
        gl.uniformMatrix4fv(viewMatrixUniform_pv, false, viewMatrix);
        gl.uniformMatrix4fv(projectionMatrixUniform_pv, false, perspectiveProjectionMatrix);
    
        if (bLight)
        {
            gl.uniform1i(lightingEnabledUniform_pv, 1);
    
            gl.uniform3fv(laUniform_pv, lightAmbient);
            gl.uniform3fv(ldUniform_pv, lightDiffuse);
            gl.uniform3fv(lsUniform_pv, lightSpecular);
            gl.uniform4fv(lightPositionUniform_pv, lightPosition);
    
            gl.uniform3fv(kaUniform_pv, materialAmbient);
            gl.uniform3fv(kdUniform_pv, materialDiffuse);
            gl.uniform3fv(ksUniform_pv, materialSpecular);
            gl.uniform1f(materialShininessUniform_pv, materialShininess);
            
        }
        else
            gl.uniform1i(lightingEnabledUniform_pv, 0);
    }

    else if (chosenShader == 'f')
    {
        gl.uniformMatrix4fv(modelMatrixUniform_pf, false, modelMatrix);
        gl.uniformMatrix4fv(viewMatrixUniform_pf, false, viewMatrix);
        gl.uniformMatrix4fv(projectionMatrixUniform_pf, false, perspectiveProjectionMatrix);
    
        if (bLight)
        {
            gl.uniform1i(lightingEnabledUniform_pf, 1);
    
            gl.uniform3fv(laUniform_pf, lightAmbient);
            gl.uniform3fv(ldUniform_pf, lightDiffuse);
            gl.uniform3fv(lsUniform_pf, lightSpecular);
            gl.uniform4fv(lightPositionUniform_pf, lightPosition);
    
            gl.uniform3fv(kaUniform_pf, materialAmbient);
            gl.uniform3fv(kdUniform_pf, materialDiffuse);
            gl.uniform3fv(ksUniform_pf, materialSpecular);
            gl.uniform1f(materialShininessUniform_pf, materialShininess);
            
        }
        else
            gl.uniform1i(lightingEnabledUniform_pf, 0);
    }


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
       toggleFullscreen();
        break;

    case 70:
        chosenShader = 'f';
        break;

    case 76: // L
        if (bLight == false)
            bLight = true;
        else
            bLight = false;
        break;  

    case 86: 
        chosenShader = 'v';
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

     // Shader Uninitialization
     if (shaderProgramObject_pf)
     {
         gl.useProgram(shaderProgramObject_pf);
 
         var shaderObjects = gl.getAttachedShaders(shaderProgramObject_pf);
         
         for (let i = 0; i < shaderObjects; i++)
         {
             gl.detachShader(shaderProgramObject_pf, shaderObjects[i]);
             gl.deleteShader(shaderObjects[i]);
             shaderObjects[i] = 0;
         }
 
         gl.useProgram(null);
         gl.deleteProgram(shaderProgramObject_pf);
         shaderProgramObject_pf = null;
     }
 
     if (shaderProgramObject_pv)
     {
         gl.useProgram(shaderProgramObject_pv);
 
         var shaderObjects = gl.getAttachedShaders(shaderProgramObject_pv);
         
         for (let i = 0; i < shaderObjects; i++)
         {
             gl.detachShader(shaderProgramObject_pv, shaderObjects[i]);
             gl.deleteShader(shaderObjects[i]);
             shaderObjects[i] = 0;
         }
 
         gl.useProgram(null);
         gl.deleteProgram(shaderProgramObject_pv);
         shaderProgramObject_pv = null;
     }
}

