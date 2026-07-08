# Changelog

## v2.0.OMNIPOTENTE (2026-07-07)

### Arquitectura
- ControlFrameBus seqlock SPSC: zero mutex en audio thread
- Determinismo por bloque garantizado
- Pipeline unificado: DSP → PDEngine → Spatial → Limiter

### DSP
- Limiter omnipotente: branchless + soft-saturation parabolica
- Sanitize NaN/Inf en cada sample
- Compressor con fastLog2/fastExp2 verificados (<0.001dB error)
- ParametricEQ 8 bandas con mapeo semantico

### PDEngine
- NHO Engine: tanh(αx+βz) con μ-gating
- BiquadEnvelopeBank: 13 gammatone bands
- CueBasedSpatial: ITD + ILD perceptual real, O(1)

### Inteligencia
- AutonomousBrain v2.0: deteccion BPM, silencio, genero mejorada
- Synthesizer: one-pole smoother 50ms, zipper-free
- EvolutionaryKernel: 128 poblacion, audio-coupling 0.4
- PhaseOracle: prediccion de coeficientes biquad

### Anti-Dolby
- Perfiles dinamicos basados en YAMNet scores
- Intercepta audio global via AudioEffect API

### Build
- CMakeLists.txt sin `-ffast-math` (estabilidad numerica)
- GitHub Actions: APK + Magisk + Benchmarks + Release automatico
- Splits ABI para APKs mas pequenos

### Seguridad
- Firmado con apksigner en CI/CD
- Checksums SHA-256 para todos los artefactos
- Sepolicy rules para Magisk

## v1.7 (previo)
- Cableado UI completo
- Presets reales conectados
- SpectralClassifier arrancado
- Modo Auto IA

## v1.5 (previo)
- Limiter hard-clip -0.1dBFS
- Validacion NaN/Inf
- Null checks JNI
- Fix phase wrap-around
