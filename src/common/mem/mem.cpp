#include "common/mem/mem.h"

#include <cstring>
#include <mach/mach.h>

#include "common/log/log.h"
#include "common/ptr/valid.h"

namespace mem {

namespace {

struct PageRange {
  uintptr_t start = 0;
  std::size_t len = 0;
};

PageRange AlignToPages(uintptr_t addr, std::size_t size) {
  const auto ps = static_cast<uintptr_t>(vm_page_size);
  const uintptr_t start = addr & ~(ps - 1);
  const uintptr_t end = (addr + size + ps - 1) & ~(ps - 1);
  return {start, static_cast<std::size_t>(end - start)};
}

bool QueryProtection(const uintptr_t addr, vm_prot_t &out_prot) {
  vm_address_t region = addr;
  vm_size_t size = 0;
  natural_t depth = 0;
  vm_region_submap_info_data_64_t info{};
  mach_msg_type_number_t count = VM_REGION_SUBMAP_INFO_COUNT_64;

  const kern_return_t kr = vm_region_recurse_64(mach_task_self(),
                                                &region,
                                                &size,
                                                &depth,
                                                reinterpret_cast<vm_region_recurse_info_t>(&info),
                                                &count);
  if (kr != KERN_SUCCESS) return false;

  out_prot = info.protection;
  return true;
}

bool MakeWritable(const PageRange &r) {
  kern_return_t kr = vm_protect(mach_task_self(), r.start, r.len, FALSE, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_COPY);
  if (kr == KERN_SUCCESS) return true;

  kr = vm_protect(mach_task_self(), r.start, r.len, FALSE, VM_PROT_READ | VM_PROT_WRITE);
  return kr == KERN_SUCCESS;
}

}  // namespace

bool WriteRaw(uintptr_t dst, const void *src, std::size_t size) {
  if (!ptr::IsValid(dst) || src == nullptr || size == 0) {
    MXLOG_ERROR("mem::WriteRaw bad args dst=0x%lx src=%p size=%zu", dst, src, size);
    return false;
  }

  vm_prot_t orig_prot = VM_PROT_READ;
  const bool have_orig = QueryProtection(dst, orig_prot);

  const PageRange r = AlignToPages(dst, size);
  if (!MakeWritable(r)) {
    MXLOG_ERROR("mem::WriteRaw make-writable fail dst=0x%lx", dst);
    return false;
  }

  std::memcpy(reinterpret_cast<void *>(dst), src, size);

  const vm_prot_t restore = have_orig ? orig_prot : VM_PROT_READ;
  const kern_return_t kr = vm_protect(mach_task_self(), r.start, r.len, FALSE, restore);
  if (kr != KERN_SUCCESS) {
    MXLOG_WARN("mem::WriteRaw reprotect fail kr=%d dst=0x%lx (write ok)", kr, dst);
  }
  return true;
}

}  // namespace mem
