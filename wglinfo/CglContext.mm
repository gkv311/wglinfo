// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#include "CglContext.h"

#if defined(__APPLE__)

#ifndef GL_GLEXT_LEGACY
#define GL_GLEXT_LEGACY // prevent inclusion of system glext.h on Mac OS X 10.6.8
#endif

// macOS 10.4 deprecated OpenGL framework - suppress useless warnings
#define GL_SILENCE_DEPRECATION

#include <Cocoa/Cocoa.h>
#include <CoreGraphics/CoreGraphics.h>
#include <OpenGL/CGLRenderers.h>
#include <OpenGL/OpenGL.h>

#include <OpenGL/gl.h>

#include <dlfcn.h>

#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <vector>

#if defined(__GNUC__) && !defined(__INTEL_COMPILER) && !defined(__clang__)
  #if (__GNUC__ > 8) || ((__GNUC__ == 8) && (__GNUC_MINOR__ >= 1))
    #pragma GCC diagnostic ignored "-Wcast-function-type"
  #endif
#endif

CglContext::CglContext(const std::string& theTitle)
: myWin(theTitle)
{
  myGlLibHandle = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);
}

void CglContext::release()
{
  if (myRendCtx != NULL)
  {
    [NSOpenGLContext clearCurrentContext];
    [myRendCtx clearDrawable];
    [myRendCtx release];
    myRendCtx = nullptr;
  }

  myWin.Destroy();
}

bool CglContext::CreateGlContext(ContextBits theBits)
{
  Release();
  if (!myWin.Create())
    return false;

  myCtxBits = theBits;

  const bool isDebugCtx = (theBits & ContextBits_Debug) != 0;
  const bool isCoreCtx  = (theBits & ContextBits_CoreProfile) != 0;
  const bool isFwdCtx   = (theBits & ContextBits_ForwardProfile) != 0;
  const bool isSoftCtx  = (theBits & ContextBits_SoftProfile) != 0;
  const bool isGles     = (theBits & ContextBits_GLES) != 0;

  if (isGles || isFwdCtx || isDebugCtx) // unsupported
    return false;

  //CocoaLocalPool aLocalPool; // #if !__has_feature(objc_arc)

  NSOpenGLPixelFormatAttribute anAttribs[32] = {};
  int aLastAttrib = 0;
  //anAttribs[aLastAttrib++] = NSOpenGLPFAColorSize;    anAttribs[aLastAttrib++] = 32,
  anAttribs[aLastAttrib++] = NSOpenGLPFADepthSize;    anAttribs[aLastAttrib++] = 24;
  anAttribs[aLastAttrib++] = NSOpenGLPFAStencilSize;  anAttribs[aLastAttrib++] = 8;
  anAttribs[aLastAttrib++] = NSOpenGLPFADoubleBuffer;
  if (isSoftCtx)
  {
    anAttribs[aLastAttrib++] = NSOpenGLPFARendererID;
    anAttribs[aLastAttrib++] = (NSOpenGLPixelFormatAttribute )kCGLRendererGenericFloatID;
  }
  else
  {
    anAttribs[aLastAttrib++] = NSOpenGLPFAAccelerated;
  }
  if (isCoreCtx)
  {
    // supported since OS X 10.7+
    anAttribs[aLastAttrib++] = 99;     // NSOpenGLPFAOpenGLProfile
    anAttribs[aLastAttrib++] = 0x3200; // NSOpenGLProfileVersion3_2Core
  }
  anAttribs[aLastAttrib] = 0;

  NSOpenGLContext* aGLCtxShare = nullptr;
  NSOpenGLPixelFormat* aGLFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes: anAttribs] autorelease];
  myRendCtx = [[NSOpenGLContext alloc] initWithFormat: aGLFormat
                                         shareContext: aGLCtxShare];

  if (myRendCtx == nullptr)
  {
    std::cerr << "Error: NSOpenGLContext creation failed\n";
    return false;
  }

  NSView* aView = (NSView* )myWin.GetDrawable();
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wdeprecated-declarations"
  [myRendCtx setView: aView];
  #pragma clang diagnostic pop

  if (!MakeCurrent())
  {
    Release();
    return false;
  }

  return true;
}

