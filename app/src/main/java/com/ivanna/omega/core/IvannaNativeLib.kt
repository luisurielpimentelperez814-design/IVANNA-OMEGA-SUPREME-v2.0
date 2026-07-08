package com.ivanna.omega.core

import android.util.Log

/**
 * IvannaNativeLib v2.0 — JNI bindings for IVANNA OMEGA SUPREME.
 * Compiled into libivanna_omega.so — unified entry point.
 * © 2026 Luis Uriel Pimentel Pérez — GORE TNS. All rights reserved.
 */
object IvannaNativeLib {
    private const val TAG = "IvannaNativeLib"

    init {
        try {
            System.loadLibrary("ivanna_omega")
            Log.i(TAG, "Native library loaded successfully — v2.0 OMNIPOTENTE")
        } catch (e: UnsatisfiedLinkError) {
            Log.e(TAG, "Failed to load native library", e)
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    //  DSP Core
    // ═══════════════════════════════════════════════════════════════════════
    external fun nativeInitDSP(sampleRate: Int): Boolean
    external fun nativeProcessBlock(
        inL: FloatArray, inR: FloatArray,
        outL: FloatArray, outR: FloatArray,
        frames: Int
    )
    external fun nativeSetParams(params: FloatArray)
    external fun nativeResetDSP()

    // ═══════════════════════════════════════════════════════════════════════
    //  Mode Control (0=DSP, 1=DSP+NHO, 2=DSP+NHO+Spatial)
    // ═══════════════════════════════════════════════════════════════════════
    external fun nativeSetMode(mode: Int)
    external fun nativeGetMode(): Int

    // ═══════════════════════════════════════════════════════════════════════
    //  PDEngine: NHO + BiquadEnvelopeBank + CueBasedSpatial
    // ═══════════════════════════════════════════════════════════════════════
    external fun nativeSetAlpha(v: Float)
    external fun nativeSetBeta(v: Float)
    external fun nativeSetGamma(v: Float)
    external fun nativeSetDelta(v: Float)
    external fun nativeSetEta(v: Float)
    external fun nativeSetHarmonicGain(v: Float)
    external fun nativeSetHRTFEnabled(en: Boolean)
    external fun nativeSetAdaptEnabled(en: Boolean)
    external fun nativeSetNPMax(v: Float)
    external fun nativeSetReflectionGain(i: Int, g: Float)
    external fun nativeSetReflectionDelay(i: Int, d: Float)

    // ═══════════════════════════════════════════════════════════════════════
    //  Evolutionary Kernel
    // ═══════════════════════════════════════════════════════════════════════
    external fun nativeInitializeEvolution(populationSize: Int, generations: Int): Boolean
    external fun nativeGetBestFitness(): Double
    external fun nativeGetGeneration(): Int
    external fun nativeEvolveStep(): Boolean
    external fun nativeSetMutationRate(rate: Float)
    external fun nativeGetMutationRate(): Float

    // ═══════════════════════════════════════════════════════════════════════
    //  Phase Oracle
    // ═══════════════════════════════════════════════════════════════════════
    external fun nativePredictSamples(audioBuffer: FloatArray, sampleCount: Int): FloatArray
    external fun nativeGetPhaseState(): Float
    external fun nativeSetPhaseParameters(alpha: Float, beta: Float, gamma: Float): Boolean

    // ═══════════════════════════════════════════════════════════════════════
    //  Spatial Engine
    // ═══════════════════════════════════════════════════════════════════════
    external fun nativeSetSpatialTheta(theta: Float)
    external fun nativeSetSpatialWidth(width: Float)
    external fun nativeSetSpatialWet(wet: Float)

    // Spatial Engine v2 (JNI real en spatial/spatial_jni.cpp)
    external fun nativeInitSpatialEngine(sampleRate: Int, bufferSize: Int): Boolean
    external fun nativeRenderSpatialBlock(
        input: FloatArray, outL: FloatArray, outR: FloatArray,
        posX: Int, posY: Int, posZ: Int, mu: Int
    ): Int
    external fun nativeReleaseSpatialEngine(): Boolean
    external fun nativeGetSpatialState(): String
    external fun nativeSetSpatialParams(paramsJson: String): Boolean

    // ═══════════════════════════════════════════════════════════════════════
    //  Autonomous Brain
    // ═══════════════════════════════════════════════════════════════════════
    external fun nativeGetGenreConfidence(): Float

    // ═══════════════════════════════════════════════════════════════════════
    //  Anti-Dolby
    // ═══════════════════════════════════════════════════════════════════════
    external fun nativeSetAntiDolbyScores(speech: Float, music: Float, bass: Float)

    // ═══════════════════════════════════════════════════════════════════════
    //  Info
    // ═══════════════════════════════════════════════════════════════════════
    external fun nativeVersion(): String
}
