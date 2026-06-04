cmake_minimum_required(VERSION 3.21)
project(hqc_unit_tests C CXX)

enable_testing()

if(NOT DEFINED HQC_ARCH)
    message(FATAL_ERROR "HQC_ARCH must be set (ref, x86_64, or riscv)")
endif()
if(HQC_ARCH STREQUAL "x86_64" AND NOT DEFINED HQC_X86_IMPL)
    message(FATAL_ERROR "HQC_X86_IMPL must be set when HQC_ARCH=x86_64")
endif()
if(HQC_ARCH STREQUAL "riscv" AND NOT DEFINED HQC_RISCV_IMPL)
    message(FATAL_ERROR "HQC_RISCV_IMPL must be set when HQC_ARCH=riscv")
endif()
if(NOT DEFINED VARIANTS)
    message(FATAL_ERROR "VARIANTS (e.g. \"hqc-1;hqc-3;hqc-5\") must be set by tests/CMakeLists.txt")
endif()

option(HQC_ENABLE_NTL_TESTS "Enable NTL verification tests" ON)

set(TESTS_DIR     ${CMAKE_SOURCE_DIR}/tests/unit)
set(GF_TEST_SRC   ${TESTS_DIR}/test_gf.c)
set(RS_TEST_SRC   ${TESTS_DIR}/test_reed_solomon.c)
set(VEC_TEST_SRC  ${TESTS_DIR}/test_vector.c)
set(MAIN_TEST_SRC ${TESTS_DIR}/tests_main.c)
set(GF2X_NTL_SRC  ${TESTS_DIR}/test_vect_mul.cpp)
set(GF32V_TOWER_TEST_SRC ${TESTS_DIR}/test_gf32v_tower.c)

set(COMMON_INCLUDES
        ${CMAKE_SOURCE_DIR}/src/common
        ${CMAKE_SOURCE_DIR}/lib/fips202
        ${CMAKE_SOURCE_DIR}/tests/external/munit
        ${CMAKE_SOURCE_DIR}/tests
)

set(HQC_NTL_FOUND OFF)
if(HQC_ENABLE_NTL_TESTS)
    find_path(NTL_INCLUDE_DIR NAMES NTL/GF2X.h)
    find_library(NTL_LIBRARY NAMES ntl)
    find_library(GMP_LIBRARY NAMES gmp)

    if(NOT NTL_INCLUDE_DIR OR NOT NTL_LIBRARY OR NOT GMP_LIBRARY)
        message(WARNING "NTL/GMP not found. NTL-based tests will be skipped. Install NTL/GMP or set HQC_ENABLE_NTL_TESTS=OFF to silence this.")
        set(HQC_NTL_FOUND OFF)
    else()
        set(HQC_NTL_FOUND ON)
    endif()
endif()

