#!/usr/bin/env bash
set -euo pipefail
echo "WARNING: SCHED_FIFO/RR can starve normal tasks. Run only on a test box."
chrt -p $$ || true
echo "Example (requires CAP_SYS_NICE/root): chrt -f 50 ./app/cpu_burner 3"
echo "Rollback/cleanup: kill the task; no persistent setting is changed by this script."
