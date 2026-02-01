# Film Grain Plugin for Adobe Premiere Pro

A CPU-based film grain effect plugin that adds realistic film grain to your footage using Perlin noise generation.

## Features

- **Perlin noise-based grain** - Organic, realistic film grain patterns
- **Luminance-adaptive intensity** - More grain in shadows, less in highlights (like real film)
- **3 Film stock presets** - 16mm, 35mm, and 65mm film characteristics
- **Temporal coherence** - Smooth grain flow between frames (no flickering)
- **Multi-threaded processing** - Efficient CPU utilization

## Installation

### Requirements

- Adobe Premiere Pro 2020 or later
- Windows 10/11 or macOS 10.15+
- CMake 3.16+ (for building from source)
- C++17 compatible compiler

### Building from Source

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd FilmGrainPlugin
   ```

2. Configure with CMake:
   ```bash
   mkdir build && cd build
   cmake .. -DADOBE_PREMIERE_SDK_PATH=/path/to/premiere/sdk
   ```

3. Build:
   ```bash
   cmake --build . --config Release
   ```

4. Install the plugin:
   - **Windows**: Copy `FilmGrainPlugin.prm` to:
     `C:\Program Files\Adobe\Common\Plug-ins\7.0\MediaCore\`
   - **macOS**: Copy `FilmGrainPlugin.plugin` to:
     `/Library/Application Support/Adobe/Common/Plug-ins/7.0/MediaCore/`

### Standalone Testing (without SDK)

Build without the SDK path to create a test executable:
```bash
mkdir build && cd build
cmake ..
cmake --build .
./libFilmGrainPlugin.so  # or FilmGrainPlugin.exe on Windows
```

## Usage

1. In Premiere Pro, apply the "Film Grain" effect from the **Stylize** category
2. Adjust the parameters:
   - **Intensity**: Control the overall grain strength (0-100%)
   - **Grain Size**: Adjust the size of grain particles (0.5-3.0 pixels)
   - **Film Stock**: Select a preset or use custom values

## Parameters

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Intensity | 0-100% | 50% | Overall grain strength |
| Grain Size | 0.5-3.0px | 1.2px | Size of grain particles |
| Film Stock | Dropdown | 35mm | Preset selection |

## Film Stock Presets

### 16mm Film
- Heavy, gritty grain with large particles
- Intensity: 80%, Size: 2.0px
- Best for: Vintage, documentary, indie film look

### 35mm Film (Default)
- Classic cinema grain, balanced visibility
- Intensity: 50%, Size: 1.2px
- Best for: General film look, narrative content

### 65mm Film
- Fine, subtle grain
- Intensity: 30%, Size: 0.7px
- Best for: High-end cinema, epic visuals

## Technical Details

### Grain Generation

The plugin uses 2D Perlin noise to generate organic grain patterns:

1. **Base noise** - Primary Perlin noise layer
2. **Detail layer** - Secondary noise at 2x frequency for added texture
3. **Luminance mapping** - Grain intensity varies based on pixel brightness:
   - Shadows (< 30% luma): Boosted grain (up to 2.5x)
   - Midtones (30-70% luma): Normal grain
   - Highlights (> 70% luma): Reduced grain (0.3-0.7x)

### Temporal Coherence

To prevent flickering, 20% of the previous frame's grain pattern is blended with the current frame. This creates smooth temporal transitions while maintaining natural variation.

### Performance

Target performance (CPU multi-threaded):
- 1080p: < 100ms per frame
- 4K: < 400ms per frame

Performance scales with available CPU cores up to 16 threads.

## Supported Formats

- 8-bit ARGB (standard)
- 16-bit ARGB (high bit depth)
- 32-bit float ARGB (HDR)

## Project Structure

```
FilmGrainPlugin/
├── src/
│   ├── PluginMain.cpp     # SDK integration & entry point
│   ├── GrainEngine.cpp    # Core grain generation
│   └── Presets.cpp        # Film stock preset data
├── include/
│   ├── PluginConfig.h     # Configuration constants
│   ├── GrainEngine.h      # Engine interface
│   └── Presets.h          # Preset interface
├── resources/             # Plugin resources
├── CMakeLists.txt         # Build configuration
└── README.md              # This file
```

## Troubleshooting

### Plugin doesn't appear in Premiere Pro
- Verify the plugin file is in the correct directory
- Check Premiere Pro's plugin loading log for errors
- Ensure you're using a compatible Premiere Pro version

### Grain looks different than expected
- Check the Film Stock preset selection
- Adjust Intensity and Grain Size for your footage
- Darker footage will naturally show more grain

### Performance issues
- Close other resource-intensive applications
- Consider rendering to preview for real-time playback
- Lower resolution previews will process faster

## License

[Your License Here]

## Version History

### 1.0.0
- Initial release
- Perlin noise grain generation
- 3 film stock presets (16mm, 35mm, 65mm)
- Luminance-adaptive intensity
- Temporal coherence
- Multi-threaded CPU processing
