#include "RomTrimmer.hpp"
#include "Localization.hpp"
#include "ValidationResult.hpp" // ou outro header onde a struct é definida
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <memory>
#include <thread>

// ==================== CONSTRUTOR ====================
RomTrimmer::RomTrimmer() {
    logger = std::make_unique<Logger>();
    romDetector = std::make_unique<RomDetector>();
    paddingAnalyzer = std::make_unique<PaddingAnalyzer>();
    safetyValidator = std::make_unique<SafetyValidator>();
    configManager = std::make_unique<ConfigManager>();
}

// ==================== RUN - PONTO DE ENTRADA PRINCIPAL ====================
void RomTrimmer::run(int argc, char** argv) {
    try {
        // 1. Configuração inicial
        initializeConfiguration();
        
        // 2. Processar argumentos da linha de comando
        if (!parseArguments(argc, argv)) {
            return;
        }
        
        // 3. Se --help ou --version foram solicitados, já foram tratados
        if (options.helpRequested || options.versionRequested) {
            return;
        }
        
        // 4. Validar que temos inputs para processar
        if (options.inputPaths.empty()) {
            std::cerr << TR("NO_INPUT") << "\n" << TR("TRY_HELP") << std::endl;
            return;
        }
        
        // 5. Iniciar processamento
        startProcessing();
        
        // 6. Coletar arquivos para processamento
        if (!collectFiles()) {
            logger->log(TR("NO_INPUT"), LogLevel::ERROR);
            return;
        }
        
        // 7. Processar cada arquivo (com suporte a threading)
        processFiles();
        
        // 8. Exibir resumo final
        printSummary();
        
        // 9. Limpar recursos
        cleanup();
        
    } catch (const std::exception& e) {
        handleCriticalError(e.what());
    } catch (...) {
        handleCriticalError(TR("UNKNOWN_ERROR"));
    }
}

// ==================== INICIALIZAÇÃO ====================
void RomTrimmer::initializeConfiguration() {
    // Carregar configurações do arquivo
    configManager->load();
    
    // Aplicar configurações padrão
    applyDefaultConfiguration();
    
    logger->log(TR("START_MSG"), LogLevel::INFO);
}

void RomTrimmer::applyDefaultConfiguration() {
    // Configurações de segurança
    options.minSize = configManager->getInt("safety.min_size", 1024);
    options.safetyMargin = configManager->getInt("safety.margin", 65536);
    options.maxCutRatio = configManager->getDouble("safety.max_cut_ratio", 0.6);
    options.backup = configManager->getBool("general.create_backup", true);
    
    // Configurações de padding
    std::string padding = configManager->getString("general.default_padding", "auto");
    if (padding == "auto") {
        options.paddingByte = 0;
    } else if (padding == "0x00") {
        options.paddingByte = 0x00;
    } else {
        options.paddingByte = 0xFF;
    }
}

// ==================== PARSING DE ARGUMENTOS ====================
bool RomTrimmer::parseArguments(int argc, char** argv) {
    try {
        cxxopts::Options opts("romtrimmer++", TR("A_POWERFUL_ROM_TRIMMING_UTILITY"));
        
        // Definir todas as opções disponíveis
        defineCommandLineOptions(opts);
        
        // Parsear argumentos
        auto result = opts.parse(argc, argv);
        
        // Tratar flags especiais
        if (result.count("help")) {
            printHelp(opts);
            options.helpRequested = true;
            return false;
        }
        
        if (result.count("version")) {
            std::cout << TR("VERSION_TEXT");
            options.versionRequested = true;
            return false;
        }
        
        // Processar opções principais
        processCommandLineOptions(result);
        
        // Validar opções
        if (!validateOptions()) {
            return false;
        }
        
        // Aplicar configurações de verbosidade
        if (options.verbose) {
            logger->log("Verbose mode enabled", LogLevel::DEBUG);
        }
        
        return true;
        
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << TR("ARGUMENT_ERROR") << ": " << e.what() << "\n";
        std::cerr << TR("TRY_HELP") << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << TR("CRITICAL_ERROR") << ": " << e.what() << "\n";
        return false;
    }
}