foreach(param_variant IN LISTS VARIANTS)
    string(REPLACE "-" "_" safe_param ${param_variant})

    if(HQC_ARCH STREQUAL "ref")
        set(lib_target    ${safe_param}_ref)
        set(exe_name      test_unit_${safe_param}_ref)

        set(PARAM_INCLUDES
                ${CMAKE_SOURCE_DIR}/src/common/${param_variant}
                ${CMAKE_SOURCE_DIR}/src/ref
                ${CMAKE_SOURCE_DIR}/src/ref/${param_variant}
        )
    elseif(HQC_ARCH STREQUAL "x86_64")
        set(lib_target    ${safe_param}_${HQC_ARCH})
        set(exe_name      test_unit_${safe_param}_${HQC_X86_IMPL})

        set(PARAM_INCLUDES
                ${CMAKE_SOURCE_DIR}/src/common/${param_variant}
                ${CMAKE_SOURCE_DIR}/src/x86_64/common
                ${CMAKE_SOURCE_DIR}/src/x86_64/common/${param_variant}
                ${CMAKE_SOURCE_DIR}/src/x86_64/${HQC_X86_IMPL}
        )
    elseif(HQC_ARCH STREQUAL "riscv")
        set(lib_target    ${safe_param}_${HQC_ARCH})
        set(exe_name      test_unit_${safe_param}_${HQC_RISCV_IMPL})

        set(PARAM_INCLUDES
                ${CMAKE_SOURCE_DIR}/src/common/${param_variant}
                ${CMAKE_SOURCE_DIR}/src/riscv/common
                ${CMAKE_SOURCE_DIR}/src/riscv/common/${param_variant}
                ${CMAKE_SOURCE_DIR}/src/riscv/${HQC_RISCV_IMPL}
        )
    endif()

    set(unit_sources
            ${GF_TEST_SRC}
            ${RS_TEST_SRC}
            ${VEC_TEST_SRC}
            ${MAIN_TEST_SRC}
    )
    if(HQC_NTL_FOUND)
        list(APPEND unit_sources ${GF2X_NTL_SRC})
    endif()

    add_executable(${exe_name} ${unit_sources})

    target_link_libraries(${exe_name} PRIVATE
            munit
            ${lib_target}
            fips202
    )

    target_compile_definitions(${exe_name} PRIVATE
            $<$<STREQUAL:${HQC_ARCH},ref>:REF_MODE>
            $<$<AND:$<STREQUAL:${HQC_ARCH},x86_64>,$<STREQUAL:${HQC_X86_IMPL},avx256>>:AVX2_MODE>
            $<$<AND:$<STREQUAL:${HQC_ARCH},riscv>,$<STREQUAL:${HQC_RISCV_IMPL},rvv>>:RVV_MODE>
    )

    target_include_directories(${exe_name} PRIVATE
            ${COMMON_INCLUDES}
            ${PARAM_INCLUDES}
    )

    if(HQC_NTL_FOUND)
        target_include_directories(${exe_name} PRIVATE
                ${NTL_INCLUDE_DIR}
        )
        target_link_libraries(${exe_name} PRIVATE
                ${NTL_LIBRARY}
                ${GMP_LIBRARY}
        )
        target_compile_definitions(${exe_name} PRIVATE
                HQC_HAVE_NTL=1
        )
        set_target_properties(${exe_name} PROPERTIES
                CXX_STANDARD 17
                CXX_STANDARD_REQUIRED ON
                CXX_EXTENSIONS OFF
        )
        if(HQC_ARCH STREQUAL "x86_64")
            target_compile_options(${exe_name} PRIVATE
                    -mavx -mavx2 -mbmi -mpclmul
            )
        endif()
    endif()

    # temporary
    if(HQC_ARCH STREQUAL "riscv" AND HQC_RISCV_IMPL STREQUAL "rvv" AND param_variant STREQUAL "hqc-1")
        set(gf32v_tower_exe_name test_gf32v_tower_${safe_param}_${HQC_RISCV_IMPL})

        add_executable(${gf32v_tower_exe_name}
                ${GF32V_TOWER_TEST_SRC}
        )

        target_link_libraries(${gf32v_tower_exe_name} PRIVATE
                ${lib_target}
                fips202
        )

        target_compile_definitions(${gf32v_tower_exe_name} PRIVATE
                RVV_MODE
        )

        target_include_directories(${gf32v_tower_exe_name} PRIVATE
                ${COMMON_INCLUDES}
                ${PARAM_INCLUDES}
                ${CMAKE_SOURCE_DIR}/src/riscv/rvv/fafft
        )

        add_test(NAME ${gf32v_tower_exe_name} COMMAND ${gf32v_tower_exe_name})
        set_tests_properties(${gf32v_tower_exe_name} PROPERTIES LABELS "unit;gf32v;rvv")
    endif()
    # end temporary

    add_test(NAME ${exe_name} COMMAND ${exe_name})
    set_tests_properties(${exe_name} PROPERTIES LABELS "unit")
endforeach()
