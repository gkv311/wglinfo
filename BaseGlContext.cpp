// Copyright © Kirill Gavrilov, 2018-2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#include "BaseGlContext.h"

#include <iomanip>
#include <iostream>

#define GL_NO_ERROR   0
#define GL_VENDOR     0x1F00
#define GL_RENDERER   0x1F01
#define GL_VERSION    0x1F02
#define GL_EXTENSIONS 0x1F03
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_NUM_EXTENSIONS 0x821D

void BaseGlContext::printInt2d(int theInt)
{
  if (theInt < 0)
    std::cout << " . ";
  else
    std::cout << std::setw(2) << theInt << " ";
}

const char* BaseGlContext::getColorBufferClass(int theNbColorBits, int theNbRedBits)
{
  if (theNbColorBits <= 8)
    return "PseudoColor";
  else if (theNbRedBits >= 10)
    return "DeepColor";

  return "TrueColor";
}

static const int THE_LINE_LEN = 80;
void BaseGlContext::printExtensions(const char* theExt)
{
  if (theExt == NULL)
  {
    std::cout << "    NULL.\n\n";
    return;
  }

  int aStart = 0, aLineLen = 0;
  for (int aCharIter = 0;; ++aCharIter)
  {
    bool toBreak = theExt[aCharIter] == '\0';
    if (theExt[aCharIter] == ' '
     || theExt[aCharIter] == '\0')
    {
      const int aLen = aCharIter - aStart;
      for (; theExt[aCharIter] == ' '; ++aCharIter) {} // skip extra spaces
      if (theExt[aCharIter] == '\0')
      {
        toBreak = true;
      }

      if (aLen > 0)
      {
        if (aLineLen != 0 && aLineLen + aLen + 2 > THE_LINE_LEN)
        {
          std::cout << "\n";
          aLineLen = 0;
        }
        if (aLineLen == 0)
        {
          aLineLen += 4;
          std::cout << "    ";
        }
        else
        {
          aLineLen += 1;
          std::cout << " ";
        }

        aLineLen += aLen + 1;
        std::cout.write(theExt + aStart, aLen);
        std::cout << (toBreak ? "." : ",");
      }
      aStart = aCharIter;
    }
    if (toBreak)
    {
      std::cout << "\n\n";
      return;
    }
  }
}

void BaseGlContext::PrintRendererInfo()
{
    std::cout << Prefix() << "vendor   string: " << GlGetString(GL_VENDOR)   << "\n"
              << Prefix() << "renderer string: " << GlGetString(GL_RENDERER) << "\n"
              << Prefix() << "version  string: " << GlGetString(GL_VERSION)  << "\n";
    if (const char* aGlslVer = (const char* )GlGetString(GL_SHADING_LANGUAGE_VERSION))
      std::cout << Prefix() << "shading language version string: " << aGlslVer << "\n";
    else
      GlGetError();
}

void BaseGlContext::PrintGpuMemoryInfo()
{
  {
    //if (checkGlExtension("GL_ATI_meminfo"))
    int aMemInfo[4] = {-1, -1, -1, -1};
    GlGetIntegerv(0x87FB, aMemInfo); // GL_VBO_FREE_MEMORY_ATI = 0x87FB
    if (GlGetError() == GL_NO_ERROR && aMemInfo[0] != -1)
    {
      std::cout << Prefix() << "Free GPU memory: " << (aMemInfo[0] / 1024) << " MiB\n";
    }
  }
  {
    //if (checkGlExtension("GL_NVX_gpu_memory_info"))
    int aDedicated = -1;
    GlGetIntegerv(0x9047, &aDedicated); // GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX = 0x9047
    if (GlGetError() == GL_NO_ERROR && aDedicated != -1)
    {
      std::cout << Prefix() << "GPU memory: " << (aDedicated / 1024) << " MiB\n";
      //GLint aDedicatedFree = -1;
      //GlGetIntegerv(0x9049, &aDedicatedFree); // GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX = 0x9049
      //std::cout << Prefix() << "Free GPU memory: " << (aDedicatedFree / 1024) << " MiB\n";
    }
  }
}

