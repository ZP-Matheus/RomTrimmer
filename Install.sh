#!/bin/bash

set -e

echo "🚀 RomTrimmer++ Installer"
echo "========================="

# Verificar dependências
echo "Checking dependencies..."

check_dep() {
    if ! command -v $1 &> /dev/null; then
        echo "❌ Missing: $1"
        echo "Please install: $2"
        exit 1
    fi
    echo "✅ $1"
}

check_dep "cmake" "CMake (https://cmake.org)"
check_dep "g++" "GCC/G++ or clang"
check_dep "git" "Git"

# Opcional
if command -v "openssl" &> /dev/null; then
    echo "✅ openssl (for checksums)"
else
    echo "⚠️  openssl not found (checksums will be limited)"
fi

# Criar diretório de build
echo -e "\n📦 Building RomTrimmer++..."
mkdir -p build
cd build

# Configurar
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Compilar
echo "Compiling..."
make -j$(nproc)

# Instalar (opcional)
if [ "$EUID" -ne 0 ]; then
    echo -e "\n⚠️  Not running as root, skipping system install"
    echo "The executable is at: $(pwd)/romtrimmer++"
else
    echo -e "\n🔧 Installing system-wide..."
    sudo make install
    echo "✅ Installed to /usr/local/bin/"
fi

# Criar configuração padrão
echo -e "\n⚙️  Creating default configuration..."
cd ..
./build/romtrimmer++ --help > /dev/null
echo "✅ Default config created at ~/.config/romtrimmer++/"

echo -e "\n🎉 Installation complete!"
echo "Usage: ./build/romtrimmer++ --help"