// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#include "XwWindow.h"

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <cstring>
#include <iomanip>
#include <iostream>

XwWindow::XwWindow(const std::string& theTitle)
: myTitle(theTitle)
{
  //
}

bool XwWindow::Create()
{
  Destroy();

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
}

void XwWindow::destroyWindow()
{
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
}

#endif
