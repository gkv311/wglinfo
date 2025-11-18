// Copyright © Kirill Gavrilov, 2018-2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifdef _WIN32
  #include <windows.h>
#endif

#include "WntWindow.h"

#ifdef _WIN32

#include <cstring>
#include <iomanip>
#include <iostream>

//! Dummy window procedure.
static LRESULT WINAPI windowProcWgl(HWND theWin, UINT theMsg, WPARAM theParamW, LPARAM theParamL)
{
  return ::DefWindowProcW(theWin, theMsg, theParamW, theParamL);
}

WntWindow::WntWindow(const std::string& theTitle)
: myTitle(theTitle)
{
  //
}

bool WntWindow::Create()
{
  Destroy();

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
}

void WntWindow::destroyWindow()
{
  if (myHandle != NULL)
  {
    ::DestroyWindow((HWND)myHandle);
    myHandle = NULL;
  }
}

void WntWindow::Quit()
{
  if (IsNull())
    return;

  ::PostQuitMessage(0);
  for (MSG aMsg; GetMessageW(&aMsg, (HWND)myHandle, 0, 0); )
  {
    TranslateMessage(&aMsg);
    DispatchMessageW(&aMsg);
  }
}

#endif // _WIN32
