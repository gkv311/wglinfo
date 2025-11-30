// Copyright © Kirill Gavrilov, 2025
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#include "CocoaWindow.h"

#import <Cocoa/Cocoa.h>

#include <cstring>
#include <iostream>

//! Main Cocoa application responder - minimal implementation for offscreen viewer.
@interface WglinfoTestNSResponder : NSObject <NSApplicationDelegate>
  {
  }

  //! Default constructor.
  - (id ) init;

  //! Access singleton.
  + (WglinfoTestNSResponder* ) sharedInstance;

  //! Dummy method for thread-safety Cocoa initialization.
  + (void ) doDummyThread: (id )theParam;

@end

@implementation WglinfoTestNSResponder

  - (id )init
  {
    self = [super init];
    return self;
  }

  // Singletone implementation
  + (WglinfoTestNSResponder* ) sharedInstance
  {
    static WglinfoTestNSResponder* TheAppResponder = [[super allocWithZone: nullptr] init];
    return TheAppResponder;
  }
  + (id ) allocWithZone: (NSZone* )theZone { return [[self sharedInstance] retain]; }
  - (id ) copyWithZone:  (NSZone* )theZone { return self; }
  - (id ) retain { return self; }
  - (NSUInteger ) retainCount { return NSUIntegerMax; }
  - (oneway void ) release {}
  - (id ) autorelease { return self; }

  - (BOOL ) application: (NSApplication* )theApplication openFile: (NSString* )theFilename { return YES; }
  - (void ) applicationDidFinishLaunching: (NSNotification* )theNotification {}
  - (void ) applicationWillTerminate: (NSNotification* )theNotification {}

  + (void ) doDummyThread: (id )theParam {}

@end

static void createNSApp()
{
  // create dummy NSThread to ensure Cocoa thread-safety
  [NSThread detachNewThreadSelector: @selector(doDummyThread: ) toTarget: [WglinfoTestNSResponder class] withObject: nullptr];

  NSApplication* anAppNs = [NSApplication sharedApplication];
  WglinfoTestNSResponder* anAppResp = [WglinfoTestNSResponder sharedInstance];
  [anAppNs setDelegate: anAppResp];
  //[anAppNs run]; // Cocoa event loop
}

std::string CocoaWindow::GetOsVersion()
{
  NSString* aStr = [[NSProcessInfo processInfo] operatingSystemVersionString];
  return aStr != nullptr ? [aStr UTF8String] : "";
}

CocoaWindow::CocoaWindow(const std::string& theTitle)
: myTitle(theTitle)
{
  //
}

bool CocoaWindow::Create()
{
  Destroy();

  if (NSApp == nullptr)
    createNSApp();

  //CocoaLocalPool aLocalPool; // #if !__has_feature(objc_arc)
  NSUInteger aWinStyle = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
  NSRect aRectNs = NSMakeRect(2.0f, 2.0f, 4.0f, 4.0f);
  myHWindow = [[NSWindow alloc] initWithContentRect: aRectNs
                                          styleMask: aWinStyle
                                            backing: NSBackingStoreBuffered
                                              defer: NO];
  if (myHWindow == nullptr)
  {
    std::cerr << "Error: unable to create NSWindow\n";
    return false;
  }
  [myHWindow setColorSpace: [NSColorSpace sRGBColorSpace]];
  myHView = [[myHWindow contentView] retain];

  NSString* aTitleNs = [[NSString alloc] initWithUTF8String: myTitle.c_str()];
  [myHWindow setTitle: aTitleNs];
  [aTitleNs release];

  // do not destroy NSWindow on close - we don't handle it!
  [myHWindow setReleasedWhenClosed: NO];
  return true;
}

void CocoaWindow::destroyWindow()
{
  if (myHWindow != 0)
  {
    //[myHWindow release];
    myHWindow = nullptr;
  }
  if (myHView != 0)
  {
    //[myHView release];
    myHView = nullptr;
  }
}
