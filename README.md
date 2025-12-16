# RomTrimmer++

A powerful ROM trimming utility with extra security against aggressive cuts

# Full Description

An advanced CLI utility for trimming GBA, DS, and other system ROMs, safely and intelligently removing end-of-file padding (0xFF or 0x00).

# Features

Automatic detection of GBA (.gba), DS (.nds), GB (.gb), and GBC (.gbc) ROMs

Smart padding removal (0xFF, 0x00) only when it is safe

Multiple protections against excessive trimming

Operation modes: normal, analysis-only, simulation (dry-run)

Automatic backup of original files

Recursive directory support

Detailed logging with verbosity levels

ROM-type-specific security validation


Installation

Requirements

CMake 3.15+

C++17 compiler (GCC 8+, Clang 7+, MSVC 2019+)


# Build

git clone https://github.com/ZP-Matheus/romtrimmer.git
cd romtrimmer
mkdir build && cd build
cmake ..
make -j$(nproc)
make install

# Usage Examples

# Analyze a ROM without modifying it
./romtrimmer++ -i "some_game.gba" --analyze --verbose

# Perform an actual trim with backup
./romtrimmer++ -i "super_good_game_ds.nds"

# Process an entire folder recursively
./romtrimmer++ -p "~/roms/gba" -r --dry-run

# Force trimming even with warnings (use with caution!)
./romtrimmer++ -i "very_suspicious_rom.gba" --force


---
