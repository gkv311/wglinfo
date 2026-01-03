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

#if defined(__EMSCRIPTEN__)
  #include <emscripten/version.h>
#endif

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

//! Information tool.
class WglInfo
{
public:
  //! Perform tool.
  int Perform(int theNbArgs, const char** theArgVec);

private:

  //! Suppress info
  void suppressInfoBut(bool& theToShow)
  {
    myToPrintPlatform = myToPrintExtensions = myToPrintVisuals = false;
    myToPrintRenderer = myToPrintGpuMem = myToPrintLimits = false;
    theToShow = true;
  }

  //! Parse arguments.
  bool parseArguments(int theNbArgs, const char** theArgVec);

  //! Print help message.
  void printHelp(const char* theName);

  //! Print WGL info.
  template<class Platform_t>
  std::vector<BaseGlContext::ContextBits> printWglInfo();

  //! Returns the CPU architecture used to build the program (may not match the system).
  static const char* getArchString();

  //! Print build and system information.
  static void printSystemInfo();

private:
  bool myToShowNgl = true;
  bool myToShowEgl = true;
  std::shared_ptr<BaseWindow> myEglWin;

  bool myToShowGl = true;
  bool myToShowGles = true;

  bool myIsCompatProfile = true;
  bool myIsCoreProfile = true;
  bool myIsSoftProfile = true;
  bool myIsFwdProfile = false;

  bool myIsFirstOnly = false;
  bool myIsVerbose = false;

  bool myToPrintPlatform = true;
  bool myToPrintRenderer = true;
  bool myToPrintGpuMem = true;
  bool myToPrintExtensions = true;
  bool myToPrintLimits = true;
  bool myToPrintVisuals = true;

  int myExitCode = 1;
};

int main(int argc, const char** argv)
{
  WglInfo aTool;
  return aTool.Perform(argc, argv);
}

int WglInfo::Perform(int theNbArgs, const char** theArgVec)
{
  printSystemInfo();

  myExitCode = 0;
  if (!parseArguments(theNbArgs, theArgVec))
    return myExitCode;

  const std::vector<BaseGlContext::ContextBits> aWglDone =
    myToShowNgl ? printWglInfo<NativeGlContext>() : std::vector<BaseGlContext::ContextBits>();

  std::vector<BaseGlContext::ContextBits> aEglDone;
  if (myToShowEgl && (!myIsFirstOnly || aWglDone.empty()))
  {
    // probably XDG_SESSION_TYPE=wayland should be respected here
    // instead of trying to connect to Wayland server via WlWindow::HasServer()...
  #ifdef HAVE_WAYLAND
    if (dynamic_cast<XwWindow*>(myEglWin.get()) != nullptr)
      aEglDone = printWglInfo<EglGlContextT<XwWindow>>();
    else if (dynamic_cast<WlWindow*>(myEglWin.get()) != nullptr || WlWindow::HasServer())
      aEglDone = printWglInfo<EglGlContextT<WlWindow>>();
    else
      aEglDone = printWglInfo<EglGlContextT<XwWindow>>();
  #else
    aEglDone = printWglInfo<EglGlContextT<NativeWindow>>();
  #endif
  }

  if (aWglDone.empty() && aEglDone.empty())
    myExitCode = 1;

  if (myToPrintVisuals)
  {
    if (!aWglDone.empty())
    {
      NativeGlContext aDummy("wglinfo_dummy");
      if (aDummy.CreateGlContext(aWglDone[0]))
        aDummy.PrintVisuals(myIsVerbose);
    }
    if (!aEglDone.empty())
    {
      std::shared_ptr<BaseWindow> anEglWin;
      if (myEglWin.get() != nullptr)
      {
        anEglWin = myEglWin->EmptyCopy("wglinfo_dummy");
      }
      else
      {
      #ifdef HAVE_WAYLAND
        if (anEglWin.get() == nullptr && WlWindow::HasServer())
          anEglWin.reset(new WlWindow("wglinfo_dummy"));
        else
          anEglWin.reset(new XwWindow("wglinfo_dummy"));
      #else
        anEglWin.reset(new NativeWindow("wglinfo_dummy"));
      #endif
      }
      EglGlContext aDummy(anEglWin);
      if (aDummy.CreateGlContext(aEglDone[0]))
        aDummy.PrintVisuals(myIsVerbose);
    }
  }

  return myExitCode;
}

