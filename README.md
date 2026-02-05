wglinfo - command line tool printing OpenGL information
=================================

![Downloads](https://img.shields.io/github/downloads/gkv311/wglinfo/total.svg)
[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE.txt)

`wglinfo` is a diagnostic tool printing information about *OpenGL* library on *Windows* system in a similar way as `glxinfo` does the job on *Linux*.
In case, if `libEGL.dll` (e.g. *Angle* or another implementation) is in `PATH`, it also prints information about `EGL`/`GLES`.

The output includes:

  * Platform (`EGL`, `WGL`, `GLX`, `CGL`)
    - Vendor, version and extensions list
  * OpenGL context (Compatibility and Core profiles).
    - Vendor, version, extensions list and API limits.
  * OpenGL ES context.
    - Vendor, version, extensions list and API limits.
  * List of visuals.

Supported platforms:

  * *WGL (Windows)*
    * OpenGL Compatibility Profile.
    * OpenGL Core Profile (`WGL_CONTEXT_CORE_PROFILE_BIT_ARB`).
    * OpenGL Software implementation (`WGL_NO_ACCELERATION_ARB`).
    * OpenGL ES context information (`WGL_CONTEXT_ES_PROFILE_BIT_EXT`).
  * *GLX (Linux)*
    * OpenGL Compatibility Profile.
    * OpenGL Core Profile (`GLX_CONTEXT_CORE_PROFILE_BIT_ARB`).
    * OpenGL Software implementation (`LIBGL_ALWAYS_SOFTWARE=1`).
  * *CGL (macOS)*
    * OpenGL Compatibility Profile.
    * OpenGL Core Profile (`NSOpenGLPFAOpenGLProfile -> NSOpenGLProfileVersion3_2Core`).
    * OpenGL Software implementation (`NSOpenGLPFARendererID -> kCGLRendererGenericFloatID`).
  * *EGL (cross-platform)*
    * OpenGL Compatibility Profile.
    * OpenGL Core Profile (`EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT`).
    * OpenGL Software implementation (`LIBGL_ALWAYS_SOFTWARE=1`).
    * OpenGL ES context information (`EGL_OPENGL_ES2_BIT`, `EGL_OPENGL_ES3_BIT`).

Here is the main repository of the project:<br/>
https://github.com/gkv311/wglinfo

## Licensing

See the [LICENSE](LICENSE.txt) file.