bool CglContext::MakeCurrent()
{
  if (myRendCtx == 0)
    return false;

  [myRendCtx makeCurrentContext];
  return true;
}

void* CglContext::GlGetProcAddress(const char* theFuncName)
{
  return (myGlLibHandle != nullptr) ? dlsym(myGlLibHandle, theFuncName) : nullptr;
}

unsigned int CglContext::GlGetError()
{
  return ::glGetError();
}

const char* CglContext::GlGetString(unsigned int theGlEnum)
{
  const char* aStr = (const char*)::glGetString(theGlEnum);
  if (aStr == nullptr)
  {
    //
  }
  return aStr;
}

const char* CglContext::GlGetStringi(unsigned int theGlEnum, unsigned int theIndex)
{
  typedef const GLubyte* (*glGetStringi_t) (GLenum name, GLuint index);
  glGetStringi_t aGetStringi = NULL;
  if (!FindProc("glGetStringi", aGetStringi))
    return NULL;

  return (const char*)aGetStringi(theGlEnum, theIndex);
}

void CglContext::GlGetIntegerv(unsigned int theGlEnum, int* theParams)
{
  ::glGetIntegerv(theGlEnum, theParams);
}

void CglContext::PrintPlatformInfo(bool theToPrintExtensions)
{
  std::cout << "[" << PlatformName() << "] CGLName: OpenGL.framework\n";
  (void)theToPrintExtensions;
}

void CglContext::PrintGpuMemoryInfo()
{
  BaseGlContext::PrintGpuMemoryInfo();

  GLint aGlRendId = 0;
  CGLGetParameter(CGLGetCurrentContext(), kCGLCPCurrentRendererID, &aGlRendId);

  CGLRendererInfoObj  aRendObj = nullptr;
  CGOpenGLDisplayMask aDispMask = CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay);
  GLint aRendNb = 0;
  CGLQueryRendererInfo(aDispMask, &aRendObj, &aRendNb);
  for (GLint aRendIter = 0; aRendIter < aRendNb; ++aRendIter)
  {
    GLint aRendId = 0;
    if (CGLDescribeRenderer(aRendObj, aRendIter, kCGLRPRendererID, &aRendId) != kCGLNoError
     || aRendId != aGlRendId)
    {
      continue;
    }

    //kCGLRPVideoMemoryMegabytes   = 131;
    //kCGLRPTextureMemoryMegabytes = 132;
    GLint aVMem = 0, aVTMem = 0;
  #if MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
    if (CGLDescribeRenderer(aRendObj, aRendIter, kCGLRPVideoMemoryMegabytes, &aVMem) == kCGLNoError && aVMem != 0)
      std::cout << Prefix() << "GPU memory: " << aVMem << " MiB\n";

    if (CGLDescribeRenderer(aRendObj, aRendIter, kCGLRPTextureMemoryMegabytes, &aVTMem) == kCGLNoError && aVTMem != aVMem)
      std::cout << Prefix() << "GPU texture memory: " << aVTMem << " MiB\n";
  #else
    if (CGLDescribeRenderer(aRendObj, aRendIter, kCGLRPVideoMemory, &aVMem) == kCGLNoError && aVMem != 0)
      std::cout << Prefix() << "GPU memory: " << (aVMem / (1024 * 1024)) << " MiB\n";

    if (CGLDescribeRenderer(aRendObj, aRendIter, kCGLRPTextureMemory, &aVTMem) == kCGLNoError && aVTMem != aVMem)
      std::cout << Prefix() << "GPU texture memory: " << (aVTMem / (1024 * 1024)) << " MiB\n";
  #endif
  }
}

struct CglContextPixelAttr
{
  CGLPixelFormatAttribute Enum;
  const char*             Name;
};

