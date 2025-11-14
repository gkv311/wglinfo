// Copyright © Kirill Gavrilov, 2018-2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#include "NativeGlContext.h"

#include <GL/gl.h>

#include <iomanip>
#include <iostream>

#if defined(__GNUC__) && !defined(__INTEL_COMPILER) && !defined(__clang__)
  #if (__GNUC__ > 8) || ((__GNUC__ == 8) && (__GNUC_MINOR__ >= 1))
    #pragma GCC diagnostic ignored "-Wcast-function-type"
  #endif
#endif

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
typedef const GLubyte* (WINAPI *glGetStringi_t) (GLenum name, GLuint index);

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

NativeGlContext::NativeGlContext(const std::string& theTitle)
: myWin(theTitle), myDevCtx(NULL), myGlCtx(NULL)
{
  myPlatform = "WGL";
  myApi = "OpenGL";
  myProfile = "";
}

void NativeGlContext::Release()
{
#ifdef _WIN32
  if (myGlCtx != NULL)
  {
    wglMakeCurrent(NULL, NULL);
  }
  if (!myWin.IsNull() && myDevCtx != NULL)
  {
    ::ReleaseDC((HWND)myWin.GetDrawable(), myDevCtx);
    myDevCtx = NULL;
  }
  if (myGlCtx != NULL)
  {
    ::wglDeleteContext(myGlCtx);
    myGlCtx = NULL;
  }
#endif
  myWin.Destroy();
}

bool NativeGlContext::createWindowHandle()
{
  Release();
  if (!myWin.Create())
    return false;

#ifdef _WIN32
  myDevCtx = ::GetDC((HWND)myWin.GetDrawable());
#endif
  return true;
}

bool NativeGlContext::setWindowPixelFormat(int theFormat)
{
#ifdef _WIN32
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
#endif
  return true;
}

bool NativeGlContext::CreateGlContext(ContextBits theBits)
{
#ifdef _WIN32
  if (!createWindowHandle())
    return false;

  if (theBits == 0)
  {
    if (!setWindowPixelFormat())
      return false;

    myGlCtx = myDevCtx != NULL ? ::wglCreateContext(myDevCtx) : NULL;
    if (!MakeCurrent())
      return false;

    ///::ShowWindow((HWND)myWin.GetDrawable(), SW_HIDE);
    return true;
  }

  const bool isDebugCtx = (theBits & ContextBits_Debug) != 0;
  const bool isCoreCtx  = (theBits & ContextBits_CoreProfile) != 0;
  const bool isSoftCtx  = (theBits & ContextBits_SoftProfile) != 0;
  const bool isGles     = (theBits & ContextBits_GLES) != 0;
  myApi = isGles ? "OpenGL ES" : "OpenGL";
  if (isCoreCtx)
    myProfile = "core profile";

  if (isSoftCtx)
    myProfile = "software";

  // have to create a tempory context first in case of WGL
  NativeGlContext aCtxCompat("wglinfoTmp");
  if (!aCtxCompat.CreateGlContext(ContextBits_NONE))
  {
    return false;
  }

  // in WGL world wglGetProcAddress() returns NULL if extensions is unavailable,
  // so that checking for extension string can be skipped
  //if (checkGlExtension (aWglExts, "WGL_ARB_pixel_format"))
  //if (checkGlExtension (aWglExts, "WGL_ARB_create_context_profile"))
  wglChoosePixelFormatARB_t    aChoosePixProc = (wglChoosePixelFormatARB_t)wglGetProcAddress("wglChoosePixelFormatARB");
  wglCreateContextAttribsARB_t aCreateCtxProc = (wglCreateContextAttribsARB_t)wglGetProcAddress("wglCreateContextAttribsARB");
  if (aChoosePixProc == NULL || aCreateCtxProc == NULL)
  {
    return false;
  }

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
  if (isCoreCtx)
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

    for (int aLowVer3 = 3; aLowVer3 >= 0 && myGlCtx == NULL; --aLowVer3)
    {
      aGlesCtxAttribs[1] = 3;
      aGlesCtxAttribs[3] = aLowVer3;
      myGlCtx = aCreateCtxProc(myDevCtx, NULL, aGlesCtxAttribs);
    }
    if (myGlCtx == NULL)
    {
      aGlesCtxAttribs[1] = 2;
      aGlesCtxAttribs[3] = 0;
      myGlCtx = aCreateCtxProc(myDevCtx, NULL, aGlesCtxAttribs);
    }
  }
  else if (isCoreCtx || isDebugCtx)
  {
    int aCoreCtxAttribs[] =
    {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
      WGL_CONTEXT_MINOR_VERSION_ARB, 2,
      WGL_CONTEXT_PROFILE_MASK_ARB,  aProfileBit,
      WGL_CONTEXT_FLAGS_ARB,         isDebugCtx ? WGL_CONTEXT_DEBUG_BIT_ARB : 0,
      0, 0
    };

    // Try to create the core profile of highest OpenGL version supported by OCCT
    // (this will be done automatically by some drivers when requesting 3.2,
    //  but some will not (e.g. AMD Catalyst) since WGL_ARB_create_context_profile specification allows both implementations).
    for (int aLowVer4 = 7; aLowVer4 >= 0 && myGlCtx == NULL; --aLowVer4)
    {
      aCoreCtxAttribs[1] = 4;
      aCoreCtxAttribs[3] = aLowVer4;
      myGlCtx = aCreateCtxProc(myDevCtx, NULL, aCoreCtxAttribs);
    }
    for (int aLowVer3 = 3; aLowVer3 >= 2 && myGlCtx == NULL; --aLowVer3)
    {
      aCoreCtxAttribs[1] = 3;
      aCoreCtxAttribs[3] = aLowVer3;
      myGlCtx = aCreateCtxProc(myDevCtx, NULL, aCoreCtxAttribs);
    }
  }
  else
  {
    myGlCtx = myDevCtx != NULL ? ::wglCreateContext(myDevCtx) : NULL;
  }

  //aCtxCompat.MakeCurrent();
  //aCtxCompat.myWin.Quit();
  //wglMakeCurrent(NULL, NULL);
  aCtxCompat.Release();

  if (!MakeCurrent())
    return false;

  ///::ShowWindow((HWND)myWin.GetDrawable(), SW_HIDE);
