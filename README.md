# IVANNA OMEGA SUPREME v2.0 OMNIPOTENTE

**Procesador de audio holografico en tiempo real para Android con nucleo nativo consolidado.**

© 2025–2026 Luis Uriel Pimentel Pérez (GORE TNS). Todos los derechos reservados.

[![Build Status](https://github.com/GORE-TNS/IVANNA-OMEGA-SUPREME/actions/workflows/build.yml/badge.svg)](https://github.com/GORE-TNS/IVANNA-OMEGA-SUPREME/actions)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-v2.0.OMNIPOTENTE-orange)](https://github.com/GORE-TNS/IVANNA-OMEGA-SUPREME/releases)

---

## 🎯 Que es IVANNA OMEGA SUPREME

IVANNA procesa audio con un pipeline DSP en C++17 optimizado para Android, pensado para ejecucion en tiempo real y visualizacion reactiva del contenido capturado. La version 2.0 OMNIPOTENTE eleva la arquitectura a su forma definitiva con:

- **ControlFrameBus seqlock SPSC**: zero mutex en audio thread, determinismo por bloque
- **AutonomousBrain v2.0**: deteccion de BPM, silencio, y heuristica de genero mejorada
- **Limiter omnipotente**: branchless + soft-saturation parabolica
- **Anti-Dolby dinamico**: perfiles adaptativos basados en clasificacion YAMNet
- **Estabilidad numerica garantizada**: compilado SIN `-ffast-math`

---

## 🏗️ Arquitectura v2.0

```
┌─────────────────────────────────────────────────────────────┐
│                    Android Framework                         │
│  MainActivity (Compose) → ControlFrameBus → Audio Thread   │
└─────────────────────────────────────────────────────────────┘
                              │
┌─────────────────────────────────────────────────────────────┐
│                    OMEGA PIPELINE v2.0                       │
│  GainStage(in) → ParametricEQ → HarmonicExciter → Comp →  │
│  [NHO + EnvelopeBank] → [CueBasedSpatial] → Widener →    │
│  GainStage(out) → Limiter Omnipotente                      │
│                                                             │
│  AutonomousBrain (analisis pasivo) → Synthesizer (smoother)│
│  EvolutionaryKernel (offline) → genome → params           │
│  PhaseOracle → prediccion de coeficientes biquad          │
└─────────────────────────────────────────────────────────────┘
```

---

## 📦 Bloques DSP

| Modulo | Descripcion | Estado |
|--------|-------------|--------|
| ParametricEQ | 8 bandas, biquad cascada, NEON-opt | ✅ Activo |
| Compressor | fastLog2/fastExp2, envelope follower | ✅ Activo |
| HarmonicExciter | Padé tanh [3/2], HPF 5kHz | ✅ Activo |
| StereoWidener | M/S matrix, width ≤ 1.5 | ✅ Activo |
| NHO Engine | Nonlinear Harmonic Oscillator | ✅ Modo 1+ |
| BiquadEnvelopeBank | 13 gammatone bands | ✅ Modo 1+ |
| CueBasedSpatial | ITD + ILD perceptual real | ✅ Modo 2 |
| AutonomousBrain | Zero FFT, zero heap, genero/BPM | ✅ Siempre |
| Visualizer | Gammatone13 + NEON 4-band | ✅ Activo |

---

## 🚀 Modos de Procesamiento

- **Modo 0 (DSP)**: EQ + Comp + Exciter + Widener — latencia minima
- **Modo 1 (DSP + NHO)**: Añade Nonlinear Harmonic Oscillator + EnvelopeBank
- **Modo 2 (DSP + NHO + Spatial)**: Añade Cue-Based ITD/ILD — experiencia espacial

---

## 📲 Instalacion

### APK (sin root)
1. Descarga el APK firmado desde [Releases](https://github.com/GORE-TNS/IVANNA-OMEGA-SUPREME/releases)
2. Instala `ivanna-omega-supreme-v2.0-arm64-v8a-signed.apk`
3. Concede permiso de captura de audio
4. Selecciona modo de procesamiento y preset

### Magisk (con root)
1. Descarga `IVANNA-OMEGA-SUPREME-v2.0-OMNIPOTENTE.zip` desde Releases
2. Instala via Magisk Manager → Modules → Install from storage
3. Reinicia el dispositivo
4. El motor se activa automaticamente en todas las apps de audio

---

## 🛠️ Build desde fuente

### Requisitos
- Android Studio Hedgehog+ o IntelliJ IDEA
- Android SDK 34
- NDK 26.1.10909125
- CMake 3.22.1
- JDK 17

### Compilar APK
```bash
./gradlew assembleRelease
```

### Compilar modulo Magisk
```bash
# El workflow de GitHub Actions lo hace automaticamente
# Para build manual:
cmake -S app/src/main/cpp -B build   -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake   -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-29
cmake --build build
```

### Tests
```bash
# Host-side (sin dispositivo)
cmake -S app/src/main/cpp/tests -B build-tests
cmake --build build-tests
./build-tests/benchmark_suite
```

---

## 📊 Benchmarks

| Metrica | Valor | Condicion |
|---------|-------|-----------|
| Latencia end-to-end | 8.3 ms | 512 frames @ 48kHz, Moto G85 |
| CPU DSP chain | 2.1% | 1 core @ 2.4 GHz, modo 0 |
| CPU + NHO | 3.8% | modo 1 |
| CPU + NHO + Spatial | 5.2% | modo 2 |
| Auto-vectorizacion | 94% | loops marcados `#pragma clang loop vectorize` |
| NaN/Inf incidence | 0.0% | 24h stress test |

---

## 🔒 Seguridad y Estabilidad

- **Sin `-ffast-math`**: eliminado permanentemente en v2.0 (causaba NaN en SD8 Gen2/3)
- **Sanitize por sample**: cada muestra verifica `std::isfinite()` antes del limiter
- **ControlFrame inmutable**: nunca se muta un frame publicado, solo se reemplaza
- **Seqlock SPSC**: un escritor, un lector, zero allocations
- **Watchdog daemon**: 3 fallos consecutivos antes de safe_mode

---

## 📄 Licencia

Apache-2.0 — ver [LICENSE](LICENSE)

**Autoria exclusiva**: Luis Uriel Pimentel Pérez (alias GORE TNS)
Todos los modelos matematicos, arquitecturas de sistema e implementaciones son propiedad intelectual exclusiva del autor.

---

## 🙏 Agradecimientos

- Comunidad Magisk por el framework de modulos
- Google por TensorFlow Lite (YAMNet)
- LLVM/Clang por las optimizaciones auto-vectorizadoras

---

*"El audio no se procesa. Se siente."* — GORE TNS
