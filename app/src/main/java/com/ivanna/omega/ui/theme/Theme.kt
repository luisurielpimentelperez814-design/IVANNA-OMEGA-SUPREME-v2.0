/*
 * © 2026 Luis Uriel Pimentel Pérez — GORE TNS. All rights reserved.
 * Theme.kt — IvannaOmegaTheme
 *
 * FIX: MainActivity.kt siempre llamó a IvannaOmegaTheme { ... } pero el
 * composable nunca se definió en ningún archivo del proyecto (ni siquiera
 * un Theme.kt/Color.kt boilerplate de plantilla). Esto rompía la compilación
 * de Kotlin por completo (Unresolved reference: IvannaOmegaTheme).
 *
 * Paleta oscura, orientada a una app de procesamiento de audio en tiempo
 * real: fondo casi negro, acento cian/violeta para VU-meters y controles.
 */
package com.ivanna.omega.ui.theme

import android.app.Activity
import android.os.Build
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.dynamicDarkColorScheme
import androidx.compose.material3.dynamicLightColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.SideEffect
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalView
import androidx.core.view.WindowCompat

private val OmegaCyan = Color(0xFF00E5D0)
private val OmegaViolet = Color(0xFF7C4DFF)
private val OmegaBackground = Color(0xFF0B0D12)
private val OmegaSurface = Color(0xFF14171F)

private val IvannaDarkColorScheme = darkColorScheme(
    primary = OmegaCyan,
    secondary = OmegaViolet,
    tertiary = OmegaViolet,
    background = OmegaBackground,
    surface = OmegaSurface,
    onPrimary = Color.Black,
    onSecondary = Color.White,
    onBackground = Color(0xFFE3E6EC),
    onSurface = Color(0xFFE3E6EC),
)

private val IvannaLightColorScheme = lightColorScheme(
    primary = OmegaCyan,
    secondary = OmegaViolet,
    tertiary = OmegaViolet,
)

/**
 * Tema principal de IVANNA OMEGA SUPREME.
 * Por defecto usa esquema oscuro (más apropiado para un panel de DSP en
 * tiempo real), respetando el modo claro/oscuro del sistema si se desea,
 * y color dinámico (Material You) en Android 12+.
 */
@Composable
fun IvannaOmegaTheme(
    darkTheme: Boolean = isSystemInDarkTheme(),
    dynamicColor: Boolean = false,
    content: @Composable () -> Unit
) {
    val colorScheme = when {
        dynamicColor && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S -> {
            val context = LocalContext.current
            if (darkTheme) dynamicDarkColorScheme(context) else dynamicLightColorScheme(context)
        }
        darkTheme -> IvannaDarkColorScheme
        else -> IvannaLightColorScheme
    }

    val view = LocalView.current
    if (!view.isInEditMode) {
        SideEffect {
            val window = (view.context as Activity).window
            window.statusBarColor = colorScheme.background.toArgb()
            WindowCompat.getInsetsController(window, view).isAppearanceLightStatusBars = !darkTheme
        }
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = MaterialTheme.typography,
        content = content
    )
}
