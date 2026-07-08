// © 2026 Luis Uriel Pimentel Pérez — GORE TNS. All rights reserved.
#pragma once

/*
 * ============================================================
 * IVANNA OMEGA SUPREME v2.0 — ControlFrame
 *
 * Snapshot inmutable de todos los parametros controlables.
 * Seqlock SPSC: un escritor (JNI/UI), un lector (audio thread).
 * Determinismo por bloque garantizado.
 * ============================================================
 */

#include <atomic>
#include <cstdint>
#include "include/dsp_types.h"

namespace ivanna {

struct alignas(64) ControlFrame {
    uint64_t seq = 0;

    // DSP chain
    float drive = 0.65f, wet = 0.50f, mix = 0.70f;
    float alpha = 0.50f, beta = 0.50f, gamma_v = 0.50f;
    float freq = 1000.f, resonance = 0.707f;
    float low = 0.f, mid = 0.f, high = 0.f;
    float presence = 0.f, master = 0.f;

    // PDEngine
    int   mode = 0;
    float nho_alpha = 0.8f, nho_beta = 0.3f, nho_mu = 0.15f;
    float nho_harmonic_gain = 0.2f, nho_wet = 0.3f;

    // Spatial
    float spatial_theta = 0.f, spatial_width = 1.f, spatial_wet = 0.5f;

    // AI scores (from YAMNet)
    float score_speech = 0.f, score_music = 0.f, score_bass = 0.f;

    // Evolutionary genome (8 floats)
    float evo_genome[8] = {};
};

class ControlFrameBus {
public:
    void publish(const ControlFrame& frame) noexcept {
        uint64_t next_seq = frame.seq;
        if (next_seq == 0) {
            next_seq = seq_.load(std::memory_order_relaxed) + 1;
        }

        // Escribir datos (copia completa)
        data_ = frame;
        data_.seq = next_seq;

        // Publicar secuencia (release barrier)
        seq_.store(next_seq, std::memory_order_release);
    }

    bool consumeIfNewer(ControlFrame& out) noexcept {
        uint64_t seq = seq_.load(std::memory_order_acquire);
        if (seq == last_read_seq_) return false;

        out = data_;
        last_read_seq_ = seq;
        return true;
    }

private:
    alignas(64) std::atomic<uint64_t> seq_{0};
    alignas(64) ControlFrame data_;
    uint64_t last_read_seq_ = 0;
};

} // namespace ivanna