static const CglContextPixelAttr ThePixelAttributes[] =
{
  { kCGLPFAColorSize,             "Color Size" },
  { kCGLPFAColorFloat,            "Color Float" },
  { kCGLPFAAlphaSize,             "Alpha Size" },
  { kCGLPFADepthSize,             "Depth Size" },
  { kCGLPFAStencilSize,           "Stencil Size" },
  { kCGLPFAAccelerated,           "Accelerated" },
  { kCGLPFAAcceleratedCompute,    "Accelerated Compute" },
  { kCGLPFAOpenGLProfile,         "OpenGL Profile" },
  { kCGLPFAAllRenderers,          "All Renderers" },
  { kCGLPFADoubleBuffer,          "Double Buffered" },
  { kCGLPFATripleBuffer,          "Tripple Buffered" },
  { kCGLPFAAuxBuffers,            "Aux Buffers" },
  { kCGLPFAAuxDepthStencil,       "Aux Depth Stencil" },
  { kCGLPFAAccumSize,             "Accum Size" },
  { kCGLPFAMinimumPolicy,         "Minimum Policy" },
  { kCGLPFAMaximumPolicy,         "Maximum Policy" },
  { kCGLPFASampleBuffers,         "Sample Buffers" },
  { kCGLPFASamples,               "Samples" },
  { kCGLPFAMultisample,           "Multisample" },
  { kCGLPFASupersample,           "Supersample" },
  { kCGLPFARendererID,            "Renderer ID" },
  { kCGLPFANoRecovery,            "No Recovery" },
  { kCGLPFAClosestPolicy,         "Closest Policy" },
  { kCGLPFABackingStore,          "Backing Store" },
  { kCGLPFADisplayMask,           "Display Mask" },
  { kCGLPFAAllowOfflineRenderers, "Allow Offline Renderers" },
  { kCGLPFAVirtualScreenCount,    "Virtual Screen Count" },
  // obsolete
  { kCGLPFAStereo,         "Stereo" },
  { kCGLPFACompliant,      "Compliant" },
  { kCGLPFARemotePBuffer,  "Remote PBuffer" },
  { kCGLPFASingleRenderer, "Single Renderer" },
  { kCGLPFAWindow,         "Window" },
//{ kCGLPFAOffScreen,      "Off Screen" },
//{ kCGLPFAPBuffer,        "PBuffer" },
//{ kCGLPFAFullScreen,     "Full Screen" },
//{ kCGLPFAMPSafe,         "MP Safe" },
//{ kCGLPFAMultiScreen,    "Multi Screen" },
//{ kCGLPFARobust,         "Robust" },
};

