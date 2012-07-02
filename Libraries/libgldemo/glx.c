/*
    libgldemo - General-purpose OpenGL demo backend
    glx.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
    Author(s): Fatbag <X-Fi6@phppoll.org>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "libgldemo.h"

static Display *display = NULL;
static int screen = 0;
static Window window = 0;
static GLXContext hrc = 0;

static uint8_t keys[256] = {0};
static uint8_t f11_pressed = 0;
static uint16_t WndWidth, WndHeight;
static uint16_t ResWidth, ResHeight;
static int fullscreen = 0;

void KillGLWindow()
{
    if(hrc){
        glXMakeCurrent(display, None, NULL);
        glXDestroyContext(display, hrc);
        hrc = 0;
    }

    if(window){
        XDestroyWindow(display, window);
        window = 0;
    }
}

static int CreateGLWindow(const char *__restrict title, uint16_t width, uint16_t height)
{
    int attlist[] = {
        GLX_RGBA,
        GLX_RED_SIZE, 4,
        GLX_GREEN_SIZE, 4,
        GLX_BLUE_SIZE, 4,
        GLX_DEPTH_SIZE, 16,
        GLX_DOUBLEBUFFER,
        None
    };
    XVisualInfo *visualinfo;
    Colormap cmap;
    Atom wmDelete;
    XSetWindowAttributes attr;
    int (APIENTRY *glXSwapIntervalSGIptr)(int);

    visualinfo = glXChooseVisual(display, screen, attlist);
    if(visualinfo == NULL){
        KillGLWindow();
        DemoErrorBox("Can't find a suitable pixel format.");
        return 0;
    }

    /* create a color map */
    cmap = XCreateColormap(display, RootWindow(display, visualinfo->screen), visualinfo->visual, AllocNone);
    attr.colormap = cmap;
    attr.border_pixel = 0;

    hrc = glXCreateContext(display, visualinfo, NULL, GL_TRUE);
    if(hrc == NULL){
        KillGLWindow();
        DemoErrorBox("Failed to create an OpenGL rendering context.");
        return 0;
    }

    attr.event_mask = KeyPressMask | KeyReleaseMask | StructureNotifyMask;
    if(fullscreen){
        attr.override_redirect = True;
        width = ResWidth;
        height = ResHeight;
        XWarpPointer(display, None, window, 0, 0, 0, 0, 0, 0);
    }else{
        attr.override_redirect = False;
    }

    if(!(window = XCreateWindow(display, XRootWindow(display, visualinfo->screen),
        (ResWidth - width)>>1,
        (ResHeight - height)>>1,
        width,
        height,
        0, visualinfo->depth, InputOutput, visualinfo->visual,
        CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect, &attr)
    )){
        KillGLWindow();
        DemoErrorBox("Window creation error.");
        return 0;
    }

    WndWidth = width;
    WndHeight = height;

    XSetStandardProperties(display, window, title, title, None, NULL, 0, NULL);
    XMapRaised(display, window);
    XGrabKeyboard(display, window, True, GrabModeAsync, GrabModeAsync, CurrentTime);
    wmDelete = XInternAtom(display, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(display, window, &wmDelete, 1);

    /* connect the glx-context to the window */
    glXMakeCurrent(display, window, hrc);
    XFlush(display);

    if(!Demo.InitGL()){
        KillGLWindow();
        DemoErrorBox("Initialization failed.");
        return 0;
    }
    if(!Demo.ResizeScene(width&&height ? width : 1, width&&height ? height : 1)){
        KillGLWindow();
        DemoErrorBox("Scene resize failed.");
        return 0;
    }

    glXSwapIntervalSGIptr = (int (APIENTRY *)(int)) glXGetProcAddressARB((const GLubyte *) "glXSwapIntervalSGI");
    if(glXSwapIntervalSGIptr) glXSwapIntervalSGIptr(1);

    return 1;
}

int main()
{
    int dotclock;
    XF86VidModeModeLine modeline;
    struct timespec time1;
    display = XOpenDisplay(0);
    if(display == NULL){
        KillGLWindow();
        DemoErrorBox("Failed to obtain the X11 display context.");
        return 0;
    }
    XkbSetDetectableAutoRepeat(display, True, 0);
    screen = XDefaultScreen(display);
    XF86VidModeGetModeLine(display, screen, &dotclock, &modeline);
    ResWidth = modeline.hdisplay;
    ResHeight = modeline.vdisplay;

    if(Demo.Startup && !Demo.Startup())
        return -1;

    if(!CreateGLWindow(Demo.Title, Demo.Width, Demo.Height))
        return -1;

    clock_gettime(CLOCK_REALTIME, &time1);

    while(1){
        struct timespec time2;
        float TimeDelta;

        while(XPending(display)){
            XEvent event;
            XNextEvent(display, &event);

            switch(event.type){

            case ConfigureNotify:
                if(((unsigned)event.xconfigure.width != WndWidth) ||
                    ((unsigned)event.xconfigure.height != WndHeight)){
                    WndWidth = event.xconfigure.width;
                    WndHeight = event.xconfigure.height;
                    Demo.ResizeScene(event.xconfigure.width, event.xconfigure.height);
                }
                break;

            case KeyPress: {
                KeySym key = XLookupKeysym(&event.xkey, 0);
                if(key <= 255)
                    keys[(key + 'A' - 'a') & 255]        = 1;
                else if(key == XK_Left)  keys[KEY_LEFT]  = 1;
                else if(key == XK_Up)    keys[KEY_UP]    = 1;
                else if(key == XK_Right) keys[KEY_RIGHT] = 1;
                else if(key == XK_Down)  keys[KEY_DOWN]  = 1;
                else{
                    if(key == XK_Escape){
                        KillGLWindow();
                        XCloseDisplay(display);
                        return (!Demo.Shutdown || Demo.Shutdown()) ? 0 : -1;
                    }
                    if(key == XK_F11 && !f11_pressed){
                        KillGLWindow();
                        fullscreen = !fullscreen;
                        CreateGLWindow(Demo.Title, Demo.Width, Demo.Height);
                        f11_pressed = 1;
                    }
                }
                break;
            }

            case KeyRelease: {
                KeySym key = XLookupKeysym(&event.xkey, 0);
                if(key <= 255)
                    keys[(key + 'A' - 'a') & 255]        = 0;
                else if(key == XK_Left)  keys[KEY_LEFT]  = 0;
                else if(key == XK_Up)    keys[KEY_UP]    = 0;
                else if(key == XK_Right) keys[KEY_RIGHT] = 0;
                else if(key == XK_Down)  keys[KEY_DOWN]  = 0;
                else if(key == XK_F11)   f11_pressed     = 0;
                break;
            }

            case ClientMessage:
                if (XGetAtomName(display, event.xclient.message_type)[0] == 'W'){
                    KillGLWindow();
                    XCloseDisplay(display);
                    return (!Demo.Shutdown || Demo.Shutdown()) ? 0 : -1;
                }
            }
        }

        /* Find the timedelta */
        clock_gettime(CLOCK_REALTIME, &time2);
        TimeDelta = (float)(time2.tv_sec - time1.tv_sec) + ((float)(time2.tv_nsec - time1.tv_nsec)) * 1e-9;
        if(TimeDelta < 0) TimeDelta = 0; /* Safe-guard in case of system delay */
        time1 = time2;

        /* Draw */
        Demo.DrawScene(TimeDelta, keys);
        glXSwapBuffers(display, window);
    }
}
