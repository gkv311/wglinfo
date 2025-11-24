wglinfo - command line tool for printing OpenGL information on Windows platform
=================================

![Downloads](https://img.shields.io/github/downloads/gkv311/wglinfo/total.svg)
[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE.txt)

![Status](https://github.com/gkv311/wglinfo/actions/workflows/build_wglinfo_windows_msvc.yml/badge.svg?branch=master)
![Status](https://github.com/gkv311/wglinfo/actions/workflows/build_wglinfo_windows_mingw.yml/badge.svg?branch=master)
![Status](https://github.com/gkv311/wglinfo/actions/workflows/build_wglinfo_linux_gcc.yml/badge.svg?branch=master)
![Status](https://github.com/gkv311/wglinfo/actions/workflows/build_wglinfo_linux_mingw.yml/badge.svg?branch=master)

`wglinfo` is a small utility printing information about *OpenGL* library available in Windows system in similar way as `glxinfo` does on *Linux*.
In case, if `libEGL.dll` (e.g. *Angle* or another implementation) is in `PATH`, it also prints information about `EGL`/`GLES`.

The output includes:

  * Platform (`EGL`, `WGL`, `GLX`)
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
  * *EGL*
    * OpenGL Compatibility Profile.
    * OpenGL Core Profile (`EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT`).
    * OpenGL Software implementation (`LIBGL_ALWAYS_SOFTWARE=1`).
    * OpenGL ES context information (`EGL_OPENGL_ES2_BIT`, `EGL_OPENGL_ES3_BIT`).

Here is the main repository of the project:<br/>
https://github.com/gkv311/wglinfo

## Licensing

See the [LICENSE](LICENSE.txt) file.
