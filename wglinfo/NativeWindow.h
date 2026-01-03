// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef NATIVEWINDOW_HEADER
#define NATIVEWINDOW_HEADER

#ifdef _WIN32
  #include "WntWindow.h"
  typedef WntWindow NativeWindow;
#elif defined(__APPLE__)
  #include "CocoaWindow.h"
  typedef CocoaWindow NativeWindow;
#elif defined(__EMSCRIPTEN__)
  #include "WasmWindow.h"
  typedef WasmWindow NativeWindow;
#elif defined(HAVE_WAYLAND)
  #include "WlWindow.h"
  typedef WlWindow NativeWindow;
#else
  #include "XwWindow.h"
  typedef XwWindow NativeWindow;
#endif

#endif // NATIVEWINDOW_HEADER
