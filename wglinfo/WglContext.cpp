// Copyright © Kirill Gavrilov, 2018-2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#include "WglContext.h"

#if defined(_WIN32)

#include <GL/gl.h>

#include <iomanip>
#include <iostream>
#include <memory>

#if defined(__GNUC__) && !defined(__INTEL_COMPILER) && !defined(__clang__)
  #if (__GNUC__ > 8) || ((__GNUC__ == 8) && (__GNUC_MINOR__ >= 1))
    #pragma GCC diagnostic ignored "-Wcast-function-type"
  #endif
#endif

#define GLAPIENTRY WINAPI

typedef const GLubyte* (GLAPIENTRY *glGetStringi_t) (GLenum name, GLuint index);

// WGL_ARB_pixel_format
#define WGL_NUMBER_PIXEL_FORMATS_ARB            0x2000
#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_DRAW_TO_BITMAP_ARB                  0x2002
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_NUMBER_OVERLAYS_ARB                 0x2008
#define WGL_NUMBER_UNDERLAYS_ARB                0x2009
#define WGL_SUPPORT_GDI_ARB                     0x200F
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_STEREO_ARB                          0x2012
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_COLOR_BITS_ARB                      0x2014
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB                    0x2023
#define WGL_RED_BITS_ARB                        0x2015
#define WGL_RED_SHIFT_ARB                       0x2016
#define WGL_GREEN_BITS_ARB                      0x2017
#define WGL_GREEN_SHIFT_ARB                     0x2018
#define WGL_BLUE_BITS_ARB                       0x2019
#define WGL_BLUE_SHIFT_ARB                      0x201A
#define WGL_ALPHA_BITS_ARB                      0x201B
#define WGL_ALPHA_SHIFT_ARB                     0x201C
#define WGL_ACCUM_BITS_ARB                      0x201D
#define WGL_ACCUM_RED_BITS_ARB                  0x201E
#define WGL_ACCUM_GREEN_BITS_ARB                0x201F
#define WGL_ACCUM_BLUE_BITS_ARB                 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB                0x2021
#define WGL_AUX_BUFFERS_ARB                     0x2024
#define WGL_NO_ACCELERATION_ARB                 0x2025
#define WGL_GENERIC_ACCELERATION_ARB            0x2026
#define WGL_FULL_ACCELERATION_ARB               0x2027

#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_TYPE_COLORINDEX_ARB                 0x202C

// WGL_ARB_pixel_format_float
#define WGL_TYPE_RGBA_FLOAT_ARB                 0x21A0

// WGL_ARB_create_context_profile
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

// WGL_CONTEXT_FLAGS bits
#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

// WGL_CONTEXT_PROFILE_MASK_ARB bits
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

// WGL_EXT_create_context_es_profile
#define WGL_CONTEXT_ES_PROFILE_BIT_EXT  0x00000004
#define WGL_CONTEXT_ES2_PROFILE_BIT_EXT 0x00000004

#define GL_NUM_EXTENSIONS 0x821D

typedef const char* (WINAPI *wglGetExtensionsStringARB_t)(HDC theDeviceContext);
typedef BOOL(WINAPI *wglChoosePixelFormatARB_t)(HDC theDevCtx, const int* theIntAttribs,
                                                const float* theFloatAttribs, unsigned int theMaxFormats,
                                                int* theFormatsOut, unsigned int* theNumFormatsOut);
typedef BOOL(WINAPI *wglGetPixelFormatAttribivARB_t)(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int   *piValues);
typedef BOOL(WINAPI *wglGetPixelFormatAttribfvARB_t)(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues);
typedef HGLRC(WINAPI *wglCreateContextAttribsARB_t)(HDC theDevCtx, HGLRC theShareContext, const int* theAttribs);

// WGL_EXT_colorspace
#define WGL_COLORSPACE_EXT        0x309D
#define WGL_COLORSPACE_SRGB_EXT   0x3089
#define WGL_COLORSPACE_LINEAR_EXT 0x308A

// WGL_EXT_multisample
#define WGL_SAMPLE_BUFFERS_EXT 0x2041
#define WGL_SAMPLES_EXT        0x2042

typedef const char* (WINAPI *wglGetExtensionsStringARB_t)(HDC theDeviceContext);

