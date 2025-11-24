// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef XWWINDOW_HEADER
#define XWWINDOW_HEADER

#include "BaseWindow.h"

//! Native Xlib window for Linux.
class XwWindow : public BaseWindow
{
public:
  //! Empty constructor.
  XwWindow(const std::string& theTitle);

  //! Destructor.
  ~XwWindow() { destroyWindow(); }

  //! Return true if handle is NULL.
  virtual bool IsNull() const override { return myHandle == 0; }

  //! Return native handle.
  virtual NativeDrawable GetDrawable() const override { return myHandle; }

  //! Return native display.
  virtual NativeXDisplay* GetDisplay() const override { return myDisplay; }

  //! Create a window handle.
  virtual bool Create() override;

  //! Close window.
  virtual void Destroy() override { destroyWindow(); }

protected:

  //! Close window.
  void destroyWindow();

private:

  std::string myTitle;

  NativeDrawable  myHandle  = 0;
  NativeXDisplay* myDisplay = NULL;

};

#endif // XWWINDOW_HEADER
