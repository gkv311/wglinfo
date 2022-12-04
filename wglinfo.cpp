// Copyright © Kirill Gavrilov, 2018-2022
//
// wglinfo is a small utility printing information about OpenGL library available in Windows system
// in similar way as glxinfo does on Linux.
//
// In case, if libEGL.dll is in PATH, it also prints information about EGL/GLES.
//
// This code is licensed under MIT license (see LICENSE.txt for details).

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

//! Return color buffer class
static const char* getColorBufferClass (int theNbColorBits, int theNbRedBits)
{
  if (theNbColorBits <= 8)
  {
    return "PseudoColor";
  }
  else if (theNbRedBits >= 10)
  {
    return "DeepColor";
  }
  return "TrueColor";
}

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
  typedef BOOL (WINAPI *wglGetPixelFormatAttribivARB_t)(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int   *piValues);
  typedef BOOL (WINAPI *wglGetPixelFormatAttribfvARB_t)(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues);
  typedef HGLRC (WINAPI *wglCreateContextAttribsARB_t)(HDC theDevCtx, HGLRC theShareContext, const int* theAttribs);
  typedef const GLubyte* (WINAPI *glGetStringi_t) (GLenum name, GLuint index);

  #define WGL_COLORSPACE_EXT        0x309D
  #define WGL_COLORSPACE_SRGB_EXT   0x3089
  #define WGL_COLORSPACE_LINEAR_EXT 0x308A

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

    {
      //if (checkGlExtension("GL_ATI_meminfo"))
      GLint aMemInfo[4] = { -1, -1, -1, -1 };
      ::glGetIntegerv (0x87FB, aMemInfo); // GL_VBO_FREE_MEMORY_ATI = 0x87FB
      if (::glGetError() == GL_NO_ERROR && aMemInfo[0] != -1)
      {
        std::cout << "[WGL] OpenGL Free GPU memory: " << (aMemInfo[0] / 1024) << " MiB\n";
      }
    }
    {
      //if (checkGlExtension("GL_NVX_gpu_memory_info"))
      GLint aDedicated = -1;
      ::glGetIntegerv (0x9047, &aDedicated); // GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX = 0x9047
      if (::glGetError() == GL_NO_ERROR && aDedicated != -1)
      {
        std::cout << "[WGL] OpenGL GPU memory: " << (aDedicated / 1024) << " MiB\n";
        //GLint aDedicatedFree = -1;
        //::glGetIntegerv (0x9049, &aDedicatedFree); // GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX = 0x9049
        //std::cout << "[WGL] OpenGL Free GPU memory: " << (aDedicatedFree / 1024) << " MiB\n";
      }
    }

    {
      typedef INT  (WINAPI *wglGetGPUInfoAMD_t )(UINT theId, INT theProperty, GLenum theDataType, UINT theSize, void* theData);
      typedef UINT (WINAPI *wglGetContextGPUIDAMD_t )(HGLRC theHglrc);
      wglGetGPUInfoAMD_t      wglGetGPUInfoAMD      = (wglGetGPUInfoAMD_t      )wglGetProcAddress ("wglGetGPUInfoAMD");
      wglGetContextGPUIDAMD_t wglGetContextGPUIDAMD = (wglGetContextGPUIDAMD_t )wglGetProcAddress ("wglGetContextGPUIDAMD");
      //if (checkGlExtension (aWglExts, "WGL_AMD_gpu_association"))
      if (wglGetGPUInfoAMD != NULL
       && wglGetContextGPUIDAMD != NULL)
      {
        GLuint aVMemMiB = 0;
        UINT anAmdId = wglGetContextGPUIDAMD (aCtxCompat.myGlCtx);
        if (anAmdId != 0 && wglGetGPUInfoAMD (anAmdId, 0x21A3, GL_UNSIGNED_INT, sizeof(aVMemMiB), &aVMemMiB) > 0) // WGL_GPU_RAM_AMD = 0x21A3
        {
          std::cout << "[WGL] OpenGL GPU memory: " << aVMemMiB << " MiB\n";
        }
      }
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
        for (int aLowVer4 = 7; aLowVer4 >= 0 && aCtxTmp.myGlCtx == NULL; --aLowVer4)
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
  //!   cl      = dwFlags & PFD_DRAW_TO_*  - render destination (window (wn), bitmap (bm), both (wb))
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
    wglGetPixelFormatAttribivARB_t aGetAttribIProc = (wglGetPixelFormatAttribivARB_t )wglGetProcAddress ("wglGetPixelFormatAttribivARB");
    //wglGetPixelFormatAttribfvARB_t aGetAttribFProc = (wglGetPixelFormatAttribfvARB_t )wglGetProcAddress ("wglGetPixelFormatAttribfvARB");

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
        const char* aColorSpace = "";
        if (aGetAttribIProc != nullptr)
        {
          // fetch colorspace information using WGL_EXT_colorspace extension
          int aColorAttribs[1] = { WGL_COLORSPACE_EXT };
          int aColorSpaceInt[1] = { 0 };
          if (aGetAttribIProc (myDevCtx, aFormatIter, 0, 1, aColorAttribs, aColorSpaceInt))
          {
            aColorSpace = *aColorSpaceInt == WGL_COLORSPACE_SRGB_EXT
                        ? ", sRGB"
                        : (*aColorSpaceInt == WGL_COLORSPACE_LINEAR_EXT
                        ? ", Linear"
                        : ", Unknown");
          }
        }
        std::cout << "Visual ID: " << aFormatIter << "\n"
                  << "    color: R" << int(aFormat.cRedBits) << "G" << int(aFormat.cGreenBits) << "B" << int(aFormat.cBlueBits) << "A" << int(aFormat.cAlphaBits)
                                    << " (" << getColorBufferClass (aFormat.cColorBits, aFormat.cRedBits) << ", " << int(aFormat.cColorBits)
                                    << aColorSpace << ")"
                                    << " depth: " << int(aFormat.cDepthBits) << " stencil: " << int(aFormat.cStencilBits) << "\n"
                  << "    doubleBuffer: " << ((aFormat.dwFlags & PFD_DOUBLEBUFFER) != 0)
                                   << " stereo: " << ((aFormat.dwFlags & PFD_STEREO) != 0)
                                   << " renderType: " << (aFormat.iPixelType == PFD_TYPE_RGBA ? "rgba" : "palette")
                                   << " level: " << int(aFormat.bReserved) << "\n"
                  << "    auxBuffers: " << int(aFormat.cAuxBuffers)
                                        << " accum: R" << int(aFormat.cAccumRedBits) << "G" << int(aFormat.cAccumGreenBits) << "B" << int(aFormat.cAccumBlueBits) << "A" << int(aFormat.cAccumAlphaBits)<< "\n";
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
  #define EGL_OPENGL_ES3_BIT                0x0040
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
  typedef EGLBoolean (WINAPI *eglGetConfigs_t) (EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
  typedef EGLBoolean (WINAPI *eglGetConfigAttrib_t) (EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
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
     || !findEglDllProcShort (eglGetConfigs)
     || !findEglDllProcShort (eglGetConfigAttrib)
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
      EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
      EGL_NONE
    };

    EGLConfig anEglCfg = NULL;
    for (int aGlesVer = theIsGles ? 3 : 2; aGlesVer >= 2; --aGlesVer)
    {
      aConfigAttribs[6 * 2 + 1] = theIsGles ? (aGlesVer == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT) : EGL_OPENGL_BIT;
      aConfigAttribs[4 * 2 + 1] = 24;
      EGLint aNbConfigs = 0;
      if (eglChooseConfig (myEglDisp, aConfigAttribs, &anEglCfg, 1, &aNbConfigs) != EGL_TRUE
       || anEglCfg == NULL)
      {
        eglGetError();
        aConfigAttribs[4 * 2 + 1] = 16; // try config with smaller depth buffer
        if (eglChooseConfig (myEglDisp, aConfigAttribs, &anEglCfg, 1, &aNbConfigs) != EGL_TRUE
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
    if (eglBindAPI (theIsGles ? EGL_OPENGL_ES_API : EGL_OPENGL_API) != EGL_TRUE)
    {
      std::cerr << "Error: EGL does not provide " << (theIsGles ? "OpenGL ES" : "OpenGL") << " client!\n";
      return false;
    }

    EGLint  anEglCtxAttribsArr2[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
    EGLint  anEglCtxAttribsArr3[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE, EGL_NONE };
    EGLint* anEglCtxAttribs = theIsGles ? (hasGLES3 ? anEglCtxAttribsArr3 : anEglCtxAttribsArr2) : NULL;
    myEglContext = eglCreateContext (myEglDisp, anEglCfg, EGL_NO_CONTEXT, anEglCtxAttribs);
    if (myEglContext == EGL_NO_CONTEXT
     && hasGLES3)
    {
      myEglContext = eglCreateContext (myEglDisp, anEglCfg, EGL_NO_CONTEXT, anEglCtxAttribsArr2);
    }
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

  void PrintEglConfigs (bool theIsVerbose)
  {
    if (myEglDll == NULL)
    {
      return;
    }

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
    eglGetConfigs (myEglDisp, NULL, 0, &aNbConfigs);
    EGLConfig* aConfigs = new EGLConfig[aNbConfigs];
    memset (aConfigs, 0, sizeof(EGLConfig) * aNbConfigs);
    if (eglGetConfigs (myEglDisp, aConfigs, aNbConfigs, &aNbConfigs) != EGL_TRUE)
    {
      delete[] aConfigs;
      return;
    }

    std::cout <<"\n[EGL] " << aNbConfigs << " EGL Configs\n";
    if (!theIsVerbose)
    {
      std::cout << "    visual  x  bf lv rg d st  r  g  b a  ax dp st accum buffs  ms \n";
      std::cout << "  id dep cl sp sz l  ci b ro sz sz sz sz bf th cl  r  g  b  a ns b\n";
      std::cout << "------------------------------------------------------------------\n";
    }
    for (int aCfgIter = 0; aCfgIter < aNbConfigs; ++aCfgIter)
    {
      const EGLConfig aCfg = aConfigs[aCfgIter];
      EGLConfigAttribs anAttribs;
      memset (&anAttribs, 0, sizeof(EGLConfigAttribs));
      eglGetConfigAttrib (myEglDisp, aCfg, EGL_CONFIG_ID,         &anAttribs.ConfigId);
      eglGetConfigAttrib (myEglDisp, aCfg, EGL_CONFIG_CAVEAT,     &anAttribs.ConfigCaveat);
      eglGetConfigAttrib (myEglDisp, aCfg, EGL_RENDERABLE_TYPE,   &anAttribs.RenderbableType);
      eglGetConfigAttrib (myEglDisp, aCfg, EGL_COLOR_BUFFER_TYPE, &anAttribs.BufferType);
      eglGetConfigAttrib (myEglDisp, aCfg, EGL_SURFACE_TYPE,      &anAttribs.SurfaceType);
      eglGetConfigAttrib (myEglDisp, aCfg, EGL_BUFFER_SIZE,       &anAttribs.ColorSize);
      eglGetConfigAttrib (myEglDisp, aCfg, EGL_RED_SIZE,          &anAttribs.RedSize);
      eglGetConfigAttrib (myEglDisp, aCfg, EGL_GREEN_SIZE,        &anAttribs.GreenSize);
      eglGetConfigAttrib (myEglDisp, aCfg, EGL_BLUE_SIZE,         &anAttribs.BlueSize);
      eglGetConfigAttrib (myEglDisp, aCfg, EGL_ALPHA_SIZE,        &anAttribs.AlphaSize);
      eglGetConfigAttrib (myEglDisp, aCfg, EGL_DEPTH_SIZE,        &anAttribs.DepthSize);
      eglGetConfigAttrib (myEglDisp, aCfg, EGL_STENCIL_SIZE,      &anAttribs.StencilSize);

      if (theIsVerbose)
      {
        std::cout << "Config: " << aCfgIter << "\n"
                  << "    color: R" << int(anAttribs.RedSize) << "G" << int(anAttribs.GreenSize) << "B" << int(anAttribs.BlueSize) << "A" << int(anAttribs.AlphaSize)
                                    << " (" << getColorBufferClass (anAttribs.ColorSize, anAttribs.RedSize) << ", " << int(anAttribs.ColorSize) << ")"
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
                 :  ((anAttribs.SurfaceType & EGL_PIXMAP_BIT) != 0
                  ? "bm "
                  : ".  "));

      std::cout << " . " << std::setw(2) << (int)anAttribs.ColorSize << " ";
      std::cout << " . ";
      std::cout << " " << (anAttribs.BufferType == EGL_RGB_BUFFER ? "r" : "l") << " "
                << '.' << " "
                << " " << '.' << " ";
      printInt2d (anAttribs.RedSize   && anAttribs.BufferType == EGL_RGB_BUFFER ? (int)anAttribs.RedSize   : -1);
      printInt2d (anAttribs.GreenSize && anAttribs.BufferType == EGL_RGB_BUFFER ? (int)anAttribs.GreenSize : -1);
      printInt2d (anAttribs.BlueSize  && anAttribs.BufferType == EGL_RGB_BUFFER ? (int)anAttribs.BlueSize  : -1);
      printInt2d (anAttribs.AlphaSize && anAttribs.BufferType == EGL_RGB_BUFFER ? (int)anAttribs.AlphaSize : -1);
      printInt2d (-1);
      printInt2d (anAttribs.DepthSize   ? (int)anAttribs.DepthSize   : -1);
      printInt2d (anAttribs.StencilSize ? (int)anAttribs.StencilSize : -1);
      printInt2d (-1);
      printInt2d (-1);
      printInt2d (-1);
      printInt2d (-1);

      std::cout <<" . .\n";
    }
    delete[] aConfigs;

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

  HMODULE myEglDll;
  eglGetError_t eglGetError;
  eglGetProcAddress_t eglGetProcAddress;
  eglGetDisplay_t eglGetDisplay;
  eglInitialize_t eglInitialize;
  eglTerminate_t eglTerminate;
  eglMakeCurrent_t eglMakeCurrent;
  eglGetConfigs_t eglGetConfigs;
  eglGetConfigAttrib_t eglGetConfigAttrib;
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
                << "This wglinfo tool variation has been created by Kirill Gavrilov Tartynskih <kirill@sview.ru>\n";
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
      if (!aDummy.SetWindowPixelFormat()
       || !aDummy.CreateGlContext()
       || !aDummy.MakeCurrent())
      {
        // wglGetPixelFormatAttribivARB is unavailable without context
      }

      aDummy.PrintVisualInfoWgl (isVerbose);
    }
  }

  if (toShowEgl)
  {
    {
      EglInfoWindow aEglCtx1;
      aEglCtx1.PrintEglInfo (true);
    }
    {
      EglInfoWindow aEglCtx2;
      aEglCtx2.PrintEglInfo (false);
      aEglCtx2.PrintEglConfigs (isVerbose);
    }
  }

  return 0;
}