static bool printLastSystemError(bool theToPrintSuccess = false)
{
  const DWORD anErrorCode = GetLastError();
  if (!theToPrintSuccess && anErrorCode == 0)
    return false;

  char* aMsgBuff = nullptr;
  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                 nullptr, anErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&aMsgBuff, 0, nullptr);
  if (aMsgBuff != nullptr)
  {
    std::cerr << aMsgBuff << "\n";
    LocalFree(aMsgBuff);
  }
  return anErrorCode != 0;
}

WglContext::WglContext(const std::string& theTitle)
: myWin(theTitle)
{
  //
}

void WglContext::release()
{
  if (myRendCtx != NULL)
  {
    wglMakeCurrent(NULL, NULL);
  }
  if (!myWin.IsNull() && myDevCtx != NULL)
  {
    ::ReleaseDC((HWND)myWin.GetDrawable(), myDevCtx);
    myDevCtx = NULL;
  }
  if (myRendCtx != NULL)
  {
    ::wglDeleteContext((HGLRC)myRendCtx);
    myRendCtx = NULL;
  }
  myWin.Destroy();
}

bool WglContext::createWindowHandle()
{
  Release();
  if (!myWin.Create())
    return false;

  myDevCtx = ::GetDC((HWND)myWin.GetDrawable());
  return true;
}

bool WglContext::setWindowPixelFormat(int theFormat)
{
  PIXELFORMATDESCRIPTOR aFormat;
  memset(&aFormat, 0, sizeof(aFormat));
  aFormat.nSize = sizeof(aFormat);
  aFormat.nVersion = 1;
  aFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
  aFormat.iPixelType = PFD_TYPE_RGBA;
  aFormat.cColorBits = 32;
  const int aFormatIndex = theFormat == -1 ? ::ChoosePixelFormat(myDevCtx, &aFormat) : theFormat;
  if (aFormatIndex == 0)
  {
    std::cerr << "Error: ChoosePixelFormat() failed, Cannot find a suitable pixel format.\n";
    return false;
  }

  if (theFormat != -1)
    ::DescribePixelFormat(myDevCtx, aFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &aFormat);

  if (::SetPixelFormat(myDevCtx, aFormatIndex, &aFormat) == FALSE)
  {
    std::cerr << "Error: SetPixelFormat(" << aFormatIndex << ") failed with error code " << GetLastError() << "\n";
    return false;
  }
  return true;
}

