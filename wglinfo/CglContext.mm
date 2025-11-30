// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#include "CglContext.h"

#if defined(__APPLE__)

#ifndef GL_GLEXT_LEGACY
#define GL_GLEXT_LEGACY // prevent inclusion of system glext.h on Mac OS X 10.6.8
#endif

// macOS 10.4 deprecated OpenGL framework - suppress useless warnings
#define GL_SILENCE_DEPRECATION

#include <Cocoa/Cocoa.h>
#include <CoreGraphics/CoreGraphics.h>
#include <OpenGL/CGLRenderers.h>
#include <OpenGL/OpenGL.h>

#include <OpenGL/gl.h>

#include <dlfcn.h>

#include <iomanip>
#include <iostream>
#include <memory>

#if defined(__GNUC__) && !defined(__INTEL_COMPILER) && !defined(__clang__)
  #if (__GNUC__ > 8) || ((__GNUC__ == 8) && (__GNUC_MINOR__ >= 1))
    #pragma GCC diagnostic ignored "-Wcast-function-type"
  #endif
#endif

CglContext::CglContext(const std::string& theTitle)
: myWin(theTitle)
{
  myGlLibHandle = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);
}

void CglContext::release()
{
  if (myRendCtx != NULL)
  {
    [NSOpenGLContext clearCurrentContext];
    [myRendCtx clearDrawable];
    [myRendCtx release];
    myRendCtx = nullptr;
  }

  myWin.Destroy();
}

bool CglContext::CreateGlContext(ContextBits theBits)
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

  if (isGles || isFwdCtx || isDebugCtx) // unsupported
    return false;

  //CocoaLocalPool aLocalPool; // #if !__has_feature(objc_arc)

  NSOpenGLPixelFormatAttribute anAttribs[32] = {};
  int aLastAttrib = 0;
  //anAttribs[aLastAttrib++] = NSOpenGLPFAColorSize;    anAttribs[aLastAttrib++] = 32,
  anAttribs[aLastAttrib++] = NSOpenGLPFADepthSize;    anAttribs[aLastAttrib++] = 24;
  anAttribs[aLastAttrib++] = NSOpenGLPFAStencilSize;  anAttribs[aLastAttrib++] = 8;
  anAttribs[aLastAttrib++] = NSOpenGLPFADoubleBuffer;
  if (isSoftCtx)
  {
    anAttribs[aLastAttrib++] = NSOpenGLPFARendererID;
    anAttribs[aLastAttrib++] = (NSOpenGLPixelFormatAttribute )kCGLRendererGenericFloatID;
  }
  else
  {
    anAttribs[aLastAttrib++] = NSOpenGLPFAAccelerated;
  }
  if (isCoreCtx)
  {
    // supported since OS X 10.7+
    anAttribs[aLastAttrib++] = 99;     // NSOpenGLPFAOpenGLProfile
    anAttribs[aLastAttrib++] = 0x3200; // NSOpenGLProfileVersion3_2Core
  }
  anAttribs[aLastAttrib] = 0;

  NSOpenGLContext* aGLCtxShare = nullptr;
  NSOpenGLPixelFormat* aGLFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes: anAttribs] autorelease];
  myRendCtx = [[NSOpenGLContext alloc] initWithFormat: aGLFormat
                                         shareContext: aGLCtxShare];

  if (myRendCtx == nullptr)
  {
    std::cerr << "Error: NSOpenGLContext creation failed\n";
    return false;
  }

  NSView* aView = (NSView* )myWin.GetDrawable();
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wdeprecated-declarations"
  [myRendCtx setView: aView];
  #pragma clang diagnostic pop

  if (!MakeCurrent())
  {
    Release();
    return false;
  }

  return true;
}

bool CglContext::MakeCurrent()
{
  if (myRendCtx == 0)
    return false;

  [myRendCtx makeCurrentContext];
  return true;
}

void* CglContext::GlGetProcAddress(const char* theFuncName)
{
  return (myGlLibHandle != nullptr) ? dlsym(myGlLibHandle, theFuncName) : nullptr;
}

unsigned int CglContext::GlGetError()
{
  return ::glGetError();
}

const char* CglContext::GlGetString(unsigned int theGlEnum)
{
  const char* aStr = (const char*)::glGetString(theGlEnum);
  if (aStr == nullptr)
  {
    //
  }
  return aStr;
}

const char* CglContext::GlGetStringi(unsigned int theGlEnum, unsigned int theIndex)
{
  typedef const GLubyte* (*glGetStringi_t) (GLenum name, GLuint index);
  glGetStringi_t aGetStringi = NULL;
  if (!FindProc("glGetStringi", aGetStringi))
    return NULL;

  return (const char*)aGetStringi(theGlEnum, theIndex);
}

void CglContext::GlGetIntegerv(unsigned int theGlEnum, int* theParams)
{
  ::glGetIntegerv(theGlEnum, theParams);
}

void CglContext::PrintPlatformInfo(bool theToPrintExtensions)
{
  std::cout << "[" << PlatformName() << "] CGLName: OpenGL.framework\n";
  (void)theToPrintExtensions;
}

void CglContext::PrintGpuMemoryInfo()
{
  BaseGlContext::PrintGpuMemoryInfo();

  GLint aGlRendId = 0;
  CGLGetParameter(CGLGetCurrentContext(), kCGLCPCurrentRendererID, &aGlRendId);

  CGLRendererInfoObj  aRendObj = nullptr;
  CGOpenGLDisplayMask aDispMask = CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay);
  GLint aRendNb = 0;
  CGLQueryRendererInfo(aDispMask, &aRendObj, &aRendNb);
  for (GLint aRendIter = 0; aRendIter < aRendNb; ++aRendIter)
  {
    GLint aRendId = 0;
    if (CGLDescribeRenderer(aRendObj, aRendIter, kCGLRPRendererID, &aRendId) != kCGLNoError
     || aRendId != aGlRendId)
    {
      continue;
    }

    //kCGLRPVideoMemoryMegabytes   = 131;
    //kCGLRPTextureMemoryMegabytes = 132;
    GLint aVMem = 0, aVTMem = 0;
  #if MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
    if (CGLDescribeRenderer(aRendObj, aRendIter, kCGLRPVideoMemoryMegabytes, &aVMem) == kCGLNoError && aVMem != 0)
      std::cout << Prefix() << "GPU memory: " << aVMem << " MiB\n";

    if (CGLDescribeRenderer(aRendObj, aRendIter, kCGLRPTextureMemoryMegabytes, &aVTMem) == kCGLNoError && aVTMem != aVMem)
      std::cout << Prefix() << "GPU texture memory: " << aVTMem << " MiB\n";
  #else
    if (CGLDescribeRenderer(aRendObj, aRendIter, kCGLRPVideoMemory, &aVMem) == kCGLNoError && aVMem != 0)
      std::cout << Prefix() << "GPU memory: " << (aVMem / (1024 * 1024)) << " MiB\n";

    if (CGLDescribeRenderer(aRendObj, aRendIter, kCGLRPTextureMemory, &aVTMem) == kCGLNoError && aVTMem != aVMem)
      std::cout << Prefix() << "GPU texture memory: " << (aVTMem / (1024 * 1024)) << " MiB\n";
  #endif
  }
}

void CglContext::PrintVisuals(bool theIsVerbose)
{
  (void)theIsVerbose;
}

#endif
