// TODO(Rajat):
/*
  --Toggle Maximize
  --Text Rendering
  --Tilemap Rendering
  --GJK Collision Detection
  --Cleaning Up Platform layer
  --Fullscreen Toggle
  --Input System Cleaning
  --2D Renderer
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/keysymdef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

#include "glad/glad.c"
#include "ennov_platform.h"

#include <GL/glx.h>
#include <dlfcn.h>
#include <fcntl.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MEGABYTES_TO_BTYES(i) \
    (i * 1024 * 1024)

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

// TODO(Rajat): Build a software renderer later

struct x11_state {
    Display* Display_;
    Screen* Screen_;
    int32 ScreenID;
    Window Window_;
    GLXContext Context;
    Colormap WindowColormap;
    Atom AtomWmDeleteWindow;
    bool32 Running;
};

internal GLXFBConfig X11GetFrameBufferConfig(x11_state* State)
{
    GLint GlxAttributes[] = {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        None
    };

    int FBcount;
    GLXFBConfig* FbC = glXChooseFBConfig(State->Display_, State->ScreenID,
        GlxAttributes, &FBcount);

    int BestFbC = -1, WorstFbC = -1, BestNumSamples = -1, WorstNumSamples = 999;
    for (int i = 0; i < FBcount; ++i) {
        XVisualInfo* VisualInfo = glXGetVisualFromFBConfig(State->Display_, FbC[i]);
        if (VisualInfo != 0) {
            int SampBuf, Samples;
            glXGetFBConfigAttrib(State->Display_, FbC[i], GLX_SAMPLE_BUFFERS, &SampBuf);
            glXGetFBConfigAttrib(State->Display_, FbC[i], GLX_SAMPLES, &Samples);

            if (BestFbC < 0 || (SampBuf && Samples > BestNumSamples)) {
                BestFbC = i;
                BestNumSamples = Samples;
            }
            if (WorstFbC < 0 || !SampBuf || Samples < WorstNumSamples)
                WorstFbC = i;
            WorstNumSamples = Samples;
        }
        XFree(VisualInfo);
    }
    GLXFBConfig BestFb = FbC[BestFbC];
    XFree(FbC);

    return BestFb;
}

internal void X11Init(x11_state* State)
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
        50, 50, 800, 600, 5, Visual->depth, InputOutput, Visual->visual,
        CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &WindowAttribs);

    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");

    int ContextAttribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 2,
        GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        None
    };

    State->Context = glXCreateContextAttribsARB(State->Display_, FBConfig, 0, True, ContextAttribs);

    State->WindowColormap = WindowAttribs.colormap;

    // Verifying that context is a direct context
    if (!glXIsDirect(State->Display_, State->Context)) {
        fprintf(stderr, "Indirect GLX rendering context obtained\n");
    } else {
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

Status
ToggleMaximize(Display* display, Window window)
{
  XClientMessageEvent Event = {};
  Atom WmState = XInternAtom(display, "_NET_WM_STATE", False);
  Atom MaxH  =  XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
  Atom MaxV  =  XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);

  if(WmState == None) return 0;

  Event.type = ClientMessage;
  Event.format = 32;
  Event.window = window;
  Event.message_type = WmState;
  Event.data.l[0] = 2; // _NET_WM_STATE_TOGGLE 2 according to spec; Not defined in my headers
  Event.data.l[1] = MaxH;
  Event.data.l[2] = MaxV;
  Event.data.l[3] = 1;

  return XSendEvent(display, DefaultRootWindow(display), False,
                    SubstructureNotifyMask,
                    (XEvent *)&Event);
}

internal void X11ProcessEvents(x11_state* State, game_input* NewInput, game_state* GameState)
{

    local_persist XEvent Event;
    NewInput->Button  = {};

    while (XPending(State->Display_) > 0) {
        XNextEvent(State->Display_, &Event);
        switch (Event.type) {
        case MapNotify:
            printf("Using OpenGL : %s\n", glGetString(GL_VERSION));
            break;
        case Expose:
            local_persist XWindowAttributes WindowAttribs;
            XGetWindowAttributes(State->Display_, State->Window_, &WindowAttribs);
            glViewport(0, 0, WindowAttribs.width, WindowAttribs.height);
            GameState->ContextAttribs.Width = WindowAttribs.width;
            GameState->ContextAttribs.Height = WindowAttribs.height;
            break;
        case MappingNotify:
            XRefreshKeyboardMapping(&Event.xmapping);
            break;
            break;
        case ClientMessage:
            if (Event.xclient.data.l[0] == State->AtomWmDeleteWindow)
                State->Running = false;
            break;
        case DestroyNotify:
            State->Running = false;
        case KeyPress:
            break;
        case KeyRelease: {
            bool32 IsDown = (Event.xkey.type == KeyPress);
            KeySym Key = XLookupKeysym(&Event.xkey, 0);

            if (Key == XK_s) {
                NewInput->Button.S.EndedDown = true;
                ++(NewInput->Button.S.Repeat);
            }
            if (Key == XK_a) {
                NewInput->Button.A.EndedDown = true;
                ++(NewInput->Button.A.Repeat);
            }
            if (Key == XK_Tab) {
                NewInput->Button.Start.EndedDown = true;
                ++(NewInput->Button.Start.Repeat);
            }
            if (Key == XK_Down) {
                NewInput->Button.MoveDown.EndedDown = true;
                ++(NewInput->Button.Start.Repeat);
            }
            if (Key == XK_Up) {
                NewInput->Button.MoveUp.EndedDown = true;
                ++(NewInput->Button.Start.Repeat);
            }
            if (Key == XK_Left) {
                NewInput->Button.MoveLeft.EndedDown = true;
                ++(NewInput->Button.Start.Repeat);
            }
            if (Key == XK_Right) {
                NewInput->Button.MoveRight.EndedDown = true;
                ++(NewInput->Button.Start.Repeat);
            }
            if(Key == XK_space) {
              NewInput->Button.Start.EndedDown = true;
              ++(NewInput->Button.Start.Repeat);
            }
            #ifdef ENNOV_DEBUG
            if (Key == XK_Tab) {
                State->Running = false;
            }
            #endif //ENNOV_DEBUG
        } break;
        case ButtonPress:
            break;
        case ButtonRelease:
            break;
        case EnterNotify: {
            XEnterWindowEvent MouseIn = (XEnterWindowEvent)Event.xcrossing;
            NewInput->Cursor.X = (real32)MouseIn.x;
            NewInput->Cursor.Y = (real32)MouseIn.y;
        }   break;
        case LeaveNotify: {
            NewInput->Cursor.X = -100.0f;
            NewInput->Cursor.Y = -100.0f;
        } break;
        case MotionNotify: {
            XMotionEvent MouseMove = Event.xmotion;                       
            NewInput->Cursor.X = (real32)MouseMove.x;
            NewInput->Cursor.Y = (real32)MouseMove.y;
        }
            break;
        }
    }
}

loaded_bitmap* PlatformLoadBitmapFrom(char* file)
{
    int TexWidth, TexHeight, Channels;
    uint8* Pixels = stbi_load(file, &TexWidth, &TexHeight, &Channels, 0);

    if(Pixels) {
        loaded_bitmap* NewBitmap = (loaded_bitmap*)malloc(sizeof(loaded_bitmap));
        NewBitmap->Width = TexWidth;
        NewBitmap->Height = TexHeight;
        NewBitmap->Channels = Channels;
        NewBitmap->Pixels = Pixels;
        return NewBitmap;
    }
    else {
        // TODO(Rajat): Logging
    }
    return NULL;
}


/* struct game_memory 
 * {
 *     uint32 Size;
 *     void* Data;
 *     void* Transform;
 * };
 */

