# Guia de Uso Avançado

## 1. Modos de Operação

### 1.1 Modo Análise
```bash
# Mostra o que seria feito sem modificar
romtrimmer++ -i game.gba --analyze --verbose

# Saída:
# Analyzing: game.gba (16.0 MB)
# Padding detected: 2.5 MB of 0xFF
# Would trim to: 13.5 MB (84.4% of original)
```

1.2 Modo Simulação (Dry Run)

```bash
# Mostra ações que seriam tomadas
romtrimmer++ -p ./roms --dry-run --log-file=dryrun.log
```

1.3 Modo Força

```bash
# Ignora avisos de segurança (USE COM CAUTELA!)
romtrimmer++ -i rom.gba --force --no-backup
```

2. Exemplos de Configuração

2.1 Configuração por Projeto

Crie um arquivo .romtrimmerrc no diretório do projeto:

```ini
# .romtrimmerrc
padding = ff
safety_margin_kb = 128
max_cut_percent = 75
create_backup = false
output_dir = ./trimmed
```

2.2 Configuração Global

Localização padrão do arquivo de configuração:

· Linux: ~/.config/romtrimmer++/romtrimmer.conf
· Windows: %APPDATA%\romtrimmer++\romtrimmer.conf

3. Scripting e Automação

3.1 Shell Script

```bash
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
```

3.2 Python Integration

```python
import subprocess
import json

def analyze_rom(rom_path):
    result = subprocess.run(
        ["romtrimmer++", "-i", rom_path, "--analyze", "--verbose"],
        capture_output=True,
        text=True
    )
    
    # Parse output
    lines = result.stdout.split('\n')
    analysis = {}
    
    for line in lines:
        if "Padding detected" in line:
            analysis['padding'] = line.split(":")[1].strip()
        elif "Would trim to" in line:
            analysis['new_size'] = line.split(":")[1].strip()
    
    return analysis
```

4. Casos de Uso Específicos

4.1 ROMs Homebrew

```bash
# Homebrew ROMs podem ter padding não padrão
romtrimmer++ -i homebrew.gba --padding=00 --min-size=1024
```

4.2 ROMs Corrompidas/Modificadas

```bash
# Ignorar validações de header
romtrimmer++ -i corrupted.nds --force --safety-margin=0
```

4.3 Batch Processing com Filas

```bash
# Processar por tamanho
find ./roms -name "*.gba" -size +32M | while read rom; do
    romtrimmer++ -i "$rom" --output ./large_trimmed
done
```

5. Solução de Problemas

5.1 Erro: "Padding detection failed"

Causa: O arquivo não tem padding ou tem padding misturado.
Solução:

```bash
# Especificar padding manualmente
romtrimmer++ -i rom.gba --padding=ff

# Ou usar modo força
romtrimmer++ -i rom.gba --force
```

5.2 Erro: "File too small after trim"

Causa: A ROM seria cortada abaixo do tamanho mínimo.
Solução:

```bash
# Ajustar tamanho mínimo
romtrimmer++ -i rom.gba --min-size=1048576  # 1MB

# Ou desativar validação (PERIGOSO)
romtrimmer++ -i rom.gba --force
```

5.3 Performance Lenta

Solução:

```bash
# Processar múltiplos arquivos em paralelo
find ./roms -name "*.gba" -print0 | xargs -0 -P4 -I{} romtrimmer++ -i "{}"

# Ou usar o próprio suporte a threads
romtrimmer++ -p ./roms -r  # Usa todos os cores disponíveis
```

6. Dicas e Truques

6.1 Verificação Rápida

```bash
# Verificar quais ROMs precisam de trim
romtrimmer++ -p ./roms --analyze | grep "can be trimmed"
```

6.2 Economia de Espaço

```bash
# Calcular espaço total economizável
romtrimmer++ -p ./roms --analyze --dry-run 2>&1 | \
    grep "removed" | \
    awk '{sum += $2} END {print "Total saveable: " sum}'
```

6.3 Integração com Outras Ferramentas

```bash
# Combinar com 7zip para compressão
romtrimmer++ -i rom.gba --output ./temp
7z a -mx=9 "rom.gba.7z" ./temp/*

# Ou com rar
rar a -m5 "rom.gba.rar" ./temp/*
```

7. FAQ

Q: O programa pode corromper minhas ROMs?

R: Em modo normal, não. O programa sempre cria backup (a menos que --no-backup seja usado) e realiza múltiplas validações antes de cortar.

Q: Funciona com ROMs de outros sistemas?

R: Sim, funciona com qualquer arquivo que tenha padding. O programa detecta automaticamente GBA, NDS, GB, GBC, mas pode processar qualquer arquivo.

Q: Posso reverter as mudanças?

R: Sim, os backups são criados com extensão .bak ou .bak.N. Você pode restaurar manualmente.

Q: Qual é o overhead de performance?

R: Mínimo. O programa usa memória mapeada para arquivos grandes e processamento paralelo para múltiplos arquivos.

```

## 8. Exemplo Completo de Uso

```bash
# 1. Primeiro, analisar a coleção
romtrimmer++ -p ~/ROMs --recursive --analyze --log-file=analysis.log

# 2. Verificar relatório
cat analysis.log | grep "can be trimmed" | wc -l
# Saída: 42 ROMs podem ser trimmed

# 3. Calcular espaço total economizável
cat analysis.log | grep "removed" | \
    awk '{sum += $2} END {print "Total: " sum/1024/1024 " MB"}'
# Saída: Total: 128.7 MB

# 4. Fazer trim real (com backup)
romtrimmer++ -p ~/ROMs --recursive \
    --output ~/ROMs_trimmed \
    --verbose \
    --log-file=trim_operation.log

# 5. Verificar resultados
echo "Processed: $(grep 'Processing:' trim_operation.log | wc -l)"
echo "Trimmed: $(grep 'Trim realizado' trim_operation.log | wc -l)"
echo "Failed: $(grep 'ERROR' trim_operation.log | wc -l)"
```