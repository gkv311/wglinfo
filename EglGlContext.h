// Copyright © Kirill Gavrilov, 2018-2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef EGLGLCONTEXT_HEADER
#define EGLGLCONTEXT_HEADER

#ifdef _WIN32
  #include <windows.h>
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
  typedef __eglMustCastToProperFunctionPointerType(WINAPI *eglGetProcAddress_t)(const char* theProcName);

  typedef EGLint      (WINAPI *eglGetError_t) (void);
  typedef EGLDisplay  (WINAPI *eglGetDisplay_t) (EGLNativeDisplayType display_id);
  typedef EGLBoolean  (WINAPI *eglInitialize_t) (EGLDisplay dpy, EGLint *major, EGLint *minor);
  typedef EGLBoolean  (WINAPI *eglTerminate_t) (EGLDisplay dpy);
  typedef EGLBoolean  (WINAPI *eglMakeCurrent_t) (EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
  typedef EGLBoolean  (WINAPI *eglGetConfigs_t) (EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
  typedef EGLBoolean  (WINAPI *eglGetConfigAttrib_t) (EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
  typedef EGLBoolean  (WINAPI *eglChooseConfig_t) (EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
  typedef EGLBoolean  (WINAPI *eglBindAPI_t) (EGLenum api);
  typedef EGLenum     (WINAPI *eglQueryAPI_t) (void);
  typedef EGLContext  (WINAPI *eglCreateContext_t) (EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
  typedef EGLBoolean  (WINAPI *eglDestroyContext_t) (EGLDisplay dpy, EGLContext ctx);
  typedef EGLSurface  (WINAPI *eglCreateWindowSurface_t) (EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
  typedef EGLBoolean  (WINAPI *eglDestroySurface_t) (EGLDisplay dpy, EGLSurface surface);
  typedef const char* (WINAPI *eglQueryString_t) (EGLDisplay dpy, EGLint name);

  typedef unsigned int         (WINAPI *glGetError_t)(void);
  typedef const unsigned char* (WINAPI *glGetString_t)(unsigned int name);
  typedef const unsigned char* (WINAPI *glGetStringi_t)(unsigned int name, unsigned int index);
  typedef void                 (WINAPI *glGetIntegerv_t)(unsigned int name, int* params);

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

  glGetError_t glGetError = NULL;
  glGetString_t glGetString = NULL;
  glGetStringi_t glGetStringi = NULL;
  glGetIntegerv_t glGetIntegerv = NULL;

  EGLDisplay myEglDisp = EGL_NO_DISPLAY;
  EGLContext myEglContext = EGL_NO_CONTEXT;
  EGLSurface myEglSurf = EGL_NO_SURFACE;

  NativeWindow myWin;

};

#endif // EGLGLCONTEXT_HEADER
