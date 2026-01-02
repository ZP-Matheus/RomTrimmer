#!/bin/bash
# compare_with_ucon64.sh
# Compare our algorithm with ucon64 results

echo "RomTrimmer++ vs uCon64 Comparison Test"
echo "======================================"

# Create results directory
mkdir -p comparison_results

# Process each ROM file
for file in *.gba *.nds *.gb *.gbc *.nes *.smc *.sfc; do
    if [ -f "$file" ]; then
        echo "Processing: $file"
        
        # Get our algorithm's result
        our_result=$(romtrimmer++ -i "$file" --analyze --verbose 2>&1 | grep -i "padding\|trim\|saved")
        
        # Get ucon64 result
        ucon_result=$(ucon64 --ispad "$file" 2>/dev/null | grep -i "padded")
        
        # Extract padding bytes
        our_padding=$(echo "$our_result" | grep -oE '[0-9]+ bytes' | head -1 | grep -oE '[0-9]+')
        ucon_padding=$(echo "$ucon_result" | grep -oE '[0-9]+ bytes' | head -1 | grep -oE '[0-9]+')
        
        # Default to 0 if not found
        our_padding=${our_padding:-0}
        ucon_padding=${ucon_padding:-0}
        
        # Calculate difference
        if [ "$ucon_padding" -ne 0 ]; then
            diff_percent=$(( (our_padding - ucon_padding) * 100 / ucon_padding ))
        else
            diff_percent=0
        fi
        
        # Save to CSV
        echo "\"$file\",$our_padding,$ucon_padding,$diff_percent%" >> comparison_results/results.csv
        
        echo "  Our padding: $our_padding bytes"
        echo "  uCon64 padding: $ucon_padding bytes"
        echo "  Difference: $diff_percent%"
        echo ""
    fi
done

# Generate report
echo "=== Summary Report ==="
echo "Total files processed: $(wc -l < comparison_results/results.csv)"
echo "Results saved to: comparison_results/results.csv"
