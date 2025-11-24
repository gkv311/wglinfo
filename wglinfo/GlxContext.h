// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef GLXCONTEXT_HEADER
#define GLXCONTEXT_HEADER
#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)

#include "BaseGlContext.h"

//! GLX context (Xlib).
class GlxContext : public BaseGlContext
{
public:

  //! Empty constructor.
  GlxContext(const std::string& theTitle);

  //! Destructor.
  ~GlxContext() { release(); }

  //! Return platform (GLX).
  virtual const char* PlatformName() const override { return "GLX"; }

  //! Release resources.
  virtual void Release() override { release(); }

  //! Create a GL context.
  virtual bool CreateGlContext(ContextBits theBits) override;

  //! Make this GL context active in current thread.
  virtual bool MakeCurrent() override;

public:

  //! Print WGL platform info.
  virtual void PrintPlatformInfo(bool theToPrintExtensions) override;

  //! Print GPU memory info.
  virtual void PrintGpuMemoryInfo() override;

  //! Print information about visuals.
  virtual void PrintVisuals(bool theIsVerbose) override;

public:

  //! glGetError() wrapper.
  virtual unsigned int GlGetError() override;

  //! glGetString() wrapper.
  virtual const char* GlGetString(unsigned int theGlEnum) override;

  //! glGetStringi() wrapper.
  virtual const char* GlGetStringi(unsigned int theGlEnum, unsigned int theIndex) override;

  //! glGetIntegerv() wrapper.
  virtual void GlGetIntegerv(unsigned int theGlEnum, int* theParams) override;

  //! Wrapper to system function to retrieve GL function pointer by name.
  virtual void* GlGetProcAddress(const char* theFuncName) override;

private:

  //! Release resources.
  void release();

private:

  typedef void* NativeRenderingContext; // GLXContext under UNIX

private:

  XwWindow myWin;
  NativeRenderingContext myRendCtx = 0; //!< GLXContext rendering context

};

#endif
#endif // NATIVEGLCONTEXT_HEADER
