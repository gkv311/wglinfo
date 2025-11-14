// Copyright © Kirill Gavrilov, 2018-2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef NATIVEGLCONTEXT_HEADER
#define NATIVEGLCONTEXT_HEADER

#ifdef _WIN32
  #include <windows.h>
#endif

#include "BaseGlContext.h"

//! Native GL context.
class NativeGlContext : public BaseGlContext
{
public:

  //! Empty constructor.
  NativeGlContext(const std::string& theTitle);

  //! Destructor.
  ~NativeGlContext() { Release(); }

  //! Return platform (WGL, GLX, etc.).
  virtual const char* PlatformName() const override;

  //! Release resources.
  virtual void Release() override;

  //! Create a GL context.
  virtual bool CreateGlContext(ContextBits theBits) override;

  //! Make this GL context active in current thread.
  virtual bool MakeCurrent() override;

public:

  //! Print WGL platform info.
  virtual void PrintPlatformInfo(bool theToPrintExtensions) override;

  //! Print GPU memory info.
  virtual void PrintGpuMemoryInfo() override;

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
  //!   rdr     = dwFlags & (PFD_GENERIC_FORMAT | PFD_GENERIC_ACCELERATED)
  //!                             - renderer (gdi (software only),
  //!                                         mcd (mini-icd cooperating with generic driver),
  //!                                         icd (standalone driver))
  virtual void PrintVisuals(bool theIsVerbose) override;

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

  //! Create a window handle.
  bool createWindowHandle();

  //! Calls SetPixelFormat().
  bool setWindowPixelFormat(int theFormat = -1);

private:

  NativeWindow myWin;
  HDC   myDevCtx = NULL;
  HGLRC myGlCtx = NULL;

};

#endif // NATIVEGLCONTEXT_HEADER
