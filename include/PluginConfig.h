#ifndef PLUGIN_CONFIG_H
#define PLUGIN_CONFIG_H

/**
 * Film Grain Plugin Configuration
 *
 * Central configuration file for plugin constants and settings.
 */

namespace FilmGrain {

// Plugin identification
constexpr const char* PLUGIN_NAME = "Film Grain";
constexpr const char* PLUGIN_MATCH_NAME = "FilmGrainEffect";
constexpr const char* PLUGIN_CATEGORY = "Stylize";
constexpr int PLUGIN_VERSION_MAJOR = 1;
constexpr int PLUGIN_VERSION_MINOR = 0;
constexpr int PLUGIN_VERSION_PATCH = 0;

// Parameter IDs (for Premiere Pro integration)
enum ParamID {
    PARAM_INTENSITY = 1,
    PARAM_GRAIN_SIZE = 2,
    PARAM_FILM_STOCK = 3,
    PARAM_COUNT = 4
};

// Parameter defaults
constexpr float DEFAULT_INTENSITY = 50.0f;      // 50%
constexpr float DEFAULT_GRAIN_SIZE = 1.2f;      // 1.2px (35mm default)
constexpr int DEFAULT_FILM_STOCK = 1;           // 35mm

// Parameter ranges
constexpr float MIN_INTENSITY = 0.0f;
constexpr float MAX_INTENSITY = 100.0f;
constexpr float MIN_GRAIN_SIZE = 0.5f;
constexpr float MAX_GRAIN_SIZE = 3.0f;

// Film stock presets
enum FilmStock {
    FILM_16MM = 0,
    FILM_35MM = 1,
    FILM_65MM = 2,
    FILM_STOCK_COUNT = 3
};

// Performance settings
constexpr int TILE_SIZE = 64;                   // Tile size for multi-threaded processing
constexpr int MAX_THREADS = 16;                 // Maximum worker threads
constexpr float TEMPORAL_BLEND = 0.2f;          // 20% previous frame blend

// Perlin noise settings
constexpr int PERLIN_TABLE_SIZE = 256;
constexpr float PERLIN_FREQUENCY = 0.05f;

} // namespace FilmGrain

#endif // PLUGIN_CONFIG_H
