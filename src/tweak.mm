#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include "bin/runtime_booter.h"
#include "common/log/log.h"
#import "overlay.h"
#include "ui/settings.h"

static NSString* ResolveLogFilePath() {
  NSArray* dirs = NSSearchPathForDirectoriesInDomains(
      NSDocumentDirectory, NSUserDomainMask, YES);
  if (dirs.count == 0)
    return nil;
  return [dirs.firstObject stringByAppendingPathComponent:@"pubg_mx.log"];
}

__attribute__((constructor)) static void pubg_mx_init(void) {
  NSString* path = ResolveLogFilePath();
  if (path) {
    mxlog::OpenFile(path.UTF8String);
  }
  MXLOG_INFO("dylib loaded pid=%d", getpid());
  if (path) {
    MXLOG_INFO("log file: %s", path.UTF8String);
  }

  ui::settings::Init();

  dispatch_async(dispatch_get_main_queue(), ^{
    [ImGuiOverlayController attachWhenReady];
    runtimebooter::RuntimeBooter::Instance().Start();
  });
}
