// Copyright © Kirill Gavrilov, 2018
//
// wglinfo is a small utility printing information about OpenGL library available in Windows system
// in similar way as glxinfo does on Linux.
//
// In case, if libEGL.dll is in PATH, it also prints information about EGL/GLES.
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file license-boost.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

#include <windows.h>

#include <iomanip>
#include <iostream>

#include <GL/gl.h>

//! Window creation tool.
struct GlWindow
{
  //! Create a window handle.
  static HWND Create (const wchar_t* theTitle)
  {
    WNDCLASSW aWinClass;
    aWinClass.lpszClassName = L"OpenGL";
    static HINSTANCE anAppInstance = NULL;
    if (anAppInstance == NULL)
    {
      // only register the window class once
      anAppInstance = GetModuleHandle (NULL);
      aWinClass.style         = CS_OWNDC;
      aWinClass.lpfnWndProc   = windowProcWgl;
      aWinClass.cbClsExtra    = 0;
      aWinClass.cbWndExtra    = 0;
      aWinClass.hInstance     = anAppInstance;
      aWinClass.hIcon         = LoadIconW  (NULL, IDI_WINLOGO);
      aWinClass.hCursor       = LoadCursorW(NULL, IDC_ARROW);
      aWinClass.hbrBackground = NULL;
      aWinClass.lpszMenuName  = NULL;
      if (!RegisterClassW (&aWinClass))
      {
        std::cerr << "Error: RegisterClass() failed, cannot register window class.\n";
        return NULL;
      }
    }

    const DWORD anExStyle = WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE;
    HWND aWin = CreateWindowExW (anExStyle, aWinClass.lpszClassName, theTitle, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                 2, 2, 4, 4, NULL, NULL, anAppInstance, NULL);
    if (aWin == NULL)
    {
      std::cerr << "Error: CreateWindow() failed, Cannot create a window.\n";
      return NULL;
    }
    return aWin;
  }
private:

  //! Dummy window procedure.
  static LRESULT WINAPI windowProcWgl (HWND theWin, UINT theMsg, WPARAM theParamW, LPARAM theParamL)
  {
    switch (theMsg)
    {
      case WM_PAINT:
      {
        PAINTSTRUCT aPaint;
        BeginPaint (theWin, &aPaint);
        EndPaint   (theWin, &aPaint);
        return 0;
      }
      case WM_SIZE:
      {
        ::glViewport (0, 0, LOWORD(theParamL), HIWORD(theParamL));
        PostMessageW (theWin, WM_PAINT, 0, 0);
        return 0;
      }
      case WM_CHAR:
      {
        if (theParamW == 27) // ESC key
        {
          PostQuitMessage (0);
        }
        return 0;
      }
      case WM_CLOSE:
      {
        ::PostQuitMessage (0);
        return 0;
      }
    }
    return ::DefWindowProcW (theWin, theMsg, theParamW, theParamL);
  }

};

//! Format extensions as a comma separated list with line size fixed to 80.
static void printExtensions (const char* theExt)
{
  static const int THE_LINE_LEN = 80;
  if (theExt == NULL)
  {
    std::cout << "    NULL.\n\n";
    return;
  }

  int aStart = 0, aLineLen = 0;
  for (int aCharIter = 0;; ++aCharIter)
  {
    bool toBreak = theExt[aCharIter] == '\0';
    if (theExt[aCharIter] == ' '
     || theExt[aCharIter] == '\0')
    {
      const int aLen = aCharIter - aStart;
      for (; theExt[aCharIter] == ' '; ++aCharIter) {} // skip extra spaces
      if (theExt[aCharIter] == '\0')
      {
        toBreak = true;
      }

      if (aLen > 0)
      {
        if (aLineLen != 0
         && aLineLen + aLen + 2 > THE_LINE_LEN)
        {
          std::cout << "\n";
          aLineLen = 0;
        }
        if (aLineLen == 0)
        {
          aLineLen += 4;
          std::cout << "    ";
        }
        else
        {
          aLineLen += 1;
          std::cout << " ";
        }

        aLineLen += aLen + 1;
        std::cout.write (theExt + aStart, aLen);
        std::cout << (toBreak ? "." : ",");
      }
      aStart = aCharIter;
    }
    if (toBreak)
    {
      std::cout << "\n\n";
      return;
    }
  }
}

//! Auxiliary structure defining WGL Window context.
class WglInfoWindow
{
private:
  #define GL_SHADING_LANGUAGE_VERSION 0x8B8C

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

  #define GL_NUM_EXTENSIONS 0x821D