//! Convert string to lower case.
static std::string stringToLowerCase(std::string theStr)
{
  for (size_t i = 0; i < theStr.length(); ++i)
    theStr[i] = (char)tolower(theStr[i]);

  return theStr;
}

bool WglInfo::parseArguments(int theNbArgs, const char** theArgVec)
{
  for (int anArgIter = 1; anArgIter < theNbArgs; ++anArgIter)
  {
    const std::string anArg = stringToLowerCase(theArgVec[anArgIter]);
    if (anArg == "-v")
    {
      myIsVerbose = true;
    }
    else if (anArg == "--first" || anArg == "-first")
    {
      myIsFirstOnly = true;
    }
    else if (anArg == "--noplatform" || anArg == "-noplatform")
    {
      myToPrintPlatform = false;
    }
    else if (anArg == "--norenderer" || anArg == "-norenderer")
    {
      myToPrintRenderer = false;
    }
    else if (anArg == "--renderer" || anArg == "-renderer")
    {
      suppressInfoBut(myToPrintRenderer);
    }
    else if (anArg == "--noextensions" || anArg == "-noextensions")
    {
      myToPrintExtensions = false;
    }
    else if (anArg == "--extensions" || anArg == "-extensions")
    {
      suppressInfoBut(myToPrintExtensions);
    }
    else if (anArg == "--nolimits" || anArg == "-nolimits")
    {
      myToPrintLimits = false;
    }
    else if (anArg == "--limits" || anArg == "-limits")
    {
      suppressInfoBut(myToPrintLimits);
    }
    else if (anArg == "--novisuals" || anArg == "-novisuals" ||  anArg == "-b")
    {
      myToPrintVisuals = false;
    }
    else if (anArg == "--visuals" || anArg == "-visuals")
    {
      suppressInfoBut(myToPrintVisuals);
    }
    else if (anArg == "--gpumemory" || anArg == "-gpumemory"
          || anArg == "--gpumem" || anArg == "-gpumem")
    {
      suppressInfoBut(myToPrintGpuMem);
    }
    else if (anArg == "--platform" || anArg == "-platform")
    {
      myToShowNgl = myToShowEgl = false;
      if (anArgIter + 1 < theNbArgs)
      {
        const std::string aVal = stringToLowerCase(theArgVec[++anArgIter]);
        if (aVal == "*")
        {
          myToShowNgl = myToShowEgl = true;
        }
      #if defined(HAVE_WAYLAND)
        else if (aVal == "egl-wayland" || aVal == "egl-wl")
        {
          myToShowEgl = true;
          myEglWin.reset(new WlWindow(""));
        }
      #endif
      #if !defined(_WIN32) && !defined(__APPLE__) && !defined(__EMSCRIPTEN__)
        else if (aVal == "egl-x11")
        {
          myToShowEgl = true;
          myEglWin.reset(new XwWindow(""));
        }
      #endif
        else if (aVal == "egl")
        {
          myToShowEgl = true;
        }
      #ifdef _WIN32
        else if (aVal == "wgl" || aVal == "native")
      #elif defined(__APPLE__)
        else if (aVal == "cgl" || aVal == "native")
      #elif defined(__EMSCRIPTEN__)
        else if (aVal == "emsdk" || aVal == "native")
      #else
        else if (aVal == "glx" || aVal == "native")
      #endif
        {
          myToShowNgl = true;
        }
        else
        {
          --anArgIter;
          myToShowNgl = myToShowEgl = true;
          suppressInfoBut(myToPrintPlatform);
        }
      }
      else
      {
        myToShowNgl = myToShowEgl = true;
        suppressInfoBut(myToPrintPlatform);
      }
    }
    else if (anArg == "egl")
    {
      myToShowEgl = true;
      myToShowNgl = false;
    }
  #ifdef _WIN32
    else if (anArg == "wgl" || anArg == "native")
  #elif defined(__APPLE__)
    else if (anArg == "cgl" || anArg == "native")
  #else
    else if (anArg == "glx" || anArg == "native")
  #endif
    {
      myToShowEgl = false;
      myToShowNgl = true;
    }
    else if ((anArg == "--api" || anArg == "-api")
           && anArgIter + 1 < theNbArgs)
    {
      const std::string aVal = stringToLowerCase(theArgVec[++anArgIter]);
      myToShowGl = myToShowGles = false;
      if (aVal == "*")
      {
        myToShowGl = myToShowGles = true;
      }
      else if (aVal == "gl" || aVal == "opengl")
      {
        myToShowGl = true;
      }
      else if (aVal == "gles" || aVal == "opengles" || aVal == "opengl_es" || aVal == "opengl es")
      {
        myToShowGles = true;
      }
      else
      {
        std::cerr << "Syntax error! Unknown api '" << theArgVec[anArgIter] << "'\n\n";
        return false;
      }
    }
    else if (anArg == "gl" || anArg == "opengl")
    {
      myToShowGl = true;
      myToShowGles = false;
    }
    else if (anArg == "gles" || anArg == "opengles" || anArg == "opengl_es" || anArg == "opengl es")
    {
      myToShowGl = false;
      myToShowGles = true;
    }
    else if ((anArg == "--profile" || anArg == "-profile")
           && anArgIter + 1 < theNbArgs)
    {
      const std::string aVal = stringToLowerCase(theArgVec[++anArgIter]);
      if (aVal != "*")
      {
        myToShowGl = true;
        myToShowGles = false;
      }
      myIsCompatProfile = myIsCoreProfile = myIsSoftProfile = myIsFwdProfile = false;
      if (aVal == "*")
      {
        myIsCompatProfile = myIsCoreProfile = myIsSoftProfile = true;
      }
      else if (aVal == "compat" || aVal == "compatible"
            || aVal == "compatible_profile" || aVal == "compatible profile")
      {
        myIsCompatProfile = true;
      }
      else if (aVal == "core" || aVal == "core_profile" || aVal == "core profile")
      {
        myIsCoreProfile = true;
      }
      else if (aVal == "fwd" || aVal == "forward" || aVal == "forward_profile" || aVal == "forward profile")
      {
        myIsFwdProfile = true;
      }
      else if (aVal == "soft" || aVal == "noacc" || aVal == "no_acceleration")
      {
        myIsSoftProfile = true;
      }
      else
      {
        std::cerr << "Syntax error! Unknown profile '" << theArgVec[anArgIter] << "'\n\n";
        return false;
      }
    }
    else if (anArg == "-h" || anArg == "--help" || anArg == "/?")
    {
      printHelp(theArgVec[0]);
      return false;
    }
    else
    {
      std::cerr << "Syntax error! Unknown argument '" << theArgVec[anArgIter] << "'\n\n";
      printHelp(theArgVec[0]);
      myExitCode = 1;
      return false;
    }
  }
  return true;
}

