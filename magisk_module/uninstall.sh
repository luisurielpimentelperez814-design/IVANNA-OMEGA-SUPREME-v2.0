#!/system/bin/sh
# IVANNA OMEGA SUPREME v2.0 — uninstall.sh
# © 2026 Luis Uriel Pimentel Pérez — GORE TNS. All rights reserved.

# Limpiar logs
rm -f /data/local/tmp/ivanna_omega.log
rm -f /data/local/tmp/ivanna_benchmark.txt

# Detener daemon si esta corriendo
pkill -f ivanna_daemon 2>/dev/null

ui_print "[✓] IVANNA OMEGA SUPREME desinstalado."
ui_print "[✓] Reinicia el dispositivo para completar."
