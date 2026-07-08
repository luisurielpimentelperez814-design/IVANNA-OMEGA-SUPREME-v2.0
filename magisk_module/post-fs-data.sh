#!/system/bin/sh
# IVANNA OMEGA SUPREME v2.0 — post-fs-data.sh
# © 2026 Luis Uriel Pimentel Pérez — GORE TNS. All rights reserved.

MODDIR=${0%/*}

# Asegurar que los permisos de sepolicy estan correctos
if [ -f "$MODDIR/sepolicy.rule" ]; then
    # Magisk aplica sepolicy.rule automaticamente
    : # no-op
fi
