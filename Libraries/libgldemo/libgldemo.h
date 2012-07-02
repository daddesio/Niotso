/*
    libgldemo - General-purpose OpenGL demo backend
    libgldemo.h - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

typedef struct {
    const char *__restrict Title;
    uint16_t Width, Height;

    int (* Startup)(void);
    int (* Shutdown)(void);
    int (* InitGL)(void);
    int (* ResizeScene)(uint16_t width, uint16_t height);
    int (* DrawScene)(float TimeDelta, uint8_t keys[256]);
} DemoConfig;

#ifdef __cplusplus
extern "C" {
#endif
    extern const DemoConfig Demo;
#ifdef __cplusplus
}
#endif

#ifdef _WIN32
    #include <windows.h>

    #define DemoMessageBox(x) MessageBox(NULL, x, NULL, MB_OK)
    #define DemoErrorBox(x) MessageBox(NULL, x, NULL, MB_OK | MB_ICONERROR)

    #define KEY_LEFT VK_LEFT
    #define KEY_UP VK_UP
    #define KEY_RIGHT VK_RIGHT
    #define KEY_DOWN VK_DOWN
#else
    #define POSIX_C_SOURCE 199309L
    #include <stdio.h>
    #include <time.h>
    #include <GL/glx.h>
    #include <X11/extensions/xf86vmode.h>
    #include <X11/XKBlib.h>
    #include <X11/keysym.h>

    #define DemoMessageBox(x) fprintf(stdout, "%s\n", x)
    #define DemoErrorBox(x) fprintf(stderr, "%s\n", x)

    #define KEY_LEFT 0x25
    #define KEY_UP 0x26
    #define KEY_RIGHT 0x27
    #define KEY_DOWN 0x28
#endif
