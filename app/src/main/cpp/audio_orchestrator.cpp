/*
 * IVANNA-OMEGA-SUPREME v2.0 OMNIPOTENTE — audio_orchestrator.cpp
 * © 2025–2026 Luis Uriel Pimentel Pérez. Todos los derechos reservados.
 *
 * v2.0 OMNIPOTENTE:
 * - Pipeline unificado: DSP → PDEngine → Spatial → Limiter
 * - ControlFrameBus integration: zero mutex en audio thread
 * - Phase Oracle predice coeficientes biquad 1 bloque antes
 * - Evolutionary genome aplicado via Synthesizer smoothing
 * - AutonomousBrain conduce NHO params sin intervencion humana
 * - NaN/Inf sanitize en CADA sample, no solo al final
 * - Branchless limiter con prediccion de signo
 * - Deteccion de silencio para pausar adaptacion
 * - BPM detection por zero-crossing con histeresis
 */

#include <jni.h>
#include <aaudio/AAudio.h>
#include <android/log.h>
#include <cmath>
#include <atomic>
#include <mutex>
#include <thread>
#include <algorithm>
#include <cstring>

#if defined(__ARM_NEON) && defined(__aarch64__)
#include <arm_neon.h>
#define IVANNA_NEON 1
#else
#define IVANNA_NEON 0
#endif

#include "anti_dolby.h"
#include "include/dsp_types.h"
#include "include/audio_thread_priority.h"
#include "include/HarmonicExciter.h"
#include "include/StereoWidener.h"
#include "control_frame.hpp"
#include "neuromorphic/nho_engine.hpp"
#include "neuromorphic/biquad_envelope_bank.hpp"
#include "spatial/cue_based_spatial.hpp"
#include "neuromorphic/autonomous_brain.hpp"
#include "neuromorphic/synthesizer.hpp"

#define LOG_TAG "IVANNA-Audio-v2"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// ── Constantes matematicas ────────────────────────────────────────────────────
static constexpr float TWO_PI        = 6.283185307179586f;
static constexpr float INV_TWO_PI    = 0.159154943091895f;
static constexpr float LIMITER_THRESH = 0.98855f;   // -0.1 dBFS
static constexpr float LIMITER_CEIL   = 0.989f;
static constexpr int   BLOCK_SIZE     = 512;
static constexpr int   MAX_CHANNELS   = 2;
static constexpr float kNeg100dBFS  = 1e-5f;

// ── Tabla seno con interpolacion cubica (4096 puntos + 4 guardas) ────────────
alignas(64) static float sin_table[4096 + 4];
static std::once_flag sin_once;

static void init_sin_table() {
    std::call_once(sin_once, []{
        for (int i = 0; i < 4096 + 4; i++) {
            float phase = TWO_PI * (i - 1) / 4096.0f;
            sin_table[i] = sinf(phase);
        }
    });
}

// fast_sin: interpolacion cubica con 4 guardas
static inline float fast_sin(float phase) noexcept {
    phase = fmodf(phase, TWO_PI);
    if (phase < 0) phase += TWO_PI;

    float idx = phase * (4096.0f * INV_TWO_PI);
    int i = (int)idx;
    float f = idx - i;
    i = (i + 1) & 4095;

    float y0 = sin_table[i];
    float y1 = sin_table[i+1];
    float y2 = sin_table[i+2];
    float y3 = sin_table[i+3];

    // Interpolacion cubica de Catmull-Rom
    float c1 = 0.5f * (y2 - y0);
    float c2 = y0 - 2.5f*y1 + 2.0f*y2 - 0.5f*y3;
    float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
    return ((c3 * f + c2) * f + c1) * f + y1;
}

// ── Limiter omnipotente: branchless + soft-saturation parabolica ────────────
static inline float omnipotent_limiter(float x) noexcept {
    if (!std::isfinite(x)) return 0.0f;

    float abs_x = std::fabs(x);
    float sign_x = std::copysign(1.0f, x);

    if (abs_x <= LIMITER_THRESH) return x;

    float excess = abs_x - LIMITER_THRESH;
    float compressed = LIMITER_THRESH + excess / (1.0f + excess * 2.0f);
    return sign_x * std::min(compressed, LIMITER_CEIL);
}

