#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "ennov_platform.h"
#include "ennov_gl.cpp"

#include <GL/glx.h>
#include <dlfcn.h>
#include <fcntl.h>



typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

// TODO(Rajat): Build a software renderer later

global_variable GLint GlxAttributes[] =
{
	    GLX_X_RENDERABLE    , True,
		GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
		GLX_RENDER_TYPE     , GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
		GLX_RED_SIZE        , 8,
		GLX_GREEN_SIZE      , 8,
		GLX_BLUE_SIZE       , 8,
		GLX_ALPHA_SIZE      , 8,
		GLX_DEPTH_SIZE      , 24,
		GLX_STENCIL_SIZE    , 8,
		GLX_DOUBLEBUFFER    , True,
		None
};

global_variable real32 Verticies[] = 
{
    -0.5f, -0.5f,
    -0.5f,  0.5f,
     0.5f,  0.5f,
     0.5f, -0.5f
};

global_variable uint32 Indicies[] =
{
    0, 1, 2,
    2, 3, 0
};

global_variable char* VertexShaderSource = 
{
    R"(
    #version 420 core

    in vec2 Pos;

    void main()
    {
        gl_Position = vec4(Pos, 0.0f, 1.0f);
    }
    )"
};

global_variable char* FragmentShaderSource = 
{
    R"(
    #version 420 core
    out vec4 Color;

    void main()
    {
        Color = vec4(0.5f, 0.3f, 0.7f, 1.0f);
    }
    )"
};

global_variable GLuint VertexBuffer;
global_variable GLuint IndexBuffer;
global_variable GLuint ShaderProgram;
global_variable GLuint VertexArray;

global_variable game_input GameInput;

void InitGL()
{
    glGenVertexArrays(1, &VertexArray);
    glBindVertexArray(VertexArray);

    GLbitfield MapMask = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    GLbitfield CreateMask = MapMask | GL_DYNAMIC_STORAGE_BIT;

    glGenBuffers(1, &VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(Verticies), Verticies, CreateMask);

    glGenBuffers(1, &IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indicies), Indicies, GL_STATIC_DRAW);

    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
    glCompileShader(VertexShader);

    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentShaderSource, NULL);
    glCompileShader(FragmentShader);

    ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, VertexShader);
    glAttachShader(ShaderProgram, FragmentShader);
    glLinkProgram(ShaderProgram);
    glValidateProgram(ShaderProgram);

    glUseProgram(ShaderProgram);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
}

