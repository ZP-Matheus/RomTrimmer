#!/bin/bash
# monitor_trim.sh - Monitora diretório e trim automaticamente

WATCH_DIR="$HOME/ROMs/incoming"
PROCESSED_DIR="$HOME/ROMs/processed"
LOG_FILE="$HOME/ROMs/trim_monitor.log"

echo "Starting ROM trim monitor..."
echo "Watching: $WATCH_DIR"
echo "Processed: $PROCESSED_DIR"
echo "Log: $LOG_FILE"

# Criar diretórios se não existirem
mkdir -p "$WATCH_DIR" "$PROCESSED_DIR"

# Usar inotifywait para monitorar novos arquivos
while true; do
    # Esperar por novos arquivos .gba ou .nds
    file=$(inotifywait -q -e create --format "%f" "$WATCH_DIR" | grep -E '\.(gba|nds|gb|gbc)$')
    
    if [ -n "$file" ]; then
        full_path="$WATCH_DIR/$file"
        timestamp=$(date "+%Y-%m-%d %H:%M:%S")
        
        echo "[$timestamp] New ROM detected: $file" | tee -a "$LOG_FILE"
        
        # Processar ROM
        romtrimmer++ -i "$full_path" \
            --output "$PROCESSED_DIR" \
            --verbose \
            --log-file="$LOG_FILE" 2>&1 | tee -a "$LOG_FILE"
        
        # Mover arquivo original
        mv "$full_path" "$PROCESSED_DIR/original_$file"
        
        echo "[$timestamp] Processed: $file" | tee -a "$LOG_FILE"
        echo "---" | tee -a "$LOG_FILE"
    fi
    
    sleep 1
done