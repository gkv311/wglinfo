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

#include <array>
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

  // accelerated or software implementation
  for (int anAccel = 1; anAccel >= 0; --anAccel)
  {
    // 128 for float
    // 164 for half-float
    // 32  for R8G8B8A8
    // 30  for R10G10B10A2 (will be returned only when requested with 2 bits for alpha!)
    // 24 & 16 are never returned -> 32 is returned instead
    //
    // software implementation supports only two color formats: R8G8B8A8 and R32G32B32A32 (float)
    for (int aColor : {128, 64, 32, 30, 24, 16})
    {
      // 32 for float
      // 16 for half-float (accelerated-only)
      // 8  for R8G8B8A8
      // 2  for R10G10B10A2 (accelerated-only)
      // 0  alpha is never returned -> 8 is returned instead
      for (int anAlpha : {2, 8, 0})
      {
        // 32 corresponds to float?
        // 24 & 16 depth are never returned
        // 0  to disable depth (but should be combined with 0 stencil!)
        for (int aDepth : {32, 24, 16, 0})
        {
          // 8 for normal stencil
          // 0 to disable stencil
          for (int aStencil : {8, 0})
          {
            // OpenGL 4   Core Profile
            // OpenGL 3.2 Core Profile
            // OpenGL 2.1 Legacy (very outdated)
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
              else
              {
                // this doesn't make any difference - 32 bit format is always returned
                anAttribs[aLastAttrib++] = kCGLPFAColorSize;
                anAttribs[aLastAttrib++] = aColor;
              }

              anAttribs[aLastAttrib++] = kCGLPFAAlphaSize;
              anAttribs[aLastAttrib++] = anAlpha;

              // this doesn't make any difference - 32 bit depth is always returned for any non-zero input
              anAttribs[aLastAttrib++] = kCGLPFADepthSize;
              anAttribs[aLastAttrib++] = aDepth;

              anAttribs[aLastAttrib++] = kCGLPFAStencilSize;
              anAttribs[aLastAttrib++] = aStencil;

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
      }
    }
  }

  std::cout << "\n[" << PlatformName() << "] " << aFormats.size() << " CGL Visuals\n";
  if (!theIsVerbose)
    VisualInfo::PrintTableHeader(true);

  int aFormatIndex = 0;
  for (const FormatInfo& aFormatIter : aFormats)
  {
    const auto getAttrib = [&aFormatIter](CGLPixelFormatAttribute theEnum, GLint theDef = 0) -> GLint
    {
      const auto aVal = aFormatIter.find(theEnum);
      return aVal != aFormatIter.cend() ? aVal->second : theDef;
    };

    // RGBA
    const int aColorSize  = getAttrib(kCGLPFAColorSize);
    const int anAlphaSize = getAttrib(kCGLPFAAlphaSize);
    const int aRedBits    = (aColorSize - anAlphaSize) / 3;

    if (!theIsVerbose)
    {
      VisualInfo anInfo;
      anInfo.ConfigId = aFormatIndex++;

      anInfo.ConfigCaveat = VisualInfo::Caveat_None;
      if (getAttrib(kCGLPFAAccelerated) == 0)
        anInfo.ConfigCaveat = VisualInfo::Caveat(anInfo.ConfigCaveat | VisualInfo::Caveat_Slow);

      anInfo.BufferType = VisualInfo::ColorBuffer_Rgba;

      if (getAttrib(kCGLPFAWindow) != 0)
        anInfo.SurfaceType = VisualInfo::Surface(anInfo.SurfaceType | VisualInfo::Surface_Window);
      if (getAttrib(kCGLPFAPBuffer) != 0)
        anInfo.SurfaceType = VisualInfo::Surface(anInfo.SurfaceType | VisualInfo::Surface_PBuffer);
      if (getAttrib(kCGLPFARemotePBuffer) != 0)
        anInfo.SurfaceType = VisualInfo::Surface(anInfo.SurfaceType | VisualInfo::Surface_PBufferRemote);

      anInfo.ColorDepth      = 0;
      anInfo.ColorBufferSize = aColorSize;
      anInfo.RedSize         = aRedBits;
      anInfo.GreenSize       = aRedBits;
      anInfo.BlueSize        = aRedBits;
      anInfo.AlphaSize       = anAlphaSize;
      anInfo.DepthSize       = getAttrib(kCGLPFADepthSize);
      anInfo.StencilSize     = getAttrib(kCGLPFAStencilSize);

      anInfo.NbSwapBuffers = 1;
      if (getAttrib(kCGLPFATripleBuffer) != 0)
        anInfo.NbSwapBuffers = 3;
      else if (getAttrib(kCGLPFADoubleBuffer) != 0)
        anInfo.NbSwapBuffers = 2;

      anInfo.IsStereoBuffer = getAttrib(kCGLPFAStereo) != 0;
      anInfo.IsColorFloat = getAttrib(kCGLPFAColorFloat) != 0;
      // probably the property is meaningless, as colorspace is assigned to window dynamically
      // [NSWindow setColorSpace: [NSColorSpace sRGBColorSpace]];
      anInfo.IsSRgb = !anInfo.IsColorFloat;

      // dummy
      anInfo.NbAuxBuffers   = getAttrib(kCGLPFAAuxBuffers);
      anInfo.AccumRedSize   = getAttrib(kCGLPFAAccumSize) / 4;
      anInfo.AccumGreenSize = getAttrib(kCGLPFAAccumSize) / 4;
      anInfo.AccumBlueSize  = getAttrib(kCGLPFAAccumSize) / 4;
      anInfo.AccumAlphaSize = getAttrib(kCGLPFAAccumSize) / 4;

      anInfo.NbSampleBuffers = getAttrib(kCGLPFASampleBuffers);
      anInfo.NbSamples       = getAttrib(kCGLPFASamples);

      anInfo.PrintTableLine();
      continue;
    }

    std::cout << "Visual ID: " << aFormatIndex << "\n";

    std::string aRendTarget;
    if (getAttrib(kCGLPFAWindow) != 0)
      aRendTarget += "|window";

    if (getAttrib(kCGLPFAPBuffer) != 0)
      aRendTarget += "|PBuffer";

    if (getAttrib(kCGLPFARemotePBuffer) != 0)
      aRendTarget += "|remotePBuffer";

    if (!aRendTarget.empty())
      aRendTarget = aRendTarget.substr(1);
    else
      aRendTarget = ".";

    std::cout << "    color: R" << aRedBits << "G" << aRedBits << "B" << aRedBits << "A" << anAlphaSize
              << " (" << getColorBufferClass(aColorSize, aRedBits) << ", " << aColorSize << ")"
              << " depth: " << getAttrib(kCGLPFADepthSize) << " stencil: " << getAttrib(kCGLPFAStencilSize) << "\n"
              << "    doubleBuffer: " << (getAttrib(kCGLPFADoubleBuffer) != 0)
              << " stereo: " << (getAttrib(kCGLPFAStereo) != 0) << "\n"
              << "    renderTarget: " << aRendTarget << " caveat: " << (getAttrib(kCGLPFAAccelerated) == 0 ? "slow " : "none ")
              << "\n"
              << "    profile: "
              << (getAttrib(kCGLPFAOpenGLProfile) == kCGLOGLPVersion_GL4_Core ? "OpenGL 4 Core Profile " : "")
              << (getAttrib(kCGLPFAOpenGLProfile) == kCGLOGLPVersion_3_2_Core ? "OpenGL 3 Core Profile " : "")
              << (getAttrib(kCGLPFAOpenGLProfile) == kCGLOGLPVersion_Legacy   ? "OpenGL 2 Legacy " : "")
              << "\n";

    for (const CglContextPixelAttr& anAttrIter : ThePixelAttributes)
    {
      const auto aVal = aFormatIter.find(anAttrIter.Enum);
      if (aVal == aFormatIter.cend())
        continue;

      if (aVal->second == 0
       && anAttrIter.Enum != kCGLPFAAccelerated)
      {
        continue; // just skip zero values
      }

      if (anAttrIter.Enum == kCGLPFAColorSize
       || anAttrIter.Enum == kCGLPFAAlphaSize
       || anAttrIter.Enum == kCGLPFADepthSize
       || anAttrIter.Enum == kCGLPFAStencilSize
       || anAttrIter.Enum == kCGLPFADoubleBuffer
       || anAttrIter.Enum == kCGLPFAAccelerated
       || anAttrIter.Enum == kCGLPFAOpenGLProfile
       || anAttrIter.Enum == kCGLPFAWindow
       || anAttrIter.Enum == kCGLPFAPBuffer
       || anAttrIter.Enum == kCGLPFARemotePBuffer)
      {
        continue; // displayed on top using special format
      }

      std::cout << "    " << anAttrIter.Name << ": ";

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
    VisualInfo::PrintTableHeader(false);
}

#endif