int main(int argc, char* argv[])
{
    Display* Display_ = XOpenDisplay(NULL);
    Screen* Screen_ = DefaultScreenOfDisplay(Display_);
    int ScreenNumber = DefaultScreen(Display_);

    int FBcount;
    GLXFBConfig* FbC = glXChooseFBConfig(Display_, ScreenNumber, 
    	                GlxAttributes, &FBcount);

    int BestFbC = -1, WorstFbC = -1, BestNumSamples = -1, WorstNumSamples = 999;
    for (int i = 0; i < FBcount; ++i) {
        XVisualInfo *VisualInfo = glXGetVisualFromFBConfig( Display_, FbC[i] );
        if ( VisualInfo != 0) {
            int SampBuf, Samples;
            glXGetFBConfigAttrib( Display_, FbC[i], GLX_SAMPLE_BUFFERS, &SampBuf );
            glXGetFBConfigAttrib( Display_, FbC[i], GLX_SAMPLES       , &Samples  );

            if ( BestFbC < 0 || (SampBuf && Samples > BestNumSamples) ) {
                BestFbC = i;
                BestNumSamples = Samples;
            }
            if ( WorstFbC < 0 || !SampBuf || Samples < WorstNumSamples )
                WorstFbC = i;
            WorstNumSamples = Samples;
        }
        XFree(VisualInfo);
    }
    GLXFBConfig BestFb = FbC[BestFbC];
    XFree(FbC);

    XVisualInfo* Visual = glXGetVisualFromFBConfig(Display_, BestFb);

    XSetWindowAttributes WindowAttribs;
    WindowAttribs.border_pixel = BlackPixel(Display_, ScreenNumber);
    WindowAttribs.background_pixel = WhitePixel(Display_, ScreenNumber);
    WindowAttribs.override_redirect = true;
    WindowAttribs.colormap = XCreateColormap(Display_, RootWindow(Display_, ScreenNumber), 
    	                                    Visual->visual, AllocNone);
    WindowAttribs.event_mask = ExposureMask;

    Window Window_ = XCreateWindow(Display_, RootWindowOfScreen(Screen_), 50, 50, 600, 400, 5, 
    	                   Visual->depth, InputOutput, Visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &WindowAttribs);

    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );


	int ContextAttribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
		GLX_CONTEXT_MINOR_VERSION_ARB, 2,
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		None
	};

	GLXContext Context = 0;
	Context = glXCreateContextAttribsARB(Display_, BestFb, 0, True, ContextAttribs);

    // Verifying that context is a direct context
    if (!glXIsDirect (Display_, Context)) {
        fprintf(stderr, "Indirect GLX rendering context obtained\n");
    }
    else {
        fprintf(stderr, "Direct GLX rendering context obtained\n");
    }
    glXMakeCurrent(Display_, Window_, Context);

    // NOTE(Rajat): Always load OpenGL after creating a context and making it current

    gladLoadGL();

	XSync(Display_, False);


    XStoreName(Display_, Window_, "Hello X11!");

    Atom AtomWmDeleteWindow = XInternAtom(Display_, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(Display_, Window_, &AtomWmDeleteWindow, 1);

    XClearWindow(Display_, Window_);
    XMapRaised(Display_, Window_);

    bool Running = true;
    XEvent Event;

    uint KeyboardEventMasks = KeyPressMask | KeyReleaseMask | KeymapStateMask;
    uint MouseEventMasks = PointerMotionMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask;
    uint EventMasks = KeyboardEventMasks | MouseEventMasks | ExposureMask | StructureNotifyMask;

    XSelectInput(Display_, Window_, EventMasks);

    char Str[25] = {0}; 
    KeySym KeySym_ = 0;
    int Len = 0;

    void* GameLibrary = NULL;
    void(*Foo)(void);

    GameLibrary = dlopen("./libEnnov.so", RTLD_LAZY);
    if(!GameLibrary) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    void(*GameUpdateAndRender)(game_input* Input) = (void(*)(game_input* Input))dlsym(GameLibrary, "GameUpdateAndRender");

    InitGL();

    while(Running)
    {
    	XNextEvent(Display_, &Event);
    	switch(Event.type)
    	{
            case MapNotify:
                printf("Using OpenGL : %s\n", glGetString(GL_VERSION));
                break;
    		case Expose:
    			local_persist XWindowAttributes WindowAttribs;
    			XGetWindowAttributes(Display_, Window_, &WindowAttribs);
                glViewport(0, 0, WindowAttribs.width, WindowAttribs.height);
    			break;
    		case KeymapNotify:
    			XRefreshKeyboardMapping(&Event.xmapping);
    			break;
            case ClientMessage:
                if(Event.xclient.data.l[0] == AtomWmDeleteWindow)
                    Running = false;
                break;
            case DestroyNotify:
                Running = false;
    		case KeyPress:
    			if(KeySym_ == XK_Escape)
    				Running = false;
    			break;
    		case KeyRelease:
    		    KeySym_ = XLookupKeysym(&Event.xkey, 0);

                switch(KeySym_)
                {
                    case XK_Up:
                        GameInput.Controller.MoveUp.EndedDown = true;
                    break;
                    case XK_Down:
                        GameInput.Controller.MoveDown.EndedDown = true;
                    break;
                    case XK_Left:
                        GameInput.Controller.MoveLeft.EndedDown = true;
                    break;
                    case XK_Right:
                        GameInput.Controller.MoveRight.EndedDown = true;
                    break;
                    case XK_Escape:
                        GameInput.Controller.Terminate.EndedDown = true;
                    break;
                }
                break;
            case ButtonPress:
            	if(Event.xbutton.button == 1)
            		printf("Left mouse button pressed\n");
            	if(Event.xbutton.button == 3)
            		printf("Right mouse button pressed\n");
                break;
            case ButtonRelease:
            	if(Event.xbutton.button == 1)
            		printf("Left mouse button released\n");
            	if(Event.xbutton.button == 3)
            		printf("Right mouse button released\n");
                break;
            case EnterNotify:
            	printf("Pointer in the window\n");
            	break;
            case LeaveNotify:
            	printf("Pointer out of the window\n");
            	break;
            case MotionNotify:
            	local_persist int x, y;
            	x = Event.xmotion.x;
            	y = Event.xmotion.y;
            	printf("Pointer at (%i, %i) \n", x, y);
            	break;
    	}

        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VertexArray);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        GameUpdateAndRender(&GameInput);

    	glXSwapBuffers(Display_, Window_);
    }

    // NOTE(Rajat): Don't forget to free resources after use
    glXDestroyContext(Display_, Context);

    XFree(Visual);
    XFreeColormap(Display_, WindowAttribs.colormap);
    XDestroyWindow(Display_, Window_);
    XCloseDisplay(Display_);

    return 0;
}
