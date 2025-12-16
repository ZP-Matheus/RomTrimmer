#include "RomTrimmer.hpp"
#include <chrono>
#include <iostream>
#include <vector>
#include <random>

class Benchmark {
public:
    struct Result {
        std::string testName;
        double durationMs;
        size_t bytesProcessed;
        double throughputMBps;
    };
    
    static Result benchmarkFileProcessing(const std::string& filename) {
        RomTrimmer trimmer;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Simular processamento
        std::string data = generateTestData(16 * 1024 * 1024); // 16MB
        
        auto end = std::chrono::high_resolution_clock::now();
        
        double duration = std::chrono::duration<double, std::milli>(end - start).count();
        
        return {
            "File Processing",
            duration,
            data.size(),
            (data.size() / (1024.0 * 1024.0)) / (duration / 1000.0)
        };
    }
    
    static Result benchmarkPaddingDetection(size_t dataSize) {
        std::string data = generateTestData(dataSize);
        
        // Adicionar padding
        size_t paddingSize = dataSize / 4;
        data.append(paddingSize, '\xFF');
        
        PaddingAnalyzer analyzer;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 1000; i++) {
            analyzer.analyze(data, 0xFF);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        
        double duration = std::chrono::duration<double, std::milli>(end - start).count();
        
        return {
            "Padding Detection (1000x)",
            duration,
            dataSize * 1000,
            (dataSize * 1000 / (1024.0 * 1024.0)) / (duration / 1000.0)
        };
    }
    
    static void runAllBenchmarks() {
        std::cout << "🧪 RomTrimmer++ Benchmark Suite\n";
        std::cout << "===============================\n\n";
        
        std::vector<Result> results;
        
        // Benchmarks
        results.push_back(benchmarkPaddingDetection(1 * 1024 * 1024)); // 1MB
        results.push_back(benchmarkPaddingDetection(16 * 1024 * 1024)); // 16MB
        results.push_back(benchmarkPaddingDetection(128 * 1024 * 1024)); // 128MB
        
        // Imprimir resultados
        for (const auto& result : results) {
            std::cout << "📊 " << result.testName << ":\n";
            std::cout << "   Duration: " << result.durationMs << " ms\n";
            std::cout << "   Throughput: " << result.throughputMBps << " MB/s\n";
            std::cout << "   Data processed: " 
                     << (result.bytesProcessed / (1024.0 * 1024.0)) 
                     << " MB\n\n";
        }
    }
    
private:
    static std::string generateTestData(size_t size) {
        std::string data(size, '\0');
        
        std::mt19937 rng(42); // Seed fixa para reproducibilidade
        std::uniform_int_distribution<uint8_t> dist(0, 255);
        
        for (size_t i = 0; i < size; i++) {
            data[i] = static_cast<char>(dist(rng));
        }
        
        return data;
    }
};

int main() {
    Benchmark::runAllBenchmarks();
    return 0;
}