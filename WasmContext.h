// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef WASMCONTEXT_HEADER
#define WASMCONTEXT_HEADER

#include "BaseGlContext.h"
#include "WasmWindow.h"

//! Emsdk WebGL context - not implemented.
class WasmContext : public BaseGlContext
{
public:

  //! Empty constructor.
  WasmContext(const std::string& theName) : myWin(theName) {}

  //! Destructor.
  ~WasmContext() { release(); }

  //! Return platform (EMSDK).
  virtual const char* PlatformName() const override { return "EMSDK"; }

  //! Release resources.
  virtual void Release() override { release(); }

  //! Create a GL context.
  virtual bool CreateGlContext(ContextBits ) override { return false; }

  //! Make this GL context active in current thread.
  virtual bool MakeCurrent() override { return false; }

public:

  //! Print platform info.
  virtual void PrintPlatformInfo(bool ) override {}

  //! Print GPU memory info.
  virtual void PrintGpuMemoryInfo() override {}

  //! Print information about visuals.
  virtual void PrintVisuals(bool ) override {}

public:

  //! glGetError() wrapper.
  virtual unsigned int GlGetError() override { return 0x0502; }

  //! glGetString() wrapper.
  virtual const char* GlGetString(unsigned int ) override { return nullptr; }

  //! glGetStringi() wrapper.
  virtual const char* GlGetStringi(unsigned int , unsigned int ) override { return nullptr; }

  //! glGetIntegerv() wrapper.
  virtual void GlGetIntegerv(unsigned int , int* ) override {}

  //! Wrapper to system function to retrieve GL function pointer by name.
  virtual void* GlGetProcAddress(const char* ) override { return nullptr; }

private:

  //! Release resources.
  void release() {}

private:

  typedef void* NativeRenderingContext;

private:

  WasmWindow myWin;
  NativeRenderingContext myRendCtx = 0; //!< rendering context

};

#endif // WASMCONTEXT_HEADER
