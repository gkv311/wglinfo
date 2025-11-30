// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef CGLCONTEXT_HEADER
#define CGLCONTEXT_HEADER
#if defined(__APPLE__)

#include "BaseGlContext.h"

#ifdef __OBJC__
@class NSOpenGLContext;
#else
struct NSOpenGLContext;
#endif

//! CGL (Core OpenGL) context for macOS.
class CglContext : public BaseGlContext
{
public:

  //! Empty constructor.
  CglContext(const std::string& theTitle);

  //! Destructor.
  ~CglContext() { release(); }

  //! Return platform (CGL).
  virtual const char* PlatformName() const override { return "CGL"; }

  //! Release resources.
  virtual void Release() override { release(); }

  //! Create a GL context.
  virtual bool CreateGlContext(ContextBits theBits) override;

  //! Make this GL context active in current thread.
  virtual bool MakeCurrent() override;

public:

  //! Print platform info.
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

  typedef NSOpenGLContext* NativeRenderingContext;

private:

  void* myGlLibHandle = nullptr;

  CocoaWindow myWin;
  NativeRenderingContext myRendCtx = 0;

};

#endif
#endif // CGLCONTEXT_HEADER
