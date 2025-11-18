// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef BASEWINDOW_HEADER
#define BASEWINDOW_HEADER

#include <string>

#ifdef _WIN32
typedef void* NativeDrawable; // HWND under WNT
#else
typedef unsigned long NativeDrawable; // Window or Pixmap under UNIX
#endif
struct NativeXDisplay;

//! Native window creation tool.
class BaseWindow
{
public:
  //! Destructor.
  virtual ~BaseWindow() {}

  //! Return true if handle is NULL.
  virtual bool IsNull() const = 0;

  //! Return native handle.
  virtual NativeDrawable GetDrawable() const = 0;

  //! Return native display.
  virtual NativeXDisplay* GetDisplay() const = 0;

  //! Create a window handle.
  virtual bool Create() = 0;

  //! Close window.
  virtual void Destroy() = 0;

};

#endif // BASEWINDOW_HEADER
