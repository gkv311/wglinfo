// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#include "WlWindow.h"

#if defined(HAVE_WAYLAND)

#include <cstring>
#include <iomanip>
#include <iostream>

#include <wayland-client-core.h>
#include <wayland-client.h>
#include <wayland-server.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h> // should be included before EGL headers

// generated from xdg-shell.xml
#include <xdg-shell-client-protocol.h>

bool WlWindow::HasServer()
{
  std::shared_ptr<wl_display> aWlDisplay(wl_display_connect(nullptr),
                                         [](wl_display* theDisp) { theDisp != nullptr ? wl_display_disconnect(theDisp) : (void)theDisp; });
  return aWlDisplay.get() != nullptr;
}

WlWindow::WlWindow(const std::string& theTitle)
: myTitle(theTitle)
{
  //
}

bool WlWindow::Create()
{
  Destroy();

  myWlDisplay.reset(wl_display_connect(nullptr),
                    [](wl_display* theDisp) { theDisp != nullptr ? wl_display_disconnect(theDisp) : (void)theDisp; });
  if (myWlDisplay.get() == nullptr)
  {
    std::cerr << "Error: cannot connect to the Wayland server\n";
    return false;
  }

  static const wl_registry_listener aWlListener =
  {
    .global = [](void* theData,
                 wl_registry* theRegistry,
                 uint32_t theId,
                 const char* theInterface,
                 uint32_t theVersion) {
      (void)theVersion;
      WlWindow* aThis = (WlWindow*)theData;
      if (::strcmp(theInterface, "wl_compositor") == 0)
      {
        aThis->myWlCompositor = (wl_compositor* )wl_registry_bind(theRegistry, theId, &wl_compositor_interface, 1);
        return;
      }

      if (::strcmp(theInterface, xdg_wm_base_interface.name) == 0)
      {
        static const xdg_wm_base_listener anXdgBaseList =
        {
          .ping = [](void* theData, xdg_wm_base* theXdgWmBase, uint32_t theSerial)
          {
            (void )theData;
            xdg_wm_base_pong(theXdgWmBase, theSerial);
          }
        };

        aThis->myWlXdgWmBase = (xdg_wm_base* )wl_registry_bind(theRegistry, theId, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(aThis->myWlXdgWmBase, &anXdgBaseList, aThis);
      }
    },
    .global_remove = [](void* theData,
                        wl_registry* theRegistry,
                        uint32_t theId) {
      (void)theData;
      (void)theRegistry;
      (void)theId;
    }
  };

  struct wl_registry* aWlRegistry = wl_display_get_registry(myWlDisplay.get());
  wl_registry_add_listener(aWlRegistry, &aWlListener, this);
  // call attached listener
  wl_display_dispatch(myWlDisplay.get());
  wl_display_roundtrip(myWlDisplay.get());
  if (myWlCompositor == nullptr)
  {
    std::cerr << "Error: Wayland server has no compositor\n";
    return false;
  }
  if (myWlXdgWmBase == nullptr)
  {
    std::cerr << "Error: Wayland server has no XDG shell\n";
    return false;
  }

  myWlSurface.reset(wl_compositor_create_surface(myWlCompositor),
                    [](wl_surface* theSurf) { theSurf != nullptr ? wl_surface_destroy(theSurf) : (void)theSurf; });
  if (myWlSurface.get() == nullptr)
  {
    std::cerr << "Error: cannot create Wayland compositor surface\n";
    return false;
  }

  bool toMapWindow = false;
  if (toMapWindow)
    myWlXdgSurf.reset(xdg_wm_base_get_xdg_surface(myWlXdgWmBase, myWlSurface.get()),
                      [](xdg_surface* theXdgSurf) { theXdgSurf != nullptr ? xdg_surface_destroy(theXdgSurf) : (void)theXdgSurf; });

  static const xdg_surface_listener anXdgSurfList =
  {
    .configure = [](void* theData,
                    xdg_surface* theXdgSurf,
                    uint32_t theSerial) {
      (void )theData;
      // just confirm existence to the compositor
      xdg_surface_ack_configure(theXdgSurf, theSerial);
    },
  };
  if (myWlXdgSurf)
    xdg_surface_add_listener(myWlXdgSurf.get(), &anXdgSurfList, this);

  if (toMapWindow)
    myWlXdgTop.reset(xdg_surface_get_toplevel(myWlXdgSurf.get()),
                     [](xdg_toplevel* theTop) { theTop != nullptr ? xdg_toplevel_destroy(theTop) : (void)theTop; });

  if (myWlXdgTop)
    xdg_toplevel_set_title(myWlXdgTop.get(), myTitle.c_str());

  static const xdg_toplevel_listener anXdgTopList =
  {
    .configure = [](void* theData,
                    xdg_toplevel* theXdgTop,
                    int32_t theWidth, int32_t theHeight,
                    wl_array* theStates) {
      WlWindow* aThis = (WlWindow*)theData;
      (void)aThis;
      (void)theXdgTop;
      (void)theWidth;
      (void)theHeight;
      (void)theStates;
    },
    .close = [](void* theData,
                xdg_toplevel* theXdgTop) {
      WlWindow* aThis = (WlWindow*)theData;
      (void)aThis;
      (void)theXdgTop;
    },
    .configure_bounds = [](void* theData,
                           xdg_toplevel* theXdgTop,
                           int32_t theWidth,
                           int32_t theHeight)
    {
      WlWindow* aThis = (WlWindow*)theData;
      (void)aThis;
      (void)theXdgTop;
      (void)theWidth;
      (void)theHeight;
    },
    .wm_capabilities = [](void* theData,
                          xdg_toplevel* theXdgTop,
                          wl_array* theCaps)
    {
      WlWindow* aThis = (WlWindow*)theData;
      (void)aThis;
      (void)theXdgTop;
      (void)theCaps;
    }
  };
  if (myWlXdgTop)
    xdg_toplevel_add_listener(myWlXdgTop.get(), &anXdgTopList, this);

  wl_surface_commit(myWlSurface.get());

  std::shared_ptr<wl_region> aWlRegion;
  aWlRegion.reset(wl_compositor_create_region(myWlCompositor),
                  [](wl_region* theReg) { theReg != nullptr ? wl_region_destroy(theReg) : (void)theReg; });

  const int aSize[2] { 4, 4 };
  wl_region_add(aWlRegion.get(), 0, 0, aSize[0], aSize[1]);
  wl_surface_set_opaque_region(myWlSurface.get(), aWlRegion.get());
  aWlRegion.reset();

  myWlEglWindow.reset(wl_egl_window_create(myWlSurface.get(), aSize[0], aSize[1]),
                      [](wl_egl_window* theWin) { theWin != nullptr ? wl_egl_window_destroy(theWin) : (void)theWin; });
  if (myWlEglWindow.get() == nullptr)
  {
    std::cerr << "Error: cannot create Wayland EGL window\n";
    return false;
  }
  //wl_display_dispatch_pending(myWlDisplay.get());

  return true;
}

void WlWindow::destroyWindow()
{
  myWlEglWindow.reset();
  myWlXdgTop.reset();
  myWlXdgSurf.reset();
  myWlSurface.reset();

  myWlDisplay.reset();
  myWlCompositor = nullptr;
  myWlXdgWmBase = nullptr;
}

#endif
