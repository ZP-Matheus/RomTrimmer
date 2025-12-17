Advanced Usage Guide

1. Operation Modes

1.1 Analysis Mode

# Shows what would be done without modifying anything
romtrimmer++ -i game.gba --analyze --verbose

# Output:
# Analyzing: game.gba (16.0 MB)
# Padding detected: 2.5 MB of 0xFF
# Would trim to: 13.5 MB (84.4% of original)

1.2 Simulation Mode (Dry Run)

# Shows actions that would be taken
romtrimmer++ -p ./roms --dry-run --log-file=dryrun.log

1.3 Force Mode

# Ignores safety warnings (USE WITH CAUTION!)
romtrimmer++ -i rom.gba --force --no-backup

2. Configuration Examples

2.1 Per-Project Configuration

Create a .romtrimmerrc file in the project directory:

# .romtrimmerrc
padding = ff
safety_margin_kb = 128
max_cut_percent = 75
create_backup = false
output_dir = ./trimmed

2.2 Global Configuration

Default configuration file locations:

· Linux: ~/.config/romtrimmer++/romtrimmer.conf
· Windows: %APPDATA%\romtrimmer++\romtrimmer.conf

3. Scripting and Automation

3.1 Shell Script

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

3.2 Python Integration

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

4. Specific Use Cases

4.1 Homebrew ROMs

# Homebrew ROMs may have non-standard padding
romtrimmer++ -i homebrew.gba --padding=00 --min-size=1024

4.2 Corrupted / Modified ROMs

# Ignore header validations
romtrimmer++ -i corrupted.nds --force --safety-margin=0

4.3 Batch Processing with Queues

# Process by size
find ./roms -name "*.gba" -size +32M | while read rom; do
    romtrimmer++ -i "$rom" --output ./large_trimmed
done

5. Troubleshooting

5.1 Error: "Padding detection failed"

Cause: The file has no padding or mixed padding.
Solution:

# Specify padding manually
romtrimmer++ -i rom.gba --padding=ff

# Or use force mode
romtrimmer++ -i rom.gba --force

5.2 Error: "File too small after trim"

Cause: The ROM would be trimmed below the minimum size.
Solution:

# Adjust minimum size
romtrimmer++ -i rom.gba --min-size=1048576  # 1MB

# Or disable validation (DANGEROUS)
romtrimmer++ -i rom.gba --force

5.3 Slow Performance

Solution:

# Process multiple files in parallel
find ./roms -name "*.gba" -print0 | xargs -0 -P4 -I{} romtrimmer++ -i "{}"

# Or use built-in thread support
romtrimmer++ -p ./roms -r  # Uses all available cores

6. Tips and Tricks

6.1 Quick Check

# Check which ROMs need trimming
romtrimmer++ -p ./roms --analyze | grep "can be trimmed"

6.2 Space Saving

# Calculate total saveable space
romtrimmer++ -p ./roms --analyze --dry-run 2>&1 | \
    grep "removed" | \
    awk '{sum += $2} END {print "Total saveable: " sum}'

6.3 Integration with Other Tools

# Combine with 7zip for compression
romtrimmer++ -i rom.gba --output ./temp
7z a -mx=9 "rom.gba.7z" ./temp/*

# Or with rar
rar a -m5 "rom.gba.rar" ./temp/*

7. FAQ

Q: Can the program corrupt my ROMs?

A: In normal mode, no. The program always creates a backup (unless --no-backup is used) and performs multiple validations before trimming.

Q: Does it work with ROMs from other systems?

A: Yes. It works with any file that contains padding. The program automatically detects GBA, NDS, GB, and GBC, but it can process any file.

Q: Can I revert the changes?

A: Yes. Backups are created with the .bak or .bak.N extension. You can restore them manually.

Q: What is the performance overhead?

A: Minimal. The program uses memory-mapped files for large files and parallel processing for multiple files.


---

8. Complete Usage Example

# 1. First, analyze the collection
romtrimmer++ -p ~/ROMs --recursive --analyze --log-file=analysis.log

# 2. Check report
cat analysis.log | grep "can be trimmed" | wc -l
# Output: 42 ROMs can be trimmed

# 3. Calculate total saveable space
cat analysis.log | grep "removed" | \
    awk '{sum += $2} END {print "Total: " sum/1024/1024 " MB"}'
# Output: Total: 128.7 MB

# 4. Perform actual trim (with backup)
romtrimmer++ -p ~/ROMs --recursive \
    --output ~/ROMs_trimmed \
    --verbose \
    --log-file=trim_operation.log

# 5. Verify results
echo "Processed: $(grep 'Processing:' trim_operation.log | wc -l)"
echo "Trimmed: $(grep 'Trim performed' trim_operation.log | wc -l)"
echo "Failed: $(grep 'ERROR' trim_operation.log | wc -l)"


---