void RomTrimmer::defineCommandLineOptions(cxxopts::Options& opts) {
    opts.add_options()
        // Opções de entrada
        ("i,input", TR("INPUT_HELP"), cxxopts::value<std::vector<std::string>>())
        ("p,path", TR("PATH_HELP"), cxxopts::value<std::string>())
        ("r,recursive", TR("RECURSIVE_HELP"))
        
        // Opções de saída
        ("o,output", TR("OUTPUT_HELP"), cxxopts::value<std::string>())
        
        // Modos de operação
        ("a,analyze", TR("ANALYSIS_MODE"))
        ("d,dry-run", TR("SIMULATION_MODE"))
        ("f,force", TR("FORCE_HELP"))
        
        // Configurações
        ("b,no-backup", TR("NO_BACKUP_HELP"))
        ("padding-byte", "Padrão de padding (0xFF, 0x00, auto)", 
         cxxopts::value<std::string>()->default_value("auto"))
        ("min-size", "Tamanho mínimo em bytes", 
         cxxopts::value<size_t>()->default_value("1024"))
        ("safety-margin", "Margem de segurança em bytes", 
         cxxopts::value<size_t>()->default_value("65536"))
        ("max-cut-ratio", "Porcentagem máxima de corte (0.0-1.0)", 
         cxxopts::value<double>()->default_value("0.6"))
        
        // Informação e debug
        ("v,verbose", TR("VERBOSE_HELP"))
        ("h,help", TR("HELP_HELP"))
        ("version", TR("VERSION_HELP"))
        ("log-file", "Arquivo de log para saída detalhada", 
         cxxopts::value<std::string>())
        ("threads", "Número de threads para processamento paralelo", 
         cxxopts::value<int>()->default_value("1"));
}

void RomTrimmer::processCommandLineOptions(const cxxopts::ParseResult& result) {
    // Flags booleanas
    options.recursive   = result.count("recursive") > 0;
    options.dryRun      = result.count("dry-run") > 0;
    options.analyzeOnly = result.count("analyze") > 0;
    options.force       = result.count("force") > 0;
    options.verbose     = result.count("verbose") > 0;
    options.backup      = result.count("no-backup") == 0; // Invertido
    
    // Caminhos de entrada
    if (result.count("input")) {
        for (const auto& path : result["input"].as<std::vector<std::string>>()) {
            options.inputPaths.emplace_back(path);
        }
    }
    
    if (result.count("path")) {
        options.inputPaths.emplace_back(result["path"].as<std::string>());
    }
    
    // Caminho de saída
    if (result.count("output")) {
        options.outputDir = result["output"].as<std::string>();
        
        // Criar diretório de saída se não existir
        if (!fs::exists(options.outputDir)) {
            fs::create_directories(options.outputDir);
        }
    }
    
    // Configurações avançadas
    if (result.count("padding-byte")) {
        std::string padding = result["padding-byte"].as<std::string>();
        if (padding == "0xFF") options.paddingByte = 0xFF;
        else if (padding == "0x00") options.paddingByte = 0x00;
        else if (padding == "auto") options.paddingByte = 0;
        else {
            throw std::runtime_error("Valor de padding inválido: " + padding);
        }
    }
    
    if (result.count("min-size")) {
        options.minSize = result["min-size"].as<size_t>();
    }
    
    if (result.count("safety-margin")) {
        options.safetyMargin = result["safety-margin"].as<size_t>();
    }
    
    if (result.count("max-cut-ratio")) {
        options.maxCutRatio = result["max-cut-ratio"].as<double>();
        if (options.maxCutRatio < 0.0 || options.maxCutRatio > 1.0) {
            throw std::runtime_error("max-cut-ratio deve estar entre 0.0 e 1.0");
        }
    }
    
    // Configuração de log
    if (result.count("log-file")) {
        logger->setLogFile(result["log-file"].as<std::string>());
    }
    
    // Configuração de threads
    if (result.count("threads")) {
        int threads = result["threads"].as<int>();
        if (threads < 1) {
            logger->log("Número de threads inválido, usando 1", LogLevel::WARNING);
        }
    }
}

