// TODO(Rajat):
/*
  --2D Renderer
  --Multithreaded Asset Loading
  --Toggle Maximize and switching b/w modes
  --Text Rendering
  --Tilemap Rendering
  --GJK Collision Detection
  --Cleaning Up Platform layer
  --Fullscreen Toggle
  --Input System Cleaning
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysymdef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <pthread.h>

#include <sys/time.h>
#include <sys/mman.h>
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

internal Status
X11ToggleFullScreen(Display* Display_, Window Window_)
{
    XClientMessageEvent Event = {};
    Atom WmState = XInternAtom(Display_, "_NET_WM_STATE", False);
    Atom FullScreenToggle  =  XInternAtom(Display_, "_NET_WM_STATE_FULLSCREEN", False);

    if(WmState == None) return 0;

    Event.type = ClientMessage;
    Event.format = 32;
    Event.window = Window_;
    Event.message_type = WmState;
    Event.data.l[0] = 2; // _NET_WM_STATE_TOGGLE 2 according to spec; Not defined in my headers
    Event.data.l[1] = FullScreenToggle;
    Event.data.l[3] = 0l;

    return XSendEvent(Display_, DefaultRootWindow(Display_), False,
                      SubstructureNotifyMask,
                      (XEvent *)&Event);

}

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

internal void
X11ProcessButton(game_button_state* NewState, bool IsDown)
{
  if(NewState->EndedDown != IsDown) {
    NewState->EndedDown = IsDown;
    ++(NewState->Repeat);
  }
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
            if (Event.xclient.data.l[0] == State->AtomWmDeleteWindow) {
                State->Running = false;
                X11ProcessButton(&NewInput->Button.Terminate, true);
            }
            break;
        case DestroyNotify:
            State->Running = false;
        case KeyPress:
          {
            bool32 IsDown = (Event.xkey.type == KeyPress);
            KeySym Key = XLookupKeysym(&Event.xkey, 0);

            if (Key == XK_s) {
                X11ProcessButton(&NewInput->Button.S, IsDown);
            }
            if (Key == XK_a) {
                X11ProcessButton(&NewInput->Button.A, IsDown);
            }
            if (Key == XK_Down) {
                X11ProcessButton(&NewInput->Button.MoveDown, IsDown);
            }
            if (Key == XK_Up) {
                X11ProcessButton(&NewInput->Button.MoveUp, IsDown);
            }
            if (Key == XK_Left) {
                X11ProcessButton(&NewInput->Button.MoveLeft, IsDown);
            }
            if (Key == XK_Right) {
                X11ProcessButton(&NewInput->Button.MoveRight, IsDown);
            }
            if(Key == XK_space) {
                X11ProcessButton(&NewInput->Button.Start, IsDown);
            }
            #ifdef ENNOV_DEBUG
            if (Key == XK_Tab) {
                State->Running = false;
                X11ProcessButton(&NewInput->Button.Terminate, IsDown);
            }
            #endif //ENNOV_DEBUG
        } break;

            break;
        case KeyRelease:
            break;
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

internal loaded_bitmap*
PlatformLoadBitmapFrom(char* file)
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

struct thread_info
{
    pthread_t ThreadId;
    u32 ThreadIndex;
    char* StringToPrint;
};


// NOTE(rajat): Adopt style used in this function and leave everything else
// NOTE(rajat): Already configured spacemacs to use this style
internal void*
ThreadFunc(void* Arg)
{
    thread_info* Info = (thread_info*)Arg;
    for(;;)
    {
        fprintf(stderr, "Thread %i: %s\n", Info->ThreadIndex, Info->StringToPrint);
    }
    return NULL;
}

// Important(rajat): Read about Vsync and context creation in X11

int
main(int argc, char* argv[])
{
    thread_info WorkerThread[3] = {};
    WorkerThread[0].StringToPrint = "Hello from thread";
    WorkerThread[1].StringToPrint = "Lol!";
    WorkerThread[2].StringToPrint = "Sorry!";
    WorkerThread[0].ThreadIndex = 0;
    WorkerThread[1].ThreadIndex = 1;
    WorkerThread[2].ThreadIndex = 2;
    // pthread_create(&WorkerThread[0].ThreadId, NULL, ThreadFunc, &WorkerThread[0]);
    // pthread_create(&WorkerThread[1].ThreadId, NULL, ThreadFunc, &WorkerThread[1]);
    // pthread_create(&WorkerThread[2].ThreadId, NULL, ThreadFunc, &WorkerThread[2]);

    x11_state State = {};

    X11Init(&State);
    // X11ToggleFullScreen(State.Display_, State.Window_);

    // NOTE(Rajat): Always load OpenGL after creating a context and making it current
    gladLoadGL();

    void* GameLibrary = NULL;

    // TODO(Rajat): Query addresses for virtual allocating
    game_memory GameMemory = {};
    GameMemory.PermanentStorageSize = MEGABYTES_TO_BTYES(512);
    GameMemory.TransientStorageSize = MEGABYTES_TO_BTYES(256);
    GameMemory.PermanentStorage = mmap(0, GameMemory.PermanentStorageSize,
                                       PROT_READ|PROT_WRITE|PROT_EXEC,
                                       MAP_ANON|MAP_PRIVATE, 0, 0);
    GameMemory.TransientStorage = mmap(0, GameMemory.TransientStorageSize,
                                       PROT_READ|PROT_WRITE|PROT_EXEC,
                                       MAP_ANON|MAP_PRIVATE, 0, 0);
    GameMemory.IsInitialized = false;

    // TODO(Rajat): Not final game saving and loading system
    int SaveFileHandle = open("./GameState.txt", O_RDWR|O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    read(SaveFileHandle, GameMemory.PermanentStorage, MEGABYTES_TO_BTYES(1));
    close(SaveFileHandle);

    void (*GameUpdateAndRender)(game_memory* Memory, game_state *State, game_input *Input);

    game_input Input[2] = {};
    game_input* OldInput = &Input[0];
    game_input* NewInput = &Input[1];
    NewInput->Cursor = {-100.0f, -100.0f};

    game_state GameState = {};

    GameState.Interface.PlatformLoadBitmapFrom = PlatformLoadBitmapFrom;

    GameLibrary = dlopen("./ennov.so", RTLD_NOW);
    GameUpdateAndRender = (void(*)(game_memory* Memory, game_state* State, game_input* Input))dlsym(GameLibrary, "GameUpdateAndRender");

    timespec Time;
    f32 TimeInMs;

    f32 TimeNowInMs;
    f32 LastTimeInMs;

    f32 LastFrameElasped = 0;
    f32 Delta;

    while (State.Running) {
        clock_gettime(CLOCK_MONOTONIC, &Time);
        TimeInMs = (Time.tv_sec * 1.0e6 + Time.tv_nsec / 1.0e3);
        X11ProcessEvents(&State, NewInput, &GameState);

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        // GameLibrary = dlopen("./ennov.so", RTLD_NOW);
        // if (!GameLibrary) {
        //   sleep(1);
        //   GameLibrary = dlopen("./ennov.so", RTLD_NOW);
        //   fprintf(stderr, "%s\n", dlerror());
        // }


        // GameUpdateAndRender = (void(*)(game_memory* Memory, game_state* State, game_input* Input))dlsym(GameLibrary, "GameUpdateAndRender");

        GameUpdateAndRender(&GameMemory, &GameState, NewInput);

        glXSwapBuffers(State.Display_, State.Window_);

        timespec TimeNow;
        clock_gettime(CLOCK_MONOTONIC, &TimeNow);

        TimeNowInMs = (TimeNow.tv_sec * 1.0e6 + (TimeNow.tv_nsec / 1.0e3));

        LastTimeInMs = TimeNowInMs - TimeInMs;

        GameState.Delta = LastTimeInMs / 1.0e6;
        printf("%f\n", GameState.Delta);

        game_input* Temp;
        Temp = OldInput;
        OldInput = NewInput;
        NewInput = Temp;
        // dlclose(GameLibrary);
    }

    // TODO(Rajat): Not final save game functionality but the simplest and dumbest
    SaveFileHandle = open("./GameState.txt", O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    write(SaveFileHandle, GameMemory.PermanentStorage, MEGABYTES_TO_BTYES(2));
    close(SaveFileHandle);

    // NOTE(Rajat): Don't forget to free resources after use
    munmap(GameMemory.PermanentStorage, GameMemory.PermanentStorageSize);
    munmap(GameMemory.TransientStorage, GameMemory.TransientStorageSize);

    dlclose(GameLibrary);

    glXDestroyContext(State.Display_, State.Context);
    XFreeColormap(State.Display_, State.WindowColormap);
    XDestroyWindow(State.Display_, State.Window_);
    XCloseDisplay(State.Display_);

    return 0;
}
