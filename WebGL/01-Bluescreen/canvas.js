var canvas = null;
var gl = null;          // this is Context internally
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;

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
    
    //Resize : This is Warmup resize
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

    //Clear screen with blue color
    gl.clearColor(0.0,0.0,1.0,1.0);
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

    gl.viewport(0,0,canvas.width,canvas.height);
}

function display()
{
    //code
    gl.clear(gl.COLOR_BUFFER_BIT);

    //Here should be call for update()

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
}
