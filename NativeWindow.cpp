// Copyright © Kirill Gavrilov, 2018-2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifdef _WIN32
  #include <windows.h>
#endif

#include "NativeWindow.h"

#ifndef _WIN32
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
#endif

#include <cstring>
#include <iomanip>
#include <iostream>

#ifdef _WIN32
//! Dummy window procedure.
static LRESULT WINAPI windowProcWgl(HWND theWin, UINT theMsg, WPARAM theParamW, LPARAM theParamL)
{
  return ::DefWindowProcW(theWin, theMsg, theParamW, theParamL);
}
#endif

NativeWindow::NativeWindow(const std::string& theTitle)
: myTitle(theTitle)
{
  //
}

bool NativeWindow::Create()
{
  Destroy();
#ifdef _WIN32
  WNDCLASSW aWinClass;
  aWinClass.lpszClassName = L"OpenGL";
  static HINSTANCE anAppInstance = NULL;
  if (anAppInstance == NULL)
  {
    // only register the window class once
    anAppInstance = GetModuleHandleW(NULL);
    aWinClass.style = CS_OWNDC;
    aWinClass.lpfnWndProc = windowProcWgl;
    aWinClass.cbClsExtra = 0;
    aWinClass.cbWndExtra = 0;
    aWinClass.hInstance = anAppInstance;
    aWinClass.hIcon = LoadIconW(NULL, IDI_WINLOGO);
    aWinClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
    aWinClass.hbrBackground = NULL;
    aWinClass.lpszMenuName = NULL;
    if (!RegisterClassW(&aWinClass))
    {
      std::cerr << "Error: RegisterClass() failed, cannot register window class.\n";
      return false;
    }
  }

  const DWORD anExStyle = WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE;
  const DWORD aStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
  const std::wstring aName(myTitle.cbegin(), myTitle.cend());
  myHandle = CreateWindowExW(anExStyle, aWinClass.lpszClassName, aName.c_str(), aStyle,
                             2, 2, 4, 4, NULL, NULL, anAppInstance, NULL);
  if (myHandle == NULL)
  {
    std::cerr << "Error: CreateWindow() failed, Cannot create a window.\n";
    return false;
  }
  return true;
#else
  if (myDisplay == NULL)
  {
    myDisplay = (NativeXDisplay* )XOpenDisplay(NULL);
  }
  if (myDisplay == NULL)
  {
    std::cerr << "Error: cannot connect to the X11 server\n";
    return false;
  }

  XVisualInfo* aVisInfo = NULL;

  Display* aDisp   = (Display*)myDisplay;
  int      aScreen = DefaultScreen(aDisp);
  Window   aParent = RootWindow(aDisp, aScreen);

  unsigned long aMask = 0;
  XSetWindowAttributes aWinAttr;
  memset(&aWinAttr, 0, sizeof(XSetWindowAttributes));
  aWinAttr.event_mask = ExposureMask | StructureNotifyMask;
  aMask |= CWEventMask;
  if (aVisInfo != NULL)
  {
    aWinAttr.colormap = XCreateColormap(aDisp, aParent, aVisInfo->visual, AllocNone);
  }
  aWinAttr.border_pixel = 0;
  aWinAttr.override_redirect = False;

  myHandle = XCreateWindow(aDisp, aParent,
                           2, 2, 4, 4,
                           0, aVisInfo != NULL ? aVisInfo->depth : CopyFromParent,
                           InputOutput,
                           aVisInfo != NULL ? aVisInfo->visual : CopyFromParent,
                           CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect, &aWinAttr);
  if (aVisInfo != NULL)
  {
    XFree(aVisInfo);
    aVisInfo = NULL;
  }
  if (myHandle == 0)
  {
    std::cerr << "Error: unable to create XWindow\n";
    return false;
  }

  return true;
#endif
}

void NativeWindow::destroyWindow()
{
#ifdef _WIN32
  if (myHandle != NULL)
  {
    ::DestroyWindow((HWND)myHandle);
    myHandle = NULL;
  }
#else
  if (myHandle != 0)
  {
    XDestroyWindow((Display* )myDisplay, (Window )myHandle);
    myHandle = 0;
  }
  if (myDisplay != NULL)
  {
    XCloseDisplay((Display* )myDisplay);
    myDisplay = NULL;
  }
#endif
}

void NativeWindow::Quit()
{
#ifdef _WIN32
  if (IsNull())
    return;

  ::PostQuitMessage(0);
  for (MSG aMsg; GetMessageW(&aMsg, (HWND)myHandle, 0, 0); )
  {
    TranslateMessage(&aMsg);
    DispatchMessageW(&aMsg);
  }
#endif
}
