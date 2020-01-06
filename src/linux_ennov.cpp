#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "glad/glad.c"

#include <GL/glx.h>
#include <dlfcn.h>
#include <fcntl.h>

#define global_variable static
#define local_persist static
#define internal static

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

int main(int argc, char* argv[])
{
    Display* Display_ = XOpenDisplay(NULL);
    Screen* Screen_ = DefaultScreenOfDisplay(Display_);
    int ScreenNumber = DefaultScreen(Display_);

    int FBcount;
    GLXFBConfig* FbC = glXChooseFBConfig(Display_, ScreenNumber, 
    	                GlxAttributes, &FBcount);

    XVisualInfo* Visual = glXGetVisualFromFBConfig(Display_, FbC[0]);

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
	Context = glXCreateContextAttribsARB(Display_, FbC[0], 0, True, ContextAttribs);
    glXMakeCurrent(Display_, Window_, Context);

    // Note(Rajat): Always load OpenGL after creating a context and making it current

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
    			Len = XLookupString(&Event.xkey, Str, 25, &KeySym_, NULL);
    			if(Len > 0)
    				printf("Key Pressed: %s : %i\n", Str, Len);
    			if(KeySym_ == XK_Escape)
    				Running = false;
                if(KeySym_ == XK_Tab)
                {
                    GameLibrary = dlopen("./libEnnov.so", RTLD_LAZY);
                    if(!GameLibrary) {
                        fprintf(stderr, "%s\n", dlerror());
                        exit(EXIT_FAILURE);
                    }
                    Foo = (void(*)(void))dlsym(GameLibrary, "Foo");
                    if(!Foo) {
                        fprintf(stderr, "%s\n", dlerror());
                    }
                    Foo();
                    dlclose(GameLibrary);
                }
    			break;
    		case KeyRelease:
    		    Len = XLookupString(&Event.xkey, Str, 25, &KeySym_, NULL);
                if (Len > 0) {
                    printf("Key Released: %s\n", Str);
                }
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

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

    	glXSwapBuffers(Display_, Window_);
    }

    // Note(Rajat): Don't forget to free resources after use
    glXDestroyContext(Display_, Context);
    XDestroyWindow(Display_, Window_);
    XCloseDisplay(Display_);

    return 0;
}
