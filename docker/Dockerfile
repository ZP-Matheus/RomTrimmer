FROM ubuntu:22.04 AS builder

# Instalar dependências
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

# Copiar código
WORKDIR /app
COPY . .

# Compilar
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Imagem final mínima
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libssl3 \
    zlib1g \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/romtrimmer++ /usr/local/bin/

# Criar diretório para configuração
RUN mkdir -p /root/.config/romtrimmer++

WORKDIR /data
ENTRYPOINT ["romtrimmer++"]
CMD ["--help"]