package com.ivanna.omega

import android.Manifest
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.media.projection.MediaProjectionManager
import android.os.Build
import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.foundation.lazy.LazyRow
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material.icons.filled.Pause
import androidx.compose.material.icons.filled.Settings
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.core.content.ContextCompat
import com.ivanna.omega.audio.*
import com.ivanna.omega.core.*
import com.ivanna.omega.dsp.DSPBridge
import com.ivanna.omega.dsp.DSPState
import com.ivanna.omega.neuromorphic.IvannaNpeEngine
import com.ivanna.omega.visualizer.VisualizerSurface
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.collectLatest

/**
 * MainActivity v2.0 OMNIPOTENTE — UI Compose Material3
 * © 2026 Luis Uriel Pimentel Pérez — GORE TNS. All rights reserved.
 *
 * Features:
 *   - ControlFrameBus integration (lock-free params)
 *   - Modo OPE: DSP / DSP+NHO / DSP+NHO+Spatial
 *   - Presets con AutonomousBrain feedback
 *   - YAMNet scores en tiempo real
 *   - Visualizer Gammatone13
 *   - Anti-Dolby global
 *   - PlaybackCaptureService (captura digital interna)
 */
class MainActivity : ComponentActivity() {
    companion object {
        private const val TAG = "IVANNA-v2"
        private const val PERM_RECORD = Manifest.permission.RECORD_AUDIO
    }

