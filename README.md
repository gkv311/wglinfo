wglinfo - command line tool for printing OpenGL information on Windows platform
=================================

[![Downloads](https://img.shields.io/github/downloads/gkv311/wglinfo/total.svg)](https://github.com/gkv311/wglinfo/releases)
[![License: GPL v3](https://img.shields.io/badge/license-MIT-green.svg)](https://github.com/gkv311/wglinfo/blob/master/LICENSE.txt)
[![Status](https://github.com/gkv311/wglinfo/actions/workflows/build_wglinfo_msvc.yml/badge.svg?branch=master)](https://github.com/gkv311/wglinfo/actions?query=branch%3Amaster)

wglinfo is a small utility printing information about OpenGL library available in Windows system in similar way as glxinfo does on Linux.
In case, if libEGL.dll (e.g. Angle or another implementation) is in PATH, it also prints information about EGL/GLES.

The output includes:

  * WGL
    * WGL extensions list
    * OpenGL context information (name, version, extensions)
      * Compatibility Profile (created by default)
      * Core Profile (WGL_CONTEXT_CORE_PROFILE_BIT_ARB)
      * Software implementation (WGL_NO_ACCELERATION_ARB)
    * List of visuals
  * EGL
    * EGL name, version and extensions list
    * OpenGL ES context information (name, version, extensions)

Here is the main repository of the project:<br/>
https://github.com/gkv311/wglinfo

## Licensing

See the [LICENSE](LICENSE.txt) file.
