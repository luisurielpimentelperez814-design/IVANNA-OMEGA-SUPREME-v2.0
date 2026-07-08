#include "../include/ParametricEQ.h"
#include <cmath>

#if defined(__ARM_NEON) && defined(__aarch64__)
#include <arm_neon.h>
#define IVANNA_EQ_NEON 1
#else
#define IVANNA_EQ_NEON 0
#endif

namespace ivanna {

ParametricEQ::ParametricEQ() noexcept { reset(); }

void ParametricEQ::reset() noexcept { 
    for(int i=0; i<NUM_BANDS; ++i){ 
        bandsL[i].reset(); 
        bandsR[i].reset(); 
    } 
}

void ParametricEQ::setSampleRate(float sr) noexcept { 
    sampleRate_ = sr; 
}

void ParametricEQ::setBand(int b, float f, float q, float g) noexcept {
    if(b < 0 || b >= NUM_BANDS) return;

    float A = powf(10.0f, g / 40.0f);
    float w0 = 2.0f * float(M_PI) * f / sampleRate_;
    float c = cosf(w0), s = sinf(w0);
    float alpha = s / (2.0f * q);

    float b0 = 1.0f + alpha * A;
    float b1 = -2.0f * c;
    float b2 = 1.0f - alpha * A;
    float a0 = 1.0f + alpha / A;
    float a1 = -2.0f * c;
    float a2 = 1.0f - alpha / A;

    float inv_a0 = 1.0f / a0;

    BiquadCoeff coeff;
    coeff.b0 = b0 * inv_a0;
    coeff.b1 = b1 * inv_a0;
    coeff.b2 = b2 * inv_a0;
    coeff.a1 = a1 * inv_a0;
    coeff.a2 = a2 * inv_a0;

    bandsL[b].setCoeff(coeff);
    bandsR[b].setCoeff(coeff);
}

void ParametricEQ::setParams(const DSPParams& p) noexcept {
    auto clampDb = [](float db) { 
        return db < -18.f ? -18.f : (db > 18.f ? 18.f : db); 
    };

    // Band 0: Low shelf ~80 Hz
    setBand(0, 80.f,   0.707f, clampDb(p.low));
    // Band 1: Peaking ~200 Hz
    setBand(1, 200.f,  p.resonance, clampDb(p.low * 0.5f));
    // Band 2: Peaking ~500 Hz
    setBand(2, 500.f,  p.resonance, 0.f);
    // Band 3: Peaking ~1000 Hz (freq param)
    setBand(3, p.freq, p.resonance, clampDb(p.mid));
    // Band 4: Peaking ~2500 Hz
    setBand(4, 2500.f, p.resonance, clampDb(p.mid * 0.7f));
    // Band 5: Peaking ~6000 Hz (presence)
    setBand(5, 6000.f, p.resonance, clampDb(p.presence));
    // Band 6: High shelf ~12000 Hz
    setBand(6, 12000.f, 0.707f, clampDb(p.high));
    // Band 7: High shelf ~16000 Hz (air)
    setBand(7, 16000.f, 0.5f, clampDb(p.high * 0.5f));
}

__attribute__((hot, flatten))
void ParametricEQ::process(float* __restrict__ left, float* __restrict__ right, int frames) {
    if (frames <= 0) return;

    for (int b = 0; b < NUM_BANDS; ++b) {
        auto& bl = bandsL[b];
        auto& br = bandsR[b];

        #pragma clang loop vectorize(enable) interleave(enable)
        for (int i = 0; i < frames; ++i) {
            left[i]  = bl.process(left[i]);
            right[i] = br.process(right[i]);
        }
    }
}

} // namespace ivanna
