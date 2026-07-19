#import "overlay.h"
#import <MetalKit/MetalKit.h>
#import <QuartzCore/QuartzCore.h>

#include <mach-o/getsect.h>
#include <mach-o/ldsyms.h> 

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_metal.h"

#include "ui/ui_root.h"
#include "state/texture.h"
#include "state/esp_state.h"
#include "state/app_state.h"
#include "common/log/log.h"

@interface HeeeNoScreenShotView : UIView
@property (nonatomic, strong) UITextField* textField;
@property (nonatomic, strong) UIView* clearView;
@end

@implementation HeeeNoScreenShotView

- (instancetype)init {
    self = [super init];
    if (self) [self setupUI];
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) [self setupUI];
    return self;
}

- (UITextField*)textField {
    if (!_textField) {
        _textField = [[UITextField alloc] init];
        _textField.secureTextEntry = YES;
    }
    return _textField;
}

- (UIView*)clearView {
    if (!_clearView) {
        _clearView = [[UIView alloc] init];
    }
    return _clearView;
}

- (UIView*)secureCanvas {
    for (UIView* sub in self.textField.subviews) {
        NSString* cls = NSStringFromClass([sub class]);
        if ([cls containsString:@"Canvas"] || [cls containsString:@"Content"]) {
            return sub;
        }
    }
    return self.textField.subviews.firstObject;
}

- (void)setupUI {
    [self addSubview:self.textField];
    [self.textField layoutIfNeeded];
    UIView* canvas = [self secureCanvas];
    if (canvas) {
        canvas.userInteractionEnabled = YES;
        [canvas addSubview:self.clearView];
    } else {
        [super addSubview:self.clearView];
    }
}

- (void)addSubview:(UIView*)view {
    [super addSubview:view];
    if (view != self.textField) {
        [self.clearView addSubview:view];
    }
}

- (void)layoutSubviews {
    [super layoutSubviews];
    UIView* canvas = [self secureCanvas];
    if (canvas && self.clearView.superview != canvas) {
        canvas.userInteractionEnabled = YES;
        [canvas addSubview:self.clearView];
    }
    self.textField.frame = self.bounds;
    self.clearView.frame = self.bounds;
}

- (UIView*)hitTest:(CGPoint)point withEvent:(UIEvent*)event {
    UIView* content = self.clearView.subviews.lastObject;
    if (content) {
        return [content hitTest:[content convertPoint:point fromView:self] withEvent:event];
    }
    return nil;
}

@end

@interface ImGuiOverlayController ()
+ (void)applyHideRecord;
@end

@interface ImGuiOverlayView : MTKView <MTKViewDelegate>
@property (nonatomic, assign) CGPoint touchPoint;
@property (nonatomic, assign) BOOL touchDown;
@property (nonatomic, assign) CFTimeInterval lastFrameTime;
@end

@implementation ImGuiOverlayView {
    id<MTLCommandQueue> _commandQueue;
    ImGuiContext* _imguiCtx;
}

- (instancetype)initWithFrame:(CGRect)frame {
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        MXLOG_ERROR("Metal is not available on this device");
        return nil;
    }
    self = [super initWithFrame:frame device:device];
    if (!self) return nil;

    self.delegate = self;
    self.framebufferOnly = NO;
    self.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    self.layer.opaque = NO;
    self.backgroundColor = [UIColor clearColor];
    self.opaque = NO;
    self.userInteractionEnabled = YES;
    self.multipleTouchEnabled = NO;
    
    self.preferredFramesPerSecond =
        (NSInteger)[UIScreen mainScreen].maximumFramesPerSecond;
    self.paused = NO;
    self.enableSetNeedsDisplay = NO;
    _commandQueue = [device newCommandQueue];

    CAMetalLayer* metalLayer = (CAMetalLayer*)self.layer;
    metalLayer.maximumDrawableCount = 3;
    metalLayer.presentsWithTransaction = NO;

    gfx::Init((__bridge void*)device);

    IMGUI_CHECKVERSION();
    _imguiCtx = ImGui::CreateContext();
    ImGui::SetCurrentContext(_imguiCtx);
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    CGFloat scale = [UIScreen mainScreen].scale;
    io.DisplaySize = ImVec2((float)frame.size.width, (float)frame.size.height);
    io.DisplayFramebufferScale = ImVec2((float)scale, (float)scale);
    io.FontGlobalScale = 1.0f;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.6f);
    style.WindowRounding = 18.0f;
    style.FrameRounding = 6.0f;

    ImGui_ImplMetal_Init(device);

    {
      unsigned long font_size = 0;
      const uint8_t* font_data =
          getsectiondata(&_mh_dylib_header, "__DATA", "__jhenghei", &font_size);
      if (font_data && font_size > 0) {
        ImFontConfig cfg;
        cfg.FontDataOwnedByAtlas = false;
        io.Fonts->AddFontFromMemoryTTF(
            const_cast<uint8_t*>(font_data), (int)font_size, 20.0f, &cfg);
        MXLOG_INFO("JhengHei font loaded from __DATA,__jhenghei (%lu bytes)",
                   font_size);
      } else {
        io.Fonts->AddFontDefault();
      }
    }

    _touchDown = NO;
    _touchPoint = CGPointMake(-1, -1);
    _lastFrameTime = CACurrentMediaTime();

    return self;
}

- (void)dealloc {
    ImGui_ImplMetal_Shutdown();
    if (_imguiCtx) {
        ImGui::DestroyContext(_imguiCtx);
        _imguiCtx = nullptr;
    }
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
}

