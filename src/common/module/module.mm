#import <Foundation/Foundation.h>

#include "common/module/module.h"
#include "common/log/log.h"

#include <mach-o/dyld.h>
#include <mach-o/loader.h>
#include <cstring>

namespace mach {

namespace {

bool FillFromImageIndex(uint32_t i, Module& out) {
    const char* name = _dyld_get_image_name(i);
    if (!name) return false;

    const auto* mh = reinterpret_cast<const struct mach_header_64*>(
        _dyld_get_image_header(i));
    if (!mh) return false;

    intptr_t slide = _dyld_get_image_vmaddr_slide(i);

    uintptr_t   text_begin = 0;
    uintptr_t   text_end   = 0;
    std::size_t image_size = 0;

    const auto* cmd_ptr = reinterpret_cast<const uint8_t*>(mh) + sizeof(*mh);
    for (uint32_t c = 0; c < mh->ncmds; ++c) {
        const auto* lc = reinterpret_cast<const struct load_command*>(cmd_ptr);

        if (lc->cmd == LC_SEGMENT_64) {
            const auto* sc = reinterpret_cast<const struct segment_command_64*>(lc);
            if (std::strcmp(sc->segname, "__TEXT") == 0) {
                image_size = sc->vmsize;

                const auto* secs = reinterpret_cast<const struct section_64*>(sc + 1);
                for (uint32_t s = 0; s < sc->nsects; ++s) {
                    if (std::strcmp(secs[s].sectname, "__text") == 0) {
                        text_begin = static_cast<uintptr_t>(secs[s].addr) + slide;
                        text_end   = text_begin + secs[s].size;
                        break;
                    }
                }
            }
        }
        cmd_ptr += lc->cmdsize;
    }

    if (text_begin == 0 || text_end == 0) return false;

    out.name       = name;
    out.base       = reinterpret_cast<uintptr_t>(mh);
    out.text_ = text_begin;
    out.text_end   = text_end;
    out.image_size = image_size;
    return true;
}

}  // namespace

bool FindModuleByName(const char* needle, Module& out) {
    if (!needle) return false;
    const uint32_t image_count = _dyld_image_count();

    for (uint32_t i = 0; i < image_count; ++i) {
        const char* path = _dyld_get_image_name(i);
        if (!path) continue;
        const char* base = std::strrchr(path, '/');
        base = base ? base + 1 : path;
        if (std::strcmp(base, needle) == 0 && FillFromImageIndex(i, out)) {
            MXLOG_INFO("dyld find '%s' base=0x%lx __text=[0x%lx..0x%lx]",
                       needle, out.base, out.text_, out.text_end);
            return true;
        }
    }

    for (uint32_t i = 0; i < image_count; ++i) {
        const char* path = _dyld_get_image_name(i);
        if (!path || !std::strstr(path, needle)) continue;
        if (FillFromImageIndex(i, out)) {
            MXLOG_INFO("dyld substring find '%s' path=%s base=0x%lx",
                       needle, path, out.base);
            return true;
        }
    }

    MXLOG_ERROR("dyld not find '%s'（finded %u image）", needle, image_count);
    return false;
}

}  // namespace mach
