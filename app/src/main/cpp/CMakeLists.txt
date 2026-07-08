cmake_minimum_required(VERSION 3.22.1)
project(ivanna_omega_supreme VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Flags de compilación: ESTABILIDAD NUMÉRICA prioridad máxima ─────────────
# FIX v1.5→v2.0: -ffast-math ELIMINADO permanentemente.
# Causaba NaN en Snapdragon 8 Gen 2/3 al vectorizar NEON con denormales.
set(BASE_FLAGS
    -O3
    -fno-fast-math
    -fno-associative-math
    -ffp-contract=off
    -Wall
    -Werror=return-type
    -Werror=uninitialized
    -Wno-unused-result
    -fno-exceptions
    -fno-rtti
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -funroll-loops
)

# ── Flags específicos por ABI ───────────────────────────────────────────────
if(ANDROID_ABI STREQUAL "arm64-v8a")
    list(APPEND BASE_FLAGS
        -march=armv8-a+simd+fp16
        -mtune=cortex-x2
        -D__ARM_NEON__=1
        -D__ARM_NEON
    )
elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
    list(APPEND BASE_FLAGS
        -march=armv7-a
        -mfpu=neon-vfpv4
        -mfloat-abi=softfp
        -D__ARM_NEON__=1
    )
endif()

# ── Definiciones globales ───────────────────────────────────────────────────
add_compile_definitions(
    OMEGA_LIMITER_ENABLED=1
    OMEGA_OMNIPOTENT=1
    IVANNA_VERSION="2.0.OMNIPOTENTE"
    IVANNA_BUILD_DATE="${CMAKE_BUILD_DATE}"
)

# ── Target: libivanna_omega.so (APK + Magisk) ──────────────────────────────
add_library(ivanna_omega SHARED
    # ═══════════════════════════════════════════════════════════════════════
    #  JNI Bridges — punto de entrada unificado
    # ═══════════════════════════════════════════════════════════════════════
    jni/ivanna_omega_jni.cpp
    jni/ivanna_visualizer_jni.cpp
    jni/ivanna_npe_jni.cpp
    jni/ivanna_jni_stub.cpp

    # ═══════════════════════════════════════════════════════════════════════
    #  DSP Core — cadena de procesamiento en tiempo real
    # ═══════════════════════════════════════════════════════════════════════
    dsp/ParametricEQ.cpp
    dsp/Compressor.cpp
    dsp/HarmonicExciter.cpp
    dsp/StereoWidener.cpp
    dsp/GainStage.cpp

    # ═══════════════════════════════════════════════════════════════════════
    #  PDEngine — Processing & Decision Engine
    # ═══════════════════════════════════════════════════════════════════════
    neuromorphic/ivanna_npe_engine.cpp
    neuromorphic/neuro_cochlear_manifold.cpp
    neuromorphic/volterra_h2_symmetric.cpp

    # ═══════════════════════════════════════════════════════════════════════
    #  Spatial Engine — Cue-Based ITD/ILD
    # ═══════════════════════════════════════════════════════════════════════
    spatial/spatial_engine.cpp
    spatial/room_model.cpp
    # cue_based_spatial.hpp es header-only (clase CueBasedSpatial inline), no tiene .cpp

    # ═══════════════════════════════════════════════════════════════════════
    #  Evolutionary Kernel — optimización genética offline
    # ═══════════════════════════════════════════════════════════════════════
    evolutionary_kernel.cpp

    # ═══════════════════════════════════════════════════════════════════════
    #  System Integration — daemon, control plane, orchestrator
    # ═══════════════════════════════════════════════════════════════════════
    shm_hyperplane.cpp
    omega_daemon.cpp
    audio_orchestrator.cpp
    audio_control_plane.cpp
    benchmark_logger.cpp

    # ═══════════════════════════════════════════════════════════════════════
    #  Anti-Dolby Integration
    # ═══════════════════════════════════════════════════════════════════════
    anti_dolby.cpp
)

# ── Include directories ───────────────────────────────────────────────────────
target_include_directories(ivanna_omega PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/dsp
    ${CMAKE_CURRENT_SOURCE_DIR}/neuromorphic
    ${CMAKE_CURRENT_SOURCE_DIR}/spatial
)

# ── Compile options ───────────────────────────────────────────────────────────
target_compile_options(ivanna_omega PRIVATE ${BASE_FLAGS})

# ── Link libraries ──────────────────────────────────────────────────────────────
find_library(log-lib log)
find_library(aaudio-lib aaudio)
find_library(android-lib android)

target_link_libraries(ivanna_omega
    ${log-lib}
    ${aaudio-lib}
    ${android-lib}
)

# ── Propiedades del target ────────────────────────────────────────────────────
set_target_properties(ivanna_omega PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
)

# ── Mensaje informativo ───────────────────────────────────────────────────────
message(STATUS "IVANNA OMEGA SUPREME v2.0 OMNIPOTENTE")
message(STATUS "  ABI: ${ANDROID_ABI}")
message(STATUS "  API: ${ANDROID_PLATFORM}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Fast Math: DISABLED (estabilidad numérica garantizada)")
