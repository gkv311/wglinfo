// Copyright © Kirill Gavrilov, 2018-2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#include "WasmWindow.h"

#if defined(__EMSCRIPTEN__)

#include <emscripten/html5.h>

#include <cstring>
#include <iomanip>
#include <iostream>

// Function creates a new HTML5 canvas and appends it to body.
EM_JS(void, wglinfoJSCreateCanvas, (const char* theId), {
  const aStr = Number(theId); // bigintToI53Checked(theId);
  const aCanvas = document.createElement('canvas');
  aCanvas.id = UTF8ToString(aStr);
  aCanvas.width  = 4;
  aCanvas.height = 4;
  document.body.appendChild(aCanvas);
});

// Function removes HTML5 canvas.
EM_JS(void, wglinfoJSDeleteCanvas, (const char* theId), {
  const aStr    = Number(theId); // bigintToI53Checked(theId);
  const aCanvas = document.getElementById(UTF8ToString(aStr));
  document.body.removeChild(aCanvas);
});

WasmWindow::WasmWindow(const std::string& theTitle)
: myTitle(theTitle)
{
  //
}

bool WasmWindow::Create()
{
  Destroy();

  myCanvasId = myTitle;
  wglinfoJSCreateCanvas(myCanvasId.c_str());
  return true;
}

void WasmWindow::destroyWindow()
{
  if (!myCanvasId.empty())
  {
    wglinfoJSDeleteCanvas(myCanvasId.c_str());
    myCanvasId.clear();
  }
}

#endif