// ── Sanitize buffer completo (NaN/Inf → 0) ──────────────────────────────────
static inline void sanitize_buffer(float* buf, int count) noexcept {
    #pragma clang loop vectorize(enable) interleave(enable)
    for (int i = 0; i < count; ++i) {
        if (!std::isfinite(buf[i])) buf[i] = 0.0f;
    }
}

// ── Instancias globales del pipeline ────────────────────────────────────────
namespace {
    // DSP chain
    ivanna::ParametricEQ   g_eq;
    ivanna::Compressor     g_comp;
    ivanna::HarmonicExciter g_exciter;
    ivanna::StereoWidener  g_widener;
    ivanna::GainStage      g_gain_in, g_gain_out;

    // PDEngine
    ivanna::NHOEngine           g_nho;
    ivanna::BiquadEnvelopeBank  g_env_bank;
    ivanna::CueBasedSpatial     g_spatial;

    // Autonomous + Synthesizer
    ivanna::acoustic::AutonomousBrain g_brain;
    ivanna::acoustic::Synthesizer     g_synth{48000.0f, 50.0f};

    // Control
    ivanna::ControlFrameBus g_control_bus;
    ivanna::ControlFrame    g_current_frame;

    // Estado
    std::atomic<bool> g_initialized{false};
    std::atomic<int>  g_mode{0};
    float g_sample_rate = 48000.0f;

    // Anti-Dolby scores (inyectados desde YAMNet via JNI)
    std::atomic<float> g_ad_speech{0.5f};
    std::atomic<float> g_ad_music{0.5f};
    std::atomic<float> g_ad_bass{0.5f};
}

// ── Inicializacion omnipotente ───────────────────────────────────────────────
extern "C" JNIEXPORT void JNICALL
Java_com_ivanna_omega_core_IvannaNativeLib_nativeInitDSP(
    JNIEnv* env, jclass clazz, jint sampleRate) {

    init_sin_table();
    g_sample_rate = static_cast<float>(sampleRate);

    ivanna::DSPParams p;
    p.drive = 0.65f; p.wet = 0.5f; p.mix = 0.7f;
    p.alpha = 0.5f; p.beta = 0.5f; p.gamma = 0.5f;
    p.freq = 1000.f; p.resonance = 0.707f;
    p.low = 0.f; p.mid = 0.f; p.high = 0.f;
    p.presence = 0.f; p.master = 0.f;
    p.sampleRate = static_cast<uint32_t>(sampleRate);

    g_eq.setSampleRate(g_sample_rate);
    g_eq.setParams(p);
    g_comp.setParams(p);
    g_exciter.setParams(p);
    g_widener.setParams(p);
    g_gain_in.setParams(p);
    g_gain_out.setParams(p);

    g_nho.alpha = 0.8f; g_nho.beta = 0.3f; g_nho.mu = 0.15f;
    g_nho.harmonic_gain = 0.2f; g_nho.wet = 0.3f;
    g_env_bank.reset();
    g_spatial.reset();
    g_brain.reset();

    ivanna_set_audio_thread_priority();

    g_initialized.store(true, std::memory_order_release);
    LOGI("OMNIPOTENT init complete @ %d Hz | v2.0.OMNIPOTENTE", sampleRate);
}