bool RomTrimmer::validateOptions() {
    // Validar diretório de saída
    if (!options.outputDir.empty() && !fs::is_directory(options.outputDir)) {
        std::cerr << TR("INVALID_OUTPUT_DIR") << ": " << options.outputDir << std::endl;
        return false;
    }
    
    // Validar limites de segurança
    if (options.maxCutRatio > 0.9 && !options.force) {
        logger->log(TR("HIGH_CUT_RATIO_WARNING"), LogLevel::WARNING);
    }
    
    if (options.minSize < 1024) {
        logger->log("Tamanho mínimo muito pequeno, ajustando para 1024 bytes", 
                   LogLevel::WARNING);
        options.minSize = 1024;
    }
    
    return true;
}

// ==================== COLETA DE ARQUIVOS ====================
bool RomTrimmer::collectFiles() {
    std::vector<fs::path> allFiles;
    
    for (const auto& inputPath : options.inputPaths) {
        try {
            if (!fs::exists(inputPath)) {
                logger->log(TR("PATH_NOT_EXIST") + inputPath.string(), LogLevel::ERROR);
                continue;
            }
            
            if (fs::is_regular_file(inputPath)) {
                // Arquivo único
                if (isSupportedFileExtension(inputPath)) {
                    allFiles.push_back(inputPath);
                } else {
                    logger->log("Extensão não suportada: " + inputPath.string(), 
                               LogLevel::WARNING);
                }
            } 
            else if (fs::is_directory(inputPath)) {
                // Diretório
                collectFilesFromDirectory(inputPath, allFiles);
            }
            
        } catch (const fs::filesystem_error& e) {
            logger->log("Erro ao acessar caminho " + inputPath.string() + 
                       ": " + e.what(), LogLevel::ERROR);
        }
    }
    
    // Remover duplicatas e ordenar
    removeDuplicatesAndSort(allFiles);
    
    // Atualizar lista de arquivos
    options.inputPaths = allFiles;
    
    // Log do resultado
    logger->log(std::to_string(allFiles.size()) + TR("FILES_FOUND"), 
               LogLevel::INFO);
    
    if (options.verbose && !allFiles.empty()) {
        logger->log("Arquivos a processar:", LogLevel::DEBUG);
        for (const auto& file : allFiles) {
            logger->log("  - " + file.string(), LogLevel::DEBUG);
        }
    }
    
    return !allFiles.empty();
}