bool WglContext::CreateGlContext(ContextBits theBits)
{
  if (!createWindowHandle())
    return false;

  myCtxBits = theBits;

  const bool isDebugCtx = (theBits & ContextBits_Debug) != 0;
  const bool isFwdCtx   = (theBits & ContextBits_ForwardProfile) != 0;
  const bool isCoreCtx  = (theBits & ContextBits_CoreProfile) != 0;
  const bool isSoftCtx  = (theBits & ContextBits_SoftProfile) != 0;
  const bool isGles     = (theBits & ContextBits_GLES) != 0;
  if (theBits == 0)
  {
    if (!setWindowPixelFormat())
      return false;

    myRendCtx = myDevCtx != NULL ? ::wglCreateContext(myDevCtx) : NULL;
    if (!MakeCurrent())
      return false;

    ///::ShowWindow((HWND)myWin.GetDrawable(), SW_HIDE);
    return true;
  }

  // have to create a tempory context first in case of WGL
  WglContext aCtxCompat("wglinfoTmp");
  if (!aCtxCompat.CreateGlContext(ContextBits_NONE))
    return false;

  // in WGL world wglGetProcAddress() returns NULL if extensions is unavailable,
  // so that checking for extension string can be skipped
  //if (hasExtension(aWglExts, "WGL_ARB_pixel_format"))
  //if (hasExtension(aWglExts, "WGL_ARB_create_context_profile"))
  wglChoosePixelFormatARB_t    aChoosePixProc = (wglChoosePixelFormatARB_t)wglGetProcAddress("wglChoosePixelFormatARB");
  wglCreateContextAttribsARB_t aCreateCtxProc = (wglCreateContextAttribsARB_t)wglGetProcAddress("wglCreateContextAttribsARB");
  if (aChoosePixProc == NULL || aCreateCtxProc == NULL)
    return false;

  const int aPixAttribs[] =
  {
    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
    WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
    WGL_STEREO_ARB,         GL_FALSE,
    WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
    WGL_COLOR_BITS_ARB,     24,
    WGL_DEPTH_BITS_ARB,     24,
    WGL_STENCIL_BITS_ARB,   8,
    WGL_ACCELERATION_ARB,   isSoftCtx ? WGL_NO_ACCELERATION_ARB : WGL_FULL_ACCELERATION_ARB,
    0, 0,
  };
  unsigned int aFrmtsNb = 0;
  int aPixelFrmtId = 0;
  if (!aChoosePixProc(myDevCtx, aPixAttribs, NULL, 1, &aPixelFrmtId, &aFrmtsNb)
    ||  aPixelFrmtId == 0
    || !setWindowPixelFormat(aPixelFrmtId))
  {
    return false;
  }

  int aProfileBit = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
  if (isCoreCtx || isFwdCtx)
    aProfileBit = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
  if (isGles)
    aProfileBit = WGL_CONTEXT_ES_PROFILE_BIT_EXT;

  if (isGles)
  {
    int aGlesCtxAttribs[] =
    {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
      WGL_CONTEXT_MINOR_VERSION_ARB, 2,
      WGL_CONTEXT_PROFILE_MASK_ARB,  aProfileBit,
      WGL_CONTEXT_FLAGS_ARB,         isDebugCtx ? WGL_CONTEXT_DEBUG_BIT_ARB : 0,
      0, 0
    };

    for (int aLowVer3 = 3; aLowVer3 >= 0 && myRendCtx == NULL; --aLowVer3)
    {
      aGlesCtxAttribs[1] = 3;
      aGlesCtxAttribs[3] = aLowVer3;
      myRendCtx = aCreateCtxProc(myDevCtx, NULL, aGlesCtxAttribs);
    }
    if (myRendCtx == NULL)
    {
      aGlesCtxAttribs[1] = 2;
      aGlesCtxAttribs[3] = 0;
      myRendCtx = aCreateCtxProc(myDevCtx, NULL, aGlesCtxAttribs);
    }
  }
  else if (isCoreCtx || isDebugCtx || isFwdCtx)
  {
    int aCoreCtxAttribs[] =
    {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
      WGL_CONTEXT_MINOR_VERSION_ARB, 2,
      WGL_CONTEXT_PROFILE_MASK_ARB,  aProfileBit,
      WGL_CONTEXT_FLAGS_ARB,         (isDebugCtx ? WGL_CONTEXT_DEBUG_BIT_ARB : 0) | (isFwdCtx ? WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB : 0),
      0, 0
    };

    // Try to create the core profile of highest OpenGL version
    // (this will be done automatically by some drivers when requesting 3.2,
    //  but some will not (e.g. AMD Catalyst) since WGL_ARB_create_context_profile specification allows both implementations).
    for (int aLowVer4 = 7; aLowVer4 >= 0 && myRendCtx == NULL; --aLowVer4)
    {
      aCoreCtxAttribs[1] = 4;
      aCoreCtxAttribs[3] = aLowVer4;
      myRendCtx = aCreateCtxProc(myDevCtx, NULL, aCoreCtxAttribs);
    }
    for (int aLowVer3 = 3; aLowVer3 >= 2 && myRendCtx == NULL; --aLowVer3)
    {
      aCoreCtxAttribs[1] = 3;
      aCoreCtxAttribs[3] = aLowVer3;
      myRendCtx = aCreateCtxProc(myDevCtx, NULL, aCoreCtxAttribs);
    }
  }
  else
  {
    myRendCtx = myDevCtx != NULL ? ::wglCreateContext(myDevCtx) : NULL;
  }

  //aCtxCompat.MakeCurrent();
  //aCtxCompat.myWin.Quit();
  //wglMakeCurrent(NULL, NULL);
  aCtxCompat.Release();

  if (!MakeCurrent())
    return false;

  ///::ShowWindow((HWND)myWin.GetDrawable(), SW_HIDE);
  return myRendCtx != NULL;
}

bool WglContext::MakeCurrent()
{
  if (myRendCtx == 0)
    return false;

  if (::wglMakeCurrent(myDevCtx, (HGLRC )myRendCtx) != TRUE)
  {
    printLastSystemError();
    return false;
  }
  return true;
}