  typedef const char* (WINAPI *wglGetExtensionsStringARB_t)(HDC theDeviceContext);
  typedef BOOL (WINAPI *wglChoosePixelFormatARB_t)(HDC theDevCtx, const int* theIntAttribs,
                                                   const float* theFloatAttribs, unsigned int theMaxFormats,
                                                   int* theFormatsOut, unsigned int* theNumFormatsOut);
  typedef HGLRC (WINAPI *wglCreateContextAttribsARB_t)(HDC theDevCtx, HGLRC theShareContext, const int* theAttribs);
  typedef const GLubyte* (WINAPI *glGetStringi_t) (GLenum name, GLuint index);

public:
  //! Empty constructor.
  WglInfoWindow (const wchar_t* theName) : myWin (NULL), myDevCtx (NULL), myGlCtx (NULL), myName (theName) {}

  //! Destructor.
  ~WglInfoWindow() { Release(); }

  //! Release resources.
  void Release()
  {
    //wglMakeCurrent (NULL, NULL);
    if (myWin != NULL && myDevCtx != NULL)
    {
      ::ReleaseDC (myWin, myDevCtx);
      myDevCtx = NULL;
    }
    if (myGlCtx != NULL)
    {
      ::wglDeleteContext (myGlCtx);
      myGlCtx = NULL;
    }
    if (myWin != NULL)
    {
      ::DestroyWindow (myWin);
      myWin = NULL;
    }
  }

  //! Return device context.
  HDC DeviceContext() const { return myDevCtx; }

  //! Create a window handle.
  bool CreateWindowHandle()
  {
    Release();
    myWin = GlWindow::Create (myName);
    myDevCtx = ::GetDC (myWin);
    return true;
  }

  //! Calls SetPixelFormat().
  bool SetWindowPixelFormat (int theFormat = -1)
  {
    PIXELFORMATDESCRIPTOR aFormat;
    memset (&aFormat, 0, sizeof(aFormat));
    aFormat.nSize      = sizeof(aFormat);
    aFormat.nVersion   = 1;
    aFormat.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    aFormat.iPixelType = PFD_TYPE_RGBA;
    aFormat.cColorBits = 32;
    const int aFormatIndex = theFormat == -1 ? ::ChoosePixelFormat (myDevCtx, &aFormat) : theFormat;
    if (aFormatIndex == 0)
    {
      std::cerr << "Error: ChoosePixelFormat() failed, Cannot find a suitable pixel format.\n";
      return false;
    }

    if (theFormat != -1)
    {
      ::DescribePixelFormat (myDevCtx, aFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &aFormat);
    }
    if (::SetPixelFormat (myDevCtx, aFormatIndex, &aFormat) == FALSE)
    {
      std::cerr << "Error: SetPixelFormat(" << aFormatIndex << ") failed with error code " << GetLastError() << "\n";
      return false;
    }
    return true;
  }

  //! Create a GL context.
  bool CreateGlContext()
  {
    myGlCtx = myDevCtx != NULL ? ::wglCreateContext (myDevCtx) : NULL;
    return myGlCtx != NULL;
  }

  //! Make this GL context active in current thread.
  bool MakeCurrent() { return myGlCtx != NULL && ::wglMakeCurrent (myDevCtx, myGlCtx); }

public:

