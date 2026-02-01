/**
 * Film Stock Presets Implementation
 *
 * Hardcoded presets for 16mm, 35mm, and 65mm film stocks.
 * These values are based on typical film grain characteristics.
 */

#include "Presets.h"

namespace FilmGrain {

// Preset data table
static const FilmStockPreset PRESETS[FILM_STOCK_COUNT] = {
    // 16mm - Heavy grain, large particles
    // Characteristic: Gritty, vintage look with pronounced grain structure
    {
        "16mm Film",        // name
        0.8f,               // intensity (80%)
        2.0f,               // grainSize (2.0px)
        2.5f,               // shadowBoost
        0.3f,               // highlightReduction
        0.15f               // colorVariance
    },

    // 35mm - Medium grain, balanced
    // Characteristic: Classic cinema look, balanced grain visibility
    {
        "35mm Film",        // name
        0.5f,               // intensity (50%)
        1.2f,               // grainSize (1.2px)
        1.8f,               // shadowBoost
        0.5f,               // highlightReduction
        0.10f               // colorVariance
    },

    // 65mm - Fine grain, subtle
    // Characteristic: High-end cinema look, fine detail preservation
    {
        "65mm Film",        // name
        0.3f,               // intensity (30%)
        0.7f,               // grainSize (0.7px)
        1.3f,               // shadowBoost
        0.7f,               // highlightReduction
        0.05f               // colorVariance
    }
};

const FilmStockPreset& getPreset(FilmStock stock) {
    // Bounds check
    int index = static_cast<int>(stock);
    if (index < 0 || index >= FILM_STOCK_COUNT) {
        // Default to 35mm if invalid
        return PRESETS[FILM_35MM];
    }
    return PRESETS[index];
}

const char* getPresetName(FilmStock stock) {
    return getPreset(stock).name;
}

void getAllPresetNames(const char** names) {
    if (!names) return;

    for (int i = 0; i < FILM_STOCK_COUNT; ++i) {
        names[i] = PRESETS[i].name;
    }
}

void applyPreset(FilmStock stock, float& intensity, float& grainSize) {
    const FilmStockPreset& preset = getPreset(stock);
    intensity = preset.intensity * 100.0f;  // Convert to percentage
    grainSize = preset.grainSize;
}

} // namespace FilmGrain
