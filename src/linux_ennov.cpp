// TODO(Rajat):
/*
  --Text Rendering
  --GJK Collision Detection (It is not necessary to use this)
  --Cleaning Up Platform layer
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
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include "glad/glad.c"
#include "ennov_platform.h"

#include <GL/glx.h>
#include <GL/glxext.h>
#include <dlfcn.h>
#include <fcntl.h>




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

internal_ Status
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

internal_ GLXFBConfig X11GetFrameBufferConfig(x11_state* State)
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

internal_ void X11Init(x11_state* State)
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

internal_ void
X11ProcessButton(game_button_state* NewState, bool IsDown)
{
  if(NewState->EndedDown != IsDown) {
    NewState->EndedDown = IsDown;
    ++(NewState->Repeat);
  }
}

internal_ void X11ProcessEvents(x11_state* State, game_input* NewInput, game_state* GameState)
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
            if(Key == XK_q)
            {
                X11ProcessButton(&NewInput->Button.Select, IsDown);
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

internal_ game_file*
PlatformLoadFile(char* File, void*(Alloc)(game_areana*, memory_index), game_areana* Areana)
{
    if(Alloc)
    {
        int FileHandle;
        struct stat FileState;

        FileHandle = open(File, O_RDONLY);
        fstat(FileHandle, &FileState);

        void* WriteAddress = Alloc(Areana, FileState.st_size);
        read(FileHandle, WriteAddress, FileState.st_size);

        game_file* File = (game_file*)Alloc(Areana, sizeof(game_file));
        File->Data = WriteAddress;
        File->Size = FileState.st_size;

        return File;
    }
    return NULL;
}

// NOTE(rajat): Adopt style used in this function and leave everything else
// NOTE(rajat): Already configured spacemacs to use this style
internal_ void*
ThreadFunc(void* Arg)
{
    for(;;)
    {
        // struct stat FileStat;
        // FileHandle = open(WorkQueue->Queue, O_RDONLY);
        // fstat(FileHandle, &FileStat);

        // // TODO(rajat): Introduce another semaphore for WorkQueue.Count to maximize parallel
        // // processing with more threads
        // Assert(WorkQueue->BufferOffset + FileStat.st_size < WorkQueue->BufferSize);
        // read(FileHandle, WorkQueue->BufferToWrite + WorkQueue->BufferOffset, FileStat.st_size);

        // WorkQueue->BufferOffset += FileStat.st_size;
        // sem_post(&WorkMutex);
    }
    return NULL;
}

// Important(rajat): Read about Vsync and context creation in X11

int
main(int argc, char* argv[])
{
    // sem_init(&WorkMutex, 0, 1);
    pthread_t WorkerThread[2] = {};
    // pthread_create(&WorkerThread[0], NULL, ThreadFunc, &WorkQueue);
    // pthread_create(&WorkerThread[1], NULL, ThreadFunc, &WorkQueue);

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
    GameMemory.AssetMemorySize = MEGABYTES_TO_BTYES(100);
    GameMemory.PermanentStorage = mmap(0, GameMemory.PermanentStorageSize,
                                       PROT_READ|PROT_WRITE|PROT_EXEC,
                                       MAP_ANON|MAP_PRIVATE, 0, 0);
    GameMemory.TransientStorage = mmap((void*)(1 << 10), GameMemory.TransientStorageSize,
                                       PROT_READ|PROT_WRITE|PROT_EXEC,
                                       MAP_ANON|MAP_PRIVATE, 0, 0);
    GameMemory.AssetMemory = mmap((void*)(1 << 20), GameMemory.AssetMemorySize,
                                  PROT_READ|PROT_WRITE|PROT_EXEC,
                                  MAP_ANON|MAP_PRIVATE, 0, 0);
    GameMemory.IsInitialized = false;

    // TODO(Rajat): Not final game saving and loading system
    int SaveFileHandle = open("./GameState.txt", O_RDWR|O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);

    // TODO(rajat): Consider using directory operations and lseek operations
    // TODO(rajat): Check the file exists or has read or write permissions
    struct stat SaveFileStat;
    fstat(SaveFileHandle, &SaveFileStat);

    read(SaveFileHandle, GameMemory.PermanentStorage, SaveFileStat.st_size);
    close(SaveFileHandle);

    void (*GameUpdateAndRender)(game_memory* Memory, game_state *State, game_input *Input, u32 *ConfigBits);
    game_input Input[2] = {};
    game_input* OldInput = &Input[0];
    game_input* NewInput = &Input[1];
    NewInput->Cursor = {-100.0f, -100.0f};

    game_state GameState = {};

    GameState.Interface.PlatformLoadFile = PlatformLoadFile;

    GameLibrary = dlopen("./ennov.so", RTLD_LAZY);
    GameUpdateAndRender = (void(*)(game_memory* Memory, game_state* State, game_input* Input, u32 *ConfigBits))dlsym(GameLibrary, "GameUpdateAndRender");

    timespec LastTime;
    f32 LastTimeInMs;

    // TODO(rajat): Query support for realtime and monotonic clocks
    clock_gettime(CLOCK_MONOTONIC, &LastTime);
    LastTimeInMs = ((f32)LastTime.tv_sec * 1.0e3f) + ((f32)LastTime.tv_nsec / 1.0e6f);

    f32 Delta;

    PFNGLXSWAPINTERVALMESAPROC glXSwapIntervalMESA;
    glXSwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalMESA");

    // TODO(rajat): Do proper Query before using this
    glXSwapIntervalMESA(1);

    // TODO(rajat): Not optimal way to doing this, will be replaced soon
    u32 ConfigBits = 0;

    glEnable(GL_CULL_FACE);

    while (State.Running) {
        X11ProcessEvents(&State, NewInput, &GameState);

        // GameLibrary = dlopen("./ennov.so", RTLD_NOW);
        // if (!GameLibrary) {
        //   sleep(1);
        //   GameLibrary = dlopen("./ennov.so", RTLD_NOW);
        //   fprintf(stderr, "%s\n", dlerror());
        // }


        // GameUpdateAndRender = (void(*)(game_memory* Memory, game_state* State, game_input* Input))dlsym(GameLibrary, "GameUpdateAndRender");

        GameUpdateAndRender(&GameMemory, &GameState, NewInput, &ConfigBits);
        glCullFace(GL_BACK);
        glXSwapBuffers(State.Display_, State.Window_);

        if(ConfigBits == PlatformFullScreenToggle_BIT)
        {
            X11ToggleFullScreen(State.Display_, State.Window_);
        }

        ConfigBits = 0;

        timespec CurrentTimeSpec;
        clock_gettime(CLOCK_MONOTONIC, &CurrentTimeSpec);

        f32 CurrentTimeInMs = ((f32)CurrentTimeSpec.tv_sec * 1.0e3f) + ((f32)CurrentTimeSpec.tv_nsec / 1.0e6f);

        // NOTE(rajat): Delta value will be in deciseconds not seconds
        GameState.Delta = (CurrentTimeInMs - LastTimeInMs) / 1.0e2f;
        // printf("%f\n", GameState.Delta * 1.0e2f);
        // printf("%f\n", 1000/(GameState.Delta * 1.0e2f));

        LastTimeInMs = CurrentTimeInMs;

        game_input* Temp;
        Temp = OldInput;
        OldInput = NewInput;
        NewInput = Temp;
        // dlclose(GameLibrary);
    }

    // TODO(Rajat): Not final save game functionality but the simplest and dumbest
    SaveFileHandle = open("./GameState.txt", O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    write(SaveFileHandle, GameMemory.PermanentStorage, MEGABYTES_TO_BTYES(1));
    close(SaveFileHandle);

    // NOTE(Rajat): Don't forget to free resources after use
    munmap(GameMemory.PermanentStorage, GameMemory.PermanentStorageSize);
    munmap(GameMemory.TransientStorage, GameMemory.TransientStorageSize);
    munmap(GameMemory.AssetMemory, GameMemory.AssetMemorySize);

    dlclose(GameLibrary);

    glXDestroyContext(State.Display_, State.Context);
    XFreeColormap(State.Display_, State.WindowColormap);
    XDestroyWindow(State.Display_, State.Window_);
    XCloseDisplay(State.Display_);

    return 0;
}