    private val requestPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestPermission()
    ) { isGranted ->
        if (isGranted) startAudioService() else showPermissionDenied()
    }

    private val captureLauncher = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { result ->
        if (result.resultCode == RESULT_OK) {
            startPlaybackCapture(result.data)
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        OmegaEngine.init(this)
        setContent { IvannaOmegaTheme { MainScreen() } }
        checkPermissions()
    }

    @OptIn(ExperimentalMaterial3Api::class)
    @Composable
    fun MainScreen() {
        val scrollState = rememberScrollState()
        val scope = rememberCoroutineScope()

        var isProcessing by remember { mutableStateOf(false) }
        var selectedMode by remember { mutableIntStateOf(OmegaEngine.getMode()) }
        var selectedPreset by remember { mutableStateOf<IvannaEffectProfile?>(null) }
        var autoMode by remember { mutableStateOf(false) }
        var yamnetSpeech by remember { mutableFloatStateOf(0f) }
        var yamnetMusic by remember { mutableFloatStateOf(0f) }
        var yamnetBass by remember { mutableFloatStateOf(0f) }
        var genreConfidence by remember { mutableFloatStateOf(0f) }

        // DSP params
        var drive by remember { mutableFloatStateOf(0.65f) }
        var wet by remember { mutableFloatStateOf(0.5f) }
        var lowDb by remember { mutableFloatStateOf(0f) }
        var midDb by remember { mutableFloatStateOf(0f) }
        var highDb by remember { mutableFloatStateOf(0f) }
        var width by remember { mutableFloatStateOf(1.0f) }
        var masterDb by remember { mutableFloatStateOf(0f) }

        // Poll genre confidence
        LaunchedEffect(Unit) {
            while (true) {
                genreConfidence = IvannaNativeLib.nativeGetGenreConfidence()
                delay(500)
            }
        }

        Column(
            modifier = Modifier
                .fillMaxSize()
                .verticalScroll(scrollState)
                .padding(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            // Header
            Text("IVANNA OMEGA SUPREME", style = MaterialTheme.typography.headlineMedium)
            Text("v2.0 OMNIPOTENTE — GORE TNS © 2026", style = MaterialTheme.typography.labelSmall)
            Text(OmegaEngine.getVersion(), style = MaterialTheme.typography.labelSmall)

            Spacer(Modifier.height(16.dp))

            // Visualizer
            VisualizerSurface(Modifier.fillMaxWidth().height(120.dp))

            Spacer(Modifier.height(16.dp))

            // YAMNet AI
            Card(Modifier.fillMaxWidth()) {
                Column(Modifier.padding(12.dp)) {
                    Text("YAMNet AI Classification", style = MaterialTheme.typography.titleSmall)
                    LinearProgressIndicator(progress = { yamnetSpeech }, Modifier.fillMaxWidth())
                    Text("Speech: ${(yamnetSpeech*100).toInt()}%")
                    LinearProgressIndicator(progress = { yamnetMusic }, Modifier.fillMaxWidth())
                    Text("Music: ${(yamnetMusic*100).toInt()}%")
                    LinearProgressIndicator(progress = { yamnetBass }, Modifier.fillMaxWidth())
                    Text("Bass: ${(yamnetBass*100).toInt()}%")
                }
            }

            Spacer(Modifier.height(16.dp))

            // Autonomous Brain
            Card(Modifier.fillMaxWidth()) {
                Column(Modifier.padding(12.dp)) {
                    Text("Autonomous Brain", style = MaterialTheme.typography.titleSmall)
                    LinearProgressIndicator(progress = { genreConfidence }, Modifier.fillMaxWidth())
                    Text("Genre Confidence: ${(genreConfidence*100).toInt()}%")
                }
            }

            Spacer(Modifier.height(16.dp))

            // Play/Stop
            Button(
                onClick = {
                    isProcessing = !isProcessing
                    if (isProcessing) startAudioService() else stopAudioService()
                },
                modifier = Modifier.size(80.dp)
            ) {
                Icon(
                    imageVector = if (isProcessing) Icons.Default.Pause else Icons.Default.PlayArrow,
                    contentDescription = if (isProcessing) "Stop" else "Play"
                )
            }

            Spacer(Modifier.height(16.dp))

            // Mode selector
            Text("Processing Mode", style = MaterialTheme.typography.titleMedium)
            SingleChoiceSegmentedButtonRow {
                SegmentedButton(
                    selected = selectedMode == 0,
                    onClick = { selectedMode = 0; OmegaEngine.setMode(0) },
                    shape = SegmentedButtonDefaults.itemShape(0, 3)
                ) { Text("DSP") }
                SegmentedButton(
                    selected = selectedMode == 1,
                    onClick = { selectedMode = 1; OmegaEngine.setMode(1) },
                    shape = SegmentedButtonDefaults.itemShape(1, 3)
                ) { Text("+NHO") }
                SegmentedButton(
                    selected = selectedMode == 2,
                    onClick = { selectedMode = 2; OmegaEngine.setMode(2) },
                    shape = SegmentedButtonDefaults.itemShape(2, 3)
                ) { Text("+Spatial") }
            }

            Spacer(Modifier.height(16.dp))

            // Presets
            Text("Presets", style = MaterialTheme.typography.titleMedium)
            LazyRow(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                items(IvannaEffectProfile.ALL) { preset ->
                    FilterChip(
                        selected = selectedPreset == preset,
                        onClick = {
                            selectedPreset = preset
                            applyPreset(preset)
                            lowDb = preset.eqBands[0] / 100f
                        },
                        label = { Text(preset.name) }
                    )
                }
            }

            Row(verticalAlignment = Alignment.CenterVertically) {
                Switch(checked = autoMode, onCheckedChange = { autoMode = it })
                Text("Auto AI Mode", Modifier.padding(start = 8.dp))
            }

            Spacer(Modifier.height(16.dp))

            // DSP Sliders
            Text("DSP Parameters", style = MaterialTheme.typography.titleMedium)
            DSPSlider("Drive", drive, 0f..1f) { drive = it; pushDSP() }
            DSPSlider("Wet", wet, 0f..1f) { wet = it; pushDSP() }
            DSPSlider("Low EQ", lowDb, -18f..18f) { lowDb = it; pushDSP() }
            DSPSlider("Mid EQ", midDb, -18f..18f) { midDb = it; pushDSP() }
            DSPSlider("High EQ", highDb, -18f..18f) { highDb = it; pushDSP() }
            DSPSlider("Stereo Width", width, 0f..1.5f) { width = it; pushDSP() }
            DSPSlider("Master", masterDb, -18f..18f) { masterDb = it; pushDSP() }
        }
    }

    @Composable
    fun DSPSlider(label: String, value: Float, range: ClosedFloatingPointRange<Float>, onChange: (Float) -> Unit) {
        Column(Modifier.fillMaxWidth().padding(vertical = 4.dp)) {
            Row {
                Text(label, Modifier.weight(1f))
                Text(String.format("%.2f", value))
            }
            Slider(value = value, onValueChange = onChange, valueRange = range)
        }
    }

    private fun pushDSP() {
        DSPBridge.setParams(
            drive, wet, 0.7f,
            0.5f, 0.5f, width / 1.5f,
            1000f, 0.707f,
            lowDb, midDb, highDb, 0f, masterDb
        )
    }

    private fun applyPreset(preset: IvannaEffectProfile) {
        IVANNAApplication.globalEffectManager.applyProfile(preset)
    }

    private fun checkPermissions() {
        when {
            ContextCompat.checkSelfPermission(this, PERM_RECORD) == PackageManager.PERMISSION_GRANTED -> {}
            shouldShowRequestPermissionRationale(PERM_RECORD) -> showPermissionRationale()
            else -> requestPermissionLauncher.launch(PERM_RECORD)
        }
    }

    private fun startAudioService() {
        val mpm = getSystemService(Context.MEDIA_PROJECTION_SERVICE) as MediaProjectionManager
        captureLauncher.launch(mpm.createScreenCaptureIntent())
    }

    private fun startPlaybackCapture(intent: Intent?) {
        startForegroundService(Intent(this, PlaybackCaptureService::class.java).apply {
            putExtra("media_projection", intent)
        })
    }

    private fun stopAudioService() {
        stopService(Intent(this, PlaybackCaptureService::class.java))
    }

    private fun showPermissionDenied() {
        Log.w(TAG, "Permission denied")
    }

    private fun showPermissionRationale() {
        requestPermissionLauncher.launch(PERM_RECORD)
    }
}
