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

void BaseGlContext::printExtensions(const char* theExt)
{
  static const int THE_LINE_LEN = 80;
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
        if (aLineLen != 0
            && aLineLen + aLen + 2 > THE_LINE_LEN)
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

void BaseGlContext::PrintExtensions()
{
  std::cout << Prefix() << "extensions:\n";

  int anExtNb = 0;
  if ((myCtxBits & ContextBits_GLES) == 0 && (myCtxBits & ContextBits_CoreProfile) != 0)
    GlGetIntegerv(GL_NUM_EXTENSIONS, &anExtNb);

  if (anExtNb == 0)
  {
    printExtensions((const char*)GlGetString(GL_EXTENSIONS));
    return;
  }

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
  printExtensions(aList.c_str());
}
