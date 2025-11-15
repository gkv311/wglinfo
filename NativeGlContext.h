// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef NATIVEGLCONTEXT_HEADER
#define NATIVEGLCONTEXT_HEADER

#ifdef _WIN32
  #include "WglContext.h"
  typedef WglContext NativeGlContext;
#else
  #include "GlxContext.h"
  typedef GlxContext NativeGlContext;
#endif

#endif // NATIVEGLCONTEXT_HEADER
