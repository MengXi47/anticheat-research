embed_asset(mx_png mx.png)
embed_asset(bg_png background.png)
embed_asset(home_png home.png)
embed_asset(vision_png vision.png)
embed_asset(target_png target.png)
set(LIBWEBP_DIR ${CMAKE_SOURCE_DIR}/third_party/libwebp)
file(GLOB LIBWEBP_SOURCES CONFIGURE_DEPENDS
    ${LIBWEBP_DIR}/src/dec/*.c
    ${LIBWEBP_DIR}/src/enc/*.c
    ${LIBWEBP_DIR}/src/dsp/*.c
    ${LIBWEBP_DIR}/src/utils/*.c
    ${LIBWEBP_DIR}/sharpyuv/*.c
)
add_library(webp_static STATIC ${LIBWEBP_SOURCES})
target_include_directories(webp_static PRIVATE ${LIBWEBP_DIR} ${LIBWEBP_DIR}/src)
set_target_properties(webp_static PROPERTIES
    C_VISIBILITY_PRESET default
    POSITION_INDEPENDENT_CODE ON)
target_compile_options(webp_static PRIVATE -w)
target_dd_shared_lib(PUBGM_MX_DYLIB
    pubgm_sdk
    imgui
    "-framework UIKit"
    "-framework Foundation"
    "-framework CoreGraphics"
    "-framework QuartzCore"
    "-framework Metal"
    "-framework MetalKit"
    "-framework IOKit"
)
set(PTFAKE_DIR ${CMAKE_SOURCE_DIR}/third_party/PTFakeTouch/PTFakeTouch)
set(PTFAKE_SOURCES
    ${PTFAKE_DIR}/PTFakeMetaTouch.m
    ${PTFAKE_DIR}/addition/UITouch-KIFAdditions.m
    ${PTFAKE_DIR}/addition/UIEvent+KIFAdditions.m
    ${PTFAKE_DIR}/addition/IOHIDEvent+KIF.m
)
target_sources(PUBGM_MX_DYLIB PRIVATE ${PTFAKE_SOURCES})
target_include_directories(PUBGM_MX_DYLIB PRIVATE
    ${PTFAKE_DIR}
    ${PTFAKE_DIR}/addition
)
set_source_files_properties(${PTFAKE_SOURCES} PROPERTIES
    COMPILE_DEFINITIONS "DLog=NSLog")
set_target_properties(PUBGM_MX_DYLIB PROPERTIES
    PREFIX ""
    SUFFIX ""
    OUTPUT_NAME "${OUTPUT_BINARY_NAME}"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
    XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
    XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
)
target_compile_options(PUBGM_MX_DYLIB PRIVATE
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -Wno-deprecated-declarations
    -Wno-unused-parameter
    $<$<COMPILE_LANGUAGE:OBJC>:-fobjc-arc>
    $<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>
)
target_link_options(PUBGM_MX_DYLIB PRIVATE
    -dynamiclib
    "SHELL:-Xlinker -install_name -Xlinker ${HIJACK_INSTALL_NAME}"
    "SHELL:-Xlinker -sectcreate -Xlinker __DATA -Xlinker __jhenghei -Xlinker ${CMAKE_SOURCE_DIR}/assets/font.ttf"
    "SHELL:-Xlinker -force_load -Xlinker $<TARGET_FILE:webp_static>"
    "SHELL:-compatibility_version 1.0.0"
    "SHELL:-current_version 1.0.0"
    "SHELL:-Xlinker -U -Xlinker _IOHIDEventCreateDigitizerEvent"
    "SHELL:-Xlinker -U -Xlinker _IOHIDEventCreateDigitizerFingerEventWithQuality"
    "SHELL:-Xlinker -U -Xlinker _IOHIDEventSetIntegerValue"
    "SHELL:-Xlinker -U -Xlinker _IOHIDEventAppendEvent"
)
add_dependencies(PUBGM_MX_DYLIB webp_static)