void* WglContext::GlGetProcAddress(const char* theFuncName)
{
  return (void*)wglGetProcAddress(theFuncName);
}

unsigned int WglContext::GlGetError()
{
  return ::glGetError();
}

const char* WglContext::GlGetString(unsigned int theGlEnum)
{
  const char* aStr = (const char*)::glGetString(theGlEnum);
  if (aStr == NULL)
  {
    //
  }
  return aStr;
}

const char* WglContext::GlGetStringi(unsigned int theGlEnum, unsigned int theIndex)
{
  glGetStringi_t aGetStringi = NULL;
  if (!FindProc("glGetStringi", aGetStringi))
    return NULL;

  return (const char*)aGetStringi(theGlEnum, theIndex);
}

void WglContext::GlGetIntegerv(unsigned int theGlEnum, int* theParams)
{
  ::glGetIntegerv(theGlEnum, theParams);
}

void WglContext::PrintPlatformInfo(bool theToPrintExtensions)
{
  std::cout << "[" << PlatformName() << "] WGLName:       opengl32.dll\n";
  if (!theToPrintExtensions)
    return;

  const char* aWglExts = NULL;
  wglGetExtensionsStringARB_t wglGetExtensionsStringARB = NULL;
  if (FindProc("wglGetExtensionsStringARB", wglGetExtensionsStringARB))
    aWglExts = wglGetExtensionsStringARB(wglGetCurrentDC());

  std::cout << "[" << PlatformName() << "] WGL extensions:\n";
  printExtensions(aWglExts);
}

void WglContext::PrintGpuMemoryInfo()
{
  BaseGlContext::PrintGpuMemoryInfo();

  typedef INT(WINAPI *wglGetGPUInfoAMD_t)(UINT theId, INT theProperty, GLenum theDataType, UINT theSize, void* theData);
  typedef UINT(WINAPI *wglGetContextGPUIDAMD_t)(HGLRC theHglrc);
  wglGetGPUInfoAMD_t      wglGetGPUInfoAMD = NULL;
  wglGetContextGPUIDAMD_t wglGetContextGPUIDAMD = NULL;
  FindProc("wglGetGPUInfoAMD", wglGetGPUInfoAMD);
  FindProc("wglGetContextGPUIDAMD", wglGetContextGPUIDAMD);
  //if (checkGlExtension (aWglExts, "WGL_AMD_gpu_association"))
  if (wglGetGPUInfoAMD != NULL && wglGetContextGPUIDAMD != NULL)
  {
    GLuint aVMemMiB = 0;
    UINT anAmdId = wglGetContextGPUIDAMD((HGLRC)myRendCtx);
    if (anAmdId != 0 && wglGetGPUInfoAMD(anAmdId, 0x21A3, GL_UNSIGNED_INT, sizeof(aVMemMiB), &aVMemMiB) > 0) // WGL_GPU_RAM_AMD = 0x21A3
    {
      std::cout << Prefix() << "GPU memory: " << aVMemMiB << " MiB\n";
    }
  }
}

