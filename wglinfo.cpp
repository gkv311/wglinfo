// Copyright © Kirill Gavrilov, 2018-2025
//
// wglinfo is a small utility printing information about OpenGL library available in Windows system
// in similar way as glxinfo does on Linux.
//
// In case, if libEGL.dll is in PATH, it also prints information about EGL/GLES.
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#include "EglGlContext.h"
#include "NativeGlContext.h"

#include <string>
#include <iomanip>
#include <iostream>

static int actual_main(int theNbArgs, const char** theArgVec)
{
  bool isVerbose = false, toPrintVisuals = true, toShowEgl = true;
  bool toShowWgl = true;
  for (int anArgIter = 1; anArgIter < theNbArgs; ++anArgIter)
  {
    if (memcmp(theArgVec[anArgIter], "-v", 2) == 0)
    {
      isVerbose = true;
    }
    else if (memcmp(theArgVec[anArgIter], "-B", 2) == 0)
    {
      toPrintVisuals = false;
    }
    else if (memcmp(theArgVec[anArgIter], "-h", 2) == 0
          || memcmp(theArgVec[anArgIter], "--help", 6) == 0
          || memcmp(theArgVec[anArgIter], "/?", 2) == 0)
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
      const char* anArgs[2] = { theArgVec[0], "-h" };
      actual_main(2, anArgs);
      return 1;
    }
  }

  if (toShowWgl)
  {
    NativeGlContext aCtxCompat("wglinfo");
    if (aCtxCompat.CreateGlContext(NativeGlContext::ContextBits_NONE))
    {
      // print WGL info
      aCtxCompat.PrintPlatformInfo();
      aCtxCompat.PrintRendererInfo();
      aCtxCompat.PrintGpuMemoryInfo();
      aCtxCompat.PrintExtensions();
    }
  }

  if (toShowWgl)
  {
    NativeGlContext aCtxCore("wglinfo_core_profile");
    if (aCtxCore.CreateGlContext(NativeGlContext::ContextBits_CoreProfile))
    {
      aCtxCore.PrintRendererInfo();
      aCtxCore.PrintExtensions();
    }
  }

  if (toShowWgl)
  {
    NativeGlContext aCtxCore("wglinfo_gles_profile");
    if (aCtxCore.CreateGlContext(NativeGlContext::ContextBits_GLES))
    {
      aCtxCore.PrintRendererInfo();
      aCtxCore.PrintExtensions();
    }
  }

  if (toShowWgl)
  {
    NativeGlContext aCtxSoft("wglinfo_ms_soft");
    if (aCtxSoft.CreateGlContext(NativeGlContext::ContextBits_SoftProfile))
    {
      aCtxSoft.PrintRendererInfo();
      aCtxSoft.PrintExtensions();
    }
  }

  // enumerate all the formats
  if (toPrintVisuals)
  {
    NativeGlContext aDummy("wglinfo_dummy");
    if (aDummy.CreateGlContext(NativeGlContext::ContextBits_NONE))
    {
      aDummy.PrintVisuals(isVerbose);
    }
  }

  if (toShowEgl)
  {
    EglGlContext aCtxEglGl("wglinfo_egl_gl");
    if (aCtxEglGl.CreateGlContext(NativeGlContext::ContextBits_NONE))
    {
      aCtxEglGl.PrintPlatformInfo();
      aCtxEglGl.PrintRendererInfo();
      aCtxEglGl.PrintGpuMemoryInfo();
      aCtxEglGl.PrintExtensions();
      aCtxEglGl.PrintVisuals(isVerbose);
    }
  }
  if (toShowEgl)
  {
    EglGlContext aCtxEglGles("wglinfo_egl_gles");
    if (aCtxEglGles.CreateGlContext(NativeGlContext::ContextBits_GLES))
    {
      aCtxEglGles.PrintPlatformInfo();
      aCtxEglGles.PrintRendererInfo();
      aCtxEglGles.PrintGpuMemoryInfo();
      aCtxEglGles.PrintExtensions();
      aCtxEglGles.PrintVisuals(isVerbose);
    }
  }

  return 0;
}

//! Returns the CPU architecture used to build the program (may not match the system).
static const char* getArchString()
{
#if defined(__amd64) || defined(__x86_64) || defined(_M_AMD64)
  return "x86_64";
#elif defined(__i386) || defined(_M_IX86) || defined(__X86__)|| defined(_X86_)
  return "x86";
#elif defined(__aarch64__) && defined(__LP64__)
  return "AArch64 64-bit";
#elif defined(__arm__) || defined(__arm64__)
  #if defined(__ARM_ARCH_7A__)
    return "ARMv7-A 32-bit";
  #elif defined(__ARM_ARCH_7R__)
    return "ARMv7-R 32-bit";
  #elif defined(__ARM_ARCH_7M__)
    return "ARMv7-M 32-bit";
  #elif defined(__LP64__)
    return "ARM 64-bit";
  #else
    return "ARM 32-bit";
  #endif
#else
  return "UNKNOWN";
#endif
}

//! Print build and system information.
static void printSystemInfo()
{
  std::cout << "wglinfo " << getArchString() << " (built with ";
#if defined(__INTEL_COMPILER)
  std::cout << "Intel " << __INTEL_COMPILER << "";
#elif defined(__BORLANDC__)
  std::cout << "Borland C++";
#elif defined(__clang__)
  std::cout << "Clang " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__;
#elif defined(_MSC_VER)
  #if _MSC_VER < 1900
    std::cout << "MS Visual C++ " << int(_MSC_VER/100-6) << "." << int((_MSC_VER/10)-60-10*(int)(_MSC_VER/100-6));
  #else
    std::cout << "MS Visual C++ " << int(_MSC_VER/100-5) << "." << int((_MSC_VER/10)-50-10*(int)(_MSC_VER/100-5));
  #endif
#elif defined(__GNUC__)
  std::cout << "GCC " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << "";
#else
  std::cout << "unrecognized";
#endif

#if defined(__MINGW64__)
  std::cout << "; MinGW64 " << __MINGW64_VERSION_MAJOR << "." << __MINGW64_VERSION_MINOR;
#elif defined(__MINGW32__)
  std::cout << "; MinGW32 " << __MINGW32_MAJOR_VERSION << "." << __MINGW32_MINOR_VERSION;
#endif
  std::cout << ")";

// suppress GetVersionExW is deprecated warning
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
  OSVERSIONINFOEXA aVerInfo; ZeroMemory(&aVerInfo, sizeof(aVerInfo));
  aVerInfo.dwOSVersionInfoSize = sizeof(aVerInfo);
  if (GetVersionExA((OSVERSIONINFOA* )&aVerInfo))
  {
    std::cout << " running on Windows " << aVerInfo.dwMajorVersion << "." << aVerInfo.dwMinorVersion << " [" << aVerInfo.dwBuildNumber << "]";
  }
#ifdef _MSC_VER
#pragma warning(pop)
#endif

  std::cout << "\n\n";
}

int main(int argc, const char** argv)
{
  printSystemInfo();
  return actual_main(argc, argv);
}