void RomTrimmer::collectFilesFromDirectory(const fs::path& dir, 
                                          std::vector<fs::path>& allFiles) {
    try {
        if (options.recursive) {
            // Busca recursiva
            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                if (fs::is_regular_file(entry) && 
                    isSupportedFileExtension(entry.path())) {
                    allFiles.push_back(entry.path());
                }
            }
        } else {
            // Busca apenas no diretório atual
            for (const auto& entry : fs::directory_iterator(dir)) {
                if (fs::is_regular_file(entry.path()) && 
                    isSupportedFileExtension(entry.path())) {
                    allFiles.push_back(entry.path());
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        logger->log("Erro ao acessar diretório " + dir.string() + 
                   ": " + e.what(), LogLevel::ERROR);
    }
}

bool RomTrimmer::isSupportedFileExtension(const fs::path& filePath) {
    static const std::unordered_set<std::string> supportedExtensions = {
        ".gba", ".nds", ".gb", ".gbc", ".nes", ".smc", 
        ".sfc", ".n64", ".z64", ".v64", ".bin", ".rom"
    };
    
    std::string ext = filePath.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    return supportedExtensions.find(ext) != supportedExtensions.end();
}

void RomTrimmer::removeDuplicatesAndSort(std::vector<fs::path>& files) {
    std::sort(files.begin(), files.end());
    files.erase(std::unique(files.begin(), files.end()), files.end());
}

// ==================== PROCESSAMENTO DE ARQUIVOS ====================
void RomTrimmer::processFiles() {
    logger->log("Iniciando processamento de " + 
               std::to_string(options.inputPaths.size()) + " arquivos...", 
               LogLevel::INFO);
    
    // Processamento sequencial (pode ser estendido para paralelo)
    for (const auto& filePath : options.inputPaths) {
        if (!processFile(filePath)) {
            filesFailed++;
        } else {
            filesProcessed++;
        }
        
        // Verificar se houve erro crítico que deve parar o processamento
        if (filesFailed > 10 && !options.force) {
            logger->log("Muitos erros ocorreram, abortando processamento", 
                       LogLevel::ERROR);
            break;
        }
    }
}

bool RomTrimmer::processFile(const fs::path& filePath) {
    FileStats stats;
    stats.path = filePath;
    stats.startTime = std::chrono::steady_clock::now();
    
    try {
        // Log inicial
        logger->log(TR("PROCESSING") + filePath.string(), LogLevel::INFO);
        
        // 1. Ler arquivo
        std::string data = readFile(filePath);
        stats.originalSize = data.size();
        
        if (data.empty()) {
            throw std::runtime_error(TR("EMPTY_FILE"));
        }
        
        // 2. Detectar tipo de ROM
        RomType romType = romDetector->detect(data);
        stats.romType = romTypeToString(romType);
        
        if (romType == RomType::UNKNOWN) {
            logger->log(TR("UNKNOWN_ROM"), LogLevel::WARNING);
            stats.error = TR("UNKNOWN_ROM");
            recordFileStats(stats);
            return false;
        }
        
        // 3. Detectar padding
        uint8_t paddingByte = determinePaddingByte(data, romType);
        logger->log(std::string(TR("AUTO_PADDING_DETECTED")) + 
                   (paddingByte == 0xFF ? "FF" : "00"), 
                   LogLevel::DEBUG);
        
        // 4. Analisar padding
        PaddingAnalysis analysis = paddingAnalyzer->analyze(data, paddingByte);
        
        if (!analysis.hasPadding) {
            logger->log(TR("NO_PADDING"), LogLevel::INFO);
            stats.trimmed = false;
            stats.trimmedSize = stats.originalSize;
            recordFileStats(stats);
            return true;
        }
        
        // 5. Calcular ponto de corte
        size_t trimPoint = analysis.trimPoint;
        stats.trimmedSize = trimPoint;
        stats.savedRatio = 1.0 - (double)trimPoint / stats.originalSize;
        
        // 6. Validar segurança
        ValidationResult validation = safetyValidator->validate(
            data, trimPoint, romType, options);
        
        if (!validation.isValid) {
            handleValidationFailure(validation, stats);
            return !options.force; // Retorna false apenas se não for forçar
        }
        
        // 7. Executar ação baseada no modo
        return executeFileAction(filePath, data, trimPoint, stats);
        
    } catch (const std::exception& e) {
        handleProcessingError(filePath, e.what(), stats);
        return false;
    }
}

uint8_t RomTrimmer::determinePaddingByte(const std::string& data, RomType romType) {
    if (options.paddingByte != 0) {
        return options.paddingByte;
    }
    return paddingAnalyzer->autoDetectPadding(data, romType);
}

void RomTrimmer::handleValidationFailure(const ValidationResult& validation, 
                                        FileStats& stats) {
    if (!options.force) {
        logger->log(TR("UNSAFE_TRIM") + validation.message, LogLevel::ERROR);
        stats.error = validation.message;
        recordFileStats(stats);
        throw std::runtime_error("Validação falhou");
    } else {
        logger->log(TR("WARNING_FORCING_TRIM") + validation.message, 
                   LogLevel::WARNING);
        stats.warnings.push_back("Forçado: " + validation.message);
    }
}

bool RomTrimmer::executeFileAction(const fs::path& filePath, 
                                  const std::string& data, 
                                  size_t trimPoint, 
                                  FileStats& stats) {
    if (options.analyzeOnly) {
        return handleAnalysisMode(data, trimPoint, stats);
    } 
    else if (options.dryRun) {
        return handleDryRunMode(data, trimPoint, stats);
    } 
    else {
        return handleActualTrim(filePath, data, trimPoint, stats);
    }
}

bool RomTrimmer::handleAnalysisMode(const std::string& data, 
                                   size_t trimPoint, 
                                   FileStats& stats) {
    size_t savedBytes = data.size() - trimPoint;
    double savedPercent = stats.savedRatio * 100;
    
    logger->log(TR("ANALYSIS") + formatBytes(savedBytes) + 
               TR("CAN_BE_REMOVED") + 
               std::to_string(savedPercent) + "%)", 
               LogLevel::INFO);
    
    stats.trimmed = false;
    recordFileStats(stats);
    return true;
}

bool RomTrimmer::handleDryRunMode(const std::string& data, 
                                 size_t trimPoint, 
                                 FileStats& stats) {
    size_t savedBytes = data.size() - trimPoint;
    
    logger->log(TR("SIMULATION_REMOVE") + formatBytes(savedBytes), 
               LogLevel::INFO);
    
    stats.trimmed = false;
    recordFileStats(stats);
    return true;
}

bool RomTrimmer::handleActualTrim(const fs::path& filePath, 
                                 const std::string& data, 
                                 size_t trimPoint, 
                                 FileStats& stats) {
    // Criar backup se necessário
    if (options.backup) {
        createBackup(filePath);
    }
    
    // Escrever arquivo trimado
    if (writeTrimmedFile(filePath, data, trimPoint)) {
        size_t savedBytes = data.size() - trimPoint;
        double savedPercent = stats.savedRatio * 100;
        
        logger->log(TR("TRIM_SUCCESS") + formatBytes(savedBytes) + 
                   " (" + std::to_string(savedPercent) + "%)", 
                   LogLevel::INFO);
        
        stats.trimmed = true;
        filesTrimmed++;
        totalSaved += savedBytes;
        
        recordFileStats(stats);
        return true;
    }
    
    return false;
}

void RomTrimmer::handleProcessingError(const fs::path& filePath, 
                                      const std::string& error, 
                                      FileStats& stats) {
    logger->log(TR("ERROR_PROCESSING") + filePath.string() + ": " + error, 
               LogLevel::ERROR);
    stats.error = error;
    stats.endTime = std::chrono::steady_clock::now();
    stats.duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        stats.endTime - stats.startTime);
    
    std::lock_guard<std::mutex> lock(statsMutex);
    fileStats.push_back(stats);
}

// ==================== OPERAÇÕES DE ARQUIVO ====================
std::string RomTrimmer::readFile(const fs::path& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error(TR("CANNOT_OPEN_FILE") + ": " + filePath.string());
    }
    
    // Obter tamanho
    std::streamsize size = file.tellg();
    if (size <= 0) {
        throw std::runtime_error(TR("EMPTY_FILE"));
    }
    
    // Verificar tamanho máximo (prevenção contra arquivos muito grandes)
    constexpr std::streamsize MAX_FILE_SIZE = 1024 * 1024 * 1024; // 1GB
    if (size > MAX_FILE_SIZE) {
        throw std::runtime_error("Arquivo muito grande (" + 
                                std::to_string(size) + " bytes)");
    }
    
    // Ler conteúdo
    file.seekg(0, std::ios::beg);
    std::string buffer(size, '\0');
    
    if (!file.read(&buffer[0], size)) {
        throw std::runtime_error(TR("ERROR_READING_FILE") + ": " + filePath.string());
    }
    
    return buffer;
}

