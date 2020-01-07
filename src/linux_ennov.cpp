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

struct x11_state
{
    Display* Display_;
    Screen* Screen_;
    int32 ScreenID;
    Window Window_;
    GLXContext Context;
    Colormap WindowColormap;
    Atom AtomWmDeleteWindow;
    bool32 Running;
};

GLXFBConfig X11GetFrameBufferConfig(x11_state* State)
{
    GLint GlxAttributes[] =
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

    int FBcount;
    GLXFBConfig* FbC = glXChooseFBConfig(State->Display_, State->ScreenID, 
                        GlxAttributes, &FBcount);

    int BestFbC = -1, WorstFbC = -1, BestNumSamples = -1, WorstNumSamples = 999;
    for (int i = 0; i < FBcount; ++i) {
        XVisualInfo *VisualInfo = glXGetVisualFromFBConfig( State->Display_, FbC[i] );
        if ( VisualInfo != 0) {
            int SampBuf, Samples;
            glXGetFBConfigAttrib( State->Display_, FbC[i], GLX_SAMPLE_BUFFERS, &SampBuf );
            glXGetFBConfigAttrib( State->Display_, FbC[i], GLX_SAMPLES       , &Samples  );

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

    return BestFb;
}

void X11Init(x11_state* State)
{
    // TODO(Rajat): Error checking and logging
    // TODO(Rajat): Separate Functionality
    State->Display_ = XOpenDisplay(NULL);
    State->Screen_ = DefaultScreenOfDisplay(State->Display_);
    State->ScreenID = DefaultScreen(State->Display_);

    GLXFBConfig FBConfig = X11GetFrameBufferConfig(State);

    XVisualInfo* Visual = glXGetVisualFromFBConfig(State->Display_, FBConfig);

    XSetWindowAttributes WindowAttribs;
    WindowAttribs.border_pixel = BlackPixel(State->Display_, State->ScreenID);
    WindowAttribs.background_pixel = WhitePixel(State->Display_, State->ScreenID);
    WindowAttribs.override_redirect = true;
    WindowAttribs.colormap = XCreateColormap(State->Display_, RootWindow(State->Display_, State->ScreenID), 
                                            Visual->visual, AllocNone);
    WindowAttribs.event_mask = ExposureMask;

    State->Window_ = XCreateWindow(State->Display_, RootWindowOfScreen(State->Screen_), 
                                   50, 50, 600, 400, 5, Visual->depth, InputOutput, Visual->visual, 
                                   CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &WindowAttribs);

    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );


    int ContextAttribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 2,
        GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        None
    };

    State->Context = glXCreateContextAttribsARB(State->Display_, FBConfig, 0, True, ContextAttribs);

    State->WindowColormap = WindowAttribs.colormap;

    // Verifying that context is a direct context
    if (!glXIsDirect (State->Display_, State->Context)) {
        fprintf(stderr, "Indirect GLX rendering context obtained\n");
    }
    else {
        fprintf(stderr, "Direct GLX rendering context obtained\n");
    }
    glXMakeCurrent(State->Display_, State->Window_, State->Context);

    XSync(State->Display_, False);
    XStoreName(State->Display_, State->Window_, "Hello X11!");

    State->AtomWmDeleteWindow = XInternAtom(State->Display_, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(State->Display_, State->Window_, &State->AtomWmDeleteWindow, 1);

    XClearWindow(State->Display_, State->Window_);
    XMapRaised(State->Display_, State->Window_);

    uint KeyboardEventMasks = KeyPressMask | KeyReleaseMask | KeymapStateMask;
    uint MouseEventMasks = PointerMotionMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask;
    uint EventMasks = KeyboardEventMasks | MouseEventMasks | ExposureMask | StructureNotifyMask;

    XSelectInput(State->Display_, State->Window_, EventMasks);

    State->Running = true;
    XFree(Visual);
}

void X11ProcessEvents(x11_state* State, game_input* NewInput)
{

    local_persist XEvent Event;
    (*NewInput) = {};

    XNextEvent(State->Display_, &Event);
    switch(Event.type)
    {
        case MapNotify:
            printf("Using OpenGL : %s\n", glGetString(GL_VERSION));
        break;
        case Expose:
            local_persist XWindowAttributes WindowAttribs;
            XGetWindowAttributes(State->Display_, State->Window_, &WindowAttribs);
            glViewport(0, 0, WindowAttribs.width, WindowAttribs.height);
        break;
        case KeymapNotify:
            XRefreshKeyboardMapping(&Event.xmapping);
        break;
        case ClientMessage:
            if(Event.xclient.data.l[0] == State->AtomWmDeleteWindow)
                State->Running = false;
        break;
        case DestroyNotify:
                State->Running = false;
        case KeyPress:
        break;
        case KeyRelease:
        {
            bool32 IsDown = (Event.xkey.type == KeyPress);
            KeySym Key = XLookupKeysym(&Event.xkey, 0);

            if(Key == XK_s)
            {
                NewInput->Button.S.EndedDown = true;
                ++(NewInput->Button.S.Repeat);
            }
            if(Key == XK_a)
            {
                NewInput->Button.A.EndedDown = true;
                ++(NewInput->Button.A.Repeat);
            }
            if(Key == XK_Tab)
            {
                NewInput->Button.Start.EndedDown = true;
                ++(NewInput->Button.Start.Repeat);
            }
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
}

internal void InitGL()
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
    x11_state State = {};

    X11Init(&State);

    // NOTE(Rajat): Always load OpenGL after creating a context and making it current
    gladLoadGL();

    void* GameLibrary = NULL;
    void(*Foo)(void);

    GameLibrary = dlopen("./libEnnov.so", RTLD_LAZY);
    if(!GameLibrary) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    void(*GameUpdateAndRender)(game_input* Input) = (void(*)(game_input* Input))dlsym(GameLibrary, "GameUpdateAndRender");

    InitGL();

    game_input Input[2] = {};
    game_input* OldInput = &Input[0];
    game_input* NewInput = &Input[1];

    while(State.Running)
    {
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VertexArray);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

    	X11ProcessEvents(&State, NewInput);
        GameUpdateAndRender(NewInput);

        game_input* Temp = OldInput;
        OldInput = NewInput;
        NewInput = Temp;
        

    	glXSwapBuffers(State.Display_, State.Window_);
    }

    // NOTE(Rajat): Don't forget to free resources after use
    glXDestroyContext(State.Display_, State.Context);
    XFreeColormap(State.Display_, State.WindowColormap);
    XDestroyWindow(State.Display_, State.Window_);
    XCloseDisplay(State.Display_);

    return 0;
}
