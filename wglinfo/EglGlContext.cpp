// Copyright © Kirill Gavrilov, 2018-2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#include "EglGlContext.h"

#include <cstring>
#include <string>
#include <iomanip>
#include <iostream>

#if defined(__GNUC__) && !defined(__INTEL_COMPILER) && !defined(__clang__)
  #if (__GNUC__ > 8) || ((__GNUC__ == 8) && (__GNUC_MINOR__ >= 1))
    #pragma GCC diagnostic ignored "-Wcast-function-type"
  #endif
#endif

#if !defined(EGL_TRUE)
#define EGL_TRUE                          1
#define EGL_ALPHA_SIZE                    0x3021
#define EGL_BAD_ACCESS                    0x3002
#define EGL_BAD_ALLOC                     0x3003
#define EGL_BAD_ATTRIBUTE                 0x3004
#define EGL_BAD_CONFIG                    0x3005
#define EGL_BAD_CONTEXT                   0x3006
#define EGL_BAD_CURRENT_SURFACE           0x3007
#define EGL_BAD_DISPLAY                   0x3008
#define EGL_BAD_MATCH                     0x3009
#define EGL_BAD_NATIVE_PIXMAP             0x300A
#define EGL_BAD_NATIVE_WINDOW             0x300B
#define EGL_BAD_PARAMETER                 0x300C
#define EGL_BAD_SURFACE                   0x300D
#define EGL_BLUE_SIZE                     0x3022
#define EGL_BUFFER_SIZE                   0x3020
#define EGL_CONFIG_CAVEAT                 0x3027
#define EGL_CONFIG_ID                     0x3028
#define EGL_CORE_NATIVE_ENGINE            0x305B
#define EGL_DEPTH_SIZE                    0x3025
#define EGL_DONT_CARE                     ((EGLint)-1)
#define EGL_DRAW                          0x3059
#define EGL_EXTENSIONS                    0x3055
#define EGL_FALSE                         0
#define EGL_GREEN_SIZE                    0x3023
#define EGL_HEIGHT                        0x3056
#define EGL_LARGEST_PBUFFER               0x3058
#define EGL_LEVEL                         0x3029
#define EGL_MAX_PBUFFER_HEIGHT            0x302A
#define EGL_MAX_PBUFFER_PIXELS            0x302B
#define EGL_MAX_PBUFFER_WIDTH             0x302C
#define EGL_NATIVE_RENDERABLE             0x302D
#define EGL_NATIVE_VISUAL_ID              0x302E
#define EGL_NATIVE_VISUAL_TYPE            0x302F
#define EGL_NONE                          0x3038
#define EGL_NON_CONFORMANT_CONFIG         0x3051
#define EGL_NOT_INITIALIZED               0x3001
#define EGL_PBUFFER_BIT                   0x0001
#define EGL_PIXMAP_BIT                    0x0002
#define EGL_READ                          0x305A
#define EGL_RED_SIZE                      0x3024
#define EGL_SAMPLES                       0x3031
#define EGL_SAMPLE_BUFFERS                0x3032
#define EGL_SLOW_CONFIG                   0x3050
#define EGL_STENCIL_SIZE                  0x3026
#define EGL_SUCCESS                       0x3000
#define EGL_SURFACE_TYPE                  0x3033
#define EGL_TRANSPARENT_BLUE_VALUE        0x3035
#define EGL_TRANSPARENT_GREEN_VALUE       0x3036
#define EGL_TRANSPARENT_RED_VALUE         0x3037
#define EGL_TRANSPARENT_RGB               0x3052
#define EGL_TRANSPARENT_TYPE              0x3034
#define EGL_TRUE                          1
#define EGL_VENDOR                        0x3053
#define EGL_VERSION                       0x3054
#define EGL_WIDTH                         0x3057
#define EGL_WINDOW_BIT                    0x0004