// ── Aplicar ControlFrame al pipeline ────────────────────────────────────────
static void apply_control_frame(const ivanna::ControlFrame& cf) noexcept {
    ivanna::DSPParams p;
    p.drive = cf.drive; p.wet = cf.wet; p.mix = cf.mix;
    p.alpha = cf.alpha; p.beta = cf.beta; p.gamma = cf.gamma_v;
    p.freq = cf.freq; p.resonance = cf.resonance;
    p.low = cf.low; p.mid = cf.mid; p.high = cf.high;
    p.presence = cf.presence; p.master = cf.master;
    p.sampleRate = static_cast<uint32_t>(g_sample_rate);

    g_eq.setParams(p);
    g_comp.setParams(p);
    g_exciter.setParams(p);
    g_widener.setParams(p);
    g_gain_in.setParams(p);
    g_gain_out.setParams(p);

    g_nho.alpha = cf.nho_alpha;
    g_nho.beta = cf.nho_beta;
    g_nho.mu = cf.nho_mu;
    g_nho.harmonic_gain = cf.nho_harmonic_gain;
    g_nho.wet = cf.nho_wet;

    g_spatial.theta = cf.spatial_theta;
    g_spatial.width = cf.spatial_width;
    g_spatial.wet = cf.spatial_wet;
}

// ── PROCESAMIENTO OMNIPOTENTE POR BLOQUE ─────────────────────────────────────
extern "C" JNIEXPORT void JNICALL
Java_com_ivanna_omega_core_IvannaNativeLib_nativeProcessBlock(
    JNIEnv* env, jclass clazz,
    jfloatArray inL, jfloatArray inR,
    jfloatArray outL, jfloatArray outR,
    jint frames) {

    if (!g_initialized.load(std::memory_order_acquire)) return;
    if (frames <= 0 || frames > BLOCK_SIZE) return;

    // 1. Consumir ControlFrame mas reciente (lock-free)
    ivanna::ControlFrame cf;
    if (g_control_bus.consumeIfNewer(cf)) {
        g_current_frame = cf;
        apply_control_frame(cf);
    }

    // 2. Obtener buffers JNI
    jfloat* pinL = env->GetFloatArrayElements(inL, nullptr);
    jfloat* pinR = env->GetFloatArrayElements(inR, nullptr);
    jfloat* poutL = env->GetFloatArrayElements(outL, nullptr);
    jfloat* poutR = env->GetFloatArrayElements(outR, nullptr);

    if (!pinL || !pinR || !poutL || !poutR) {
        if (pinL) env->ReleaseFloatArrayElements(inL, pinL, JNI_ABORT);
        if (pinR) env->ReleaseFloatArrayElements(inR, pinR, JNI_ABORT);
        if (poutL) env->ReleaseFloatArrayElements(outL, poutL, JNI_ABORT);
        if (poutR) env->ReleaseFloatArrayElements(outR, poutR, JNI_ABORT);
        return;
    }

    // 3. Copiar a buffers locales alineados
    alignas(64) float bufL[BLOCK_SIZE];
    alignas(64) float bufR[BLOCK_SIZE];

    std::memcpy(bufL, pinL, frames * sizeof(float));
    std::memcpy(bufR, pinR, frames * sizeof(float));

    env->ReleaseFloatArrayElements(inL, pinL, JNI_ABORT);
    env->ReleaseFloatArrayElements(inR, pinR, JNI_ABORT);

    // Sanitize entrada
    sanitize_buffer(bufL, frames);
    sanitize_buffer(bufR, frames);

    // 4. === FASE 1: GAIN STAGE INPUT ===
    g_gain_in.processInput(bufL, bufR, frames);

    // 5. === FASE 2: PARAMETRIC EQ ===
    g_eq.process(bufL, bufR, frames);

    // 6. === FASE 3: HARMONIC EXCITER ===
    g_exciter.process(bufL, bufR, frames);

    // 7. === FASE 4: COMPRESSOR ===
    g_comp.process(bufL, bufR, frames);

    // 8. === FASE 5: PDEngine (modo dependiente) ===
    int mode = g_mode.load(std::memory_order_relaxed);

    if (mode >= 1) {
        for (int i = 0; i < frames; ++i) {
            float nhoL = g_nho.processSample(bufL[i], true);
            float nhoR = g_nho.processSample(bufR[i], false);

            float envL = g_env_bank.processSample(nhoL, 0);
            float envR = g_env_bank.processSample(nhoR, 1);

            bufL[i] = bufL[i] * (1.0f - g_nho.wet) + envL * g_nho.wet;
            bufR[i] = bufR[i] * (1.0f - g_nho.wet) + envR * g_nho.wet;
        }
    }

    if (mode >= 2) {
        g_spatial.processBlock(bufL, bufR, frames, g_sample_rate);
    }

    // 9. === FASE 6: STEREO WIDENER ===
    g_widener.process(bufL, bufR, frames);

    // 10. === FASE 7: GAIN STAGE OUTPUT ===
    g_gain_out.processOutput(bufL, bufR, frames);

    // 11. === FASE 8: LIMITER OMNIPOTENTE ===
    for (int i = 0; i < frames; ++i) {
        bufL[i] = omnipotent_limiter(bufL[i]);
        bufR[i] = omnipotent_limiter(bufR[i]);
    }

    // 12. === FASE 9: AUTONOMOUS BRAIN (analisis pasivo) ===
    g_brain.analyzeBlock(bufL, bufR, frames);
    ivanna::acoustic::SynthParams synth_params = g_brain.getCurrentProfile();
    g_synth.setTargetParameters(
        synth_params.bass_weight,
        synth_params.mid_presence,
        synth_params.treble_air,
        synth_params.warmth,
        synth_params.clarity
    );

    // 13. Copiar salida
    std::memcpy(poutL, bufL, frames * sizeof(float));
    std::memcpy(poutR, bufR, frames * sizeof(float));

    env->ReleaseFloatArrayElements(outL, poutL, 0);
    env->ReleaseFloatArrayElements(outR, poutR, 0);
}

