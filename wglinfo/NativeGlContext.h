// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef NATIVEGLCONTEXT_HEADER
#define NATIVEGLCONTEXT_HEADER

#ifdef _WIN32
  #include "WglContext.h"
  typedef WglContext NativeGlContext;
#elif defined(__APPLE__)
  #include "CglContext.h"
  typedef CglContext NativeGlContext;
#elif defined(__EMSCRIPTEN__)
  #include "WasmContext.h"
  typedef WasmContext NativeGlContext;
#else
  #include "GlxContext.h"
  typedef GlxContext NativeGlContext;
#endif

#endif // NATIVEGLCONTEXT_HEADER