std::string BaseGlContext::getGlExtensions()
{
  if ((myCtxBits & ContextBits_GLES) != 0 || (myCtxBits & ContextBits_CoreProfile) == 0)
    return ((const char*)GlGetString(GL_EXTENSIONS));

  int anExtNb = 0;
  GlGetIntegerv(GL_NUM_EXTENSIONS, &anExtNb);

  std::string aList;
  for (int anExtIter = 0; anExtIter < anExtNb; ++anExtIter)
  {
    const char* anExtension = (const char*)GlGetStringi(GL_EXTENSIONS, anExtIter);
    if (anExtension != NULL)
    {
      aList += std::string(anExtension);
      aList += std::string(" ");
    }
    //const size_t aTestExtNameLen = strlen (anExtension);
  }
  return aList.c_str();
}

void BaseGlContext::PrintExtensions()
{
  std::cout << Prefix() << "extensions:\n";
  const std::string anExtList = getGlExtensions();
  printExtensions(anExtList.c_str());
}

#define GL_MAX_VIEWPORT_DIMS              0x0D3A
#define GL_MAX_RENDERBUFFER_SIZE          0x84E8
#define GL_MAX_COLOR_ATTACHMENTS          0x8CDF
#define GL_MAX_DRAW_BUFFERS               0x8824

#define GL_MAX_FRAMEBUFFER_WIDTH          0x9315
#define GL_MAX_FRAMEBUFFER_HEIGHT         0x9316
#define GL_MAX_FRAMEBUFFER_LAYERS         0x9317
#define GL_MAX_FRAMEBUFFER_SAMPLES        0x9318

#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_RECTANGLE_TEXTURE_SIZE     0x84F8
#define GL_MAX_3D_TEXTURE_SIZE            0x8073
#define GL_MAX_TEXTURE_MAX_ANISOTROPY     0x84FF
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D // GLSL
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_TEXTURE_UNITS              0x84E2 // OpenGL 1.3, FFP
#define GL_MAX_ARRAY_TEXTURE_LAYERS       0x88FF

#define GL_MAX_SAMPLES                    0x8D57
#define GL_MAX_COLOR_TEXTURE_SAMPLES      0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES      0x910F
#define GL_MAX_INTEGER_SAMPLES            0x9110

#define GL_MAX_TEXTURE_BUFFER_SIZE        0x8C2B

#define GL_ALIASED_LINE_WIDTH_RANGE       0x846E
#define GL_SMOOTH_LINE_WIDTH_RANGE        0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY  0x0B23

#define GL_ALIASED_POINT_SIZE_RANGE       0x846D
#define GL_SMOOTH_POINT_SIZE_RANGE        0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY  0x0B13

#define GL_MAX_VERTEX_UNIFORM_BLOCKS      0x8A2B
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS    0x8A2C
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS    0x8A2D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS    0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS    0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE         0x8A30
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS 0x8A31
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS 0x8A32
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS 0x8A33
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT 0x8A34

#define GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET 0x82D9
#define GL_MAX_VERTEX_ATTRIB_BINDINGS     0x82DA

void BaseGlContext::printLimitInt(unsigned int theGlEnum, const char* theName)
{
  int aVal = 0;
  GlGetIntegerv(theGlEnum, &aVal);
  if (GlGetError() != GL_NO_ERROR)
    return;

  std::cout << "  " << theName << " = " << aVal << "\n";
}

void BaseGlContext::printLimitIntRange(unsigned int theGlEnum, const char* theName)
{
  int aVal[2] = {0, 0};
  GlGetIntegerv(theGlEnum, aVal);
  if (GlGetError() != GL_NO_ERROR)
    return;

  std::cout << "  " << theName << " = " << aVal[0] << ", " << aVal[1] << "\n";
}

#define LimitIntValue(theId) LimitDefinition(#theId, theId, 1)
#define LimitIntRange(theId) LimitDefinition(#theId, theId, 2)

