/*
 *          Copyright 2020, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "opengl32.lib")

#define UNICODE

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <gl/GL.h>

#define fatal(fmt,...) do {                             \
    char buf[200];                                      \
    snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__);     \
    MessageBoxA(NULL, buf, "Fatal Error", MB_OK);       \
    abort();                                            \
} while (0)

static void draw()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2i(0, 1);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex2i(-1, -1);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex2i(1, -1);
    glEnd();
    glFlush();
}

static HDC global_hdc = NULL;
static HGLRC global_gl_context = NULL;

static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        draw();
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_SIZE:
        glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_CREATE: {
        global_hdc = GetDC(hWnd);
        if (!global_hdc)
            fatal("GetDC failed, error=%d", GetLastError());

        PIXELFORMATDESCRIPTOR pixelFormatDesc;
        memset(&pixelFormatDesc, 0, sizeof(PIXELFORMATDESCRIPTOR));
        pixelFormatDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pixelFormatDesc.nVersion = 1;
        pixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
        pixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
        pixelFormatDesc.cColorBits = 32;
        pixelFormatDesc.cAlphaBits = 8;
        pixelFormatDesc.cDepthBits = 24;

        int pixelFormat = ChoosePixelFormat(global_hdc, &pixelFormatDesc);
        if (!pixelFormat)
            fatal("ChoosePixelFormat failed, error=%d", GetLastError());
        if (!SetPixelFormat(global_hdc, pixelFormat, &pixelFormatDesc))
            fatal("SetPixelFormat failed, error=%d", GetLastError());
        global_gl_context = wglCreateContext(global_hdc);
        if (!global_gl_context)
            fatal("wglCreateContext failed, error=%d", GetLastError());
        wglMakeCurrent(global_hdc, global_gl_context);
        return 0;
    }
    case WM_DESTROY: {
        wglMakeCurrent(global_hdc, NULL);
        wglDeleteContext(global_gl_context);
        ReleaseDC(hWnd, global_hdc);
        DestroyWindow(hWnd);
        PostQuitMessage(0);
        break;
    }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int APIENTRY wWinMain(HINSTANCE inst, HINSTANCE pinst, LPWSTR cmdl, int show)
{
#define WINDOW_CLASS_NAME L"Opengl"

    {
        WNDCLASSEXW wcex;
        memset(&wcex, 0, sizeof(wcex));
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.lpfnWndProc = (WNDPROC)wndProc;
        wcex.hInstance = GetModuleHandleW(NULL);
        wcex.hIcon = LoadIcon(NULL, IDI_WINLOGO);
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = WINDOW_CLASS_NAME;
        wcex.hIconSm = NULL;

        if (!RegisterClassExW(&wcex))
            fatal("RegisterClass failed, error=%d", GetLastError());
    }

    {
        HWND hWnd = CreateWindowW(
            WINDOW_CLASS_NAME,
            L"OpenGL Example",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, // position
            640, 480,
            NULL,
            NULL,
            GetModuleHandle(NULL),
            NULL
        );
        if (!hWnd)
            fatal("CreateWindow failed, error={}", GetLastError());
        UpdateWindow(hWnd);
        ShowWindow(hWnd, SW_SHOWNORMAL);
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
