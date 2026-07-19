#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <Foundation/Foundation.h>
#import <CoreText/CoreText.h>

#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#endif

#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "state/texture.h"
#include "common/log/log.h"
#include "assets/mx_png.h"
#include "assets/bg_png.h"
#include "assets/home_png.h"
#include "assets/vision_png.h"
#include "assets/target_png.h"

namespace gfx {

static id<MTLDevice>  g_device = nil;
static id<MTLTexture> g_mx_tex = nil;
static id<MTLTexture> g_bg_tex = nil;
static id<MTLTexture> g_home_tex = nil;
static id<MTLTexture> g_vision_tex = nil;
static id<MTLTexture> g_target_tex = nil;

static id<MTLTexture> LoadTex(
    MTKTextureLoader* loader, NSDictionary* options,
    const unsigned char* data, NSUInteger size, const char* name) {
    NSData* nsdata =
        [NSData dataWithBytesNoCopy:(void*)data length:size freeWhenDone:NO];
    NSError* error = nil;
    id<MTLTexture> tex = [loader newTextureWithData:nsdata options:options error:&error];
    if (!tex) {
        MXLOG_ERROR("failed to load %s: %s", name,
                    error ? error.localizedDescription.UTF8String : "(unknown)");
    } else {
        MXLOG_INFO("%s Metal texture uploaded to GPU", name);
    }
    return tex;
}

void Init(void* metal_device) {
    g_device = (__bridge id<MTLDevice>)metal_device;
    if (!g_device) {
        return;
    }

    MTKTextureLoader* loader = [[MTKTextureLoader alloc] initWithDevice:g_device];
    NSDictionary* plain = @{
        MTKTextureLoaderOptionSRGB: @NO,
        MTKTextureLoaderOptionGenerateMipmaps: @NO,
    };
    NSDictionary* mipped = @{
        MTKTextureLoaderOptionSRGB: @NO,
        MTKTextureLoaderOptionGenerateMipmaps: @YES,
    };

    g_mx_tex = LoadTex(loader, plain, assets::mx_png_data, assets::mx_png_size, "mx");
    g_bg_tex = LoadTex(loader, plain, assets::bg_png_data, assets::bg_png_size, "background");
    g_home_tex = LoadTex(loader, mipped, assets::home_png_data, assets::home_png_size, "home icon");
    g_vision_tex = LoadTex(loader, mipped, assets::vision_png_data, assets::vision_png_size, "vision icon");
    g_target_tex = LoadTex(loader, mipped, assets::target_png_data, assets::target_png_size, "target icon");
}

void* MxIconTexture() { return (__bridge void*)g_mx_tex; }
void* BackgroundTexture() { return (__bridge void*)g_bg_tex; }
void* HomeIcon() { return (__bridge void*)g_home_tex; }
void* VisionIcon() { return (__bridge void*)g_vision_tex; }
void* TargetIcon() { return (__bridge void*)g_target_tex; }

namespace {

struct CachedLabel {
    id<MTLTexture> tex;
    float w;
    float h;
    uint64_t used_frame;
};

std::unordered_map<std::string, CachedLabel> g_labels;
uint64_t g_label_frame = 0;
constexpr uint64_t kLabelTTL = 180;
constexpr CGFloat kLabelSupersample = 3.0;

id<MTLTexture> RenderLabelTexture(NSString* s, CGFloat font_px, float* out_w, float* out_h) {
#if TARGET_OS_IPHONE
    UIFont* font = [UIFont systemFontOfSize:font_px];
    NSDictionary* attrs = @{
        NSFontAttributeName: font,
        NSForegroundColorAttributeName: UIColor.whiteColor,
    };
    const CGSize sz = [s sizeWithAttributes:attrs];
    const int lw = (int)std::ceil(sz.width);
    const int lh = (int)std::ceil(sz.height);
#else
    CTFontRef ctFont = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, font_px, nullptr);
    if (!ctFont) return nil;
    CGColorSpaceRef rgbcs = CGColorSpaceCreateDeviceRGB();
    const CGFloat whiteComps[4] = {1.0, 1.0, 1.0, 1.0};
    CGColorRef white = CGColorCreate(rgbcs, whiteComps);
    NSDictionary* attrs = @{
        (__bridge id)kCTFontAttributeName: (__bridge id)ctFont,
        (__bridge id)kCTForegroundColorAttributeName: (__bridge id)white,
    };
    CGColorRelease(white);
    CGColorSpaceRelease(rgbcs);
    NSAttributedString* as = [[NSAttributedString alloc] initWithString:s attributes:attrs];
    CTLineRef line = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)as);
    CFRelease(ctFont);
    if (!line) return nil;
    CGFloat ascent = 0.0, descent = 0.0, leading = 0.0;
    const double tw = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);
    const int lw = (int)std::ceil(tw);
    const int lh = (int)std::ceil(ascent + descent);