void WglInfo::printHelp(const char* theName)
{
  std::string aName = theName;
  if (aName.size() > 4 && aName.substr(aName.size() - 4) == ".exe")
  {
    aName = aName.substr(0, aName.size() - 4);
  }

  static const char aPlatforms[] =
#ifdef _WIN32
    "EGL|WGL";
#elif defined(__APPLE__)
    "EGL|CGL";
#elif defined(__EMSCRIPTEN__)
    "EGL|EMSDK";
#elif defined(HAVE_WAYLAND)
    "EGL|EGL-X11|EGL-WAYLAND|GLX";
#else
    "EGL|GLX";
#endif

  std::cout << "Usage: " << aName << " [-v] [-h] [--platform {" << aPlatforms << "}]=*\n"
    "               [--api {GL|GLES}]=* [--profile {core|compat|soft}]=*\n"
    "               [--first] [--gpumemory]\n"
    "               [--novisuals] [--noextensions] [--norenderer] [--noplatform]\n"
    "  -B             Brief output, print only the basics.\n"
    "  -v             Print visuals info in verbose form.\n"
    "  -h             This information.\n"
    "  --platform     Platform (" << aPlatforms << ") to create context;\n"
    "                 by default main platforms will be evaluated.\n"
    "  --api          Api (OpenGL or OpenGL ES) to create context;\n"
    "                 by default all available APIs will be evaluated.\n"
    "  --profile      Profile to create OpenGL context;\n"
    "                 by default several main profiles will be evaluated.\n"
    "  --first        Print only first context.\n"
    "  --gpumemory    Print only GPU memory info (suppresses all other info).\n"
    "  --noplatform   Do not print platform (EGL|WGL|GLX|CGL etc.) info.\n"
    "  --norenderer   Do not print renderer info.\n"
    "  --noextensions Do not list extensions.\n"
    "  --novisuals    Do not list visuals, same as -B.\n"
    "This wglinfo tool variation has been created by Kirill Gavrilov Tartynskih <kirill@sview.ru>\n";
}

