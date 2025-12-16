#!/bin/bash
# batch_trim.sh

INPUT_DIR="./roms"
OUTPUT_DIR="./trimmed"
LOG_FILE="trim_$(date +%Y%m%d_%H%M%S).log"

echo "Starting batch trim at $(date)" | tee -a "$LOG_FILE"

# Processar todas as ROMs
romtrimmer++ -p "$INPUT_DIR" -r \
    --output "$OUTPUT_DIR" \
    --padding=auto \
    --dry-run \
    --log-file="$LOG_FILE" \
    --verbose

# Gerar relatório
echo -e "\n=== Batch Complete ===" | tee -a "$LOG_FILE"
echo "Input: $INPUT_DIR" | tee -a "$LOG_FILE"
echo "Output: $OUTPUT_DIR" | tee -a "$LOG_FILE"
echo "Log: $LOG_FILE" | tee -a "$LOG_FILE"

# Calcular espaço economizado
if [ -f "$LOG_FILE" ]; then
    SAVED=$(grep "removed" "$LOG_FILE" | awk '{sum += $2} END {print sum}')
    echo "Total space saved: $SAVED" | tee -a "$LOG_FILE"
fi