void CglContext::PrintVisuals(bool theIsVerbose)
{
  // there is no way to enumeration all formats;
  // instead - try calling CGLChoosePixelFormat() with different arguments,
  // which are known to return different formats
  typedef std::map<CGLPixelFormatAttribute, GLint> FormatInfo;
  std::set<FormatInfo> aFormatsMap;
  std::vector<FormatInfo> aFormats;

  const auto addFormat = [&aFormatsMap, &aFormats](const CGLPixelFormatAttribute* theAttribs)
  {
    CGLPixelFormatObj aFormat = nullptr;
    GLint aNbPixs = 0;
    if (CGLChoosePixelFormat(theAttribs, &aFormat, &aNbPixs) != kCGLNoError)
      return;

    for (GLint aPixIter = 0; aPixIter < aNbPixs; ++aPixIter)
    {
      FormatInfo anInfo;
      for (const CglContextPixelAttr& anAttrIter : ThePixelAttributes)
      {
        GLint aVal = 0;
        if (CGLDescribePixelFormat(aFormat, aPixIter, anAttrIter.Enum, &aVal) == kCGLNoError)
          anInfo[anAttrIter.Enum] = aVal;
      }

      if (aFormatsMap.find(anInfo) != aFormatsMap.cend())
        return;

      aFormatsMap.insert(anInfo);
      aFormats.push_back(anInfo);
    }

    CGLDestroyPixelFormat(aFormat);
  };

  for (int anAccel = 1; anAccel >= 0; --anAccel)
  {
    for (int aColor : {128, 64, 0})
    {
      for (int aProf = 4; aProf >= 2; --aProf)
      {
        std::array<int, 32> anAttribs = {};
        int aLastAttrib = 0;

        //anAttribs[aLastAttrib++] = kCGLPFAMinimumPolicy;
        //anAttribs[aLastAttrib++] = kCGLPFAClosestPolicy;

        if (aColor >= 64)
        {
          // half-float or 32-bit float format
          anAttribs[aLastAttrib++] = kCGLPFAColorFloat;
          anAttribs[aLastAttrib++] = kCGLPFAColorSize;
          anAttribs[aLastAttrib++] = aColor;
        }
        else if (aColor != 0)
        {
          // this doesn't make any difference on modern macOS - 32 bit format is always returned
          anAttribs[aLastAttrib++] = kCGLPFAColorSize;
          anAttribs[aLastAttrib++] = aColor;
        }

        // this doesn't make any difference on modern macOS - 32 bit depth is always returned
        anAttribs[aLastAttrib++] = kCGLPFADepthSize;
        anAttribs[aLastAttrib++] = 24;

        anAttribs[aLastAttrib++] = kCGLPFAStencilSize;
        anAttribs[aLastAttrib++] = 8;

        anAttribs[aLastAttrib++] = kCGLPFADoubleBuffer;

        if (anAccel == 1)
        {
          anAttribs[aLastAttrib++] = kCGLPFAAccelerated;
        }
        else
        {
          anAttribs[aLastAttrib++] = kCGLPFARendererID;
          anAttribs[aLastAttrib++] = kCGLRendererGenericFloatID;
        }

        // obsolete, modern macOS doesn't support it
        //anAttribs[aLastAttrib++] = kCGLPFAStereo;

        if (aProf != 2)
        {
          anAttribs[aLastAttrib++] = kCGLPFAOpenGLProfile;
          //kCGLOGLPVersion_Legacy
          anAttribs[aLastAttrib++] = aProf == 4 ? kCGLOGLPVersion_GL4_Core : kCGLOGLPVersion_3_2_Core;
        }

        addFormat((const CGLPixelFormatAttribute*)anAttribs.data());
      }
    }
  }

  std::cout << "\n[" << PlatformName() << "] " << aFormats.size() << " CGL Visuals\n";
  if (!theIsVerbose)
  {
    std::cout << "    visual   x   bf lv rg d st  colorbuffer  sr ax dp st accumbuffer  ms  sw cav\n"
                 "  id  dep cl sp  sz l  ci b ro  r  g  b  a F gb bf th cl  r  g  b  a ns b ap eat\n"
                 "-----------------------------------------------------------------------------\n";
  }

  int aFormatIndex = 0;
  for (const FormatInfo& aFormatIter : aFormats)
  {
    if (!theIsVerbose)
    {
      const auto getAttrib = [&aFormatIter](CGLPixelFormatAttribute theEnum, GLint theDef = 0) -> GLint
      {
        const auto aVal = aFormatIter.find(theEnum);
        return aVal != aFormatIter.cend() ? aVal->second : theDef;
      };

      const GLint aColorSize = getAttrib(kCGLPFAColorSize);
      std::cout << "0x" << std::hex << std::setw(3) << std::setfill('0') << aFormatIndex++ << std::dec << std::setfill(' ') << " ";

      // color buffer depth
      printInt3d(aColorSize);

      const bool isWindow  = getAttrib(kCGLPFAWindow)  != 0;
      const bool isPBuffer = getAttrib(kCGLPFAPBuffer) != 0 || getAttrib(kCGLPFARemotePBuffer) != 0;
      if (isWindow && isPBuffer)
        std::cout << "wb ";
      else if (isWindow)
        std::cout << "wn ";
      else if (isPBuffer)
        std::cout << "bm ";
      else
        std::cout << " . ";

      // x sp
      std::cout << " . ";

      // color buffer size
      printInt3d(aColorSize);
      // number of over/underlays
      std::cout << " . ";

      const bool isRgba = true;
      std::cout << " " << (isRgba ? "r" : "c") << " "
                <<  (getAttrib(kCGLPFADoubleBuffer) != 0 ? 'y' : '.') << " "
                << " " << (getAttrib(kCGLPFAStereo) != 0 ? 'y' : '.') << " ";

      int anRgbaBits[4] = {-1, -1, -1, getAttrib(kCGLPFAAlphaSize, -1)};
      if (aColorSize == 32 || anRgbaBits[3] == 8)
        anRgbaBits[0] = anRgbaBits[1] = anRgbaBits[2] = 8;
      else if (aColorSize == 64 || anRgbaBits[3] == 16)
        anRgbaBits[0] = anRgbaBits[1] = anRgbaBits[2] = 16;
      else if (aColorSize == 128 || anRgbaBits[3] == 32)
        anRgbaBits[0] = anRgbaBits[1] = anRgbaBits[2] = 32;

      printInt2d(anRgbaBits[0]);
      printInt2d(anRgbaBits[1]);
      printInt2d(anRgbaBits[2]);
      printInt2d(anRgbaBits[3]);

      // float
      if (getAttrib(kCGLPFAColorFloat) != 0)
        std::cout << "y ";
      else
        std::cout << ". ";

      // srgb
      std::cout << " . ";

      printInt2d(getAttrib(kCGLPFAAuxBuffers,  -1));
      printInt2d(getAttrib(kCGLPFADepthSize,   -1));
      printInt2d(getAttrib(kCGLPFAStencilSize, -1));

      printInt2d(-1); // AccumRedBits
      printInt2d(-1); // AccumGreemBits
      printInt2d(-1); // AccumBlueBits
      printInt2d(-1); // AccumAlphaBits

      // ms: ns b
      std::cout << " 0 0 ";

      // swap
      std::cout << ".  ";

      // caveat
      if (getAttrib(kCGLPFAAccelerated) != 0)
        std::cout << "None ";
      else
        std::cout << "Slow ";

      std::cout << "\n";
      continue;
    }

    std::cout << "Visual ID: " << aFormatIndex << "\n";
    for (const CglContextPixelAttr& anAttrIter : ThePixelAttributes)
    {
      const auto aVal = aFormatIter.find(anAttrIter.Enum);
      if (aVal == aFormatIter.cend())
        continue;

      if (aVal->second == 0
       && anAttrIter.Enum != kCGLPFAAccelerated)
      {
        continue;
      }

      std::cout << "  " << anAttrIter.Name << ": ";
      if (anAttrIter.Enum == kCGLPFAOpenGLProfile)
      {
        switch (aVal->second)
        {
          case kCGLOGLPVersion_Legacy:   std::cout << "Legacy"; break;
          case kCGLOGLPVersion_3_2_Core: std::cout << "Core3";  break;
          case kCGLOGLPVersion_GL4_Core: std::cout << "Core4";  break;
          default: std::cout << aVal->second; break;
        }
      }
      else if (anAttrIter.Enum == kCGLPFARendererID)
      {
        const GLint aMasked = aVal->second & kCGLRendererIDMatchingMask;
        std::cout <<   "0x" << std::hex << aVal->second << std::dec
                  << " [0x" << std::hex << aMasked << std::dec << "]";
        switch (aMasked)
        {
          case kCGLRendererGenericFloatID: std::cout << " [GenericFloat]"; break;
          case kCGLRendererGeForce8xxxID:  std::cout << " [GeForce8]"; break;
          case kCGLRendererGeForceID:      std::cout << " [GeForce6]"; break;
          case 0x27f00: std::cout << " [AppleM]"; break;
          default: break;
        }
      }
      else
      {
        std::cout << aVal->second;
      }
      std::cout << "\n";
    }
    ++aFormatIndex;
  }

  // table footer
  if (!theIsVerbose)
  {
    std::cout << "-----------------------------------------------------------------------------\n"
                 "    visual   x   bf lv rg d st  colorbuffer  sr ax dp st accumbuffer  ms  sw cav\n"
                 "  id  dep cl sp  sz l  ci b ro  r  g  b  a F gb bf th cl  r  g  b  a ns b ap eat\n"
                 "-----------------------------------------------------------------------------\n\n";
  }
}

#endif
