#ifndef PRESETS_H
#define PRESETS_H

/**
 * Film Stock Presets
 *
 * Hardcoded presets for different film stock characteristics.
 * Each preset defines grain intensity, size, and shadow boost values.
 */

#include "PluginConfig.h"

namespace FilmGrain {

/**
 * Film stock preset data structure
 */
struct FilmStockPreset {
    const char* name;           // Display name
    float intensity;            // Base grain intensity (0.0 - 1.0)
    float grainSize;            // Grain size in pixels
    float shadowBoost;          // Multiplier for grain in shadows
    float highlightReduction;   // Reduction factor for grain in highlights
    float colorVariance;        // Color channel variance (simplified)
};

/**
 * Get preset by film stock type
 * @param stock The film stock enum value
 * @return Reference to the preset data
 */
const FilmStockPreset& getPreset(FilmStock stock);

/**
 * Get preset name for UI display
 * @param stock The film stock enum value
 * @return Preset name string
 */
const char* getPresetName(FilmStock stock);

/**
 * Get all preset names for UI dropdown
 * @param names Output array of names (must be FILM_STOCK_COUNT size)
 */
void getAllPresetNames(const char** names);

/**
 * Apply preset to parameters
 * Modifies the given intensity and grain size based on preset
 * @param stock The film stock to apply
 * @param intensity Output intensity value
 * @param grainSize Output grain size value
 */
void applyPreset(FilmStock stock, float& intensity, float& grainSize);

} // namespace FilmGrain

#endif // PRESETS_H
