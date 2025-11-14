// Copyright © Kirill Gavrilov, 2018-2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#ifndef BASEGLCONTEXT_HEADER
#define BASEGLCONTEXT_HEADER

#include "NativeWindow.h"

//! Base GL context interface.
class BaseGlContext
{
public:
  enum ContextBits
  {
    ContextBits_NONE        = 0x000,
    ContextBits_Debug       = 0x001,
    ContextBits_CoreProfile = 0x002,
    ContextBits_SoftProfile = 0x004,
    ContextBits_GLES        = 0x008,
  };

public:

  //! Return message prefix.
  std::string Prefix() const
  {
    return std::string("[") + PlatformName() + "] " + ApiName() + ProfileSuffix() + " ";
  }

  //! Return rendering API (OpenGL, OpenGL ES, etc.).
  const char* ApiName() const { return (myCtxBits & ContextBits_GLES) != 0 ? "OpenGL ES" : "OpenGL"; }

  //! Return rendering profile.
  const char* ProfileSuffix() const
  {
    if ((myCtxBits & ContextBits_GLES) != 0)
      return "";
    else if ((myCtxBits & ContextBits_CoreProfile) != 0)
      return " (core profile)";
    else if ((myCtxBits & ContextBits_SoftProfile) != 0)
      return " (software)";

    return "";
  }

  //! Return platform (EGL, WGL, GLX, etc.).
  virtual const char* PlatformName() const = 0;

  //! Create a GL context.
  virtual bool CreateGlContext(ContextBits theBits) = 0;

  //! Release resources.
  virtual void Release() = 0;

  //! Make this GL context active in current thread.
  virtual bool MakeCurrent() = 0;

public:

  //! Print platform info.
  virtual void PrintPlatformInfo(bool theToPrintExtensions) = 0;

  //! Print renderer info.
  virtual void PrintRendererInfo();

  //! Print GPU memory info.
  virtual void PrintGpuMemoryInfo();

  //! Print renderer extensions.
  virtual void PrintExtensions();

  //! Print renderer limits.
  virtual void PrintLimits();

  //! Print all visuals.
  virtual void PrintVisuals(bool theIsVerbose) = 0;

public:

  //! Auxiliary template to retrieve GL function pointer.
  template<typename FuncType_t>
  bool FindProc(const char* theFuncName, FuncType_t& theFuncPtr)
  {
    theFuncPtr = (FuncType_t)GlGetProcAddress(theFuncName);
    return (theFuncPtr != NULL);
  }

  //! glGetError() wrapper.
  virtual unsigned int GlGetError() = 0;

  //! glGetString() wrapper.
  virtual const char* GlGetString(unsigned int theGlEnum) = 0;

  //! glGetStringi() wrapper.
  virtual const char* GlGetStringi(unsigned int theGlEnum, unsigned int theIndex) = 0;

  //! glGetIntegerv() wrapper.
  virtual void GlGetIntegerv(unsigned int theGlEnum, int* theParams) = 0;

protected:

  //! Wrapper to system function to retrieve GL function pointer by name.
  virtual void* GlGetProcAddress(const char* theFuncName) = 0;

protected:

  //! Print integer value.
  static void printInt2d(int theInt);

  //! Return color buffer class
  static const char* getColorBufferClass(int theNbColorBits, int theNbRedBits);

  //! Format extensions as a comma separated list with line size fixed to 80.
  static void printExtensions(const char* theExt);

  //! Find extension in the list of extensions.
  static bool hasExtension(const std::string& theList, const std::string& theName)
  {
    std::size_t aPos = theList.find(theName);
    if (aPos == std::string::npos)
      return false;

    if (aPos + theName.length() < theList.length()
     && theList[aPos + theName.length()] != ' ')
    {
      return false; // make this is the whole word
    }
    return true;
  }

  //! Return list of extensions.
  std::string getGlExtensions();

  //! Print integer limit.
  void printLimitInt(unsigned int theGlEnum, const char* theName);

  //! Print integer range limit.
  void printLimitIntRange(unsigned int theGlEnum, const char* theName);

  //! Limit definition.
  struct LimitDefinition
  {
    const char*  Name = "";
    unsigned int Enum = 0;
    int          NbVals = 1;

    LimitDefinition(const char* theName, unsigned int theEnum, int theNbVals)
    : Name(theName), Enum(theEnum), NbVals(theNbVals) {}
  };

  //! Auxiliary tool to force software OpenGL context creation
  //! when using Mesa via LIBGL_ALWAYS_SOFTWARE=1 environment.
  class SoftMesaSentry
  {
  public:
    ~SoftMesaSentry() { Reset(); }
    static bool IsSoftContext(BaseGlContext& theDefCtx);
    bool Init(BaseGlContext& theDefCtx);
    void Reset();
  private:
    std::string myVendLibName;
    std::string myAlwaysSoft;

    bool myHadVendLibName = false;
    bool myHadAlwaysSoft  = false;
    bool myToRestore      = false;
  };

protected:

  ContextBits myCtxBits = ContextBits_NONE;

};

#endif // BASEGLCONTEXT_HEADER
