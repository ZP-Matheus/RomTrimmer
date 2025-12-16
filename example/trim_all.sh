#!/bin/bash
# trim_all.sh

LOG_DIR="./logs"
OUTPUT_DIR="./trimmed"
mkdir -p "$LOG_DIR" "$OUTPUT_DIR"

for rom in *.gba *.nds; do
    if [ -f "$rom" ]; then
        echo "Processing $rom..."
        
        romtrimmer++ -i "$rom" \
            --output "$OUTPUT_DIR" \
            --log-file "$LOG_DIR/${rom}.log" \
            --padding=auto \
            --verbose 2>&1 | tee "$LOG_DIR/${rom}.console.log"
        
        if [ $? -eq 0 ]; then
            echo "✓ $rom processed successfully"
        else
            echo "✗ $rom failed"
        fi
    fi
done