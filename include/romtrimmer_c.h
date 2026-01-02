// romtrimmer_c.h - C interface for library integration
#ifndef ROMTRIMMER_C_H
#define ROMTRIMMER_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Error codes
typedef enum {
    RT_SUCCESS = 0,
    RT_ERROR_INVALID_PARAM,
    RT_ERROR_FILE_NOT_FOUND,
    RT_ERROR_READ_FAILED,
    RT_ERROR_WRITE_FAILED,
    RT_ERROR_UNSUPPORTED_FORMAT,
    RT_ERROR_VALIDATION_FAILED
} rt_error_t;

// ROM type
typedef enum {
    RT_ROM_UNKNOWN = 0,
    RT_ROM_GBA,
    RT_ROM_NDS,
    RT_ROM_GB,
    RT_ROM_GBC,
    RT_ROM_NES,
    RT_ROM_SNES,
    RT_ROM_N64
} rt_rom_type_t;

// Configuration
typedef struct {
    bool create_backup;
    bool force;
    bool analyze_only;
    uint8_t padding_byte;  // 0 for auto
    size_t min_size;
    size_t safety_margin;
    double max_cut_ratio;
} rt_config_t;

// Analysis result
typedef struct {
    bool has_padding;
    size_t original_size;
    size_t trimmed_size;
    size_t padding_bytes;
    double saved_percentage;
    rt_rom_type_t rom_type;
    char rom_type_str[32];
    bool validation_passed;
} rt_analysis_result_t;

// Initialize/finalize
rt_error_t rt_init();
void rt_cleanup();

// Configuration
void rt_set_default_config(rt_config_t* config);
rt_error_t rt_load_config(const char* config_file);

// Core functions
rt_error_t rt_analyze_file(const char* filename, rt_analysis_result_t* result);
rt_error_t rt_trim_file(const char* input_file, const char* output_file, 
                        const rt_config_t* config, rt_analysis_result_t* result);
rt_error_t rt_trim_memory(const uint8_t* data, size_t size, 
                          uint8_t** trimmed_data, size_t* trimmed_size,
                          const rt_config_t* config);

// Batch operations
rt_error_t rt_process_directory(const char* directory, const rt_config_t* config,
                                bool recursive);
rt_error_t rt_process_archive(const char* archive_file, const rt_config_t* config,
                             const char* extract_dir);

// Patch generation (for reverse operation)
rt_error_t rt_generate_patch(const char* original_file, const char* trimmed_file,
                            const char* patch_file);
rt_error_t rt_apply_patch(const char* trimmed_file, const char* patch_file,
                         const char* restored_file);

// Memory management
void rt_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif // ROMTRIMMER_C_H// romtrimmer_c.h - C interface for library integration
#ifndef ROMTRIMMER_C_H
#define ROMTRIMMER_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Error codes
typedef enum {
    RT_SUCCESS = 0,
    RT_ERROR_INVALID_PARAM,
    RT_ERROR_FILE_NOT_FOUND,
    RT_ERROR_READ_FAILED,
    RT_ERROR_WRITE_FAILED,
    RT_ERROR_UNSUPPORTED_FORMAT,
    RT_ERROR_VALIDATION_FAILED
} rt_error_t;

// ROM type
typedef enum {
    RT_ROM_UNKNOWN = 0,
    RT_ROM_GBA,
    RT_ROM_NDS,
    RT_ROM_GB,
    RT_ROM_GBC,
    RT_ROM_NES,
    RT_ROM_SNES,
    RT_ROM_N64
} rt_rom_type_t;

// Configuration
typedef struct {
    bool create_backup;
    bool force;
    bool analyze_only;
    uint8_t padding_byte;  // 0 for auto
    size_t min_size;
    size_t safety_margin;
    double max_cut_ratio;
} rt_config_t;

// Analysis result
typedef struct {
    bool has_padding;
    size_t original_size;
    size_t trimmed_size;
    size_t padding_bytes;
    double saved_percentage;
    rt_rom_type_t rom_type;
    char rom_type_str[32];
    bool validation_passed;
} rt_analysis_result_t;

// Initialize/finalize
rt_error_t rt_init();
void rt_cleanup();

// Configuration
void rt_set_default_config(rt_config_t* config);
rt_error_t rt_load_config(const char* config_file);

// Core functions
rt_error_t rt_analyze_file(const char* filename, rt_analysis_result_t* result);
rt_error_t rt_trim_file(const char* input_file, const char* output_file, 
                        const rt_config_t* config, rt_analysis_result_t* result);
rt_error_t rt_trim_memory(const uint8_t* data, size_t size, 
                          uint8_t** trimmed_data, size_t* trimmed_size,
                          const rt_config_t* config);

// Batch operations
rt_error_t rt_process_directory(const char* directory, const rt_config_t* config,
                                bool recursive);
rt_error_t rt_process_archive(const char* archive_file, const rt_config_t* config,
                             const char* extract_dir);

// Patch generation (for reverse operation)
rt_error_t rt_generate_patch(const char* original_file, const char* trimmed_file,
                            const char* patch_file);
rt_error_t rt_apply_patch(const char* trimmed_file, const char* patch_file,
                         const char* restored_file);

// Memory management
void rt_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif // ROMTRIMMER_C_H