#define EGL_ALPHA_FORMAT                  0x3088
#define EGL_ALPHA_FORMAT_NONPRE           0x308B
#define EGL_ALPHA_FORMAT_PRE              0x308C
#define EGL_ALPHA_MASK_SIZE               0x303E
#define EGL_BUFFER_PRESERVED              0x3094
#define EGL_BUFFER_DESTROYED              0x3095
#define EGL_CLIENT_APIS                   0x308D
#define EGL_COLORSPACE                    0x3087
#define EGL_COLORSPACE_sRGB               0x3089
#define EGL_COLORSPACE_LINEAR             0x308A
#define EGL_COLOR_BUFFER_TYPE             0x303F
#define EGL_CONTEXT_CLIENT_TYPE           0x3097
#define EGL_DISPLAY_SCALING               10000
#define EGL_HORIZONTAL_RESOLUTION         0x3090
#define EGL_LUMINANCE_BUFFER              0x308F
#define EGL_LUMINANCE_SIZE                0x303D
#define EGL_OPENGL_ES_BIT                 0x0001
#define EGL_OPENVG_BIT                    0x0002
#define EGL_OPENGL_ES_API                 0x30A0
#define EGL_OPENVG_API                    0x30A1
#define EGL_OPENVG_IMAGE                  0x3096
#define EGL_PIXEL_ASPECT_RATIO            0x3092
#define EGL_RENDERABLE_TYPE               0x3040
#define EGL_RENDER_BUFFER                 0x3086
#define EGL_RGB_BUFFER                    0x308E
#define EGL_SINGLE_BUFFER                 0x3085
#define EGL_SWAP_BEHAVIOR                 0x3093
#define EGL_UNKNOWN                       ((EGLint)-1)
#define EGL_VERTICAL_RESOLUTION           0x3091

#define EGL_DEFAULT_DISPLAY               ((EGLNativeDisplayType)0)
#define EGL_MULTISAMPLE_RESOLVE_BOX_BIT   0x0200
#define EGL_MULTISAMPLE_RESOLVE           0x3099
#define EGL_MULTISAMPLE_RESOLVE_DEFAULT   0x309A
#define EGL_MULTISAMPLE_RESOLVE_BOX       0x309B
#define EGL_OPENGL_API                    0x30A2
#define EGL_OPENGL_BIT                    0x0008
#define EGL_SWAP_BEHAVIOR_PRESERVED_BIT   0x0400

#define EGL_CONFORMANT                    0x3042
#define EGL_CONTEXT_CLIENT_VERSION        0x3098
#define EGL_MATCH_NATIVE_PIXMAP           0x3041
#define EGL_OPENGL_ES2_BIT                0x0004
#define EGL_OPENGL_ES3_BIT                0x0040
#define EGL_VG_ALPHA_FORMAT               0x3088
#define EGL_VG_ALPHA_FORMAT_NONPRE        0x308B
#define EGL_VG_ALPHA_FORMAT_PRE           0x308C
#define EGL_VG_ALPHA_FORMAT_PRE_BIT       0x0040
#define EGL_VG_COLORSPACE                 0x3087
#define EGL_VG_COLORSPACE_sRGB            0x3089
#define EGL_VG_COLORSPACE_LINEAR          0x308A
#define EGL_VG_COLORSPACE_LINEAR_BIT      0x0020

#define EGL_CONTEXT_MAJOR_VERSION         0x3098
#define EGL_CONTEXT_MINOR_VERSION         0x30FB
#define EGL_CONTEXT_OPENGL_PROFILE_MASK   0x30FD
#define EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT 0x00000001
#define EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT 0x00000002
#define EGL_CONTEXT_OPENGL_DEBUG          0x31B0
#define EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE 0x31B1
#define EGL_CONTEXT_OPENGL_ROBUST_ACCESS  0x31B2

//! Auxiliary template to retrieve function pointer within libEGL.dll.
template<typename FuncType_t> bool EglGlContext::findEglDllProc(const char* theFuncName, FuncType_t& theFuncPtr)
{
#ifdef _WIN32
  theFuncPtr = (FuncType_t)(void*)GetProcAddress((HMODULE)myEglDll, theFuncName);
#else
  theFuncPtr = NULL;
#endif
  return (theFuncPtr != NULL);
}
#endif

void* EglGlContext::GlGetProcAddress(const char* theFuncName)
{
#ifdef _WIN32
  return eglGetProcAddress != NULL ? (void*)eglGetProcAddress(theFuncName) : NULL;
#else
  return (void*)eglGetProcAddress(theFuncName);
#endif
}

EglGlContext::EglGlContext(const std::string& theTitle)
: myWin(new NativeWindow(theTitle))
{
  //
}

EglGlContext::EglGlContext(const std::shared_ptr<BaseWindow>& theWin)
: myWin(theWin)
{
  //
}

