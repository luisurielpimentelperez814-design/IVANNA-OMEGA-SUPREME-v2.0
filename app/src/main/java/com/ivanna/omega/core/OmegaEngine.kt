package com.ivanna.omega.core

import android.content.Context
import android.media.AudioManager
import android.util.Log
import com.ivanna.omega.dsp.DSPBridge

/**
 * IVANNA-OMEGA-SUPREME v2.0 — Central engine facade.
 * © 2026 Luis Uriel Pimentel Pérez — GORE TNS. All rights reserved.
 *
 * Processing modes:
 *   0 = DSP only (FUSION-PRO chain: EQ+Comp+Exciter+Widener)
 *   1 = DSP + NHO (Nonlinear Harmonic Oscillator + BiquadEnvelopeBank)
 *   2 = DSP + NHO + Spatial (Cue-Based ITD+ILD)
 */
object OmegaEngine {
    private const val TAG = "IVANNA_OMEGA"
    private var loaded = false

    init {
        try {
            System.loadLibrary("ivanna_omega")
            loaded = true
        } catch (e: UnsatisfiedLinkError) {
            Log.e(TAG, "Native lib unavailable for OmegaEngine: ${e.message}")
        }
    }

    fun init(context: Context) {
        val am = context.getSystemService(Context.AUDIO_SERVICE) as AudioManager
        val sr = am.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE)?.toIntOrNull() ?: 48000
        DSPBridge.init(sr)
        IvannaNativeLib.nativeInitDSP(sr)
        Log.i(TAG, "OmegaEngine initialized @ ${sr}Hz | v2.0 OMNIPOTENTE")
    }

    /** 0 = DSP, 1 = DSP+NHO, 2 = DSP+NHO+Spatial */
    fun setMode(mode: Int) {
        if (!loaded) return
        if (mode in 0..2) {
            try {
                IvannaNativeLib.nativeSetMode(mode)
                Log.i(TAG, "Mode set to: $mode")
            } catch (e: UnsatisfiedLinkError) {
                Log.e(TAG, "nativeSetMode unavailable", e)
            }
        }
    }

    fun getMode(): Int = if (!loaded) 0 else try {
        IvannaNativeLib.nativeGetMode()
    } catch (e: UnsatisfiedLinkError) {
        Log.e(TAG, "nativeGetMode unavailable", e)
        0
    }

    fun getVersion(): String = IvannaNativeLib.nativeVersion()
}