int main(int argc, char* argv[])
{
    x11_state State = {};

    X11Init(&State);

    // NOTE(Rajat): Always load OpenGL after creating a context and making it current
    gladLoadGL();

    glm::mat4 Transform = glm::ortho(0.0f, 600.0f, 400.0f, 0.0f, -1.0f, 1.0f);

    float* MatrixArray = glm::value_ptr(Transform);
    for(int i = 0; i < 16; ++i)
    {
        fprintf(stderr, "Value: %f ", MatrixArray[i]);
        if(i == 3 || i == 7 || i == 11 || i == 15)
        {
            fprintf(stderr, "\n");
        }
    }

    void* GameLibrary = NULL;
    void (*Foo)(void);

    char* GameMemory = Xpermalloc(MEGABYTES_TO_BTYES(200));

    GameLibrary = dlopen("./ennov.so", RTLD_LAZY);
    if (!GameLibrary) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    void (*GameUpdateAndRender)(game_state *State, game_input *Input) = (void (*)(game_state *State, game_input *Input)) 
                                                                         dlsym(GameLibrary, "GameUpdateAndRender");

    game_input Input[2] = {};
    game_input* OldInput = &Input[0];
    game_input* NewInput = &Input[1];
    NewInput->Cursor = {-100.0f, -100.0f};

    game_state GameState = {};

    GameState.Interface.PlatformLoadBitmapFrom = PlatformLoadBitmapFrom;

    while (State.Running) {
        X11ProcessEvents(&State, NewInput, &GameState);

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.5f, 0.5f, 0.5f, 0.5f);

        GameUpdateAndRender(&GameState, NewInput);

        glXSwapBuffers(State.Display_, State.Window_);
    }

    // NOTE(Rajat): Don't forget to free resources after use
    glXDestroyContext(State.Display_, State.Context);
    XFreeColormap(State.Display_, State.WindowColormap);
    XDestroyWindow(State.Display_, State.Window_);
    XCloseDisplay(State.Display_);

    return 0;
}
