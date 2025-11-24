// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef WASMWINDOW_HEADER
#define WASMWINDOW_HEADER

#include "BaseWindow.h"

//! Native WebAssembly window.
class WasmWindow : public BaseWindow
{
public:
  //! Empty constructor.
  WasmWindow(const std::string& theTitle);

  //! Destructor.
  ~WasmWindow() { destroyWindow(); }

  //! Return canvas id.
  const std::string& GetCanvasId() const { return myCanvasId; }

  //! Return true if handle is NULL.
  virtual bool IsNull() const override { return true; }

  //! Return native handle.
  virtual NativeDrawable GetDrawable() const override { return 0; }

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
  std::string myCanvasId;

};

#endif // WASMWINDOW_HEADER
