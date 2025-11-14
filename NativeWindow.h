// Copyright © Kirill Gavrilov, 2018-2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef NATIVEWINDOW_HEADER
#define NATIVEWINDOW_HEADER

#include <string>

#ifdef _WIN32
typedef void* NativeDrawable; // HWND under WNT
#else
typedef unsigned long NativeDrawable; // Window or Pixmap under UNIX
#endif
struct NativeXDisplay;

//! Native window creation tool.
class NativeWindow
{
public:
  //! Empty constructor.
  NativeWindow(const std::string& theTitle);

  //! Destructor.
  ~NativeWindow() { destroyWindow(); }

  //! Return true if handle is NULL.
  bool IsNull() const { return myHandle == 0; }

  //! Return native handle.
  NativeDrawable GetDrawable() const { return myHandle; }

  //! Return native display.
  NativeXDisplay* GetDisplay() const { return myDisplay; }

  //! Create a window handle.
  bool Create();

  //! Close window.
  virtual void Destroy() { destroyWindow(); }

  //! Post quit message.
  void Quit();

protected:

  //! Close window.
  void destroyWindow();

private:

  std::string myTitle;

  NativeDrawable  myHandle  = 0;
  NativeXDisplay* myDisplay = NULL;

};

#endif // NATIVEWINDOW_HEADER
