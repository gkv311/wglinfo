// Copyright © Kirill Gavrilov, 2018-2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#include "WglContext.h"

#ifdef _WIN32

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
#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_DRAW_TO_BITMAP_ARB                  0x2002
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_SUPPORT_GDI_ARB                     0x200F
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_STEREO_ARB                          0x2012
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_COLOR_BITS_ARB                      0x2014
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB                    0x2023

#define WGL_NO_ACCELERATION_ARB                 0x2025
#define WGL_GENERIC_ACCELERATION_ARB            0x2026
#define WGL_FULL_ACCELERATION_ARB               0x2027

#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_TYPE_COLORINDEX_ARB                 0x202C

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

#define WGL_COLORSPACE_EXT        0x309D
#define WGL_COLORSPACE_SRGB_EXT   0x3089
#define WGL_COLORSPACE_LINEAR_EXT 0x308A

typedef const char* (WINAPI *wglGetExtensionsStringARB_t)(HDC theDeviceContext);

static void printLastSystemError()
{
  char* aMsgBuff = NULL;
  DWORD anErrorCode = GetLastError();
  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, anErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&aMsgBuff, 0, NULL);
  if (aMsgBuff != NULL)
  {
    std::cerr << aMsgBuff << "\n";
    LocalFree(aMsgBuff);
  }
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
  wglGetPixelFormatAttribivARB_t aGetAttribIProc = NULL;
  //wglGetPixelFormatAttribfvARB_t aGetAttribFProc = NULL;
  FindProc("wglGetPixelFormatAttribivARB", aGetAttribIProc);
  //FindProc("wglGetPixelFormatAttribfvARB", aGetAttribFProc);

  const int aNbFormats = DescribePixelFormat(myDevCtx, 0, 0, NULL);
  std::cout << "\n[" << PlatformName() << "] " << aNbFormats << " WGL Visuals\n";
  if (!theIsVerbose)
  {
    std::cout << "    visual  x  bf lv rg d st  r  g  b a  ax dp st accum buffs  ms \n"
                 "  id dep cl sp sz l  ci b ro sz sz sz sz bf th cl  r  g  b  a ns b rdr\n"
                 "----------------------------------------------------------------------\n";
  }

  for (int aFormatIter = 1; aFormatIter <= aNbFormats; ++aFormatIter)
  {
    PIXELFORMATDESCRIPTOR aFormat;
    memset(&aFormat, 0, sizeof(aFormat));
    DescribePixelFormat(myDevCtx, aFormatIter, sizeof(PIXELFORMATDESCRIPTOR), &aFormat);
    if ((aFormat.dwFlags & PFD_SUPPORT_OPENGL) == 0)
      continue;

    const char* renderer = "gdi";
    if      ((aFormat.dwFlags & PFD_GENERIC_FORMAT) == 0)      renderer = "icd";
    else if ((aFormat.dwFlags & PFD_GENERIC_ACCELERATED) != 0) renderer = "mcd";

    if (theIsVerbose)
    {
      const char* aColorSpace = "";
      if (aGetAttribIProc != NULL)
      {
        // fetch colorspace information using WGL_EXT_colorspace extension
        int aColorAttribs[1] = { WGL_COLORSPACE_EXT };
        int aColorSpaceInt[1] = { 0 };
        if (aGetAttribIProc(myDevCtx, aFormatIter, 0, 1, aColorAttribs, aColorSpaceInt))
        {
          aColorSpace = *aColorSpaceInt == WGL_COLORSPACE_SRGB_EXT
                      ? ", sRGB"
                      : (*aColorSpaceInt == WGL_COLORSPACE_LINEAR_EXT
                      ? ", Linear"
                      : ", Unknown");
        }
      }

      const char* rendertarget = "bitmap";
      if ((aFormat.dwFlags & (PFD_DRAW_TO_WINDOW | PFD_DRAW_TO_BITMAP)) == (PFD_DRAW_TO_WINDOW | PFD_DRAW_TO_BITMAP))
        rendertarget = "window/bitmap";
      else if ((aFormat.dwFlags & PFD_DRAW_TO_WINDOW) != 0)
        rendertarget = "window";

      std::cout << "Visual ID: " << aFormatIter << "\n"
                << "    color: R" << int(aFormat.cRedBits) << "G" << int(aFormat.cGreenBits) << "B" << int(aFormat.cBlueBits) << "A" << int(aFormat.cAlphaBits)
                                  << " (" << getColorBufferClass(aFormat.cColorBits, aFormat.cRedBits) << ", " << int(aFormat.cColorBits)
                                  << aColorSpace << ")"
                                  << " depth: " << int(aFormat.cDepthBits) << " stencil: " << int(aFormat.cStencilBits) << "\n"
                << "    doubleBuffer: " << ((aFormat.dwFlags & PFD_DOUBLEBUFFER) != 0)
                                  << " stereo: " << ((aFormat.dwFlags & PFD_STEREO) != 0)
                                  << " renderType: " << (aFormat.iPixelType == PFD_TYPE_RGBA ? "rgba" : "palette")
                                  << " level: " << int(aFormat.bReserved) << "\n"
                << "    auxBuffers: " << int(aFormat.cAuxBuffers)
                                      << " accum: R" << int(aFormat.cAccumRedBits) << "G" << int(aFormat.cAccumGreenBits) << "B" << int(aFormat.cAccumBlueBits) << "A" << int(aFormat.cAccumAlphaBits)<< "\n"
                << "    renderer: " << renderer << " target: " << rendertarget << "\n";
      continue;
    }

    std::cout << "0x" << std::hex << std::setw(3) << std::setfill('0') << aFormatIter << std::dec << std::setfill(' ') << " ";
    std::cout << std::setw(2) << (int)aFormat.cColorBits << " ";

    std::cout << ((aFormat.dwFlags & (PFD_DRAW_TO_WINDOW | PFD_DRAW_TO_BITMAP)) == (PFD_DRAW_TO_WINDOW | PFD_DRAW_TO_BITMAP)
              ? "wb "
              : ((aFormat.dwFlags & PFD_DRAW_TO_WINDOW) != 0
                ? "wn "
                :  ((aFormat.dwFlags & PFD_DRAW_TO_BITMAP) != 0
                ? "bm "
                : ".  ")));

    std::cout << " . " << std::setw(2) << (int)aFormat.cColorBits << " ";

    // bReserved indicates number of over/underlays
    if (aFormat.bReserved) std::cout << " " << (int)aFormat.bReserved << " ";
    else                   std::cout << " . ";

    std::cout << " " << (aFormat.iPixelType == PFD_TYPE_RGBA ? "r" : "c") << " "
              <<  ((aFormat.dwFlags & PFD_DOUBLEBUFFER) != 0 ? 'y' : '.') << " "
              << " " << ((aFormat.dwFlags & PFD_STEREO) != 0 ? 'y' : '.') << " ";
    printInt2d(aFormat.cRedBits   && aFormat.iPixelType == PFD_TYPE_RGBA ? (int)aFormat.cRedBits : -1);
    printInt2d(aFormat.cGreenBits && aFormat.iPixelType == PFD_TYPE_RGBA ? (int)aFormat.cGreenBits : -1);
    printInt2d(aFormat.cBlueBits  && aFormat.iPixelType == PFD_TYPE_RGBA ? (int)aFormat.cBlueBits : -1);
    printInt2d(aFormat.cAlphaBits && aFormat.iPixelType == PFD_TYPE_RGBA ? (int)aFormat.cAlphaBits : -1);
    printInt2d(aFormat.cAuxBuffers     ? (int)aFormat.cAuxBuffers : -1);
    printInt2d(aFormat.cDepthBits      ? (int)aFormat.cDepthBits : -1);
    printInt2d(aFormat.cStencilBits    ? (int)aFormat.cStencilBits : -1);
    printInt2d(aFormat.cAccumRedBits   ? (int)aFormat.cAccumRedBits : -1);
    printInt2d(aFormat.cAccumGreenBits ? (int)aFormat.cAccumGreenBits : -1);
    printInt2d(aFormat.cAccumBlueBits  ? (int)aFormat.cAccumBlueBits : -1);
    printInt2d(aFormat.cAccumAlphaBits ? (int)aFormat.cAccumAlphaBits : -1);

    std::cout << " . . " << renderer << "\n";
  }

  // table footer
  if (!theIsVerbose)
  {
    std::cout << "----------------------------------------------------------------------\n"
                 "    visual  x  bf lv rg d st  r  g  b a  ax dp st accum buffs  ms  rdr\n"
                 "  id dep cl sp sz l  ci b ro sz sz sz sz bf th cl  r  g  b  a ns b\n"
                 "----------------------------------------------------------------------\n\n";
  }
}

#endif