void WglContext::PrintVisuals(bool theIsVerbose)
{
  if (myDevCtx == nullptr)
    return;

  std::string aWglExt;
  wglGetExtensionsStringARB_t wglGetExtensionsStringARB = nullptr;
  if (FindProc("wglGetExtensionsStringARB", wglGetExtensionsStringARB))
  {
    if (const char* aWglExtRaw = wglGetExtensionsStringARB(myDevCtx))
      aWglExt = aWglExtRaw;
  }

  const bool hasExtColorspace  = hasExtension(aWglExt, "WGL_EXT_colorspace");
  const bool hasExtMultisample = hasExtension(aWglExt, "WGL_ARB_multisample")
                              || hasExtension(aWglExt, "WGL_EXT_multisample");

  wglGetPixelFormatAttribivARB_t aGetAttribIProc = nullptr;
  //wglGetPixelFormatAttribfvARB_t aGetAttribFProc = nullptr;
  FindProc("wglGetPixelFormatAttribivARB", aGetAttribIProc);
  //FindProc("wglGetPixelFormatAttribfvARB", aGetAttribFProc);
  const auto getAttrEx = [this, &aWglExt, aGetAttribIProc]
    (int theFormat, int theAttr, int theDef = 0) -> int
  {
    if (aGetAttribIProc == nullptr)
      return theDef;

    const int anAttribs[1] = { theAttr };
    int       aValInt[1]   = { theDef };
    if (aGetAttribIProc(myDevCtx, theFormat, 0, 1, anAttribs, aValInt))
      return *aValInt;

    printLastSystemError();
    return theDef;
  };

  const int aNbFormatsBase = DescribePixelFormat(myDevCtx, 0, 0, nullptr);
  const int aNbFormatsEx   = hasExtension(aWglExt, "WGL_ARB_pixel_format")
                           ? getAttrEx(0, WGL_NUMBER_PIXEL_FORMATS_ARB)
                           : 0;
  const int aNbFormatsAll = aNbFormatsEx > aNbFormatsBase ? aNbFormatsEx : aNbFormatsBase;

  std::cout << "\n[" << PlatformName() << "] " << aNbFormatsAll << " WGL Visuals";
  if (aNbFormatsEx > aNbFormatsBase)
    std::cout << " (" << aNbFormatsBase << " basic + " << (aNbFormatsEx - aNbFormatsBase) << " extra)";

  std::cout << "\n";

  if (!theIsVerbose)
    VisualInfo::PrintTableHeader(true);

  for (int aFormatIter = 1; aFormatIter <= aNbFormatsAll; ++aFormatIter)
  {
    VisualInfo anInfo;
    anInfo.ConfigId = aFormatIter;
    anInfo.LayerLevel = 0;

    const char* anAccelStr = "";
    if (aFormatIter > aNbFormatsBase)
    {
      if (aFormatIter == aNbFormatsBase + 1)
        VisualInfo::PrintTableSeparator();

      if (getAttrEx(aFormatIter, WGL_SUPPORT_OPENGL_ARB) == 0)
        continue;

      anInfo.ColorBufferSize = getAttrEx(aFormatIter, WGL_COLOR_BITS_ARB);

      const int aPixType = getAttrEx(aFormatIter, WGL_PIXEL_TYPE_ARB);
      anInfo.IsColorFloat = aPixType == WGL_TYPE_RGBA_FLOAT_ARB;
      anInfo.BufferType = VisualInfo::ColorBuffer_Rgba;
      if (aPixType == WGL_TYPE_COLORINDEX_ARB)
        anInfo.BufferType = VisualInfo::ColorBuffer_ColorIndex;
      else if (aPixType == WGL_TYPE_RGBA_ARB || aPixType == WGL_TYPE_RGBA_FLOAT_ARB)
        anInfo.BufferType = VisualInfo::ColorBuffer_Rgba;

      if (aPixType != WGL_TYPE_COLORINDEX_ARB)
      {
        anInfo.RedSize   = getAttrEx(aFormatIter, WGL_RED_BITS_ARB);
        anInfo.GreenSize = getAttrEx(aFormatIter, WGL_GREEN_BITS_ARB);
        anInfo.BlueSize  = getAttrEx(aFormatIter, WGL_BLUE_BITS_ARB);
        anInfo.AlphaSize = getAttrEx(aFormatIter, WGL_ALPHA_BITS_ARB);
      }
      anInfo.DepthSize   = getAttrEx(aFormatIter, WGL_DEPTH_BITS_ARB);
      anInfo.StencilSize = getAttrEx(aFormatIter, WGL_STENCIL_BITS_ARB);

      const int anAccel = getAttrEx(aFormatIter, WGL_ACCELERATION_ARB);
      anInfo.IsSoftware = anAccel == WGL_NO_ACCELERATION_ARB;

      anInfo.ConfigCaveat = VisualInfo::Caveat_None;
      switch (anAccel)
      {
        case WGL_FULL_ACCELERATION_ARB:
          anAccelStr = "icd";
          break;
        case WGL_GENERIC_ACCELERATION_ARB:
          anAccelStr = "mcd";
          anInfo.ConfigCaveat = VisualInfo::Caveat_Slow;
          break;
        case WGL_NO_ACCELERATION_ARB:
          anAccelStr = "gdi";
          break;
        default:
          anAccelStr = "unknown";
          anInfo.ConfigCaveat = VisualInfo::Caveat_NonConformant;
          break;
      }

      // extended pixel formats are "non displayable",
      // hence window/bitmap bits are always zeros
      anInfo.SurfaceType = VisualInfo::Surface_None;
      if (getAttrEx(aFormatIter, WGL_DRAW_TO_WINDOW_ARB) != 0)
        anInfo.SurfaceType = VisualInfo::Surface(anInfo.SurfaceType | VisualInfo::Surface_Window);
      if (getAttrEx(aFormatIter, WGL_DRAW_TO_BITMAP_ARB) != 0)
        anInfo.SurfaceType = VisualInfo::Surface(anInfo.SurfaceType | VisualInfo::Surface_Pixmap);

      anInfo.SwapIntervalMin = 0;
      anInfo.SwapIntervalMax = getAttrEx(aFormatIter, WGL_DOUBLE_BUFFER_ARB) != 0 ? 1 : 0;
      anInfo.IsStereoBuffer  = getAttrEx(aFormatIter, WGL_STEREO_ARB) != 0;

      anInfo.NbAuxBuffers    = getAttrEx(aFormatIter, WGL_AUX_BUFFERS_ARB);
      //anInfo.AccumBufferSize = getAttrEx(aFormatIter, WGL_ACCUM_BITS_ARB);
      anInfo.AccumRedSize    = getAttrEx(aFormatIter, WGL_ACCUM_RED_BITS_ARB);
      anInfo.AccumGreenSize  = getAttrEx(aFormatIter, WGL_ACCUM_GREEN_BITS_ARB);
      anInfo.AccumBlueSize   = getAttrEx(aFormatIter, WGL_ACCUM_BLUE_BITS_ARB);
      anInfo.AccumAlphaSize  = getAttrEx(aFormatIter, WGL_ACCUM_ALPHA_BITS_ARB);

      anInfo.NbLayersUnderlay = getAttrEx(aFormatIter, WGL_NUMBER_UNDERLAYS_ARB);
      anInfo.NbLayersOverlay  = getAttrEx(aFormatIter, WGL_NUMBER_OVERLAYS_ARB);
    }
    else
    {
      PIXELFORMATDESCRIPTOR aFormat = {};
      aFormat.nSize = (WORD)sizeof(PIXELFORMATDESCRIPTOR);
      DescribePixelFormat(myDevCtx, aFormatIter, aFormat.nSize, &aFormat);
      if ((aFormat.dwFlags & PFD_SUPPORT_OPENGL) == 0)
        continue;

      anInfo.ColorBufferSize = (int)aFormat.cColorBits;
      if (aFormat.iPixelType != PFD_TYPE_COLORINDEX)
      {
        anInfo.RedSize   = (int)aFormat.cRedBits;
        anInfo.GreenSize = (int)aFormat.cGreenBits;
        anInfo.BlueSize  = (int)aFormat.cBlueBits;
        anInfo.AlphaSize = (int)aFormat.cAlphaBits;
      }
      anInfo.DepthSize   = (int)aFormat.cDepthBits;
      anInfo.StencilSize = (int)aFormat.cStencilBits;

      if ((aFormat.dwFlags & PFD_GENERIC_FORMAT) != 0)
        anInfo.IsSoftware = true;

      anInfo.SurfaceType = VisualInfo::Surface_None;
      if ((aFormat.dwFlags & PFD_DRAW_TO_WINDOW) != 0)
        anInfo.SurfaceType = VisualInfo::Surface(anInfo.SurfaceType | VisualInfo::Surface_Window);
      if ((aFormat.dwFlags & PFD_DRAW_TO_BITMAP) != 0)
        anInfo.SurfaceType = VisualInfo::Surface(anInfo.SurfaceType | VisualInfo::Surface_Pixmap);

      anInfo.BufferType = VisualInfo::ColorBuffer_Rgba;
      if (aFormat.iPixelType == PFD_TYPE_RGBA)
        anInfo.BufferType = VisualInfo::ColorBuffer_Rgba;
      else if (aFormat.iPixelType == PFD_TYPE_COLORINDEX)
        anInfo.BufferType = VisualInfo::ColorBuffer_ColorIndex;

      anInfo.SwapIntervalMin = 0;
      anInfo.SwapIntervalMax = (aFormat.dwFlags & PFD_DOUBLEBUFFER) != 0 ? 1 : 0;
      anInfo.IsStereoBuffer  = (aFormat.dwFlags & PFD_STEREO) != 0;

      anInfo.NbAuxBuffers   = (int)aFormat.cAuxBuffers;
      anInfo.AccumRedSize   = (int)aFormat.cAccumRedBits;
      anInfo.AccumGreenSize = (int)aFormat.cAccumGreenBits;
      anInfo.AccumBlueSize  = (int)aFormat.cAccumBlueBits;
      anInfo.AccumAlphaSize = (int)aFormat.cAccumAlphaBits;

      // bits 0 through 3 specify up to 15 overlay planes,
      // bits 4 through 7 specify up to 15 underlay planes
      anInfo.NbLayersUnderlay = (aFormat.bReserved & 0x0F);
      anInfo.NbLayersOverlay  = (aFormat.bReserved & 0xF0);

      if ((aFormat.dwFlags & PFD_GENERIC_FORMAT) == 0)           anAccelStr = "icd"; // hardware-accelerated
      else if ((aFormat.dwFlags & PFD_GENERIC_ACCELERATED) != 0) anAccelStr = "mcd"; // generic-accelerated
      else                                                       anAccelStr = "gdi"; // software
    }

    const char* aColorSpace = "";
    if (aGetAttribIProc != nullptr)
    {
      const int aColSpace = getAttrEx(aFormatIter, WGL_COLORSPACE_EXT);
      anInfo.IsSRgb = hasExtColorspace && aColSpace == WGL_COLORSPACE_SRGB_EXT;
      if (hasExtColorspace)
      {
        aColorSpace = aColSpace == WGL_COLORSPACE_SRGB_EXT
          ? ", sRGB"
          : (aColSpace == WGL_COLORSPACE_LINEAR_EXT
              ? ", Linear"
              : ", Unknown");
      }
      if (hasExtMultisample)
      {
        anInfo.NbSampleBuffers = getAttrEx(aFormatIter, WGL_SAMPLE_BUFFERS_EXT);
        anInfo.NbSamples       = getAttrEx(aFormatIter, WGL_SAMPLES_EXT);
      }

      const int aPixType = getAttrEx(aFormatIter, WGL_PIXEL_TYPE_ARB);
      anInfo.IsColorFloat = aPixType == WGL_TYPE_RGBA_FLOAT_ARB;
    }

    if (!theIsVerbose)
    {
      anInfo.PrintTableLine(false);
      continue;
    }

    const char* aSurfTypeStr = "N/A";
    if ((anInfo.SurfaceType & VisualInfo::Surface_Window) != 0)
      aSurfTypeStr = (anInfo.SurfaceType & VisualInfo::Surface_Pixmap) != 0
                   ? "window|bitmap"
                   : "window";
    else if ((anInfo.SurfaceType & VisualInfo::Surface_Pixmap) != 0)
      aSurfTypeStr = "bitmap";

    std::cout << "Visual ID: " << aFormatIter << "\n"
              << "    color: R" << anInfo.RedSize << "G" << anInfo.GreenSize << "B" << anInfo.BlueSize << "A" << anInfo.AlphaSize
                                << " (" << getColorBufferClass(anInfo.ColorBufferSize, anInfo.RedSize) << ", " << anInfo.ColorBufferSize
                                << aColorSpace << ")"
                                << " depth: " << anInfo.DepthSize << " stencil: " << anInfo.StencilSize << "\n"
              << "    doubleBuffer: " << (anInfo.SwapIntervalMax >= 1)
                                << " stereo: " << anInfo.IsStereoBuffer
                                << " renderType: " << (anInfo.BufferType == VisualInfo::ColorBuffer_Rgba ? "rgba" : "palette")
                                << " nbLayers: " << anInfo.NbLayersUnderlay << "+" << anInfo.NbLayersOverlay << "\n"
              << "    auxBuffers: " << anInfo.NbAuxBuffers
                                    << " accum: R" << anInfo.AccumRedSize << "G" << anInfo.AccumGreenSize << "B" << anInfo.AccumBlueSize << "A" << anInfo.AccumAlphaSize << "\n"
              << "    renderer: " << anAccelStr << " target: " << aSurfTypeStr << "\n";
  }

  // table footer
  if (!theIsVerbose)
    VisualInfo::PrintTableHeader(false);
}

#endif
