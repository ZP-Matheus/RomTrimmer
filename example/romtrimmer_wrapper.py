#!/usr/bin/env python3
# romtrimmer_wrapper.py - Wrapper Python para RomTrimmer++

import subprocess
import json
import os
from pathlib import Path
from typing import List, Dict, Any

class RomTrimmerWrapper:
    def __init__(self, binary_path: str = "romtrimmer++"):
        self.binary = binary_path
        
    def analyze_rom(self, rom_path: str) -> Dict[str, Any]:
        """Analisa uma ROM sem modificá-la."""
        cmd = [
            self.binary,
            "-i", rom_path,
            "--analyze",
            "--verbose",
            "--padding=auto"
        ]
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        # Parse output
        analysis = {
            "file": rom_path,
            "size": os.path.getsize(rom_path),
            "trim_possible": "can be removed" in result.stdout,
            "output": result.stdout
        }
        
        return analysis
    
    def batch_process(self, directory: str, recursive: bool = True) -> List[Dict[str, Any]]:
        """Processa um diretório inteiro."""
        cmd = [self.binary, "-p", directory]
        
        if recursive:
            cmd.append("-r")
        
        cmd.extend(["--output", f"{directory}_trimmed", "--verbose"])
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        # Aqui você poderia parsear o output para uma estrutura mais organizada
        return self._parse_batch_output(result.stdout)
    
    def _parse_batch_output(self, output: str) -> List[Dict[str, Any]]:
        """Parseia a saída do batch processing."""
        # Implementação simplificada
        results = []
        lines = output.split('\n')
        
        current_file = None
        for line in lines:
            if "Processing:" in line:
                current_file = line.split("Processing:")[1].strip()
            elif "removed" in line and current_file:
                results.append({
                    "file": current_file,
                    "action": "trimmed",
                    "details": line.strip()
                })
                current_file = None
        
        return results

if __name__ == "__main__":
    wrapper = RomTrimmerWrapper()
    
    # Exemplo de uso
    roms_dir = Path.home() / "ROMs" / "GBA"
    
    if roms_dir.exists():
        print(f"Analyzing ROMs in {roms_dir}")
        
        for rom_file in roms_dir.glob("*.gba"):
            analysis = wrapper.analyze_rom(str(rom_file))
            
            if analysis["trim_possible"]:
                print(f"✓ {rom_file.name} can be trimmed")
            else:
                print(f"✗ {rom_file.name} doesn't need trimming")