#endif
    if (lw <= 0 || lh <= 0) {
#if !TARGET_OS_IPHONE
        CFRelease(line);
#endif
        return nil;
    }

    const int pw = (int)std::ceil(lw * kLabelSupersample);
    const int ph = (int)std::ceil(lh * kLabelSupersample);
    const size_t bpr = (size_t)pw * 4;
    std::vector<uint8_t> buf(bpr * (size_t)ph, 0);

    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    const CGBitmapInfo bmp = (CGBitmapInfo)((uint32_t)kCGImageAlphaPremultipliedLast |
                                            (uint32_t)kCGBitmapByteOrder32Big);
    CGContextRef ctx = CGBitmapContextCreate(buf.data(), pw, ph, 8, bpr, cs, bmp);
    CGColorSpaceRelease(cs);
    if (!ctx) {
#if !TARGET_OS_IPHONE
        CFRelease(line);
#endif
        return nil;
    }
#if TARGET_OS_IPHONE
    CGContextTranslateCTM(ctx, 0, ph);
    CGContextScaleCTM(ctx, kLabelSupersample, -kLabelSupersample);
    UIGraphicsPushContext(ctx);
    [s drawAtPoint:CGPointZero withAttributes:attrs];
    UIGraphicsPopContext();
#else
    CGContextScaleCTM(ctx, kLabelSupersample, kLabelSupersample);
    CGContextSetTextPosition(ctx, 0.0, descent);
    CTLineDraw(line, ctx);
    CFRelease(line);
#endif
    CGContextRelease(ctx);
    for (size_t i = 0; i + 3 < buf.size(); i += 4) {
        buf[i] = 255;
        buf[i + 1] = 255;
        buf[i + 2] = 255;
    }
    MTLTextureDescriptor* desc =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                           width:pw
                                                          height:ph
                                                       mipmapped:NO];
    desc.usage = MTLTextureUsageShaderRead;
    id<MTLTexture> tex = [g_device newTextureWithDescriptor:desc];
    if (!tex) return nil;
    [tex replaceRegion:MTLRegionMake2D(0, 0, pw, ph)
           mipmapLevel:0
             withBytes:buf.data()
           bytesPerRow:bpr];

    *out_w = (float)lw;
    *out_h = (float)lh;
    return tex;
}

}  // namespace

LabelTex AcquireLabel(const char* utf8, float font_px) {
    if (!g_device || !utf8 || !utf8[0]) return {nullptr, 0.0f, 0.0f};

    std::string key(utf8);
    key.push_back('\x01');
    key += std::to_string((int)font_px);

    auto it = g_labels.find(key);
    if (it != g_labels.end()) {
        it->second.used_frame = g_label_frame;
        return {(__bridge void*)it->second.tex, it->second.w, it->second.h};
    }

    @autoreleasepool {
        NSString* s = [NSString stringWithUTF8String:utf8];
        if (!s) return {nullptr, 0.0f, 0.0f};
        float w = 0.0f, h = 0.0f;
        id<MTLTexture> tex = RenderLabelTexture(s, font_px, &w, &h);
        if (!tex) return {nullptr, 0.0f, 0.0f};
        g_labels[key] = CachedLabel{tex, w, h, g_label_frame};
        return {(__bridge void*)tex, w, h};
    }
}

void SweepLabelCache() {
    ++g_label_frame;
    for (auto it = g_labels.begin(); it != g_labels.end();) {
        if (g_label_frame - it->second.used_frame > kLabelTTL) {
            it = g_labels.erase(it);
        } else {
            ++it;
        }
    }
}

}  // namespace gfx