bool EglGlContext::LoadEglLibrary(bool theIsMandatory)
{
#if defined(_WIN32)
  if (myEglDll != NULL)
    return true;

#define findEglDllProcShort(theFunc) findEglDllProc(#theFunc, theFunc)

  myEglDll = LoadLibraryW(L"libEGL.dll");
  if (myEglDll == NULL)
  {
    if (theIsMandatory)
    {
      std::cerr << "Error: unable to find libEGL.dll\n";
    }
    return false;
  }

  if (!findEglDllProcShort(eglGetError)
   || !findEglDllProcShort(eglGetProcAddress)
   || !findEglDllProcShort(eglGetDisplay)
   || !findEglDllProcShort(eglInitialize)
   || !findEglDllProcShort(eglTerminate)
   || !findEglDllProcShort(eglMakeCurrent)
   || !findEglDllProcShort(eglGetConfigs)
   || !findEglDllProcShort(eglGetConfigAttrib)
   || !findEglDllProcShort(eglChooseConfig)
   || !findEglDllProcShort(eglBindAPI)
   || !findEglDllProcShort(eglQueryAPI)
   || !findEglDllProcShort(eglCreateContext)
   || !findEglDllProcShort(eglDestroyContext)
   || !findEglDllProcShort(eglCreateWindowSurface)
   || !findEglDllProcShort(eglDestroySurface)
   || !findEglDllProcShort(eglQueryString))
  {
    std::cerr << "Error: broken libEGL.dll\n";
    myEglDll = NULL;
    return false;
  }
  return true;
#elif defined(__APPLE__)
  (void)theIsMandatory;
  return false;
#else
  (void)theIsMandatory;
  return true;
#endif
}

