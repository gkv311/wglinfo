// Copyright © Kirill Gavrilov, 2018-2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef EGLGLCONTEXT_HEADER
#define EGLGLCONTEXT_HEADER

#ifdef _WIN32
  #include <windows.h>
  #define EGLAPIENTRY WINAPI
#else
  #include <EGL/egl.h>
#endif

#include "BaseGlContext.h"

//! EGL OpenGL/GLES context creation tool.
class EglGlContext : public BaseGlContext
{
public:

  //! Empty constructor.
  EglGlContext(const std::string& theTitle);

  //! Destructor.
  ~EglGlContext() { Release(); }

  //! Load EGL library.
  bool LoadEglLibrary(bool theIsMandatory = false);

  //! Return platform name "EGL".
  virtual const char* PlatformName() const override { return "EGL"; }

  //! Release resources.
  virtual void Release() override;

  //! Create a GL context.
  virtual bool CreateGlContext(ContextBits theBits) override;

  //! Make this GL context active in current thread.
  virtual bool MakeCurrent() override;

public:

  //! Print WGL platform info.
  virtual void PrintPlatformInfo(bool theToPrintExtensions) override;

  //! Print EGL configs.
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

#ifdef _WIN32
  // some declarations from EGL.h
  #define EGL_NO_CONTEXT ((EGLContext)0)
  #define EGL_NO_DISPLAY ((EGLDisplay)0)
  #define EGL_NO_SURFACE ((EGLSurface)0)

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
  typedef void(*__eglMustCastToProperFunctionPointerType)(void);
  typedef __eglMustCastToProperFunctionPointerType(EGLAPIENTRY *eglGetProcAddress_t)(const char* theProcName);

  typedef EGLint      (EGLAPIENTRY *eglGetError_t) (void);
  typedef EGLDisplay  (EGLAPIENTRY *eglGetDisplay_t) (EGLNativeDisplayType display_id);
  typedef EGLBoolean  (EGLAPIENTRY *eglInitialize_t) (EGLDisplay dpy, EGLint *major, EGLint *minor);
  typedef EGLBoolean  (EGLAPIENTRY *eglTerminate_t) (EGLDisplay dpy);
  typedef EGLBoolean  (EGLAPIENTRY *eglMakeCurrent_t) (EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
  typedef EGLBoolean  (EGLAPIENTRY *eglGetConfigs_t) (EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
  typedef EGLBoolean  (EGLAPIENTRY *eglGetConfigAttrib_t) (EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
  typedef EGLBoolean  (EGLAPIENTRY *eglChooseConfig_t) (EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
  typedef EGLBoolean  (EGLAPIENTRY *eglBindAPI_t) (EGLenum api);
  typedef EGLenum     (EGLAPIENTRY *eglQueryAPI_t) (void);
  typedef EGLContext  (EGLAPIENTRY *eglCreateContext_t) (EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
  typedef EGLBoolean  (EGLAPIENTRY *eglDestroyContext_t) (EGLDisplay dpy, EGLContext ctx);
  typedef EGLSurface  (EGLAPIENTRY *eglCreateWindowSurface_t) (EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
  typedef EGLBoolean  (EGLAPIENTRY *eglDestroySurface_t) (EGLDisplay dpy, EGLSurface surface);
  typedef const char* (EGLAPIENTRY *eglQueryString_t) (EGLDisplay dpy, EGLint name);

  //! Auxiliary template to retrieve function pointer within libEGL.dll.
  template<typename FuncType_t> bool findEglDllProc(const char* theFuncName, FuncType_t& theFuncPtr);

private:

  HMODULE myEglDll = NULL;
  eglGetError_t eglGetError = NULL;
  eglGetProcAddress_t eglGetProcAddress = NULL;
  eglGetDisplay_t eglGetDisplay = NULL;
  eglInitialize_t eglInitialize = NULL;
  eglTerminate_t eglTerminate = NULL;
  eglMakeCurrent_t eglMakeCurrent = NULL;
  eglGetConfigs_t eglGetConfigs = NULL;
  eglGetConfigAttrib_t eglGetConfigAttrib = NULL;
  eglChooseConfig_t eglChooseConfig = NULL;
  eglBindAPI_t eglBindAPI = NULL;
  eglQueryAPI_t eglQueryAPI = NULL;
  eglCreateContext_t eglCreateContext = NULL;
  eglDestroyContext_t eglDestroyContext = NULL;
  eglCreateWindowSurface_t eglCreateWindowSurface = NULL;
  eglDestroySurface_t eglDestroySurface = NULL;
  eglQueryString_t eglQueryString = NULL;
#endif

private:

  typedef unsigned int         (EGLAPIENTRY *glGetError_t)(void);
  typedef const unsigned char* (EGLAPIENTRY *glGetString_t)(unsigned int name);
  typedef const unsigned char* (EGLAPIENTRY *glGetStringi_t)(unsigned int name, unsigned int index);
  typedef void                 (EGLAPIENTRY *glGetIntegerv_t)(unsigned int name, int* params);

private:

  glGetError_t glGetError = NULL;
  glGetString_t glGetString = NULL;
  glGetStringi_t glGetStringi = NULL;
  glGetIntegerv_t glGetIntegerv = NULL;

private:

  EGLDisplay myEglDisp = EGL_NO_DISPLAY;
  EGLContext myEglContext = EGL_NO_CONTEXT;
  EGLSurface myEglSurf = EGL_NO_SURFACE;

  NativeWindow myWin;

};

#endif // EGLGLCONTEXT_HEADER
