// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef WASMCONTEXT_HEADER
#define WASMCONTEXT_HEADER

#include "BaseGlContext.h"
#include "WasmWindow.h"

#include <cstdint>

//! Emsdk WebGL context.
class WasmContext : public BaseGlContext
{
public:

  //! Empty constructor.
  WasmContext(const std::string& theName);

  //! Destructor.
  ~WasmContext() { release(); }

  //! Return platform (EMSDK).
  virtual const char* PlatformName() const override { return "EMSDK"; }

  //! Release resources.
  virtual void Release() override { release(); }

  //! Create a GL context.
  virtual bool CreateGlContext(ContextBits ) override;

  //! Make this GL context active in current thread.
  virtual bool MakeCurrent() override;

public:

  //! Print platform info.
  virtual void PrintPlatformInfo(bool theToPrintExtensions) override;

  //! Print information about visuals.
  virtual void PrintVisuals(bool ) override {}

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

  typedef intptr_t NativeRenderingContext; // EMSCRIPTEN_WEBGL_CONTEXT_HANDLE

private:

  WasmWindow myWin;
  NativeRenderingContext myRendCtx = 0; //!< rendering context

};

#endif // WASMCONTEXT_HEADER
