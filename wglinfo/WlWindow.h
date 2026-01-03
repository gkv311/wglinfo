// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef WLWINDOW_HEADER
#define WLWINDOW_HEADER

#include "BaseWindow.h"

struct wl_compositor;
struct wl_display;
struct wl_registry;
struct xdg_wm_base;

struct wl_egl_window;
struct wl_region;
struct wl_surface;

struct xdg_surface;
struct xdg_toplevel;

//! Native Wayland window for Linux.
class WlWindow : public BaseWindow
{
public:
  //! Check if there is default Wayland server to connect to.
  static bool HasServer();

public:
  //! Empty constructor.
  WlWindow(const std::string& theTitle);

  //! Destructor.
  ~WlWindow() { destroyWindow(); }

  //! Return true if handle is NULL.
  virtual bool IsNull() const override { return myWlEglWindow.get() == nullptr; }

  //! Return native handle.
  virtual NativeDrawable GetDrawable() const override { return (NativeDrawable)myWlEglWindow.get(); }

  //! Return native display.
  virtual NativeXDisplay* GetDisplay() const override { return (NativeXDisplay*)myWlDisplay.get(); }

  //! Create a window handle.
  virtual bool Create() override;

  //! Close window.
  virtual void Destroy() override { destroyWindow(); }

  //! Create instance of the same class.
  virtual std::shared_ptr<BaseWindow> EmptyCopy(const std::string& theTitle) override
  {
    return std::make_shared<WlWindow>(theTitle);
  }

protected:

  //! Close window.
  void destroyWindow();

private:

  std::string myTitle;

  std::shared_ptr<wl_display> myWlDisplay;

  wl_compositor* myWlCompositor = nullptr;
  xdg_wm_base*   myWlXdgWmBase  = nullptr;

  std::shared_ptr<wl_surface>    myWlSurface;
  std::shared_ptr<xdg_surface>   myWlXdgSurf;
  std::shared_ptr<xdg_toplevel>  myWlXdgTop;
  std::shared_ptr<wl_region>     myWlRegion;
  std::shared_ptr<wl_egl_window> myWlEglWindow;

};

#endif // WLWINDOW_HEADER
