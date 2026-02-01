#ifndef GRAIN_ENGINE_H
#define GRAIN_ENGINE_H

/**
 * Film Grain Engine
 *
 * Core grain generation and application logic.
 * Uses Perlin noise for realistic film grain simulation.
 */

#include "PluginConfig.h"
#include "Presets.h"
#include <cstdint>
#include <vector>
#include <mutex>
#include <memory>

namespace FilmGrain {

/**
 * Grain generation parameters
 */
struct GrainParams {
    float intensity;        // 0-100% (normalized internally to 0-1)
    float grainSize;        // 0.5-3.0 pixels
    FilmStock filmStock;    // Preset selection
    uint32_t seed;          // Random seed for noise
    int64_t frameNumber;    // Current frame for temporal coherence
};

/**
 * Pixel formats supported
 */
enum class PixelFormat {
    ARGB_8BIT,      // 8-bit per channel, ARGB order
    ARGB_16BIT,     // 16-bit per channel, ARGB order
    ARGB_32F        // 32-bit float per channel, ARGB order
};

/**
 * Frame buffer descriptor
 */
struct FrameBuffer {
    void* data;             // Pointer to pixel data
    int width;              // Frame width in pixels
    int height;             // Frame height in pixels
    int rowBytes;           // Bytes per row (stride)
    PixelFormat format;     // Pixel format
};

/**
 * Perlin Noise Generator
 *
 * Generates coherent 2D Perlin noise for grain patterns.
 */
class PerlinNoise {
public:
    /**
     * Construct with seed for reproducible noise
     * @param seed Random seed
     */
    explicit PerlinNoise(uint32_t seed = 0);

    /**
     * Get 2D noise value at coordinates
     * @param x X coordinate
     * @param y Y coordinate
     * @return Noise value in range [-1, 1]
     */
    float noise2D(float x, float y) const;

    /**
     * Reseed the noise generator
     * @param seed New random seed
     */
    void reseed(uint32_t seed);

private:
    uint8_t permutation[PERLIN_TABLE_SIZE * 2];

    static float fade(float t);
    static float lerp(float t, float a, float b);
    static float grad(int hash, float x, float y);
};

/**
 * Grain Engine
 *
 * Main class for applying film grain to frames.
 * Handles multi-threaded processing and temporal coherence.
 */
class GrainEngine {
public:
    GrainEngine();
    ~GrainEngine();

    // Prevent copying
    GrainEngine(const GrainEngine&) = delete;
    GrainEngine& operator=(const GrainEngine&) = delete;

    /**
     * Apply grain to a frame
     *
     * @param input Input frame buffer
     * @param output Output frame buffer (can be same as input for in-place)
     * @param params Grain generation parameters
     * @return true on success, false on error
     */
    bool applyGrain(const FrameBuffer& input, FrameBuffer& output,
                    const GrainParams& params);

    /**
     * Reset temporal state
     * Call when switching clips or seeking to non-sequential frame
     */
    void resetTemporalState();

    /**
     * Set number of worker threads
     * @param numThreads Number of threads (0 = auto-detect)
     */
    void setThreadCount(int numThreads);

    /**
     * Get current thread count
     * @return Number of worker threads
     */
    int getThreadCount() const;

private:
    // Processing methods
    void processRegion(const FrameBuffer& input, FrameBuffer& output,
                       const GrainParams& params, const FilmStockPreset& preset,
                       int startY, int endY);

    // 8-bit processing
    void processRow8Bit(const uint8_t* srcRow, uint8_t* dstRow,
                        int width, int y, const GrainParams& params,
                        const FilmStockPreset& preset);

    // 16-bit processing
    void processRow16Bit(const uint16_t* srcRow, uint16_t* dstRow,
                         int width, int y, const GrainParams& params,
                         const FilmStockPreset& preset);

    // Float processing
    void processRowFloat(const float* srcRow, float* dstRow,
                         int width, int y, const GrainParams& params,
                         const FilmStockPreset& preset);

    // Calculate luminance from RGB
    float calculateLuminance(float r, float g, float b) const;

    // Calculate grain intensity based on luminance
    float getLuminanceBasedIntensity(float luminance, const FilmStockPreset& preset) const;

    // Generate grain value for pixel
    float generateGrainValue(int x, int y, const GrainParams& params) const;

    // Apply temporal coherence
    float applyTemporalBlend(float currentGrain, int x, int y);

    // Noise generator
    std::unique_ptr<PerlinNoise> noise_;

    // Temporal coherence buffer
    std::vector<float> previousGrainBuffer_;
    int previousWidth_ = 0;
    int previousHeight_ = 0;
    int64_t previousFrame_ = -1;

    // Threading
    int threadCount_ = 0;
    mutable std::mutex mutex_;

    // Lookup tables for performance
    std::vector<float> luminanceLUT_;
    bool lutInitialized_ = false;

    void initializeLUT();
};

/**
 * Utility functions
 */

// Clamp value to range
template<typename T>
inline T clamp(T value, T min, T max) {
    return (value < min) ? min : (value > max) ? max : value;
}

// Convert 8-bit to float
inline float toFloat8(uint8_t value) {
    return value / 255.0f;
}

// Convert float to 8-bit
inline uint8_t toByte(float value) {
    return static_cast<uint8_t>(clamp(value * 255.0f, 0.0f, 255.0f));
}

// Convert 16-bit to float
inline float toFloat16(uint16_t value) {
    return value / 65535.0f;
}

// Convert float to 16-bit
inline uint16_t toWord(float value) {
    return static_cast<uint16_t>(clamp(value * 65535.0f, 0.0f, 65535.0f));
}

} // namespace FilmGrain

#endif // GRAIN_ENGINE_H