  //! Print information about OpenGL through WGL.
  static bool PrintWglInfo()
  {
    WglInfoWindow aCtxCompat (L"wglinfo");
    if (!aCtxCompat.CreateWindowHandle()
     || !aCtxCompat.SetWindowPixelFormat()
     || !aCtxCompat.CreateGlContext()
     || !aCtxCompat.MakeCurrent())
    {
      return false;
    }
    ::ShowWindow (aCtxCompat.myWin, SW_HIDE);

    // print WGL info
    const char* aWglExts = NULL;
    {
      wglGetExtensionsStringARB_t wglGetExtensionsStringARB = (wglGetExtensionsStringARB_t )wglGetProcAddress ("wglGetExtensionsStringARB");
      if (wglGetExtensionsStringARB != NULL)
      {
        aWglExts = wglGetExtensionsStringARB (wglGetCurrentDC());
      }
      // output header information
      std::cout << "[WGL] WGL extensions:\n";
      printExtensions (aWglExts);
    }

    std::cout << "[WGL] OpenGL vendor string: "   << ::glGetString (GL_VENDOR)   << "\n";
    std::cout << "[WGL] OpenGL renderer string: " << ::glGetString (GL_RENDERER) << "\n";
    std::cout << "[WGL] OpenGL version string: "  << ::glGetString (GL_VERSION)  << "\n";
    if (const char* aGlslVer = (const char* )::glGetString (GL_SHADING_LANGUAGE_VERSION))
    {
      std::cout << "[WGL] OpenGL shading language version string: " << aGlslVer << "\n";
    }
    else
    {
      ::glGetError();
    }
    std::cout << "[WGL] OpenGL extensions:\n";
    printExtensions ((const char* )::glGetString (GL_EXTENSIONS));

    // in WGL world wglGetProcAddress() returns NULL if extensions is unavailable,
    // so that checking for extension string can be skipped
    //if (checkGlExtension (aWglExts, "WGL_ARB_pixel_format"))
    //if (checkGlExtension (aWglExts, "WGL_ARB_create_context_profile"))
    wglChoosePixelFormatARB_t    aChoosePixProc = (wglChoosePixelFormatARB_t    )wglGetProcAddress ("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB_t aCreateCtxProc = (wglCreateContextAttribsARB_t )wglGetProcAddress ("wglCreateContextAttribsARB");
    for (int aCtxIter = 0; aCtxIter < (aChoosePixProc != NULL ? 2 : 0); ++aCtxIter)
    {
      const bool isDebugCtx = false;
      const bool isCoreCtx = aCtxIter == 0;
      if (isCoreCtx && aCreateCtxProc == NULL)
      {
        continue;
      }

      WglInfoWindow aCtxTmp (isCoreCtx ? L"wglinfo_core_profile" : L"wglinfo_ms_soft");
      if (!aCtxTmp.CreateWindowHandle())
      {
        break;
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
        WGL_ACCELERATION_ARB,   isCoreCtx ? WGL_FULL_ACCELERATION_ARB : WGL_NO_ACCELERATION_ARB,
        0, 0,
      };
      unsigned int aFrmtsNb = 0;
      int aPixelFrmtId = 0;
      if (!aChoosePixProc (aCtxTmp.myDevCtx, aPixAttribs, NULL, 1, &aPixelFrmtId, &aFrmtsNb)
       ||  aPixelFrmtId == 0
       || !aCtxTmp.SetWindowPixelFormat (aPixelFrmtId))
      {
        continue;
      }

      if (isCoreCtx)
      {
        int aCoreCtxAttribs[] =
        {
          WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
          WGL_CONTEXT_MINOR_VERSION_ARB, 2,
          WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
          WGL_CONTEXT_FLAGS_ARB,         isDebugCtx ? WGL_CONTEXT_DEBUG_BIT_ARB : 0,
          0, 0
        };

        // Try to create the core profile of highest OpenGL version supported by OCCT
        // (this will be done automatically by some drivers when requesting 3.2,
        //  but some will not (e.g. AMD Catalyst) since WGL_ARB_create_context_profile specification allows both implementations).
        for (int aLowVer4 = 5; aLowVer4 >= 0 && aCtxTmp.myGlCtx == NULL; --aLowVer4)
        {
          aCoreCtxAttribs[1] = 4;
          aCoreCtxAttribs[3] = aLowVer4;
          aCtxTmp.myGlCtx = aCreateCtxProc (aCtxTmp.myDevCtx, NULL, aCoreCtxAttribs);
        }
        for (int aLowVer3 = 3; aLowVer3 >= 2 && aCtxTmp.myGlCtx == NULL; --aLowVer3)
        {
          aCoreCtxAttribs[1] = 3;
          aCoreCtxAttribs[3] = aLowVer3;
          aCtxTmp.myGlCtx = aCreateCtxProc (aCtxTmp.myDevCtx, NULL, aCoreCtxAttribs);
        }
        if (aCtxTmp.myGlCtx == NULL
        || !aCtxTmp.MakeCurrent())
        {
          continue;
        }
      }
      else if (!aCtxTmp.CreateGlContext()
            || !aCtxTmp.MakeCurrent())
      {
        continue;
      }

      if (isCoreCtx)
      {
        std::cout << "[WGL] OpenGL (core profile) vendor string: "   << ::glGetString (GL_VENDOR) << "\n";
        std::cout << "[WGL] OpenGL (core profile) renderer string: " << ::glGetString (GL_RENDERER) << "\n";
        std::cout << "[WGL] OpenGL (core profile) version string: "  << ::glGetString (GL_VERSION) << "\n";
        std::cout << "[WGL] OpenGL (core profile) shading language version string: " << (const char* )::glGetString (GL_SHADING_LANGUAGE_VERSION) << "\n";
        std::cout << "[WGL] OpenGL (core profile) extensions:\n";

        glGetStringi_t aGetStringi = (glGetStringi_t )wglGetProcAddress ("glGetStringi");
        if (aGetStringi != NULL)
        {
          GLint anExtNb = 0;
          ::glGetIntegerv (GL_NUM_EXTENSIONS, &anExtNb);
          std::string aList;
          for (GLint anExtIter = 0; anExtIter < anExtNb; ++anExtIter)
          {
            const char* anExtension = (const char* )aGetStringi (GL_EXTENSIONS, (GLuint )anExtIter);
            if (anExtension != NULL)
            {
              aList += std::string (anExtension);
              aList += std::string (" ");
            }
            //const size_t aTestExtNameLen = strlen (anExtension);
          }
          printExtensions (aList.c_str());
        }
      }
      else
      {
        std::cout << "[WGL] OpenGL (software) vendor string: "   << ::glGetString (GL_VENDOR) << "\n";
        std::cout << "[WGL] OpenGL (software) renderer string: " << ::glGetString (GL_RENDERER) << "\n";
        std::cout << "[WGL] OpenGL (software) version string: "  << ::glGetString (GL_VERSION) << "\n";
        std::cout << "[WGL] OpenGL (software) extensions:\n";
        printExtensions ((const char* )::glGetString (GL_EXTENSIONS));
      }
    }

    aCtxCompat.MakeCurrent();
    ::PostQuitMessage (0);
    for (MSG aMsg; GetMessageW (&aMsg, aCtxCompat.myWin, 0, 0); )
    {
      TranslateMessage (&aMsg);
      DispatchMessageW (&aMsg);
    }
    wglMakeCurrent (NULL, NULL);
    return true;
  }

  //! Print information about visuals.
  //! A legend:
  //!   visual ~= pixel format descriptor
  //!   id      = pixel format number (integer from 1 - max pixel formats)
  //!   dep     = cColorBits      - color depth
  //!   xsp     = no analog       - transparent pixel (currently always ".")
  //!   bfsz    = cColorBits      - framebuffer size (no analog in Win32?)
  //!   lvl     = bReserved       - overlay(>0), underlay(<0), main plane(0).
  //!   rgci    = iPixelType      - rb = rgba mode, ci = color index mode.
  //!   db      = dwFlags & PFD_DOUBLEBUFFER - double buffer flag (y = yes)
  //!   stro    = dwFlags & PFD_STEREO       - stereo flag        (y = yes)
  //!   rsz     = cRedBits        - # bits of red
  //!   gsz     = cGreenBits      - # bits of green
  //!   bsz     = cBlueBits       - # bits of blue
  //!   asz     = cAlphaBits      - # bits of alpha
  //!   axbf    = cAuxBuffers     - # of aux buffers
  //!   dpth    = cDepthBits      - # bits of depth
  //!   stcl    = cStencilBits    - # bits of stencil
  //!   accum r = cAccumRedBits   - # bits of red in accumulation buffer
  //!   accum g = cAccumGreenBits - # bits of green in accumulation buffer
  //!   accum b = cAccumBlueBits  - # bits of blue in accumulation buffer
  //!   accum a = cAccumAlphaBits - # bits of alpha in accumulation buffer
  //!   ms      = no analog  - multisample buffers
  void PrintVisualInfoWgl (bool theIsVerbose)
  {
    const int aNbFormats = DescribePixelFormat (myDevCtx, 0, 0, NULL);
    std::cout <<"\n[WGL] " << aNbFormats << " WGL Visuals\n";
    if (!theIsVerbose)
    {
      std::cout << "    visual  x  bf lv rg d st  r  g  b a  ax dp st accum buffs  ms \n";
      std::cout <<"  id dep cl sp sz l  ci b ro sz sz sz sz bf th cl  r  g  b  a ns b\n";
      std::cout <<"------------------------------------------------------------------\n";
    }
    for (int aFormatIter = 1; aFormatIter <= aNbFormats; ++aFormatIter)
    {
      PIXELFORMATDESCRIPTOR aFormat;
      memset (&aFormat, 0, sizeof(aFormat));
      DescribePixelFormat (myDevCtx, aFormatIter, sizeof(PIXELFORMATDESCRIPTOR), &aFormat);
      if ((aFormat.dwFlags & PFD_SUPPORT_OPENGL) == 0)
      {
        continue;
      }

      if (theIsVerbose)
      {
        std::cout << "Visual ID: " << aFormatIter << "  depth=" << int(aFormat.cDepthBits) << "  class=" << (aFormat.cColorBits <= 8 ? "PseudoColor" : "TrueColor") << "\n"
                  << "    bufferSize=" << int(aFormat.cColorBits) << " level=" << int(aFormat.bReserved) << " renderType=" << (aFormat.iPixelType == PFD_TYPE_RGBA ? "rgba" : "ci")
                     << " doubleBuffer=" << (aFormat.dwFlags & PFD_DOUBLEBUFFER) << " stereo=" << (aFormat.dwFlags & PFD_STEREO) << "\n"
                  << "    rgba: redSize=" << int(aFormat.cRedBits) << " greenSize=" << int(aFormat.cGreenBits) << " blueSize=" << int(aFormat.cBlueBits) << " alphaSize=" << int(aFormat.cAlphaBits) << "\n"
                  << "    auxBuffers=" << int(aFormat.cAuxBuffers) << " depthSize=" << int(aFormat.cDepthBits) << " stencilSize=" << int(aFormat.cStencilBits) << "\n"
                  << "    accum: redSize=" << int(aFormat.cAccumRedBits) << " greenSize=" << int(aFormat.cAccumGreenBits) << " blueSize=" << int(aFormat.cAccumBlueBits) << " alphaSize=" << int(aFormat.cAccumAlphaBits)<< "\n";
        continue;
      }

      std::cout << "0x" << std::hex << std::setw(3) << std::setfill('0') << aFormatIter << std::dec << std::setfill(' ') << " ";
      std::cout << std::setw(2) << (int)aFormat.cColorBits << " ";

      std::cout << ((aFormat.dwFlags & PFD_DRAW_TO_WINDOW) != 0
                 ? "wn "
                 :  ((aFormat.dwFlags & PFD_DRAW_TO_BITMAP) != 0
                  ? "bm "
                  : ".  "));

      std::cout << " . " << std::setw(2) << (int)aFormat.cColorBits << " ";

      // bReserved indicates number of over/underlays
      if (aFormat.bReserved) std::cout << " " << (int)aFormat.bReserved << " ";
      else                   std::cout << " . ";

      std::cout << " " << (aFormat.iPixelType == PFD_TYPE_RGBA ? "r" : "c") << " "
                <<  ((aFormat.dwFlags & PFD_DOUBLEBUFFER) != 0 ? 'y' : '.') << " "
                << " " << ((aFormat.dwFlags & PFD_STEREO) != 0 ? 'y' : '.') << " ";
      printInt2d (aFormat.cRedBits   && aFormat.iPixelType == PFD_TYPE_RGBA ? (int)aFormat.cRedBits : -1);
      printInt2d (aFormat.cGreenBits && aFormat.iPixelType == PFD_TYPE_RGBA ? (int)aFormat.cGreenBits : -1);
      printInt2d (aFormat.cBlueBits  && aFormat.iPixelType == PFD_TYPE_RGBA ? (int)aFormat.cBlueBits : -1);
      printInt2d (aFormat.cAlphaBits && aFormat.iPixelType == PFD_TYPE_RGBA ? (int)aFormat.cAlphaBits : -1);
      printInt2d (aFormat.cAuxBuffers     ? (int)aFormat.cAuxBuffers : -1);
      printInt2d (aFormat.cDepthBits      ? (int)aFormat.cDepthBits : -1);
      printInt2d (aFormat.cStencilBits    ? (int)aFormat.cStencilBits : -1);
      printInt2d (aFormat.cAccumRedBits   ? (int)aFormat.cAccumRedBits : -1);
      printInt2d (aFormat.cAccumGreenBits ? (int)aFormat.cAccumGreenBits : -1);
      printInt2d (aFormat.cAccumBlueBits  ? (int)aFormat.cAccumBlueBits : -1);
      printInt2d (aFormat.cAccumAlphaBits ? (int)aFormat.cAccumAlphaBits : -1);

      std::cout <<" . .\n";
    }

    // table footer
    if (!theIsVerbose)
    {
      std::cout << "------------------------------------------------------------------\n";
      std::cout << "    visual  x  bf lv rg d st  r  g  b a  ax dp st accum buffs  ms \n";
      std::cout << "  id dep cl sp sz l  ci b ro sz sz sz sz bf th cl  r  g  b  a ns b\n";
      std::cout << "------------------------------------------------------------------\n\n";
    }
  }

private:

  //! Print integer value.
  static void printInt2d (int theInt)
  {
    if (theInt < 0)
    {
      std::cout << " . ";
    }
    else
    {
      std::cout << std::setw(2) << theInt << " ";
    }
  }

private:

  HWND  myWin;
  HDC   myDevCtx;
  HGLRC myGlCtx;
  const wchar_t* myName;

};

//! EGL tool.
class EglInfoWindow
{
private:
  // some declarations from EGL.h
  typedef unsigned int EGLBoolean;
  typedef unsigned int EGLenum;
  typedef int     EGLint;
  typedef void*   EGLConfig;
  typedef void*   EGLSurface;
  typedef void*   EGLContext;
  typedef void*   EGLDisplay;
  typedef void*   EGLClientBuffer;
  typedef HDC     EGLNativeDisplayType;
  typedef HBITMAP EGLNativePixmapType;
  typedef HWND    EGLNativeWindowType;
  typedef void (*__eglMustCastToProperFunctionPointerType)(void);
  typedef __eglMustCastToProperFunctionPointerType (WINAPI *eglGetProcAddress_t)(const char* theProcName);

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
  #define EGL_NO_CONTEXT                    ((EGLContext)0)
  #define EGL_NO_DISPLAY                    ((EGLDisplay)0)
  #define EGL_NO_SURFACE                    ((EGLSurface)0)
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
  #define EGL_VG_ALPHA_FORMAT               0x3088
  #define EGL_VG_ALPHA_FORMAT_NONPRE        0x308B
  #define EGL_VG_ALPHA_FORMAT_PRE           0x308C
  #define EGL_VG_ALPHA_FORMAT_PRE_BIT       0x0040
  #define EGL_VG_COLORSPACE                 0x3087
  #define EGL_VG_COLORSPACE_sRGB            0x3089
  #define EGL_VG_COLORSPACE_LINEAR          0x308A
  #define EGL_VG_COLORSPACE_LINEAR_BIT      0x0020

