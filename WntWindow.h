// Copyright © Kirill Gavrilov, 2018-2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef WNTWINDOW_HEADER
#define WNTWINDOW_HEADER

#include "BaseWindow.h"

//! Native WinAPI window for Windows.
class WntWindow : public BaseWindow
{
public:
  //! Empty constructor.
  WntWindow(const std::string& theTitle);

  //! Destructor.
  ~WntWindow() { destroyWindow(); }

  //! Return true if handle is NULL.
  virtual bool IsNull() const override { return myHandle == 0; }

  //! Return native handle.
  virtual NativeDrawable GetDrawable() const override { return myHandle; }

  //! Return native display.
  virtual NativeXDisplay* GetDisplay() const override { return 0; }

  //! Create a window handle.
  virtual bool Create() override;

  //! Close window.
  virtual void Destroy() { destroyWindow(); }

  //! Post quit message.
  void Quit();

protected:

  //! Close window.
  void destroyWindow();

private:

  std::string    myTitle;
  NativeDrawable myHandle  = 0;

};

#endif // WNTWINDOW_HEADER
