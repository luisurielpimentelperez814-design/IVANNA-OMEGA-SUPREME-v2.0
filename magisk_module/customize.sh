#!/system/bin/sh
# IVANNA OMEGA SUPREME v2.0 — customize.sh
# © 2026 Luis Uriel Pimentel Pérez — GORE TNS. All rights reserved.
#
# Script de instalacion del modulo Magisk.
# Detecta la arquitectura del dispositivo e instala la libreria nativa correcta.

ui_print "========================================"
ui_print "  IVANNA OMEGA SUPREME v2.0"
ui_print "  OMNIPOTENTE — GORE TNS © 2026"
ui_print "========================================"
ui_print ""

# Detectar ABI
ABI=$(getprop ro.product.cpu.abi)
ui_print "[*] Arquitectura detectada: $ABI"

# Verificar compatibilidad
case $ABI in
    arm64-v8a)
        ui_print "[*] Instalando libreria arm64-v8a..."
        mkdir -p $MODPATH/system/lib64
        cp -f $MODPATH/libs/arm64-v8a/libivanna_omega.so $MODPATH/system/lib64/
        set_perm $MODPATH/system/lib64/libivanna_omega.so 0 0 0644
        ;;
    armeabi-v7a)
        ui_print "[*] Instalando libreria armeabi-v7a..."
        mkdir -p $MODPATH/system/lib
        cp -f $MODPATH/libs/armeabi-v7a/libivanna_omega.so $MODPATH/system/lib/
        set_perm $MODPATH/system/lib/libivanna_omega.so 0 0 0644
        ;;
    *)
        ui_print "[!] Arquitectura no soportada: $ABI"
        ui_print "[!] Solo arm64-v8a y armeabi-v7a son compatibles."
        abort "Instalacion cancelada."
        ;;
esac

# Crear directorios necesarios
mkdir -p $MODPATH/system/vendor/etc

# Instalar configuraciones de audio effects por SKU
for sku in blair holi; do
    if [ -f "$MODPATH/vendor_base/sku_${sku}_audio_effects.xml" ]; then
        ui_print "[*] Instalando configuracion audio_effects para SKU: $sku"
        cp -f $MODPATH/vendor_base/sku_${sku}_audio_effects.xml $MODPATH/system/vendor/etc/
        set_perm $MODPATH/system/vendor/etc/sku_${sku}_audio_effects.xml 0 0 0644
    fi
done

# Permisos generales
set_perm_recursive $MODPATH/system 0 0 0755 0644

ui_print ""
ui_print "[✓] IVANNA OMEGA SUPREME instalado correctamente."
ui_print "[✓] Reinicia el dispositivo para activar el motor."
ui_print ""
ui_print "  Modos de procesamiento:"
ui_print "    0 = DSP (EQ+Comp+Exciter+Widener)"
ui_print "    1 = DSP + NHO (Nonlinear Harmonic Oscillator)"
ui_print "    2 = DSP + NHO + Spatial (ITD/ILD)"
ui_print ""
ui_print "  AutonomousBrain: analisis pasivo de genero en tiempo real"
ui_print "  Anti-Dolby: intercepta y mejora audio de otras apps"
ui_print ""
