package com.ivanna.omega.dsp

import android.util.Log
import com.ivanna.omega.core.IvannaNativeLib

/**
 * IVANNA-OMEGA-SUPREME v2.0 — DSP Bridge
 * Wraps libivanna_omega.so, providing the full DSP chain:
 *   GainStage(in) → ParametricEQ → HarmonicExciter → Compressor → StereoWidener → GainStage(out)
 * © 2026 Luis Uriel Pimentel Pérez — GORE TNS. All rights reserved.
 */
object DSPBridge {

    private const val TAG = "IVANNA_OMEGA_DSP"
    private var loaded = false
    private var sampleRate = 48000

    init {
        try {
            System.loadLibrary("ivanna_omega")
            loaded = true
            Log.i(TAG, "libivanna_omega loaded — ${nativeVersion()}")
        } catch (e: UnsatisfiedLinkError) {
            Log.e(TAG, "Native lib unavailable: ${e.message}")
        }
    }

    val isLoaded: Boolean get() = loaded

    fun init(sr: Int = 48000) {
        sampleRate = sr
        if (loaded) {
            nativeInit(sr)
            IvannaNativeLib.nativeInitDSP(sr)
        }
    }

    fun setParams(
        drive: Float, wet: Float, mix: Float,
        alpha: Float, beta: Float, gamma: Float,
        freq: Float, resonance: Float,
        low: Float, mid: Float, high: Float,
        presence: Float, master: Float
    ) {
        if (!loaded) return
        val params = floatArrayOf(
            drive, wet, mix, alpha, beta, gamma,
            freq, resonance, low, mid, high, presence, master
        )
        IvannaNativeLib.nativeSetParams(params)
    }

    fun process(buffer: FloatArray, numFrames: Int) {
        if (!loaded || numFrames <= 0) return
        // Procesamiento intercalado estereo → planar → intercalado
        val inL = FloatArray(numFrames)
        val inR = FloatArray(numFrames)
        val outL = FloatArray(numFrames)
        val outR = FloatArray(numFrames)

        for (i in 0 until numFrames) {
            inL[i] = buffer[i * 2]
            inR[i] = buffer[i * 2 + 1]
        }

        IvannaNativeLib.nativeProcessBlock(inL, inR, outL, outR, numFrames)

        for (i in 0 until numFrames) {
            buffer[i * 2] = outL[i]
            buffer[i * 2 + 1] = outR[i]
        }
    }

    fun reset() { if (loaded) nativeReset() }

    fun version(): String = if (loaded) nativeVersion() else "native unavailable"

    // Native methods (fallback directo si IvannaNativeLib falla)
    private external fun nativeInit(sampleRate: Int)
    private external fun nativeProcess(buf: FloatArray, numFrames: Int)
    private external fun nativeReset()
    private external fun nativeVersion(): String
}