  typedef EGLint (WINAPI *eglGetError_t) (void);
  typedef EGLDisplay (WINAPI *eglGetDisplay_t) (EGLNativeDisplayType display_id);
  typedef EGLBoolean (WINAPI *eglInitialize_t) (EGLDisplay dpy, EGLint *major, EGLint *minor);
  typedef EGLBoolean (WINAPI *eglTerminate_t) (EGLDisplay dpy);
  typedef EGLBoolean (WINAPI *eglMakeCurrent_t) (EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
  typedef EGLBoolean (WINAPI *eglChooseConfig_t) (EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
  typedef EGLBoolean (WINAPI *eglBindAPI_t) (EGLenum api);
  typedef EGLContext (WINAPI *eglCreateContext_t) (EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
  typedef EGLBoolean (WINAPI *eglDestroyContext_t) (EGLDisplay dpy, EGLContext ctx);
  typedef EGLSurface (WINAPI *eglCreateWindowSurface_t) (EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
  typedef EGLBoolean (WINAPI *eglDestroySurface_t) (EGLDisplay dpy, EGLSurface surface);
  typedef const char* (WINAPI *eglQueryString_t) (EGLDisplay dpy, EGLint name);

  //! Auxiliary template to retrieve function pointer within libEGL.dll.
  template<typename FuncType_t> bool findEglDllProc (const char* theFuncName, FuncType_t& theFuncPtr)
  {
    theFuncPtr = (FuncType_t )(void* )GetProcAddress (myEglDll, theFuncName);
    return (theFuncPtr != NULL);
  }

public:

  //! Empty constructor.
  EglInfoWindow (bool theIsMandatory = false)
  : myEglDll (NULL), myEglDisp (EGL_NO_DISPLAY), myEglContext (EGL_NO_CONTEXT), myEglSurf (EGL_NO_SURFACE), myWin (NULL)
  {
    #define findEglDllProcShort(theFunc) findEglDllProc(#theFunc, theFunc)

    myEglDll = LoadLibraryW (L"libEGL.dll");
    if (myEglDll == NULL)
    {
      if (theIsMandatory)
      {
        std::cerr << "Error: unable to find libEGL.dll\n";
      }
      return;
    }

    if (!findEglDllProcShort (eglGetError)
     || !findEglDllProcShort (eglGetProcAddress)
     || !findEglDllProcShort (eglGetDisplay)
     || !findEglDllProcShort (eglInitialize)
     || !findEglDllProcShort (eglTerminate)
     || !findEglDllProcShort (eglMakeCurrent)
     || !findEglDllProcShort (eglChooseConfig)
     || !findEglDllProcShort (eglBindAPI)
     || !findEglDllProcShort (eglCreateContext)
     || !findEglDllProcShort (eglDestroyContext)
     || !findEglDllProcShort (eglCreateWindowSurface)
     || !findEglDllProcShort (eglDestroySurface)
     || !findEglDllProcShort (eglQueryString))
    {
      std::cerr << "Error: broken libEGL.dll\n";
      myEglDll = NULL;
    }
  }

  //! Destructor.
  ~EglInfoWindow() { Release(); }

  //! Release resources.
  void Release()
  {
    if (myEglSurf != EGL_NO_SURFACE)
    {
      eglDestroySurface (myEglDisp, myEglSurf);
      myEglSurf = EGL_NO_SURFACE;
    }
    if (myWin != NULL)
    {
      ::DestroyWindow (myWin);
      myWin = NULL;
    }

    if (myEglContext != EGL_NO_CONTEXT)
    {
      if (eglMakeCurrent (myEglDisp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_TRUE)
      {
        std::cerr << "Error: FAILED to release OpenGL context!\n";
      }
      eglDestroyContext (myEglDisp, myEglContext);
      myEglContext = EGL_NO_CONTEXT;
    }

    if (myEglDisp != EGL_NO_DISPLAY)
    {
      if (eglTerminate (myEglDisp) != EGL_TRUE)
      {
        std::cerr << "Error: EGL, eglTerminate FAILED!\n";
      }
      myEglDisp = EGL_NO_DISPLAY;
    }
  }

  //! Function to print EGL info.
  bool PrintEglInfo (bool theIsGles)
  {
    if (myEglDll == NULL)
    {
      return false;
    }

    myEglDisp = eglGetDisplay (EGL_DEFAULT_DISPLAY);
    if (myEglDisp == EGL_NO_DISPLAY)
    {
      std::cerr << "Error: no EGL display!\n";
      return false;
    }

    EGLint aVerMajor = 0; EGLint aVerMinor = 0;
    if (eglInitialize (myEglDisp, &aVerMajor, &aVerMinor) != EGL_TRUE)
    {
      std::cerr << "Error: EGL display is unavailable!\n";
      return false;
    }

    static bool isFirstTime = true;
    if (isFirstTime)
    {
      std::cout << "[EGL] EGLVersion: "    << eglQueryString (myEglDisp, EGL_VERSION) << "\n";
      std::cout << "[EGL] EGLVendor: "     << eglQueryString (myEglDisp, EGL_VENDOR)  << "\n";
      std::cout << "[EGL] EGLClientAPIs: " << eglQueryString (myEglDisp, EGL_CLIENT_APIS) << "\n";
      std::cout << "[EGL] EGL extensions:\n";
      printExtensions (eglQueryString (myEglDisp, EGL_EXTENSIONS));
      isFirstTime = false;
    }

    EGLint aConfigAttribs[] =
    {
      EGL_RED_SIZE,     8,
      EGL_GREEN_SIZE,   8,
      EGL_BLUE_SIZE,    8,
      EGL_ALPHA_SIZE,   0,
      EGL_DEPTH_SIZE,   24,
      EGL_STENCIL_SIZE, 8,
      EGL_RENDERABLE_TYPE, theIsGles ? EGL_OPENGL_ES2_BIT : EGL_OPENGL_BIT,
      EGL_NONE
    };

    EGLint aNbConfigs = 0;
    EGLConfig anEglCfg = NULL;
    if (eglChooseConfig (myEglDisp, aConfigAttribs, &anEglCfg, 1, &aNbConfigs) != EGL_TRUE
     || anEglCfg == NULL)
    {
      eglGetError();
      aConfigAttribs[4 * 2 + 1] = 16; // try config with smaller depth buffer
      if (eglChooseConfig (myEglDisp, aConfigAttribs, &anEglCfg, 1, &aNbConfigs) != EGL_TRUE
       || anEglCfg == NULL)
      {
        //std::cerr << "Error: EGL does not provide compatible configurations!\n";
        return false;
      }
    }

    if (eglBindAPI (theIsGles ? EGL_OPENGL_ES_API : EGL_OPENGL_API) != EGL_TRUE)
    {
      std::cerr << "Error: EGL does not provide " << (theIsGles ? "OpenGL ES" : "OpenGL") << " client!\n";
      return false;
    }

    EGLint  anEglCtxAttribsArr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
    EGLint* anEglCtxAttribs = theIsGles ? anEglCtxAttribsArr : NULL;
    myEglContext = eglCreateContext (myEglDisp, anEglCfg, EGL_NO_CONTEXT, anEglCtxAttribs);
    if (myEglContext == EGL_NO_CONTEXT)
    {
      std::cerr << "Error: EGL is unable to create OpenGL context!\n";
      return false;
    }

    myWin = GlWindow::Create (theIsGles ? L"wglinfo_egl_gles" : L"wglinfo_egl_gl");
    if (myWin == NULL)
    {
      return false;
    }

    myEglSurf = eglCreateWindowSurface (myEglDisp, anEglCfg, myWin, NULL);
    if (myEglSurf == EGL_NO_SURFACE)
    {
      std::cerr << "Error: EGL is unable to create surface for window!\n";
      return false;
    }

    if (eglMakeCurrent (myEglDisp, myEglSurf, myEglSurf, myEglContext) != EGL_TRUE)
    {
      std::cerr << "Error: eglMakeCurrent() has failed!\n";
      return false;
    }

    typedef const GLubyte* (WINAPI *glGetString_t)(GLenum name);
    glGetString_t aGlesGetString = (glGetString_t )eglGetProcAddress ("glGetString");
    if (aGlesGetString != NULL)
    {
      if (theIsGles)
      {
        std::cout << "[EGL] OpenGL ES vendor string: "   << aGlesGetString (GL_VENDOR)   << "\n";
        std::cout << "[EGL] OpenGL ES renderer string: " << aGlesGetString (GL_RENDERER) << "\n";
        std::cout << "[EGL] OpenGL ES version string: "  << aGlesGetString (GL_VERSION)  << "\n";
        if (const char* aGlslVer = (const char* )aGlesGetString (GL_SHADING_LANGUAGE_VERSION))
        {
          std::cout << "[EGL] OpenGL ES shading language version string: " << aGlslVer << "\n";
        }
        std::cout << "[EGL] OpenGL ES extensions:\n";
        printExtensions ((const char* )aGlesGetString (GL_EXTENSIONS));
      }
      else
      {
        std::cout << "[EGL] OpenGL vendor string: "   << aGlesGetString (GL_VENDOR)   << "\n";
        std::cout << "[EGL] OpenGL renderer string: " << aGlesGetString (GL_RENDERER) << "\n";
        std::cout << "[EGL] OpenGL version string: "  << aGlesGetString (GL_VERSION)  << "\n";
        if (const char* aGlslVer = (const char* )aGlesGetString (GL_SHADING_LANGUAGE_VERSION))
        {
          std::cout << "[EGL] OpenGL shading language version string: " << aGlslVer << "\n";
        }
        std::cout << "[EGL] OpenGL extensions:\n";
        printExtensions ((const char* )aGlesGetString (GL_EXTENSIONS));
      }
    }
    return true;
  }

private:

  HMODULE myEglDll;
  eglGetError_t eglGetError;
  eglGetProcAddress_t eglGetProcAddress;
  eglGetDisplay_t eglGetDisplay;
  eglInitialize_t eglInitialize;
  eglTerminate_t eglTerminate;
  eglMakeCurrent_t eglMakeCurrent;
  eglChooseConfig_t eglChooseConfig;
  eglBindAPI_t eglBindAPI;
  eglCreateContext_t eglCreateContext;
  eglDestroyContext_t eglDestroyContext;
  eglCreateWindowSurface_t eglCreateWindowSurface;
  eglDestroySurface_t eglDestroySurface;
  eglQueryString_t eglQueryString;

  EGLDisplay myEglDisp;
  EGLContext myEglContext;
  EGLSurface myEglSurf;
  HWND myWin;

};

int main (int theNbArgs, char** theArgVec)
{
  bool isVerbose = false, toPrintVisuals = true, toShowEgl = true;
  for (int anArgIter = 1; anArgIter < theNbArgs; ++anArgIter)
  {
    if (memcmp (theArgVec[anArgIter], "-v", 2) == 0)
    {
      isVerbose = true;
    }
    else if (memcmp (theArgVec[anArgIter], "-B", 2) == 0)
    {
      toPrintVisuals = false;
    }
    else if (memcmp (theArgVec[anArgIter], "-h", 2) == 0
          || memcmp (theArgVec[anArgIter], "--help", 6) == 0
          || memcmp (theArgVec[anArgIter], "/?", 2) == 0)
    {
      std::cout << "Usage: " << theArgVec[0] << "\n"
                << "  -B: brief output, print only the basics.\n"
	                 "  -v: Print visuals info in verbose form.\n"
	                 "  -h: This information\n"
                << "This wglinfo tool variation has been created by Kirill Gavrilov <kirill@sview.ru>\n";
      return 0;
    }
    else
    {
      std::cerr << "Syntax error! Unknown argument '" << theArgVec[anArgIter] << "'\n\n";
      char* anArgs[2] = { theArgVec[0], "-h" };
      main (2, anArgs);
      return 1;
    }
  }

  WglInfoWindow::PrintWglInfo();

  // enumerate all the formats
  if (toPrintVisuals)
  {
    WglInfoWindow aDummy (L"wglinfo_dummy");
    if (aDummy.CreateWindowHandle())
    {
      aDummy.PrintVisualInfoWgl (isVerbose);
    }
  }

  if (toShowEgl)
  {
    EglInfoWindow().PrintEglInfo (true);
    EglInfoWindow().PrintEglInfo (false);
  }

  return 0;
}
