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