bool RomTrimmer::writeTrimmedFile(const fs::path& filePath, 
                                 const std::string& data, 
                                 size_t trimPoint) {
    try {
        // Determinar caminho de saída
        fs::path outputPath = determineOutputPath(filePath);
        
        // Verificar se o arquivo de saída já existe
        if (fs::exists(outputPath) && !options.force) {
            logger->log("Arquivo de saída já existe: " + outputPath.string(), 
                       LogLevel::WARNING);
            return false;
        }
        
        // Criar diretório pai se não existir
        fs::create_directories(outputPath.parent_path());
        
        // Escrever arquivo
        std::ofstream outFile(outputPath, std::ios::binary);
        if (!outFile) {
            throw std::runtime_error(TR("CANNOT_CREATE_OUTPUT") + ": " + 
                                    outputPath.string());
        }
        
        outFile.write(data.data(), trimPoint);
        
        // Verificar se a escrita foi bem-sucedida
        if (!outFile.good()) {
            throw std::runtime_error("Falha na escrita do arquivo");
        }
        
        outFile.close();
        
        // Verificar tamanho do arquivo escrito
        if (fs::file_size(outputPath) != trimPoint) {
            throw std::runtime_error("Tamanho do arquivo escrito incorreto");
        }
        
        return true;
        
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string(TR("ERROR_WRITING")) + e.what());
    }
}