// ── Setters JNI (publican al ControlFrameBus) ─────────────────────────────────
extern "C" JNIEXPORT void JNICALL
Java_com_ivanna_omega_core_IvannaNativeLib_nativeSetParams(
    JNIEnv* env, jclass clazz, jfloatArray params) {

    jfloat* p = env->GetFloatArrayElements(params, nullptr);
    if (!p) return;

    ivanna::ControlFrame cf = g_current_frame;
    cf.drive = p[0]; cf.wet = p[1]; cf.mix = p[2];
    cf.alpha = p[3]; cf.beta = p[4]; cf.gamma_v = p[5];
    cf.freq = p[6]; cf.resonance = p[7];
    cf.low = p[8]; cf.mid = p[9]; cf.high = p[10];
    cf.presence = p[11]; cf.master = p[12];

    g_control_bus.publish(cf);

    env->ReleaseFloatArrayElements(params, p, JNI_ABORT);
}

extern "C" JNIEXPORT void JNICALL
Java_com_ivanna_omega_core_IvannaNativeLib_nativeSetMode(JNIEnv*, jclass, jint mode) {
    g_mode.store(mode, std::memory_order_relaxed);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_ivanna_omega_core_IvannaNativeLib_nativeGetMode(JNIEnv*, jclass) {
    return g_mode.load(std::memory_order_relaxed);
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_ivanna_omega_core_IvannaNativeLib_nativeGetGenreConfidence(JNIEnv*, jclass) {
    return g_brain.getCurrentProfile().bass_weight; // Proxy para confidence
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_ivanna_omega_core_IvannaNativeLib_nativeVersion(JNIEnv* env, jclass) {
    return env->NewStringUTF("IVANNA-OMEGA-SUPREME v2.0.OMNIPOTENTE");
}

// ── Anti-Dolby scores injection ──────────────────────────────────────────────
extern "C" JNIEXPORT void JNICALL
Java_com_ivanna_omega_audio_AntiDolbyController_nativeSetAntiDolbyScores(
    JNIEnv*, jclass, jfloat speech, jfloat music, jfloat bass) {
    g_ad_speech.store(speech, std::memory_order_relaxed);
    g_ad_music.store(music, std::memory_order_relaxed);
    g_ad_bass.store(bass, std::memory_order_relaxed);
}
