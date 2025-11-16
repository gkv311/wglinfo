// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#include "GlxContext.h"

#ifndef _WIN32

#include <GL/gl.h>
#include <GL/glx.h>

#include <iomanip>
#include <iostream>
#include <memory>

#if defined(__GNUC__) && !defined(__INTEL_COMPILER) && !defined(__clang__)
  #if (__GNUC__ > 8) || ((__GNUC__ == 8) && (__GNUC_MINOR__ >= 1))
    #pragma GCC diagnostic ignored "-Wcast-function-type"
  #endif
#endif

typedef const GLubyte* (GLAPIENTRY *glGetStringi_t) (GLenum name, GLuint index);

GlxContext::GlxContext(const std::string& theTitle)
: myWin(theTitle)
{
  //
}

void GlxContext::release()
{
  Display* aDisp = (Display*)myWin.GetDisplay();
  if (myRendCtx != NULL && aDisp != NULL)
  {
    glXMakeCurrent(aDisp, None, NULL);

    // FSXXX sync necessary if non-direct rendering
    glXWaitGL();
    glXDestroyContext(aDisp, (GLXContext)myRendCtx);
    myRendCtx = NULL;
  }
  myWin.Destroy();
}

bool GlxContext::CreateGlContext(ContextBits theBits)
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

  if (isGles) // unsupported
    return false;

  Display*  aDisp   = (Display*)myWin.GetDisplay();
  const int aScreen = DefaultScreen(aDisp);

  int aDummy = 0;
  if (!XQueryExtension(aDisp, "GLX", &aDummy, &aDummy, &aDummy)
   || !glXQueryExtension(aDisp, &aDummy, &aDummy))
  {
    std::cerr << "Error: GLX extension is unavailable";
    return false;
  }

  SoftMesaSentry aMesaEnvSentry;
  if (isSoftCtx)
  {
    GlxContext aCtxCompat("wglinfoTmp");
    if (!aCtxCompat.CreateGlContext(ContextBits_NONE)
     || !aMesaEnvSentry.Init(aCtxCompat))
    {
      return false;
    }
  }

  XWindowAttributes aWinAttribs;
  XGetWindowAttributes(aDisp, (Window )myWin.GetDrawable(), &aWinAttribs);
  XVisualInfo aVisInfo;
  aVisInfo.visualid = aWinAttribs.visual->visualid;
  aVisInfo.screen   = aScreen;
  int aNbItems = 0;
  std::unique_ptr<XVisualInfo, int(*)(void*)> aVis(XGetVisualInfo(aDisp, VisualIDMask | VisualScreenMask, &aVisInfo, &aNbItems), &XFree);
  int isGl = 0;
  if (aVis.get() == NULL)
  {
    std::cerr << "Error: XGetVisualInfo is unable to choose needed configuration in existing OpenGL context\n";
    return false;
  }
  else if (glXGetConfig(aDisp, aVis.get(), GLX_USE_GL, &isGl) != 0 || !isGl)
  {
    std::cerr << "Error: window Visual does not support GL rendering\n";
    return false;
  }

  if (theBits == 0)
  {
    myRendCtx = glXCreateContext(aDisp, aVis.get(), NULL, GL_TRUE);
    if (!MakeCurrent())
      return false;

    return true;
  }

  const char* aGlxExts = glXQueryExtensionsString(aDisp, aVisInfo.screen);
  if (!hasExtension(aGlxExts, "GLX_ARB_create_context_profile"))
    return false;

  // FBConfigs were added in GLX version 1.3
  int aGlxMajor = 0, aGlxMinor = 0;
  const bool hasFBCfg = glXQueryVersion(aDisp, &aGlxMajor, &aGlxMinor)
                    && ((aGlxMajor == 1 && aGlxMinor >= 3) || (aGlxMajor > 1));
  if (!hasFBCfg)
    return false;

  // Search for RGBA double-buffered visual with stencil buffer
  static int TheDoubleBuffFBConfig[] =
  {
    GLX_X_RENDERABLE,  True,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_RENDER_TYPE,   GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
    GLX_DEPTH_SIZE,    16,
    GLX_STENCIL_SIZE,  1,
    GLX_DOUBLEBUFFER,  True,
    None
  };

  int aFBCount = 0;
  GLXFBConfig* aFBCfgList = glXChooseFBConfig(aDisp, aScreen, TheDoubleBuffFBConfig, &aFBCount);
  GLXFBConfig  anFBConfig = (aFBCfgList != NULL && aFBCount >= 1) ? aFBCfgList[0] : 0;
  XFree(aFBCfgList);

  // Replace default XError handler to ignore errors.
  // Warning - this is global for all threads!
  struct XErrorsSuppressor
  {
    XErrorsSuppressor() : myOldHandler(XSetErrorHandler(xErrorDummyHandler)) {}
    ~XErrorsSuppressor() { XSetErrorHandler(myOldHandler); }

    static int xErrorDummyHandler(Display* , XErrorEvent* ) { return 0; }

    typedef int (*xerrorhandler_t)(Display* , XErrorEvent* );
    xerrorhandler_t myOldHandler = NULL;
  } anXErrSuppressor;

  typedef GLXContext (*glXCreateContextAttribsARB_t)(Display* dpy, GLXFBConfig config,
                                                     GLXContext share_context, Bool direct,
                                                     const int* attrib_list);
  glXCreateContextAttribsARB_t aCreateCtxProc = (glXCreateContextAttribsARB_t )glXGetProcAddress((const GLubyte* )"glXCreateContextAttribsARB");
  int aCtxAttribs[] =
  {
    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    GLX_CONTEXT_MINOR_VERSION_ARB, 2,
    GLX_CONTEXT_PROFILE_MASK_ARB,  (isCoreCtx || isFwdCtx) ? GLX_CONTEXT_CORE_PROFILE_BIT_ARB : GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
    GLX_CONTEXT_FLAGS_ARB,         (isDebugCtx ? GLX_CONTEXT_DEBUG_BIT_ARB : 0) | (isFwdCtx ? GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB : 0),
    0, 0
  };

  // try to create the core profile of highest OpenGL version
  for (int aLowVer4 = 5; aLowVer4 >= 0 && myRendCtx == NULL; --aLowVer4)
  {
    aCtxAttribs[1] = 4;
    aCtxAttribs[3] = aLowVer4;
    myRendCtx = aCreateCtxProc(aDisp, anFBConfig, NULL, True, aCtxAttribs);
  }
  for (int aLowVer3 = 3; aLowVer3 >= 2 && myRendCtx == NULL; --aLowVer3)
  {
    aCtxAttribs[1] = 3;
    aCtxAttribs[3] = aLowVer3;
    myRendCtx = aCreateCtxProc(aDisp, anFBConfig, NULL, True, aCtxAttribs);
  }

  if (!MakeCurrent())
  {
    Release();
    return false;
  }

  if (isSoftCtx && !aMesaEnvSentry.IsSoftContext(*this))
  {
    Release();
    return false;
  }

  return true;
}

