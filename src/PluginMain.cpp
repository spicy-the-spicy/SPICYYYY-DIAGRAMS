/**
 * Film Grain Plugin - Premiere Pro Entry Point
 *
 * SDK integration for Adobe Premiere Pro.
 * Handles plugin lifecycle: GLOBAL_SETUP, PARAMS_SETUP, RENDER
 *
 * When PREMIERE_SDK_AVAILABLE is 0, provides a standalone test interface.
 */

#include "GrainEngine.h"
#include "Presets.h"
#include "PluginConfig.h"

#include <memory>
#include <cstring>

#if PREMIERE_SDK_AVAILABLE
// Adobe Premiere Pro SDK headers
#include "PrSDKEffect.h"
#include "PrSDKPixelFormat.h"
#include "PrSDKMALErrors.h"
#include "PrSDKAESupport.h"
#else
// Standalone types for testing without SDK
#include <cstdint>
#include <iostream>
#endif

namespace FilmGrain {

// Global plugin state
static std::unique_ptr<GrainEngine> g_grainEngine;
static bool g_initialized = false;

#if PREMIERE_SDK_AVAILABLE

// ============================================================================
// Premiere Pro SDK Implementation
// ============================================================================

/**
 * Global setup - called when plugin is loaded
 */
static prSuiteError GlobalSetup(
    PF_InData* in_data,
    PF_OutData* out_data)
{
    // Set plugin version
    out_data->my_version = PF_VERSION(
        PLUGIN_VERSION_MAJOR,
        PLUGIN_VERSION_MINOR,
        PLUGIN_VERSION_PATCH,
        PF_Stage_RELEASE,
        0
    );

    // Initialize grain engine
    g_grainEngine = std::make_unique<GrainEngine>();
    g_initialized = true;

    return PF_Err_NONE;
}

/**
 * Global setdown - called when plugin is unloaded
 */
static prSuiteError GlobalSetdown(PF_InData* in_data)
{
    g_grainEngine.reset();
    g_initialized = false;
    return PF_Err_NONE;
}

/**
 * Parameter setup - define UI controls
 */
static prSuiteError ParamsSetup(
    PF_InData* in_data,
    PF_OutData* out_data)
{
    PF_ParamDef param;

    // --- Intensity Slider ---
    AEFX_CLR_STRUCT(param);
    param.param_type = PF_Param_FLOAT_SLIDER;
    param.u.fs_d.value = DEFAULT_INTENSITY;
    param.u.fs_d.dephault = DEFAULT_INTENSITY;
    param.u.fs_d.slider_min = MIN_INTENSITY;
    param.u.fs_d.slider_max = MAX_INTENSITY;
    param.u.fs_d.precision = 1;
    param.u.fs_d.display_flags = PF_ValueDisplayFlag_PERCENT;
    PF_STRCPY(param.name, "Intensity");
    PF_ADD_PARAM(in_data, PARAM_INTENSITY, &param);

    // --- Grain Size Slider ---
    AEFX_CLR_STRUCT(param);
    param.param_type = PF_Param_FLOAT_SLIDER;
    param.u.fs_d.value = DEFAULT_GRAIN_SIZE;
    param.u.fs_d.dephault = DEFAULT_GRAIN_SIZE;
    param.u.fs_d.slider_min = MIN_GRAIN_SIZE;
    param.u.fs_d.slider_max = MAX_GRAIN_SIZE;
    param.u.fs_d.precision = 2;
    param.u.fs_d.display_flags = 0;
    PF_STRCPY(param.name, "Grain Size (px)");
    PF_ADD_PARAM(in_data, PARAM_GRAIN_SIZE, &param);

    // --- Film Stock Popup ---
    AEFX_CLR_STRUCT(param);
    param.param_type = PF_Param_POPUP;
    param.u.pd.value = DEFAULT_FILM_STOCK + 1;  // 1-indexed
    param.u.pd.dephault = DEFAULT_FILM_STOCK + 1;
    param.u.pd.num_choices = FILM_STOCK_COUNT;

    // Build popup menu string
    const char* presetNames[FILM_STOCK_COUNT];
    getAllPresetNames(presetNames);
    char menuStr[256] = "";
    for (int i = 0; i < FILM_STOCK_COUNT; ++i) {
        if (i > 0) strcat(menuStr, "|");
        strcat(menuStr, presetNames[i]);
    }
    PF_STRCPY(param.u.pd.u.namesptr, menuStr);
    PF_STRCPY(param.name, "Film Stock");
    PF_ADD_PARAM(in_data, PARAM_FILM_STOCK, &param);

    // Set parameter count
    out_data->num_params = PARAM_COUNT;

    return PF_Err_NONE;
}

/**
 * Render - apply grain effect to frame
 */
static prSuiteError Render(
    PF_InData* in_data,
    PF_OutData* out_data,
    PF_ParamDef* params[],
    PF_LayerDef* output)
{
    if (!g_initialized || !g_grainEngine) {
        return PF_Err_INTERNAL_STRUCT_DAMAGED;
    }

    // Get input layer
    PF_LayerDef* input = &params[0]->u.ld;

    // Read parameters
    float intensity = params[PARAM_INTENSITY]->u.fs_d.value;
    float grainSize = params[PARAM_GRAIN_SIZE]->u.fs_d.value;
    int filmStock = params[PARAM_FILM_STOCK]->u.pd.value - 1;  // Convert to 0-indexed

    // Build grain parameters
    GrainParams grainParams;
    grainParams.intensity = intensity;
    grainParams.grainSize = grainSize;
    grainParams.filmStock = static_cast<FilmStock>(filmStock);
    grainParams.seed = 42;  // Could be made adjustable
    grainParams.frameNumber = in_data->current_time / in_data->time_step;

    // Determine pixel format
    PixelFormat format;
    switch (input->pix_fmt) {
        case PrPixelFormat_BGRA_4444_8u:
            format = PixelFormat::ARGB_8BIT;
            break;
        case PrPixelFormat_BGRA_4444_16u:
            format = PixelFormat::ARGB_16BIT;
            break;
        case PrPixelFormat_BGRA_4444_32f:
            format = PixelFormat::ARGB_32F;
            break;
        default:
            // Unsupported format - passthrough
            return PF_Err_NONE;
    }

    // Build frame buffers
    FrameBuffer inputBuffer;
    inputBuffer.data = input->data;
    inputBuffer.width = input->width;
    inputBuffer.height = input->height;
    inputBuffer.rowBytes = input->rowbytes;
    inputBuffer.format = format;

    FrameBuffer outputBuffer;
    outputBuffer.data = output->data;
    outputBuffer.width = output->width;
    outputBuffer.height = output->height;
    outputBuffer.rowBytes = output->rowbytes;
    outputBuffer.format = format;

    // Apply grain
    if (!g_grainEngine->applyGrain(inputBuffer, outputBuffer, grainParams)) {
        return PF_Err_INTERNAL_STRUCT_DAMAGED;
    }

    return PF_Err_NONE;
}

/**
 * Handle parameter changes (apply presets)
 */
static prSuiteError HandleChangedParam(
    PF_InData* in_data,
    PF_OutData* out_data,
    PF_ParamDef* params[],
    PF_UserChangedParamExtra* extra)
{
    if (extra->param_index == PARAM_FILM_STOCK) {
        // Film stock changed - apply preset values
        int filmStock = params[PARAM_FILM_STOCK]->u.pd.value - 1;
        float intensity, grainSize;
        applyPreset(static_cast<FilmStock>(filmStock), intensity, grainSize);

        // Update parameter values
        params[PARAM_INTENSITY]->u.fs_d.value = intensity;
        params[PARAM_GRAIN_SIZE]->u.fs_d.value = grainSize;

        // Mark parameters as changed
        params[PARAM_INTENSITY]->uu.change_flags = PF_ChangeFlag_CHANGED_VALUE;
        params[PARAM_GRAIN_SIZE]->uu.change_flags = PF_ChangeFlag_CHANGED_VALUE;
    }

    return PF_Err_NONE;
}

/**
 * Main entry point for Premiere Pro
 */
extern "C" DllExport PF_Err
EffectMain(
    PF_Cmd cmd,
    PF_InData* in_data,
    PF_OutData* out_data,
    PF_ParamDef* params[],
    PF_LayerDef* output,
    void* extra)
{
    PF_Err err = PF_Err_NONE;

    switch (cmd) {
        case PF_Cmd_ABOUT:
            // Could show about dialog
            break;

        case PF_Cmd_GLOBAL_SETUP:
            err = GlobalSetup(in_data, out_data);
            break;

        case PF_Cmd_GLOBAL_SETDOWN:
            err = GlobalSetdown(in_data);
            break;

        case PF_Cmd_PARAMS_SETUP:
            err = ParamsSetup(in_data, out_data);
            break;

        case PF_Cmd_RENDER:
            err = Render(in_data, out_data, params, output);
            break;

        case PF_Cmd_USER_CHANGED_PARAM:
            err = HandleChangedParam(in_data, out_data, params,
                                     static_cast<PF_UserChangedParamExtra*>(extra));
            break;

        default:
            break;
    }

    return err;
}

#else  // !PREMIERE_SDK_AVAILABLE

// ============================================================================
// Standalone Test Implementation
// ============================================================================

/**
 * Initialize the grain engine (standalone mode)
 */
bool initialize() {
    if (g_initialized) {
        return true;
    }

    g_grainEngine = std::make_unique<GrainEngine>();
    g_initialized = true;

    std::cout << "Film Grain Plugin initialized (standalone mode)\n";
    std::cout << "  Version: " << PLUGIN_VERSION_MAJOR << "."
              << PLUGIN_VERSION_MINOR << "." << PLUGIN_VERSION_PATCH << "\n";
    std::cout << "  Threads: " << g_grainEngine->getThreadCount() << "\n";

    return true;
}

/**
 * Shutdown the grain engine
 */
void shutdown() {
    g_grainEngine.reset();
    g_initialized = false;
}

/**
 * Process a frame (standalone mode)
 */
bool processFrame(void* pixelData, int width, int height, int rowBytes,
                  PixelFormat format, float intensity, float grainSize,
                  FilmStock filmStock, int64_t frameNumber) {
    if (!g_initialized || !g_grainEngine) {
        return false;
    }

    FrameBuffer buffer;
    buffer.data = pixelData;
    buffer.width = width;
    buffer.height = height;
    buffer.rowBytes = rowBytes;
    buffer.format = format;

    GrainParams params;
    params.intensity = intensity;
    params.grainSize = grainSize;
    params.filmStock = filmStock;
    params.seed = 42;
    params.frameNumber = frameNumber;

    return g_grainEngine->applyGrain(buffer, buffer, params);
}

/**
 * Test function - processes a synthetic test pattern
 */
void runTest() {
    std::cout << "\n=== Film Grain Plugin Test ===\n\n";

    initialize();

    // Create test image (gradient)
    const int width = 256;
    const int height = 256;
    const int rowBytes = width * 4;
    std::vector<uint8_t> pixels(height * rowBytes);

    // Fill with gradient (dark to light)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = y * rowBytes + x * 4;
            uint8_t val = static_cast<uint8_t>((x + y) / 2);
            pixels[idx] = 255;   // Alpha
            pixels[idx + 1] = val;  // Red
            pixels[idx + 2] = val;  // Green
            pixels[idx + 3] = val;  // Blue
        }
    }

    // Test each preset
    const char* presetNames[FILM_STOCK_COUNT];
    getAllPresetNames(presetNames);

    for (int p = 0; p < FILM_STOCK_COUNT; ++p) {
        FilmStock stock = static_cast<FilmStock>(p);
        const FilmStockPreset& preset = getPreset(stock);

        std::cout << "Testing " << presetNames[p] << ":\n";
        std::cout << "  Intensity: " << (preset.intensity * 100.0f) << "%\n";
        std::cout << "  Grain Size: " << preset.grainSize << "px\n";
        std::cout << "  Shadow Boost: " << preset.shadowBoost << "x\n";

        // Make a copy of test image
        std::vector<uint8_t> testPixels = pixels;

        // Process
        bool success = processFrame(
            testPixels.data(), width, height, rowBytes,
            PixelFormat::ARGB_8BIT,
            preset.intensity * 100.0f,
            preset.grainSize,
            stock,
            0  // Frame 0
        );

        if (success) {
            // Calculate average difference
            double totalDiff = 0.0;
            for (size_t i = 0; i < pixels.size(); ++i) {
                totalDiff += std::abs(static_cast<int>(testPixels[i]) -
                                      static_cast<int>(pixels[i]));
            }
            double avgDiff = totalDiff / pixels.size();
            std::cout << "  Average grain magnitude: " << avgDiff << "\n";
            std::cout << "  Status: OK\n\n";
        } else {
            std::cout << "  Status: FAILED\n\n";
        }
    }

    // Test temporal coherence
    std::cout << "Testing temporal coherence (5 frames):\n";
    std::vector<uint8_t> testPixels = pixels;

    for (int frame = 0; frame < 5; ++frame) {
        processFrame(
            testPixels.data(), width, height, rowBytes,
            PixelFormat::ARGB_8BIT,
            50.0f, 1.2f, FILM_35MM, frame
        );

        // Sample center pixel
        int centerIdx = (height / 2) * rowBytes + (width / 2) * 4;
        std::cout << "  Frame " << frame << ": center pixel R="
                  << static_cast<int>(testPixels[centerIdx + 1]) << "\n";

        // Reset for next frame
        testPixels = pixels;
    }

    std::cout << "\n=== Test Complete ===\n";

    shutdown();
}

// Entry point for standalone testing
int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    runTest();
    return 0;
}

#endif  // PREMIERE_SDK_AVAILABLE

} // namespace FilmGrain

#if !PREMIERE_SDK_AVAILABLE
// C-style main for standalone executable
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return FilmGrain::main(0, nullptr);
}
#endif