fs::path RomTrimmer::determineOutputPath(const fs::path& inputPath) {
    if (!options.outputDir.empty()) {
        return options.outputDir / inputPath.filename();
    }
    return inputPath;
}

void RomTrimmer::createBackup(const fs::path& filePath) const {
    fs::path backupPath = filePath;
    backupPath += ".bak";
    
    try {
        if (fs::exists(backupPath)) {
            logger->log(TR("BACKUP_EXISTS_OVERWRITING") + backupPath.string(), 
                       LogLevel::WARNING);
        }
        
        fs::copy_file(filePath, backupPath, 
                     fs::copy_options::overwrite_existing);
        
        logger->log(TR("BACKUP_CREATED") + backupPath.string(), 
                   LogLevel::DEBUG);
        
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string(TR("BACKUP_FAILED")) + e.what());
    }
}

// ==================== RESUMO E ESTATÍSTICAS ====================
void RomTrimmer::printSummary() const {
    std::cout << "\n" << TR("EXEC_SUMMARY") << "\n";
    std::cout << std::string(40, '=') << "\n\n";
    
    // Estatísticas básicas
    std::cout << TR("FILES_PROCESSED") << ": " << filesProcessed << "\n";
    std::cout << TR("FILES_TRIMMED") << ": " << filesTrimmed << "\n";
    std::cout << TR("FILES_FAILED") << ": " << filesFailed << "\n";
    
    if (totalSaved > 0) {
        std::cout << TR("SPACE_RECOVERED") << ": " << formatBytes(totalSaved) << "\n";
        std::cout << "Média por arquivo: " << formatBytes(totalSaved / filesTrimmed) << "\n";
    }
    
    // Modo de operação
    std::cout << "\nModo de operação: ";
    if (options.analyzeOnly) std::cout << "Análise";
    else if (options.dryRun) std::cout << "Simulação";
    else std::cout << "Execução real";
    std::cout << "\n";
    
    // Detalhes verbosos
    if (options.verbose && !fileStats.empty()) {
        printDetailedSummary();
    }
    
    // Tempo total
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - processingStartTime);
    std::cout << "\nTempo total: " << totalDuration.count() << "ms\n";
}

void RomTrimmer::printDetailedSummary() const {
    std::cout << "\n" << TR("DETAILS_TITLE") << "\n";
    std::cout << std::string(40, '-') << "\n";
    
    for (const auto& stats : fileStats) {
        std::cout << "\n" << stats.path.filename().string() << ":\n";
        
        // Tamanhos
        std::cout << "  " << TR("ORIGINAL_SIZE") << ": " 
                  << formatBytes(stats.originalSize) << "\n";
        std::cout << "  " << TR("FINAL_SIZE") << ": " 
                  << formatBytes(stats.trimmedSize) << "\n";
        
        // Redução
        if (stats.originalSize > 0) {
            double savedPercent = 100.0 * (1.0 - (double)stats.trimmedSize / stats.originalSize);
            std::cout << "  " << TR("REDUCTION") << ": " 
                      << std::fixed << std::setprecision(1) 
                      << savedPercent << "%\n";
        }
        
        // Tipo de ROM
        if (!stats.romType.empty()) {
            std::cout << "  Tipo: " << stats.romType << "\n";
        }
        
        // Duração
        if (stats.duration.count() > 0) {
            std::cout << "  Duração: " << stats.duration.count() << "ms\n";
        }
        
        // Status
        if (stats.trimmed) {
            std::cout << "  ✅ " << TR("SUCCESSFULLY_TRIMMED") << "\n";
        } else if (!stats.error.empty()) {
            std::cout << "  ❌ " << TR("ERROR_LABEL") << ": " << stats.error << "\n";
        } else {
            std::cout << "  ⚠️  " << TR("NO_CHANGES") << "\n";
        }
        
        // Avisos
        for (const auto& warning : stats.warnings) {
            std::cout << "  ⚠️  Aviso: " << warning << "\n";
        }
    }
}