bool GlxContext::MakeCurrent()
{
  if (myRendCtx == 0)
    return false;

  if (!glXMakeCurrent((Display* )myWin.GetDisplay(), (GLXDrawable )myWin.GetDrawable(), (GLXContext )myRendCtx))
  {
    // if there is no current context it might be impossible to use glGetError() correctly
    std::cerr << "glXMakeCurrent() has failed\n";
    return false;
  }
  return true;
}

void* GlxContext::GlGetProcAddress(const char* theFuncName)
{
  return (void*)glXGetProcAddress((const GLubyte*)theFuncName);
}

unsigned int GlxContext::GlGetError()
{
  return ::glGetError();
}

const char* GlxContext::GlGetString(unsigned int theGlEnum)
{
  const char* aStr = (const char*)::glGetString(theGlEnum);
  if (aStr == NULL)
  {
    //
  }
  return aStr;
}

const char* GlxContext::GlGetStringi(unsigned int theGlEnum, unsigned int theIndex)
{
  glGetStringi_t aGetStringi = NULL;
  if (!FindProc("glGetStringi", aGetStringi))
    return NULL;

  return (const char*)aGetStringi(theGlEnum, theIndex);
}

void GlxContext::GlGetIntegerv(unsigned int theGlEnum, int* theParams)
{
  ::glGetIntegerv(theGlEnum, theParams);
}

void GlxContext::PrintPlatformInfo(bool theToPrintExtensions)
{
  Display*  aDisp   = (Display*)myWin.GetDisplay();
  const int aScreen = DefaultScreen(aDisp);

  std::cout << "[" << PlatformName() << "] GLXDirectRendering: " << (glXIsDirect(aDisp, (GLXContext )myRendCtx) ? "Yes" : "No") << "\n";
  std::cout << "[" << PlatformName() << "] GLXVendor:          " << glXQueryServerString(aDisp, aScreen, GLX_VENDOR) << "\n";
  std::cout << "[" << PlatformName() << "] GLXVersion:         " << glXQueryServerString(aDisp, aScreen, GLX_VERSION) << "\n";
  if (theToPrintExtensions)
  {
    const char* aGlxExts = glXQueryExtensionsString(aDisp, aScreen);
    std::cout << "[" << PlatformName() << "] GLX extensions:\n";
    printExtensions(aGlxExts);
  }

  std::cout << "[" << PlatformName() << "] GLXClientVendor:    " << glXGetClientString(aDisp, GLX_VENDOR) << "\n";
  std::cout << "[" << PlatformName() << "] GLXClientVersion:   " << glXGetClientString(aDisp, GLX_VERSION) << "\n";
  if (theToPrintExtensions)
  {
    const char* aGlxExts = glXGetClientString(aDisp, GLX_EXTENSIONS);
    std::cout << "[" << PlatformName() << "] GLXClient extensions:\n";
    printExtensions(aGlxExts);
  }
}

void GlxContext::PrintGpuMemoryInfo()
{
  BaseGlContext::PrintGpuMemoryInfo();

  // Mesa implements other extensions - no need to use GLX_MESA_query_renderer here
  /*Display*    aDisp    = (Display*)myWin.GetDisplay();
  const int   aScreen  = DefaultScreen(aDisp);
  const char* aGlxExts = glXQueryExtensionsString(aDisp, aScreen);

  typedef Bool (*glXQueryCurrentRendererIntegerMESAProc_t)(int attribute, unsigned int* value);
  glXQueryCurrentRendererIntegerMESAProc_t aQueryMESAProc = NULL;
  if (hasExtension(aGlxExts, "GLX_MESA_query_renderer")
   && FindProc("glXQueryCurrentRendererIntegerMESA", aQueryMESAProc))
  {
    unsigned int aVideoMemoryMB = 0;
    aQueryMESAProc(GLX_RENDERER_VIDEO_MEMORY_MESA, &aVideoMemoryMB);
    std::cout << Prefix() << "Mesa GPU memory: " << aVideoMemoryMB << " MiB\n";
  }*/
}

void GlxContext::PrintVisuals(bool theIsVerbose)
{
  (void)theIsVerbose;
}

#endif