template<class Platform_t>
std::vector<BaseGlContext::ContextBits> WglInfo::printWglInfo()
{
  std::vector<BaseGlContext::ContextBits> anOptions;
  if (myToShowGl && myIsCompatProfile)
    anOptions.push_back(BaseGlContext::ContextBits_NONE);

  if (myToShowGl && myIsCoreProfile)
    anOptions.push_back(BaseGlContext::ContextBits_CoreProfile);

  if (myToShowGl && myIsFwdProfile)
      anOptions.push_back(BaseGlContext::ContextBits_ForwardProfile);

  if (myToShowGles)
    anOptions.push_back(BaseGlContext::ContextBits_GLES);

  if (myToShowGl && myIsSoftProfile)
    anOptions.push_back(BaseGlContext::ContextBits_SoftProfile);

#if defined(__APPLE__)
  if (myToShowGl && myIsSoftProfile && myIsCoreProfile)
    anOptions.push_back(BaseGlContext::ContextBits(BaseGlContext::ContextBits_CoreProfile | BaseGlContext::ContextBits_SoftProfile));
#endif

  std::vector<BaseGlContext::ContextBits> aSucceeded;
  for (size_t anOptIter = 0; anOptIter < anOptions.size(); ++anOptIter)
  {
    const BaseGlContext::ContextBits anOpt = anOptions[anOptIter];

    Platform_t aCtx("wglinfo");
    if (!aCtx.CreateGlContext(anOpt))
      continue;

    aSucceeded.push_back(anOpt);
    if (myToPrintPlatform && aSucceeded.size() == 1)
      aCtx.PrintPlatformInfo(myToPrintExtensions); // print platform once

    if (myToPrintRenderer)
      aCtx.PrintRendererInfo();

    if (myToPrintGpuMem)
      aCtx.PrintGpuMemoryInfo();

    if (myToPrintExtensions)
      aCtx.PrintExtensions();

    if (myToPrintLimits)
      aCtx.PrintLimits();

    if (myIsFirstOnly)
      return aSucceeded;
  }

  return aSucceeded;
}

const char* WglInfo::getArchString()
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
#elif defined(__EMSCRIPTEN__)
#if defined(__LP64__)
  return "WASM64";
#else
  return "WASM32";
#endif
#else
  return "UNKNOWN";
#endif
}

void WglInfo::printSystemInfo()
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
  std::cout << "MS Visual C++ " << int(_MSC_VER / 100 - 6) << "." << int((_MSC_VER / 10) - 60 - 10 * (int)(_MSC_VER / 100 - 6));
#else
  std::cout << "MS Visual C++ " << int(_MSC_VER / 100 - 5) << "." << int((_MSC_VER / 10) - 50 - 10 * (int)(_MSC_VER / 100 - 5));
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
#elif defined(__EMSCRIPTEN__)
  std::cout << "; Emscripten SDK " << __EMSCRIPTEN_major__ << "." << __EMSCRIPTEN_minor__ << "." << __EMSCRIPTEN_tiny__;
#endif

  std::cout << ")";

#ifdef _WIN32
  // suppress GetVersionExW is deprecated warning
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
  OSVERSIONINFOEXA aVerInfo; ZeroMemory(&aVerInfo, sizeof(aVerInfo));
  aVerInfo.dwOSVersionInfoSize = sizeof(aVerInfo);
  if (GetVersionExA((OSVERSIONINFOA*)&aVerInfo))
  {
    std::cout << " running on Windows " << aVerInfo.dwMajorVersion << "." << aVerInfo.dwMinorVersion << " [" << aVerInfo.dwBuildNumber << "]";
  }
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#elif defined(__APPLE__)
  std::cout << " running on macOS " << CocoaWindow::GetOsVersion();
#endif

  std::cout << "\n\n";
}
