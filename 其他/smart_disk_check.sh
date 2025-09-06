#!/bin/bash
#
# Use smartctl tool to check SSD or NVMe disk health status.
# Modified by hualongfeiyyy@163.com on 2025-08-25.
#

DEV=$1
sudo smartctl -H "$DEV" | grep -q "PASSED" || { echo "❌ HEALTH FAILED"; exit 1; }

# NVMe 检查
if [[ $DEV == *nvme* ]]; then
    warn=$(sudo smartctl -A "$DEV" | awk '/Critical Warning/ {print $3}')
    used=$(sudo smartctl -A "$DEV" | awk '/Percentage_Used/ {print $3}')
    [[ $warn -ne 0 ]] && echo "⚠️ Critical Warning=$warn"
    [[ $used -gt 90 ]] && echo "⚠️ Percentage_Used=$used%"
else  # SATA/SAS
    reall=$(sudo smartctl -A "$DEV" | awk '/Reallocated_Sector_Ct/ {print $10}')
    pend=$(sudo smartctl -A "$DEV" | awk '/Current_Pending_Sector/ {print $10}')
    [[ $reall -gt 0 ]] && echo "⚠️ Reallocated=$reall"
    [[ $pend  -gt 0 ]] && echo "⚠️ Pending=$pend"
fi
