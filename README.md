 Black Hole Lab

An interactive real-time black-hole visualization written in C++ and OpenGL for macOS. The renderer combines curved-ray integration, an accretion disk, Doppler beaming, optical absorption, relativistic jets, HDR bloom, and interactive camera controls.

## Features

- Interactive camera orbit and zoom
- Sagittarius A* and TON 618 presets
- Kerr-inspired light bending and null-geodesic ray tracing
- Event-horizon capture
- Accretion-disk emission and optical depth
- Doppler beaming and gravitational redshift approximation
- Orbiting turbulent plasma structures
- Bipolar relativistic jets
- HDR `RGBA16F` rendering and separable bloom
- Half-resolution internal rendering for Retina performance
- On-screen controls and FPS monitor

## Requirements

- macOS
- Xcode Command Line Tools
- GLFW installed with Homebrew
- A Mac with OpenGL 3.3 support

Install GLFW if necessary:

```bash
brew install glfw
```

## Build and Run

From the project directory:

```bash
clang++ -std=c++17 -O2 -Wall -Wextra \
  -I/opt/homebrew/include -I/usr/local/include \
  ZLATAN.cpp -o ZLATAN \
  -L/opt/homebrew/lib -L/usr/local/lib \
  -lglfw -framework OpenGL

./ZLATAN
```

For debugging, replace `-O2` with `-g`.

## Controls

- Left mouse drag: orbit the camera
- Mouse wheel: zoom
- `1`: Sagittarius A* preset
- `2`: TON 618 preset
- `[` and `]`: change normalized visual mass scale
- `,` and `.`: decrease or increase spin
- `-` and `=`: decrease or increase disk brightness
- `R`: reset the camera

The HUD displays the active preset, physical mass label, normalized render scale, spin, disk brightness, and FPS.

## Rendering Pipeline

Each pixel starts as a camera ray. The shader traces that ray through the black-hole field and accumulates light from the disk and jets. Rays that reach the event horizon are absorbed. Rays that escape sample a procedural star field.

The display pipeline is:

1. Ray-trace the scene into an HDR `RGBA16F` texture.
2. Extract bright emission and blur it horizontally.
3. Blur the result vertically.
4. Combine the bloom with the HDR scene.
5. Apply ACES-style tone mapping and gamma correction.
6. Upscale the internal render to the window.

The internal render target is 75% of the framebuffer dimensions to reduce Retina GPU cost while preserving detail.

## Scientific Model

The renderer uses dimensionless geometric units for the ray calculation. The event-horizon scale is normalized for display, while the presets expose astrophysical reference values:

- Sagittarius A*: approximately `4.3 million solar masses`
- TON 618: approximately `66 billion solar masses`

The Schwarzschild radius is:

```text
r_s = 2 G M / c^2
```

The displayed preset mass and the normalized visual scale are intentionally separate. This allows both objects to fit in one interactive view without falsely claiming that their screen sizes use the literal astronomical ratio.

The current implementation is a real-time scientific visualization approximation. Disk radiation, jet emission, optical depth, and spin effects are procedural. The spin path uses Kerr-inspired equations and frame-dragging behavior, but it is not a validated full radiation-transfer simulation. This distinction should be stated during judging.

## Competition Presentation

A strong demonstration should follow this order:

1. Start with Sagittarius A* and explain the event-horizon shadow.
2. Orbit the camera to show the disk and lensed background.
3. Switch to TON 618 and show the brighter disk and jets.
4. Change spin and disk brightness live.
5. Point out the FPS value and explain the half-resolution/HDR pipeline.
6. Explain which effects are physically modeled and which are visual approximations.

Recommended submission material:

- A short screen recording showing both presets
- A screenshot of the HUD and black-hole scene
- A one-page explanation of the ray equations
- A performance note with the observed FPS and Mac model
- A build command that judges can run directly

## Project Structure

The current competition prototype is intentionally self-contained:

- `ZLATAN.cpp`: OpenGL setup, UI, shaders, ray tracer, HDR pipeline, and controls
- `README.md`: build, usage, rendering, and scientific notes

For a larger final version, shaders and renderer components can be separated into individual files after the visual result is stable.
