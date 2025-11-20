// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#include "WasmContext.h"

#if defined(__EMSCRIPTEN__)

#include <GLES3/gl3.h>

#include <emscripten/html5_webgl.h>

#include <iomanip>
#include <iostream>
#include <memory>

#if defined(__GNUC__) && !defined(__INTEL_COMPILER) && !defined(__clang__)
  #if (__GNUC__ > 8) || ((__GNUC__ == 8) && (__GNUC_MINOR__ >= 1))
    #pragma GCC diagnostic ignored "-Wcast-function-type"
  #endif
#endif

typedef const GLubyte* (GL_APIENTRY *glGetStringi_t) (GLenum name, GLuint index);

WasmContext::WasmContext(const std::string& theTitle)
: myWin(theTitle)
{
  //
}

void WasmContext::release()
{
  if (myRendCtx == 0)
    return;

  emscripten_webgl_destroy_context(myRendCtx);
  myRendCtx = 0;

  myWin.Destroy();
}

bool WasmContext::CreateGlContext(ContextBits theBits)
{
  Release();
  if (!myWin.Create())
    return false;

  myCtxBits = theBits;

  const bool isDebugCtx = (theBits & ContextBits_Debug) != 0;
  const bool isCoreCtx  = (theBits & ContextBits_CoreProfile) != 0;
  const bool isFwdCtx   = (theBits & ContextBits_ForwardProfile) != 0;
  const bool isSoftCtx  = (theBits & ContextBits_SoftProfile) != 0;
  const bool isGles     = (theBits & ContextBits_GLES) != 0;
  if (!isGles || isDebugCtx || isCoreCtx || isFwdCtx || isSoftCtx) // unsupported
    return false;

  EmscriptenWebGLContextAttributes aCtxAttribs = {};
  emscripten_webgl_init_context_attributes(&aCtxAttribs);
  aCtxAttribs.alpha = false;
  aCtxAttribs.depth = true;
  aCtxAttribs.stencil = true;
  aCtxAttribs.antialias = false;
  aCtxAttribs.premultipliedAlpha = true;
  aCtxAttribs.preserveDrawingBuffer = true;
  aCtxAttribs.enableExtensionsByDefault = true;
  aCtxAttribs.explicitSwapControl = false;
  aCtxAttribs.majorVersion = 2;
  aCtxAttribs.minorVersion = 0;

  myRendCtx = emscripten_webgl_create_context("#canvas", &aCtxAttribs);
  if (myRendCtx == 0)
  {
    aCtxAttribs.majorVersion = 1;
    myRendCtx = emscripten_webgl_create_context("#canvas", &aCtxAttribs);
  }

  if (!MakeCurrent())
  {
    Release();
    return false;
  }

  const std::string anExtList = getGlExtensions();
  if (hasExtension(anExtList.c_str(), "GL_WEBGL_debug_renderer_info"))
    emscripten_webgl_enable_extension(myRendCtx, "GL_WEBGL_debug_renderer_info");

  return true;
}

bool WasmContext::MakeCurrent()
{
  if (myRendCtx == 0)
    return false;

  if (emscripten_webgl_make_context_current(myRendCtx) != EMSCRIPTEN_RESULT_SUCCESS)
  {
    std::cerr << "emscripten_webgl_make_context_current() has failed\n";
    return false;
  }
  return true;
}

void* WasmContext::GlGetProcAddress(const char* theFuncName)
{
  return (void*)emscripten_webgl_get_proc_address(theFuncName);
}

unsigned int WasmContext::GlGetError()
{
  return ::glGetError();
}

const char* WasmContext::GlGetString(unsigned int theGlEnum)
{
  const char* aStr = (const char*)::glGetString(theGlEnum);
  if (aStr == NULL)
  {
    //
  }
  return aStr;
}

const char* WasmContext::GlGetStringi(unsigned int theGlEnum, unsigned int theIndex)
{
  glGetStringi_t aGetStringi = NULL;
  if (!FindProc("glGetStringi", aGetStringi))
    return NULL;

  return (const char*)aGetStringi(theGlEnum, theIndex);
}

void WasmContext::GlGetIntegerv(unsigned int theGlEnum, int* theParams)
{
  ::glGetIntegerv(theGlEnum, theParams);
}

void WasmContext::PrintPlatformInfo(bool )
{
  const std::string anExtList = getGlExtensions();
  if (hasExtension(anExtList.c_str(), "GL_WEBGL_debug_renderer_info"))
  {
    std::cout << Prefix() << "unmasked vendor:   " << GlGetString(0x9245) << "\n";
    std::cout << Prefix() << "unmasked renderer: " << GlGetString(0x9246) << "\n";
  }
}

#endif