- (void)drawInMTKView:(MTKView *)view {
    if (!_imguiCtx) return;
    ImGui::SetCurrentContext(_imguiCtx);

    [ImGuiOverlayController applyHideRecord];

    ImGuiIO& io = ImGui::GetIO();
    CGSize boundsSize = self.bounds.size;
    io.DisplaySize = ImVec2((float)boundsSize.width, (float)boundsSize.height);
    state::g_screen_w.store((float)boundsSize.width, std::memory_order_relaxed);
    state::g_screen_h.store((float)boundsSize.height, std::memory_order_relaxed);
    CGFloat scale = self.contentScaleFactor > 0 ? self.contentScaleFactor : [UIScreen mainScreen].scale;
    io.DisplayFramebufferScale = ImVec2((float)scale, (float)scale);

    CFTimeInterval now = CACurrentMediaTime();
    float dt = (float)(now - _lastFrameTime);
    if (dt <= 0.0f || dt > 1.0f) dt = 1.0f / 60.0f;
    _lastFrameTime = now;
    io.DeltaTime = dt;

    if (_touchPoint.x >= 0) {
        io.MousePos = ImVec2((float)_touchPoint.x, (float)_touchPoint.y);
    } else {
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    }
    io.MouseDown[0] = _touchDown;

    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    MTLRenderPassDescriptor* renderPassDescriptor = self.currentRenderPassDescriptor;
    if (!renderPassDescriptor) {
        [commandBuffer commit];
        return;
    }
    renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 0);
    renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;

    ImGui_ImplMetal_NewFrame(renderPassDescriptor);
    ImGui::NewFrame();

    ui::Draw();

    gfx::SweepLabelCache();

    ImGui::Render();
    id<MTLRenderCommandEncoder> renderEncoder =
        [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    [renderEncoder pushDebugGroup:@"ImGui"];
    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderEncoder);
    [renderEncoder popDebugGroup];
    [renderEncoder endEncoding];

    id<CAMetalDrawable> drawable = self.currentDrawable;
    if (drawable) {
        [commandBuffer presentDrawable:drawable];
    }
    [commandBuffer commit];
}

- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event {
    if (!_imguiCtx) return nil;
    ImGui::SetCurrentContext(_imguiCtx);
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    if (!ctx) return nil;

    for (int i = 0; i < ctx->Windows.Size; i++) {
        ImGuiWindow* w = ctx->Windows[i];
        if (!w || w->Hidden || !w->WasActive) continue;
        ImVec2 mn = w->Pos;
        ImVec2 mx = ImVec2(w->Pos.x + w->Size.x, w->Pos.y + w->Size.y);
        if (point.x >= mn.x && point.x <= mx.x &&
            point.y >= mn.y && point.y <= mx.y) {
            return self;
        }
    }
    return nil;
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    UITouch* t = touches.anyObject;
    if (!t) return;
    _touchPoint = [t locationInView:self];
    _touchDown = YES;
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    UITouch* t = touches.anyObject;
    if (!t) return;
    _touchPoint = [t locationInView:self];
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    _touchDown = NO;
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    _touchDown = NO;
}

@end

@implementation ImGuiOverlayController

static ImGuiOverlayView* sOverlay = nil;
static HeeeNoScreenShotView* sHideEsp = nil;
static __weak UIWindow* sHostWindow = nil;
static int sRetryCount = 0;
static const int kMaxRetries = 30;

+ (UIWindow *)findKeyWindow {
    UIApplication* app = UIApplication.sharedApplication;
    if (@available(iOS 13.0, *)) {
        for (UIScene* scene in app.connectedScenes) {
            if (scene.activationState != UISceneActivationStateForegroundActive) continue;
            if (![scene isKindOfClass:UIWindowScene.class]) continue;
            UIWindowScene* ws = (UIWindowScene*)scene;
            for (UIWindow* w in ws.windows) {
                if (w.isKeyWindow) return w;
            }
            if (ws.windows.count > 0) return ws.windows.firstObject;
        }
    }
    UIWindow* kw = app.keyWindow;
    if (kw) return kw;
    if (app.windows.count > 0) return app.windows.firstObject;
    return nil;
}

+ (void)attachWhenReady {
    if (sOverlay) return;

    UIWindow* window = [self findKeyWindow];
    if (!window) {
        if (++sRetryCount > kMaxRetries) {
            MXLOG_ERROR("keyWindow not found, giving up after %d retries", sRetryCount);
            return;
        }
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)),
                       dispatch_get_main_queue(), ^{
            [ImGuiOverlayController attachWhenReady];
        });
        return;
    }

    sOverlay = [[ImGuiOverlayView alloc] initWithFrame:window.bounds];
    if (!sOverlay) {
        MXLOG_ERROR("failed to create overlay view");
        return;
    }
    sOverlay.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [window addSubview:sOverlay];
    [window bringSubviewToFront:sOverlay];

    sHostWindow = window;
    sHideEsp = [[HeeeNoScreenShotView alloc] initWithFrame:window.bounds];
    sHideEsp.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [window addSubview:sHideEsp];
    [window bringSubviewToFront:sOverlay];

    MXLOG_INFO("overlay attached to UIWindow %p (hideEsp ready)", window);
}

+ (void)applyHideRecord {
    if (!sOverlay || !sHideEsp || !sHostWindow) return;
    BOOL hide = state::g_hide_record.load(std::memory_order_relaxed);
    if (hide) {
        if (sOverlay.superview != sHideEsp.clearView) {
            [sHideEsp addSubview:sOverlay];
            [sHideEsp layoutIfNeeded];
            [sHostWindow bringSubviewToFront:sHideEsp];
        }
    } else {
        if (sOverlay.superview != sHostWindow) {
            [sHostWindow addSubview:sOverlay];
            [sHostWindow bringSubviewToFront:sOverlay];
        }
    }
}

@end
