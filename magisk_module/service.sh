#!/system/bin/sh
# IVANNA OMEGA SUPREME v2.0 — service.sh
# © 2026 Luis Uriel Pimentel Pérez — GORE TNS. All rights reserved.
#
# Se ejecuta en late_start service mode.
# Inicializa el daemon de audio y configura prioridades de thread.

MODDIR=${0%/*}
LOGFILE=/data/local/tmp/ivanna_omega.log

# Logging
log() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" >> $LOGFILE
}

log "========================================"
log "IVANNA OMEGA SUPREME v2.0 iniciando..."
log "========================================"

# Verificar que la libreria esta instalada
if [ -f "$MODDIR/system/lib64/libivanna_omega.so" ]; then
    LIB_PATH="$MODDIR/system/lib64/libivanna_omega.so"
    log "[*] Libreria arm64-v8a encontrada"
elif [ -f "$MODDIR/system/lib/libivanna_omega.so" ]; then
    LIB_PATH="$MODDIR/system/lib/libivanna_omega.so"
    log "[*] Libreria armeabi-v7a encontrada"
else
    log "[!] ERROR: Libreria nativa no encontrada"
    exit 1
fi

# Configurar propiedades del sistema para audio
case $(getprop ro.board.platform) in
    sm8550|kalama|pineapple)
        # Snapdragon 8 Gen 2/3
        resetprop ro.audio.monitor.rotation true
        resetprop ro.audio.monitor.window true
        log "[*] Optimizado para Snapdragon 8 Gen 2/3"
        ;;
    s5e9945|s5e9945)
        # Exynos 2400
        resetprop ro.audio.monitor.rotation true
        log "[*] Optimizado para Exynos 2400"
        ;;
    *)
        log "[*] Configuracion generica aplicada"
        ;;
esac

# Iniciar daemon si existe
if [ -f "$MODDIR/system/bin/ivanna_daemon" ]; then
    log "[*] Iniciando IVANNA daemon..."
    nohup $MODDIR/system/bin/ivanna_daemon >> $LOGFILE 2>&1 &
    log "[*] Daemon PID: $!"
fi

log "[✓] IVANNA OMEGA SUPREME v2.0 activo"
log "========================================"
