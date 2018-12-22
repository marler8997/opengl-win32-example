/*
 *          Copyright 2018, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include "stdafx.h"


HDC     hDC;				/* device context */
HGLRC   hRC;				/* opengl context */
HWND    hWnd;				/* window */

char MyWindowClassName[7] = "OpenGL";

void MessageToString(char * str, size_t strSize, UINT uMsg)
{
	sprintf_s(str, strSize, "message ");
	switch (uMsg)
	{
		case WM_PRINT: sprintf_s(str + 8, strSize - 8, "%d (WM_PRINT)\n", (int)(uMsg)); break;
		case WM_PRINTCLIENT: sprintf_s(str + 8, strSize - 8, "%d (WM_PRINTCLIENT)\n", (int)(uMsg)); break;
		case WM_PAINT: sprintf_s(str + 8, strSize - 8, "%d (WM_PAINT)\n", (int)(uMsg)); break;
		case WM_SIZE: sprintf_s(str + 8, strSize - 8, "%d (WM_SIZE)\n", (int)(uMsg)); break;
		case WM_CHAR: sprintf_s(str + 8, strSize - 8, "%d (WM_CHAR)\n", (int)(uMsg)); break;
		case WM_CLOSE: sprintf_s(str + 8, strSize - 8, "%d (WM_CLOSE)\n", (int)(uMsg)); break;
		case WM_QUIT: sprintf_s(str + 8, strSize - 8, "%d (WM_QUIT)\n", (int)(uMsg)); break;
		case WM_DESTROY: sprintf_s(str + 8, strSize - 8, "%d (WM_DESTROY)\n", (int)(uMsg)); break;
		case WM_MOUSEMOVE: sprintf_s(str + 8, strSize - 8, "%d (WM_MOUSEMOVE)\n", (int)(uMsg)); break;
		case WM_SETCURSOR: sprintf_s(str + 8, strSize - 8, "%d (WM_SETCURSOR)\n", (int)(uMsg)); break;
		case WM_NCHITTEST: sprintf_s(str + 8, strSize - 8, "%d (WM_NCHITTEST)\n", (int)(uMsg)); break;
		case WM_NCMOUSEMOVE: sprintf_s(str + 8, strSize - 8, "%d (WM_NCMOUSEMOVE)\n", (int)(uMsg)); break;
		case WM_NCMOUSELEAVE: sprintf_s(str + 8, strSize - 8, "%d (WM_NCMOUSELEAVE)\n", (int)(uMsg)); break;
		default: sprintf_s(str + 8, strSize - 8, "%d ( )\n", (int)(uMsg)); break;
	}
}

void DrawGraphics()
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char str[40];
	MessageToString(str, 40, message);
	OutputDebugString(str);

	static PAINTSTRUCT ps;
	static GLsizei viewPortWidth = 0;
	static GLsizei viewPortHeight = 0;

	switch (message)
	{
		case WM_PRINT:
		case WM_PRINTCLIENT:
			DrawGraphics();
			return 0;

		case WM_PAINT:
			DrawGraphics();
			/* stops windows to dispatch WM_PAINT */
			BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			return 0;

		case WM_SIZE:
			/* windows posts WM_PAINT automaticaly, when window is enlarged, otherwise not */
			if (LOWORD(lParam) < viewPortWidth || HIWORD(lParam) < viewPortHeight)
				PostMessage(hWnd, WM_PRINTCLIENT, 0, 0);
			viewPortWidth = LOWORD(lParam);
			viewPortHeight = HIWORD(lParam);
			glViewport(0, 0, viewPortWidth, viewPortHeight);
			return 0;

		case WM_CHAR:
			switch (wParam)
			{
				/* ESC key */
				case 27:
					PostMessage(hWnd, WM_CLOSE, 0, 0);;
					break;
			}
			return 0;

		case WM_CLOSE:
			wglMakeCurrent(NULL, NULL);
			ReleaseDC(hWnd, hDC);
			wglDeleteContext(hRC);
			DestroyWindow(hWnd);
			return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

ATOM RegisterOpenGLWindowClass(HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style = CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC) WndProc; // event handler
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = MyWindowClassName;

	return RegisterClass(&wc);
}

BOOL CreateOpenGLWindow(HINSTANCE hInstance)
{
	DWORD  dwStyle;
	int    x = 0;
	int    y = 0;
	int    width = 256;
	int    height = 256;

	dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	hWnd = CreateWindow(MyWindowClassName, "Example", dwStyle, x, y, width, height, NULL, NULL, hInstance, NULL);

	return (hWnd != NULL);
}

BOOL CreateOpenGLContext()
{
	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	int                   pixelFormat;

	hDC = GetDC(hWnd);

	/* Initialize bits to 0. */
	memset(&pixelFormatDescriptor, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 32;
	pixelFormat = ChoosePixelFormat(hDC, &pixelFormatDescriptor);

	if (pixelFormat == 0)
	{
		MessageBox(NULL, "ChoosePixelFormat() failed: Cannot find a suitable pixel format.", "Error", MB_OK);
		return FALSE;
	}
	if (SetPixelFormat(hDC, pixelFormat, &pixelFormatDescriptor) == FALSE)
	{
		MessageBox(NULL, "SetPixelFormat() failed: Cannot set format specified.", "Error", MB_OK);
		return FALSE;
	}
	DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pixelFormatDescriptor);
	hRC = wglCreateContext(hDC);

	return (hRC != NULL);
}

int APIENTRY wWinMain(_In_ HINSTANCE hCurrInstance,
                  _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;

	if (!RegisterOpenGLWindowClass(hCurrInstance))
	{
		MessageBox(NULL, "RegisterClass() failed: Cannot register window class.", "Error", MB_OK);
		return FALSE;
	}
	if (!CreateOpenGLWindow(hCurrInstance))
	{
		MessageBox(NULL, "CreateWindow() failed: Cannot create a window.", "Error", MB_OK);
		return FALSE;
	}
	if (!CreateOpenGLContext())
	{
		MessageBox(NULL, "wglCreateContext() failed: Cannot create an OpenGL context.", "Error", MB_OK);
		return FALSE;
	}
	wglMakeCurrent(hDC, hRC);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&msg, hWnd, 0, 0) == TRUE) /* GetMessage can return -1 */
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}