// ==================== UTILITÁRIOS ====================
std::string RomTrimmer::formatBytes(size_t bytes) const {
    if (bytes == 0) return "0 B";
    
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
    return ss.str();
}

std::string RomTrimmer::romTypeToString(RomType type) const {
    switch (type) {
        case RomType::GBA: return "GBA";
        case RomType::NDS: return "NDS";
        case RomType::GB: return "GB";
        case RomType::GBC: return "GBC";
        default: return "Desconhecido";
    }
}

// ==================== MANIPULAÇÃO DE ERROS ====================
void RomTrimmer::handleCriticalError(const std::string& error) {
    std::cerr << "\n❌ " << TR("CRITICAL_ERROR") << ": " << error << "\n";
    
    // Tentar registrar no log
    try {
        logger->log("Erro crítico: " + error, LogLevel::ERROR);
    } catch (...) {
        // Ignorar se o logger também falhar
    }
    
    // Exibir últimos logs se disponíveis
    if (!fileStats.empty()) {
        std::cerr << "\nÚltimos arquivos processados:\n";
        for (size_t i = std::max(0, (int)fileStats.size() - 5); i < fileStats.size(); ++i) {
            std::cerr << "  - " << fileStats[i].path.filename().string();
            if (!fileStats[i].error.empty()) {
                std::cerr << " (ERRO: " << fileStats[i].error << ")";
            }
            std::cerr << "\n";
        }
    }
}

void RomTrimmer::startProcessing() {
    processingStartTime = std::chrono::steady_clock::now();
    logger->log("Iniciando processamento...", LogLevel::INFO);
}

void RomTrimmer::cleanup() {
    // Salvar configurações se necessário
    try {
        configManager->saveConfig();
    } catch (const std::exception& e) {
        logger->log("Não foi possível salvar configurações: " + 
                   std::string(e.what()), LogLevel::WARNING);
    }
    
    // Limpar estatísticas
    fileStats.clear();
    filesProcessed = 0;
    filesTrimmed = 0;
    filesFailed = 0;
    totalSaved = 0;
}

void RomTrimmer::recordFileStats(FileStats& stats) {
    stats.endTime = std::chrono::steady_clock::now();
    stats.duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        stats.endTime - stats.startTime);
    
    std::lock_guard<std::mutex> lock(statsMutex);
    fileStats.push_back(stats);
}

// ==================== HELP ====================
void RomTrimmer::printHelp(const cxxopts::Options& options) const {
    std::cout << "\n" << TR("A_POWERFUL_ROM_TRIMMING_UTILITY") << "\n";
    std::cout << std::string(50, '=') << "\n\n";
    
    std::cout << TR("USAGE") << "\n";
    std::cout << "  romtrimmer++ [OPTIONS] -i <arquivo>\n";
    std::cout << "  romtrimmer++ [OPTIONS] -p <diretório>\n\n";
    
    std::cout << TR("EXAMPLES") << "\n";
    std::cout << "  " << TR("EXAMPLE_TRIM_SINGLE") << "\n";
    std::cout << "  " << TR("EXAMPLE_PROCESS_DIR") << "\n";
    std::cout << "  " << TR("EXAMPLE_ANALYZE_ONLY") << "\n\n";
    
    std::cout << TR("OPTIONS") << "\n";
    std::cout << options.help() << "\n";
    
    std::cout << TR("SUPPORTED_FORMATS") << "\n";
    std::cout << "  GBA (.gba), NDS (.nds), GB (.gb), GBC (.gbc)\n";
    std::cout << "  NES (.nes), SNES (.smc, .sfc), N64 (.n64, .z64, .v64)\n";
    std::cout << "  Binários genéricos (.bin, .rom)\n\n";
    
    std::cout << TR("SAFETY_NOTES") << "\n";
    std::cout << "  • Sempre faça backup de seus arquivos originais\n";
    std::cout << "  • Use --analyze primeiro para verificar o que será feito\n";
    std::cout << "  • Use --dry-run para simular sem alterar arquivos\n";
    std::cout << "  • Use --force apenas se entender os riscos\n\n";
}