void BaseGlContext::PrintLimits()
{
  std::cout << Prefix() << "limits:\n";

  const std::string anExtList = getGlExtensions();
  GlGetError(); // reset error if any

  static const LimitDefinition THE_LIMITS[] =
  {
    // viewport
    LimitIntRange(GL_MAX_VIEWPORT_DIMS),
    LimitIntValue(GL_MAX_RENDERBUFFER_SIZE),
    LimitIntValue(GL_MAX_SAMPLES),
    LimitIntValue(GL_MAX_COLOR_ATTACHMENTS),
    LimitIntValue(GL_MAX_DRAW_BUFFERS),
    // FBO
    LimitIntValue(GL_MAX_FRAMEBUFFER_WIDTH),
    LimitIntValue(GL_MAX_FRAMEBUFFER_HEIGHT),
    LimitIntValue(GL_MAX_FRAMEBUFFER_LAYERS),
    LimitIntValue(GL_MAX_FRAMEBUFFER_SAMPLES),
    // textures
    LimitIntValue(GL_MAX_TEXTURE_SIZE),
    LimitIntValue(GL_MAX_RECTANGLE_TEXTURE_SIZE),
    LimitIntValue(GL_MAX_3D_TEXTURE_SIZE),
    LimitIntValue(GL_MAX_ARRAY_TEXTURE_LAYERS),
    LimitIntValue(GL_MAX_TEXTURE_UNITS),
    LimitIntValue(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS),
    LimitIntValue(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS),
    LimitIntValue(GL_MAX_TEXTURE_MAX_ANISOTROPY),
    // MSAA
    LimitIntValue(GL_MAX_COLOR_TEXTURE_SAMPLES),
    LimitIntValue(GL_MAX_DEPTH_TEXTURE_SAMPLES),
    LimitIntValue(GL_MAX_INTEGER_SAMPLES),
    // TBO
    LimitIntValue(GL_MAX_TEXTURE_BUFFER_SIZE),
    // UBO
    LimitIntValue(GL_MAX_COMBINED_UNIFORM_BLOCKS),
    LimitIntValue(GL_MAX_VERTEX_UNIFORM_BLOCKS),
    LimitIntValue(GL_MAX_FRAGMENT_UNIFORM_BLOCKS),
    //LimitIntValue(GL_MAX_GEOMETRY_UNIFORM_BLOCKS),
    LimitIntValue(GL_MAX_UNIFORM_BUFFER_BINDINGS),
    LimitIntValue(GL_MAX_UNIFORM_BLOCK_SIZE),
    LimitIntValue(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT),
    // vertex attributes
    LimitIntValue(GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET),
    LimitIntValue(GL_MAX_VERTEX_ATTRIB_BINDINGS),
    // lines
    LimitIntRange(GL_ALIASED_LINE_WIDTH_RANGE),
    LimitIntRange(GL_SMOOTH_LINE_WIDTH_RANGE),
    // points
    LimitIntRange(GL_ALIASED_POINT_SIZE_RANGE),
    LimitIntRange(GL_SMOOTH_POINT_SIZE_RANGE)
  };

  for (const LimitDefinition& aLim : THE_LIMITS)
  {
    if (aLim.NbVals == 2)
      printLimitIntRange(aLim.Enum, aLim.Name);
    else
      printLimitInt(aLim.Enum, aLim.Name);
  }

#define GL_NUM_SHADING_LANGUAGE_VERSIONS  0x82E9
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
  if ((myCtxBits & ContextBits_GLES) != 0)
    return;

  int aNbVers = 0;
  GlGetIntegerv(GL_NUM_SHADING_LANGUAGE_VERSIONS, &aNbVers);
  if (GlGetError() != GL_NO_ERROR || aNbVers == 0)
    return;

  std::cout << "  GL_SHADING_LANGUAGE_VERSION =";
  size_t aLineLen = THE_LINE_LEN * 2;
  for (int aVerIter = 0; aVerIter < aNbVers; ++aVerIter)
  {
    const std::string aName = GlGetStringi(GL_SHADING_LANGUAGE_VERSION, aVerIter);
    aLineLen += aName.length();
    if (aLineLen > THE_LINE_LEN)
    {
      std::cout << "\n    ";
      aLineLen = aName.length() + 4;
    }
    else if (aVerIter > 0)
    {
      std::cout << ", ";
    }
    std::cout << aName;
  }
  std::cout << "\n";
}
