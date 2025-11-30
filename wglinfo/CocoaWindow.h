// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef COCOAWINDOW_HEADER
#define COCOAWINDOW_HEADER

#include "BaseWindow.h"

//! Native Cocoa window for macOS.
class CocoaWindow : public BaseWindow
{
public:
  //! Empty constructor.
  CocoaWindow(const std::string& theTitle);

  //! Destructor.
  ~CocoaWindow() { destroyWindow(); }

  //! Return true if handle is NULL.
  virtual bool IsNull() const override { return myHView == 0; }

  //! Return native handle.
  virtual NativeDrawable GetDrawable() const override { return myHView; }

  //! Return native display.
  virtual NativeXDisplay* GetDisplay() const override { return 0; }

  //! Create a window handle.
  virtual bool Create() override;

  //! Close window.
  virtual void Destroy() override { destroyWindow(); }

protected:

  //! Close window.
  void destroyWindow();

private:

  std::string myTitle;
  NSWindow*   myHWindow = 0;
  NSView*     myHView  = 0;

};

#endif // COCOAWINDOW_HEADER
