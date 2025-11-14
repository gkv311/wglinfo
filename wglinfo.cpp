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
  bool myToShowEgl = true;
  bool myToShowWgl = true;

  bool myToShowGl = true;
  bool myToShowGles = true;

  bool myIsCompatProfile = true;
  bool myIsCoreProfile = true;
  bool myIsSoftProfile = true;

  bool myIsVerbose = false;
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
    myToShowWgl ? printWglInfo<NativeGlContext>() : std::vector<BaseGlContext::ContextBits>();
  const std::vector<BaseGlContext::ContextBits> aEglDone =
    myToShowEgl ? printWglInfo<EglGlContext>() : std::vector<BaseGlContext::ContextBits>();
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
      EglGlContext aDummy("wglinfo_dummy");
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
    else if (anArg == "-b")
    {
      myToPrintVisuals = false;
    }
    else if ((anArg == "--platform" || anArg == "-platform")
              && anArgIter + 1 < theNbArgs)
    {
      const std::string aVal = stringToLowerCase(theArgVec[++anArgIter]);
      myToShowWgl = myToShowEgl = false;
      if (aVal == "*")
      {
        myToShowWgl = myToShowEgl = true;
      }
      else if (aVal == "egl")
      {
        myToShowEgl = true;
      }
      else if (aVal == "wgl")
      {
        myToShowWgl = true;
      }
      else
      {
        std::cerr << "Syntax error! Unknown platform '" << theArgVec[anArgIter] << "'\n\n";
        return false;
      }
    }
    else if (anArg == "egl")
    {
      myToShowEgl = true;
      myToShowWgl = false;
    }
    else if (anArg == "wgl")
    {
      myToShowEgl = false;
      myToShowWgl = true;
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
      myIsCompatProfile = myIsCoreProfile = myIsSoftProfile = false;
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
    else if (anArg == "compat" || anArg == "compatible"
          || anArg == "compatible_profile" || anArg == "compatible profile")
    {
      myToShowGl = true;
      myToShowGles = false;
      myIsCompatProfile = true;
      myIsCoreProfile = false;
      myIsSoftProfile = false;
    }
    else if (anArg == "core" || anArg == "core_profile" || anArg == "core profile")
    {
      myToShowGl = true;
      myToShowGles = false;
      myIsCompatProfile = false;
      myIsCoreProfile = true;
      myIsSoftProfile = false;
    }
    else if (anArg == "soft" || anArg == "noacc" || anArg == "no_acceleration")
    {
      myToShowGl = true;
      myToShowGles = false;
      myIsCompatProfile = false;
      myIsCoreProfile = false;
      myIsSoftProfile = true;
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
  std::cout << "Usage: " << aName << " [-v] [-h] [--platform {egl|wgl}]=* [--api {gl|gles}]=*\n"
    "               [--profile {core|compat|soft}]=*\n"
    "  -B         Brief output, print only the basics.\n"
    "  -v         Print visuals info in verbose form.\n"
    "  -h         This information.\n"
    "  --platform Platform to create OpenGL/OpenGL ES context.\n"
    "  --api      Api to create context (OpenGL or OpenGL ES).\n"
    "  --profile  Profile to create OpenGL context.\n"
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

  if (myToShowGles)
    anOptions.push_back(BaseGlContext::ContextBits_GLES);

  if (myToShowGl && myIsSoftProfile)
    anOptions.push_back(BaseGlContext::ContextBits_SoftProfile);

  std::vector<BaseGlContext::ContextBits> aSucceeded;
  for (size_t anOptIter = 0; anOptIter < anOptions.size(); ++anOptIter)
  {
    const BaseGlContext::ContextBits anOpt = anOptions[anOptIter];

    Platform_t aCtx("wglinfo");
    if (!aCtx.CreateGlContext(anOpt))
      continue;

    aSucceeded.push_back(anOpt);
    if (aSucceeded.size() == 1)
      aCtx.PrintPlatformInfo(); // print platform once

    aCtx.PrintRendererInfo();
    aCtx.PrintGpuMemoryInfo();
    aCtx.PrintExtensions();
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
#endif
  std::cout << ")";

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

  std::cout << "\n\n";
}
