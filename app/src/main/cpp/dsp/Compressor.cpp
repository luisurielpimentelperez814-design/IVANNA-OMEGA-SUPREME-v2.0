#include "../include/Compressor.h"
#include <cmath>
#include <algorithm>

namespace ivanna {

// Aproximaciones bit-trick con correccion polinomica
// Error < 0.001 dB en log, < 0.006% relativo en exp
static inline float fastLog2(float x) noexcept {
    union { float f; uint32_t i; } vx{x};
    union { uint32_t i; float f; } mx{(vx.i & 0x007FFFFFu) | 0x3f000000u};
    float y = (float)vx.i * 1.1920928955078125e-7f;
    return y - 124.22551499f - 1.498030302f * mx.f - 1.72587999f / (0.3520887068f + mx.f);
}

static inline float fastExp2(float p) noexcept {
    const float clipped = p < -126.f ? -126.f : p;
    const float w = clipped - (float)(int)clipped + (clipped < 0.f ? 1.f : 0.f);
    union { int32_t i; float f; } v;
    v.i = (int32_t)((1 << 23) * (clipped + 121.2740575f + 27.7280233f / (4.84252568f - w) - 1.49012907f * w));
    return v.f;
}

Compressor::Compressor() { reset(); }

void Compressor::reset() {
    env_ = 0.f;
    threshold_ = -18.f;
    ratio_ = 4.0f;
    attackCoef_ = 0.99f;
    releaseCoef_ = 0.999f;
    makeupGain_ = 1.0f;
    inv_atk_ = 0.01f;
    inv_rel_ = 0.001f;
    slope_ = 0.75f;
    sr_ = 48000.f;
}

void Compressor::setParams(const DSPParams& p) {
    sr_ = static_cast<float>(p.sampleRate);
    threshold_ = -24.0f + p.alpha * 24.0f;
    ratio_ = 1.0f + p.beta * 19.0f;
    float atMs = 5.0f + (1.0f - p.gamma) * 95.0f;
    float relMs = 50.0f + (1.0f - p.gamma) * 450.0f;
    attackCoef_ = std::exp(-1.0f / (sr_ * atMs * 0.001f));
    releaseCoef_ = std::exp(-1.0f / (sr_ * relMs * 0.001f));
    float reduction = threshold_ * (1.0f - 1.0f / ratio_);
    makeupGain_ = std::pow(10.0f, (-reduction * 0.5f) / 20.0f);
    inv_atk_ = 1.0f - attackCoef_;
    inv_rel_ = 1.0f - releaseCoef_;
    slope_ = 1.0f - 1.0f / ratio_;
}

void Compressor::setThreshold(float db) { threshold_ = db; }
void Compressor::setRatio(float ratio) { ratio_ = ratio; slope_ = 1.0f - 1.0f / ratio_; }

void Compressor::setAttack(float ms) {
    attackCoef_ = std::exp(-1.0f / (sr_ * ms * 0.001f));
    inv_atk_ = 1.0f - attackCoef_;
}

void Compressor::setRelease(float ms) {
    releaseCoef_ = std::exp(-1.0f / (sr_ * ms * 0.001f));
    inv_rel_ = 1.0f - releaseCoef_;
}

__attribute__((hot, flatten))
void Compressor::process(float* __restrict__ left, float* __restrict__ right, int frames) {
    if (frames <= 0) return;

    constexpr float k20DivLog2_10 = 6.0205999133f;
    constexpr float kLog2_10Div20 = 0.1660964047f;
    constexpr float kNeg120dB = 1e-6f;

    const float atk = attackCoef_, rel = releaseCoef_;
    const float inv_atk = inv_atk_, inv_rel = inv_rel_;
    const float slope = slope_, thresh = threshold_;
    const float makeup = makeupGain_;
    float env = env_;

    #pragma clang loop vectorize(enable) interleave(enable)
    for (int i = 0; i < frames; ++i) {
        float peak = std::max(std::fabs(left[i]), std::fabs(right[i]));
        peak = std::max(peak, kNeg120dB);

        float peak_dB = fastLog2(peak) * k20DivLog2_10;
        float over_dB = peak_dB - thresh;

        float targetEnv = (over_dB > 0.f) ? (over_dB * slope) : 0.f;

        float coef = (targetEnv > env) ? inv_atk : inv_rel;
        env = env + coef * (targetEnv - env);

        float gain_dB = -env;
        float gain = fastExp2(gain_dB * kLog2_10Div20) * makeup;

        left[i]  *= gain;
        right[i] *= gain;
    }

    env_ = env;
}

} // namespace ivanna