void EglGlContext::release()
{
  if (myEglSurf != EGL_NO_SURFACE)
  {
    eglDestroySurface(myEglDisp, myEglSurf);
    myEglSurf = EGL_NO_SURFACE;
  }

  if (myEglContext != EGL_NO_CONTEXT)
  {
    if (eglMakeCurrent(myEglDisp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_TRUE)
      std::cerr << "Error: FAILED to release OpenGL context!\n";

    eglDestroyContext(myEglDisp, myEglContext);
    myEglContext = EGL_NO_CONTEXT;
  }

  if (myEglDisp != EGL_NO_DISPLAY)
  {
    if (eglTerminate(myEglDisp) != EGL_TRUE)
      std::cerr << "Error: EGL, eglTerminate FAILED!\n";

    myEglDisp = EGL_NO_DISPLAY;
  }
  myWin->Destroy();
}

unsigned int EglGlContext::GlGetError()
{
  if (this->glGetError == NULL)
    FindProc("glGetError", this->glGetError);

  return this->glGetError();
}

const char* EglGlContext::GlGetString(unsigned int theGlEnum)
{
  if (this->glGetString == NULL)
    FindProc("glGetString", this->glGetString);

  return (const char*)this->glGetString(theGlEnum);
}

const char* EglGlContext::GlGetStringi(unsigned int theGlEnum, unsigned int theIndex)
{
  if (this->glGetStringi == NULL)
    FindProc("glGetStringi", this->glGetStringi);

  return (const char*)this->glGetStringi(theGlEnum, theIndex);
}

void EglGlContext::GlGetIntegerv(unsigned int theGlEnum, int* theParams)
{
  if (this->glGetIntegerv == NULL)
    FindProc("glGetIntegerv", this->glGetIntegerv);

  this->glGetIntegerv(theGlEnum, theParams);
}

bool EglGlContext::MakeCurrent()
{
  if (myEglContext == EGL_NO_CONTEXT)
    return false;

  return eglMakeCurrent(myEglDisp, myEglSurf, myEglSurf, myEglContext) == EGL_TRUE;
}

bool EglGlContext::CreateGlContext(ContextBits theBits)
{
  Release();
  if (!LoadEglLibrary())
    return false;

  myCtxBits = theBits;

  const bool isDebugCtx = (theBits & ContextBits_Debug) != 0;
  const bool isFwdCtx   = (theBits & ContextBits_ForwardProfile) != 0;
  const bool isCoreCtx  = (theBits & ContextBits_CoreProfile) != 0;
  const bool isSoftCtx  = (theBits & ContextBits_SoftProfile) != 0;
  const bool isGles     = (theBits & ContextBits_GLES) != 0;

  SoftMesaSentry aMesaEnvSentry;
  if (isSoftCtx)
  {
  #ifdef _WIN32
    return false;
  #else
    EglGlContext aCtxCompat(myWin->EmptyCopy("wglinfoTmp"));
    if (!aCtxCompat.CreateGlContext(ContextBits_NONE)
     || !aMesaEnvSentry.Init(aCtxCompat))
    {
      return false;
    }
  #endif
  }

  if (!myWin->Create())
  {
    Release();
    return false;
  }

  if (myWin->GetDisplay() != 0)
    myEglDisp = eglGetDisplay((EGLNativeDisplayType)myWin->GetDisplay());
  else
    myEglDisp = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  if (myEglDisp == EGL_NO_DISPLAY)
  {
    std::cerr << "Error: no EGL display!\n";
    return false;
  }

  EGLint aVerMajor = 0, aVerMinor = 0;
  if (eglInitialize(myEglDisp, &aVerMajor, &aVerMinor) != EGL_TRUE)
  {
    std::cerr << "Error: EGL display is unavailable!\n";
    return false;
  }

  EGLint aConfigAttribs[] =
  {
    EGL_RED_SIZE,     8,
    EGL_GREEN_SIZE,   8,
    EGL_BLUE_SIZE,    8,
    EGL_ALPHA_SIZE,   0,
    EGL_DEPTH_SIZE,   24,
    EGL_STENCIL_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE
  };

  EGLConfig anEglCfg = NULL;
  for (int aGlesVer = isGles ? 3 : 2; aGlesVer >= 2; --aGlesVer)
  {
    aConfigAttribs[6 * 2 + 1] = isGles ? (aGlesVer == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT) : EGL_OPENGL_BIT;
    aConfigAttribs[4 * 2 + 1] = 24;
    EGLint aNbConfigs = 0;
    if (eglChooseConfig(myEglDisp, aConfigAttribs, &anEglCfg, 1, &aNbConfigs) != EGL_TRUE
     || anEglCfg == NULL)
    {
      eglGetError();
      aConfigAttribs[4 * 2 + 1] = 16; // try config with smaller depth buffer
      if (eglChooseConfig(myEglDisp, aConfigAttribs, &anEglCfg, 1, &aNbConfigs) != EGL_TRUE
       || anEglCfg == NULL)
      {
        eglGetError();
        continue;
      }
    }
    break;
  }
  if (anEglCfg == NULL)
  {
    //std::cerr << "Error: EGL does not provide compatible configurations!\n";
    return false;
  }

  const bool hasGLES3 = (aConfigAttribs[6 * 2 + 1] == EGL_OPENGL_ES3_BIT);
  if (eglBindAPI(isGles ? EGL_OPENGL_ES_API : EGL_OPENGL_API) != EGL_TRUE)
  {
    std::cerr << "Error: EGL does not provide " << (isGles ? "OpenGL ES" : "OpenGL") << " client!\n";
    return false;
  }

  if (isGles)
  {
    EGLint aCtxAttribs[] =
    {
      EGL_CONTEXT_CLIENT_VERSION, hasGLES3 ? 3 : 2, EGL_NONE, EGL_NONE
    };

    myEglContext = eglCreateContext(myEglDisp, anEglCfg, EGL_NO_CONTEXT, aCtxAttribs);
    if (myEglContext == EGL_NO_CONTEXT && hasGLES3)
    {
      aCtxAttribs[1] = 2;
      myEglContext = eglCreateContext(myEglDisp, anEglCfg, EGL_NO_CONTEXT, aCtxAttribs);
    }
  }
  else if (theBits != 0)
  {
    EGLint aCtxAttribs[] =
    {
      EGL_CONTEXT_MAJOR_VERSION, 3,
      EGL_CONTEXT_MINOR_VERSION, 2,
      EGL_CONTEXT_OPENGL_PROFILE_MASK, isCoreCtx ? EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT : EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT,
      EGL_CONTEXT_OPENGL_DEBUG, isDebugCtx ? EGL_TRUE : EGL_FALSE,
      EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, isFwdCtx ? EGL_TRUE : EGL_FALSE,
      EGL_NONE, EGL_NONE
    };

    // try to create the core profile of highest OpenGL version
    for (int aLowVer4 = 5; aLowVer4 >= 0 && myEglContext == EGL_NO_CONTEXT; --aLowVer4)
    {
      aCtxAttribs[1] = 4;
      aCtxAttribs[3] = aLowVer4;
      myEglContext = eglCreateContext(myEglDisp, anEglCfg, EGL_NO_CONTEXT, aCtxAttribs);
    }
    for (int aLowVer3 = 3; aLowVer3 >= 2 && myEglContext == EGL_NO_CONTEXT; --aLowVer3)
    {
      aCtxAttribs[1] = 3;
      aCtxAttribs[3] = aLowVer3;
      myEglContext = eglCreateContext(myEglDisp, anEglCfg, EGL_NO_CONTEXT, aCtxAttribs);
    }
  }
  else
  {
    myEglContext = eglCreateContext(myEglDisp, anEglCfg, EGL_NO_CONTEXT, NULL);
  }

  if (myEglContext == EGL_NO_CONTEXT)
  {
    //std::cerr << "Error: EGL is unable to create OpenGL context!\n";
    return false;
  }

  myEglSurf = eglCreateWindowSurface(myEglDisp, anEglCfg, (EGLNativeWindowType)myWin->GetDrawable(), NULL);
  if (myEglSurf == EGL_NO_SURFACE)
  {
    std::cerr << "Error: EGL is unable to create surface for window!\n";
    return false;
  }

  if (!MakeCurrent())
  {
    std::cerr << "Error: eglMakeCurrent() has failed!\n";
    return false;
  }

#ifndef _WIN32
  if (isSoftCtx && !aMesaEnvSentry.IsSoftContext(*this))
  {
    Release();
    return false;
  }
#endif

  return true;
}

void EglGlContext::PrintPlatformInfo(bool theToPrintExtensions)
{
  if (myEglDisp == EGL_NO_DISPLAY)
    return;

  std::cout << "[" << PlatformName() << "] EGLVersion:    " << eglQueryString(myEglDisp, EGL_VERSION) << "\n";
  std::cout << "[" << PlatformName() << "] EGLVendor:     " << eglQueryString(myEglDisp, EGL_VENDOR) << "\n";
  std::cout << "[" << PlatformName() << "] EGLClientAPIs: " << eglQueryString(myEglDisp, EGL_CLIENT_APIS) << "\n";
  if (theToPrintExtensions)
  {
    std::cout << "[" << PlatformName() << "] EGL extensions:\n";
    printExtensions(eglQueryString(myEglDisp, EGL_EXTENSIONS));
  }
}

void EglGlContext::PrintVisuals(bool theIsVerbose)
{
#ifdef _WIN32
  if (myEglDll == NULL)
    return;
#endif

  struct EGLConfigAttribs
  {
    EGLint ConfigId;
    EGLint ConfigCaveat;
    EGLint RenderbableType;
    EGLint BufferType;
    EGLint SurfaceType;
    EGLint ColorSize;
    EGLint RedSize;
    EGLint GreenSize;
    EGLint BlueSize;
    EGLint AlphaSize;
    EGLint DepthSize;
    EGLint StencilSize;
  };

  EGLint aNbConfigs = 0;
  eglGetConfigs(myEglDisp, NULL, 0, &aNbConfigs);
  EGLConfig* aConfigs = new EGLConfig[aNbConfigs];
  memset(aConfigs, 0, sizeof(EGLConfig) * aNbConfigs);
  if (eglGetConfigs(myEglDisp, aConfigs, aNbConfigs, &aNbConfigs) != EGL_TRUE)
  {
    delete[] aConfigs;
    return;
  }

  std::cout << "\n[" << PlatformName() << "] " << aNbConfigs << " EGL Configs\n";
  if (!theIsVerbose)
  {
    std::cout << "    visual  x  bf lv rg d st  r  g  b a  ax dp st accum buffs  ms \n"
                 "  id dep cl sp sz l  ci b ro sz sz sz sz bf th cl  r  g  b  a ns b\n"
                 "------------------------------------------------------------------\n";
  }
  for (int aCfgIter = 0; aCfgIter < aNbConfigs; ++aCfgIter)
  {
    const EGLConfig aCfg = aConfigs[aCfgIter];
    EGLConfigAttribs anAttribs;
    memset(&anAttribs, 0, sizeof(EGLConfigAttribs));
    eglGetConfigAttrib(myEglDisp, aCfg, EGL_CONFIG_ID, &anAttribs.ConfigId);
    eglGetConfigAttrib(myEglDisp, aCfg, EGL_CONFIG_CAVEAT, &anAttribs.ConfigCaveat);
    eglGetConfigAttrib(myEglDisp, aCfg, EGL_RENDERABLE_TYPE, &anAttribs.RenderbableType);
    eglGetConfigAttrib(myEglDisp, aCfg, EGL_COLOR_BUFFER_TYPE, &anAttribs.BufferType);
    eglGetConfigAttrib(myEglDisp, aCfg, EGL_SURFACE_TYPE, &anAttribs.SurfaceType);
    eglGetConfigAttrib(myEglDisp, aCfg, EGL_BUFFER_SIZE, &anAttribs.ColorSize);
    eglGetConfigAttrib(myEglDisp, aCfg, EGL_RED_SIZE, &anAttribs.RedSize);
    eglGetConfigAttrib(myEglDisp, aCfg, EGL_GREEN_SIZE, &anAttribs.GreenSize);
    eglGetConfigAttrib(myEglDisp, aCfg, EGL_BLUE_SIZE, &anAttribs.BlueSize);
    eglGetConfigAttrib(myEglDisp, aCfg, EGL_ALPHA_SIZE, &anAttribs.AlphaSize);
    eglGetConfigAttrib(myEglDisp, aCfg, EGL_DEPTH_SIZE, &anAttribs.DepthSize);
    eglGetConfigAttrib(myEglDisp, aCfg, EGL_STENCIL_SIZE, &anAttribs.StencilSize);

    if (theIsVerbose)
    {
      std::cout << "Config: " << aCfgIter << "\n"
        << "    color: R" << int(anAttribs.RedSize) << "G" << int(anAttribs.GreenSize) << "B" << int(anAttribs.BlueSize) << "A" << int(anAttribs.AlphaSize)
        << " (" << getColorBufferClass(anAttribs.ColorSize, anAttribs.RedSize) << ", " << int(anAttribs.ColorSize) << ")"
        << " depth: " << int(anAttribs.DepthSize) << " stencil: " << int(anAttribs.StencilSize) << "\n"
        << "    caveat: " << ((anAttribs.ConfigCaveat & EGL_SLOW_CONFIG) != 0 ? "slow " : " ")
        << ((anAttribs.ConfigCaveat & EGL_NON_CONFORMANT_CONFIG) != 0 ? "non-conformant" : " ") << "\n"
        << "    renderableTypes: " << ((anAttribs.RenderbableType & EGL_OPENGL_ES2_BIT) != 0 ? "GLES2 " : " ")
        << ((anAttribs.RenderbableType & EGL_OPENGL_ES3_BIT) != 0 ? "GLES3 " : " ")
        << ((anAttribs.RenderbableType & EGL_OPENGL_BIT) != 0 ? "GL" : " ") << "\n";
      continue;
    }

    std::cout << "0x" << std::hex << std::setw(3) << std::setfill('0') << aCfgIter << std::dec << std::setfill(' ') << " ";
    std::cout << std::setw(2) << (int)anAttribs.ColorSize << " ";

    std::cout << ((anAttribs.SurfaceType & EGL_WINDOW_BIT) != 0
                  ? "wn "
                  : ((anAttribs.SurfaceType & EGL_PIXMAP_BIT) != 0
                      ? "bm "
                      : ".  "));

    std::cout << " . " << std::setw(2) << (int)anAttribs.ColorSize << " ";
    std::cout << " . ";
    std::cout << " " << (anAttribs.BufferType == EGL_RGB_BUFFER ? "r" : "l") << " "
      << '.' << " "
      << " " << '.' << " ";
    printInt2d(anAttribs.RedSize   && anAttribs.BufferType == EGL_RGB_BUFFER ? (int)anAttribs.RedSize : -1);
    printInt2d(anAttribs.GreenSize && anAttribs.BufferType == EGL_RGB_BUFFER ? (int)anAttribs.GreenSize : -1);
    printInt2d(anAttribs.BlueSize  && anAttribs.BufferType == EGL_RGB_BUFFER ? (int)anAttribs.BlueSize : -1);
    printInt2d(anAttribs.AlphaSize && anAttribs.BufferType == EGL_RGB_BUFFER ? (int)anAttribs.AlphaSize : -1);
    printInt2d(-1);
    printInt2d(anAttribs.DepthSize ? (int)anAttribs.DepthSize : -1);
    printInt2d(anAttribs.StencilSize ? (int)anAttribs.StencilSize : -1);
    printInt2d(-1);
    printInt2d(-1);
    printInt2d(-1);
    printInt2d(-1);

    std::cout << " . .\n";
  }
  delete[] aConfigs;

  // table footer
  if (!theIsVerbose)
  {
    std::cout << "------------------------------------------------------------------\n"
                 "    visual  x  bf lv rg d st  r  g  b a  ax dp st accum buffs  ms \n"
                 "  id dep cl sp sz l  ci b ro sz sz sz sz bf th cl  r  g  b  a ns b\n"
                 "------------------------------------------------------------------\n\n";
  }
}
