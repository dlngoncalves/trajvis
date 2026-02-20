# TrajVis Modernization Review

## Historical context and constraints
This codebase reflects a practical OpenGL prototype era, likely tuned for macOS/OpenGL 4.1 constraints and single-developer velocity. The architecture favors direct control over abstraction, and many comments indicate iterative experimentation over polished framework design.

## Clarity and maintainability overview

### What is clear and positive
- The application flow is easy to follow from `main.cpp`: context setup, shader setup, trajectory load, map creation, render loop, and per-frame input/updates.
- Data and rendering responsibilities are conceptually split across `TrajParser`, `Map`, and shader files.
- Coordinate conversion logic is explicit and includes useful domain comments.

### What reduces clarity today
- `main.cpp` carries too many responsibilities (input, camera, shader wiring, zoom policy, map updates, trajectory rendering, mode switches).
- Significant commented-out code and exploratory notes make intent harder to separate from legacy experiments.
- Mixed abstraction depth: some wrappers (`GLSLShader`, `gl_utils`) coexist with raw OpenGL calls scattered across business logic classes.
- Placeholder/legacy classes (`Renderer`, `Trajectory`, `InputManager`) are not integrated, which creates cognitive noise for newcomers.

## Code quality overview

### Strengths
- The project has a functioning end-to-end vertical slice: ingest trajectory -> augment data -> upload GPU buffers -> render with map and custom shaders.
- Shader stages are deliberately organized by pipeline role (trajectory vs map), including tessellation for map terrain and geometry expansion for trajectory strips.

### Risks / technical debt
- Hard-coded external API tokens and endpoints in source (`Mapbox`, `Dark Sky`) are fragile and insecure for modern distribution.
- Runtime data dependencies are implicit (`trajectories/`, `images/`, `trajectories.db`) and not surfaced as config.
- Error handling is sparse around network/database failures and file IO edge cases.
- Resource lifetime management is manual and partial (limited RAII; potential leaks or repeated allocations during scale reset/re-buffering).
- Some loops copy larger objects unnecessarily (e.g., trajectory iteration by value in render loop).

## Efficiency overview

### What was reasonable at the time
- For a prototype and moderate trajectory count, per-tile draw calls and per-trajectory draw calls are understandable trade-offs for flexibility.
- Geometry and tessellation shader usage aligns with OpenGL-era techniques for line thickening and terrain displacement.

### Bottlenecks in a modern context
- Frequent CPU-side map/trajectory rescaling and full buffer re-uploads on zoom changes can become expensive.
- Map tiles are drawn with many state changes (texture binds + uniforms + draw per tile).
- Network tile/height fetch is synchronous-style and tightly coupled to rendering lifecycle.
- Weather enrichment is effectively one-point-per-trajectory and then broadcast, limiting visual fidelity while still incurring API complexity.

## Modern cross-platform redesign suggestions

### 1) Architecture and modularity
- Split into modules/services:
  - `core` (trajectory domain model, projections, stats)
  - `data` (trajectory loaders, cache, sqlite abstraction)
  - `map` (tile source, cache policy, reprojection)
  - `render` (backend-agnostic scene graph/commands)
  - `platform` (window/input/events)
- Move from monolithic `main.cpp` to an `App` + `Scene` + `Systems` style update loop.

### 2) Rendering backend strategy
- Replace direct OpenGL dependency with a backend abstraction and target:
  - Vulkan/Metal/D3D12 via **bgfx** or **wgpu**/**WebGPU-native** style abstraction, or
  - Keep OpenGL as fallback backend where necessary.
- Use instancing and texture arrays/atlases for tile rendering.
- Batch trajectories into fewer draw calls where possible (or use compute/preprocess for widths/colors).
- Consider screen-space polyline techniques for robust line thickness and joins.

### 3) Data pipeline and async streaming
- Introduce async task system for:
  - tile fetch/decode,
  - weather fetch,
  - DB reads/writes,
  - geometry/buffer preparation.
- Implement cache layers with explicit TTL/versioning.
- Decouple “what to load” from “what to render this frame” via queues.

### 4) Security, configuration, and deployability
- Move API keys and endpoints into environment/config files.
- Add startup config (paths, tile provider, cache dirs, default zoom, render modes).
- Provide deterministic offline mode using local MBTiles/PMTiles + precomputed terrain assets.

### 5) Quality and testing
- Add projection/math unit tests (lat/lon <-> tile conversions, speed/time calculations).
- Add parser regression tests for supported trajectory formats.
- Add GPU validation/debug modes and frame capture docs.
- Add formatting/linting and CI for Linux/macOS/Windows.

### 6) Visual fidelity and UX
- Replace single-sample weather-per-trajectory with sampled temporal interpolation along points.
- Add UI overlays (legend, mode, FPS, cache status, zoom level).
- Add camera modes: free-fly, track-follow, top-down stabilized map mode.

## Suggested migration plan
1. **Stabilize** current code (config extraction, API key removal, basic tests, logging).
2. **Refactor boundaries** without changing visuals (extract app systems and services).
3. **Introduce async data streaming** and cache abstraction.
4. **Port renderer** behind backend interface (OpenGL first, second backend next).
5. **Optimize draw path** (instancing/batching + modern polyline strategy).
6. **Expand product features** (UI, richer analytics, playback controls, export).
