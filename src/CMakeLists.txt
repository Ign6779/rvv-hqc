cmake_minimum_required(VERSION 3.21)
project(hqc-src C ASM)

# List of HQC parameter variants
set(VARIANTS "hqc-1;hqc-3;hqc-5" CACHE INTERNAL "List of variants")

# 1) cross-arch common sources
file(GLOB COMMON_SOURCES
        "${CMAKE_SOURCE_DIR}/src/common/*.c"
)

# 2) arch-common: shared across variants (top-level .c)
if(HQC_ARCH STREQUAL "ref")
    set(ARCH_COMMON_DIR "${CMAKE_SOURCE_DIR}/src/ref")
elseif(HQC_ARCH STREQUAL "x86_64")
    set(ARCH_COMMON_DIR "${CMAKE_SOURCE_DIR}/src/x86_64/common")
#reusing top-level files for now
elseif(HQC_ARCH STREQUAL "riscv")
    set(ARCH_COMMON_DIR "${CMAKE_SOURCE_DIR}/src/riscv/common")
else()
    message(FATAL_ERROR "Unsupported HQC_ARCH: ${HQC_ARCH}")
endif()
file(GLOB ARCH_COMMON_TOP_SOURCES
        "${ARCH_COMMON_DIR}/*.c"
        "${ARCH_COMMON_DIR}/*.S"
)

# 3) arch-specific implementation sources
if(HQC_ARCH STREQUAL "ref")
    set(PLATFORM_DIR "${CMAKE_SOURCE_DIR}/src/ref")
elseif(HQC_ARCH STREQUAL "x86_64")
    set(PLATFORM_DIR "${CMAKE_SOURCE_DIR}/src/x86_64/${HQC_X86_IMPL}")
elseif(HQC_ARCH STREQUAL "riscv")
    set(PLATFORM_DIR "${CMAKE_SOURCE_DIR}/src/riscv/${HQC_RISCV_IMPL}")
else()
    message(FATAL_ERROR "Unsupported HQC_ARCH: ${HQC_ARCH}")
endif()
file(GLOB PLATFORM_SOURCES
        "${PLATFORM_DIR}/*.c"
        "${PLATFORM_DIR}/*.S"
)
file(GLOB PLATFORM_FAFFT_SOURCES
        "${PLATFORM_DIR}/fafft/*.c"
        "${PLATFORM_DIR}/fafft/*.S"
)

# 4) build one static library per variant
foreach(var IN LISTS VARIANTS)
    string(REPLACE "-" "_" safe ${var})

    if(HQC_ARCH STREQUAL "ref")
        set(VAR_DIR "${CMAKE_SOURCE_DIR}/src/ref/${var}")
    elseif(HQC_ARCH STREQUAL "x86_64")
        set(VAR_DIR "${CMAKE_SOURCE_DIR}/src/x86_64/${HQC_X86_IMPL}/${var}")
    elseif(HQC_ARCH STREQUAL "riscv")
        set(VAR_DIR "${CMAKE_SOURCE_DIR}/src/riscv/${HQC_RISCV_IMPL}/${var}")
    else()
        message(FATAL_ERROR "Unsupported HQC_ARCH: ${HQC_ARCH}")
    endif()
    file(GLOB VARIANT_SOURCES
            "${VAR_DIR}/*.c"
            "${VAR_DIR}/*.S"
    )

    file(GLOB ARCH_COMMON_VARIANT_SOURCES
            "${ARCH_COMMON_DIR}/${var}/*.c"
            "${ARCH_COMMON_DIR}/${var}/*.S"
    )

    add_library(${safe}_${HQC_ARCH} STATIC
            ${COMMON_SOURCES}
            ${ARCH_COMMON_TOP_SOURCES}
            ${ARCH_COMMON_VARIANT_SOURCES}
            ${PLATFORM_SOURCES}
            ${PLATFORM_FAFFT_SOURCES}
            ${VARIANT_SOURCES}
    )

    target_include_directories(${safe}_${HQC_ARCH} PUBLIC
            "${CMAKE_SOURCE_DIR}/src/common"
            "${CMAKE_SOURCE_DIR}/src/common/${var}"
            "${ARCH_COMMON_DIR}"
            "${ARCH_COMMON_DIR}/${var}"
            "${PLATFORM_DIR}"
            "${PLATFORM_DIR}/fafft"
            "${VAR_DIR}"
            "${CMAKE_SOURCE_DIR}/lib/fips202"
    )

    target_link_libraries(${safe}_${HQC_ARCH} PUBLIC
            fips202
    )
endforeach()
