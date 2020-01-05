#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <stdio.h>
#include <stdint.h>

#define global_variable static
#define local_presist static
#define internal static

int main(int argc, char* argv[])
{
    Display* Display_ = XOpenDisplay(NULL);
    Screen* Screen_ = DefaultScreenOfDisplay(Display_);
    int ScreenNumber = DefaultScreen(Display_);

    Window Window_ = XCreateSimpleWindow(Display_, RootWindowOfScreen(Screen_), 50, 50, 600, 400, 5, 
    	                   BlackPixel(Display_, ScreenNumber), WhitePixel(Display_, ScreenNumber));

    XStoreName(Display_, Window_, "Hello X11!");

    XClearWindow(Display_, Window_);
    XMapRaised(Display_, Window_);

    bool Running = true;
    XEvent Event;

    uint KeyboardEventMasks = KeyPressMask | KeyReleaseMask | KeymapStateMask;
    uint MouseEventMasks = PointerMotionMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask;
    uint EventMasks = KeyboardEventMasks | MouseEventMasks | ExposureMask;

    XSelectInput(Display_, Window_, EventMasks);

    char Str[25] = {0}; 
    KeySym KeySym_ = 0;
    int Len = 0;

    while(Running)
    {
    	XNextEvent(Display_, &Event);
    	switch(Event.type)
    	{
    		case Expose:
    			local_presist XWindowAttributes WindowAttribs;
    			XGetWindowAttributes(Display_, Window_, &WindowAttribs);
    			printf("Window resized: (%i, %i)\n", WindowAttribs.width, WindowAttribs.height);
    			break;
    		case KeymapNotify:
    			XRefreshKeyboardMapping(&Event.xmapping);
    			break;
    		case KeyPress:
    			Len = XLookupString(&Event.xkey, Str, 25, &KeySym_, NULL);
    			if(Len > 0)
    				printf("Key Pressed: %s : %i\n", Str, Len);
    			if(KeySym_ == XK_Escape)
    				Running = false;
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
            	local_presist int x, y;
            	x = Event.xmotion.x;
            	y = Event.xmotion.y;
            	printf("Pointer at (%i, %i) \n", x, y);
            	break;
    	}
    }

    XDestroyWindow(Display_, Window_);
    XCloseDisplay(Display_);

    return 0;
}