#endif
  return myGlCtx != NULL;
}

bool NativeGlContext::MakeCurrent()
{
#ifdef _WIN32
  if (myGlCtx == NULL)
    return false;

  if (::wglMakeCurrent(myDevCtx, myGlCtx) != TRUE)
  {
    printLastSystemError();
    return false;
  }
  return true;
#else
  return false;
#endif
}

void* NativeGlContext::GlGetProcAddress(const char* theFuncName)
{
#if defined(_WIN32)
  return (void*)wglGetProcAddress(theFuncName);
#else
  (void)theFuncName;
  return NULL;
#endif
}

unsigned int NativeGlContext::GlGetError()
{
  return ::glGetError();
}

const char* NativeGlContext::GlGetString(unsigned int theGlEnum)
{
  const char* aStr = (const char*)::glGetString(theGlEnum);
  if (aStr == NULL)
  {
    //
  }
  return aStr;
}

const char* NativeGlContext::GlGetStringi(unsigned int theGlEnum, unsigned int theIndex)
{
  glGetStringi_t aGetStringi = (glGetStringi_t)wglGetProcAddress("glGetStringi");
  if (!FindProc("glGetStringi", aGetStringi))
    return NULL;

  return (const char*)aGetStringi(theGlEnum, theIndex);
}

void NativeGlContext::GlGetIntegerv(unsigned int theGlEnum, int* theParams)
{
  ::glGetIntegerv(theGlEnum, theParams);
}

void NativeGlContext::PrintPlatformInfo(bool theToPrintExtensions)
{
  std::cout << "[" << myPlatform << "] WGLName:       opengl32.dll\n";
  if (!theToPrintExtensions)
    return;

  const char* aWglExts = NULL;
  wglGetExtensionsStringARB_t wglGetExtensionsStringARB = NULL;
  if (FindProc("wglGetExtensionsStringARB", wglGetExtensionsStringARB))
    aWglExts = wglGetExtensionsStringARB(wglGetCurrentDC());

  // output header information
  std::cout << "[" << myPlatform << "] WGL extensions:\n";
  printExtensions(aWglExts);
}

void NativeGlContext::PrintGpuMemoryInfo()
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
    UINT anAmdId = wglGetContextGPUIDAMD(myGlCtx);
    if (anAmdId != 0 && wglGetGPUInfoAMD(anAmdId, 0x21A3, GL_UNSIGNED_INT, sizeof(aVMemMiB), &aVMemMiB) > 0) // WGL_GPU_RAM_AMD = 0x21A3
    {
      std::cout << Prefix() << "GPU memory: " << aVMemMiB << " MiB\n";
    }
  }
}

void NativeGlContext::PrintVisuals(bool theIsVerbose)
{
  wglGetPixelFormatAttribivARB_t aGetAttribIProc = NULL;
  //wglGetPixelFormatAttribfvARB_t aGetAttribFProc = NULL;
  FindProc("wglGetPixelFormatAttribivARB", aGetAttribIProc);
  //FindProc("wglGetPixelFormatAttribfvARB", aGetAttribFProc);

  const int aNbFormats = DescribePixelFormat(myDevCtx, 0, 0, NULL);
  std::cout << "\n[" << myPlatform << "] " << aNbFormats << " WGL Visuals\n";
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
