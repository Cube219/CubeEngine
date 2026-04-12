# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

CubeEngine is a cross-platform graphics rendering engine written in C++20 with support for multiple graphics APIs (DirectX 12 on Windows, Metal on macOS). The architecture uses a modular design with runtime-loaded graphics backends, platform abstraction layers, and a reflection-based shader parameter system.

## Build System

### CMake Configuration

Configure and build with CMake:

```bash
# Configure (Debug build)
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Configure (Release build)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Build specific configuration on Windows
cmake --build build --config Debug
cmake --build build --config Release
```

Build outputs are placed in `Binaries/{BuildType}/`.

### Platform-Specific Build Options

CMake options control which graphics backends are compiled:

- `CUBE_GAPI_ENABLE_DX12` - Enable DirectX 12 backend (Windows only, default ON)
- `CUBE_GAPI_ENABLE_METAL` - Enable Metal backend (macOS only, default ON)

Platform detection is automatic:
- Windows: Sets `PLATFORM_WINDOWS` and `CUBE_PLATFORM_WINDOWS` definition
- macOS: Sets `PLATFORM_MACOS`, `CUBE_PLATFORM_MACOS` definition, and `CMAKE_OSX_DEPLOYMENT_TARGET=14.0`

### Code Formatting

The project uses clang-format with a custom configuration (`.clang-format`):

```bash
# Format a single file
clang-format -i path/to/file.cpp

# Format all source files
find Source -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

Key formatting rules:
- BasedOnStyle: LLVM
- IndentWidth: 4 spaces
- BreakBeforeBraces: Custom (Allman-style)
- PointerAlignment: Left
- NamespaceIndentation: All

## Run the app for testing

Transfer this command line parameter in each run if you want to check the program run correctly:

`--test --model=gltf_1`

`--test --model=default_1`

### Testing process (Windows)

```bash
# Run the app from the Binaries directory, wait 60s for rendering, then close via taskkill
cd Binaries/Debug/Debug
./CE-Main.exe <parameter> &
sleep 3
WIN_PID=$(ps | grep CE-Main | awk '{print $4}')
sleep 57
taskkill //PID $WIN_PID
```

- Must run from the `Binaries/Debug/Debug` directory (running from the project root has a path resolution issue).
- Use `ps | grep CE-Main | awk '{print $4}'` to get the Windows PID from Git Bash (`$!` returns a Bash PID, not a Windows PID).
- Use `taskkill //PID <WIN_PID>` to send WM_CLOSE (graceful shutdown).
- Exit code 0 means clean shutdown. Check logs for errors.

### Testing process (macOS)

```bash
# Run the app in background, wait 60s for rendering, then quit via osascript
MTL_DEBUG_LAYER=1 <path-to>/CE-Main.app/Contents/MacOS/CE-Main <parameter> &
APP_PID=$!
sleep 60
osascript -e 'tell application "CE-Main" to quit'
wait $APP_PID 2>/dev/null
```

- Use `osascript` to send the quit command (equivalent to Cmd+Q).
- Exit code 0 means clean shutdown. Check logs for errors.

### Test mode behavior

Test mode (`--test`) enables automatic testing by modifying platform behavior:
- **No logger window**: Debug console/logger window is not created.
- **No blocking UI**: Debug message boxes/alerts are suppressed.
- **Window not focused**: Main window is sent to the back (not hidden).
- **Debugger ignored**: `IsDebuggerAttached()` returns false to avoid debug breaks.
- **Force terminate on CHECK failure**: Calls `exit(3)` immediately.
- **No shutdown wait**: Skips "press any key" prompts on shutdown.

## Git Commit Conventions
- Always prefix commit message with `AGENT: `

## Other Coding Conventions
- Not use auto keyword except the length of type name is long. (e.g. iterator in STL container...)
- Use redefined types instead of STL. (Defined in Base/Public/Types.h)
- Always use braces({}) even one line.
- Always add virtual keyword at prefix of virtual function.
- Always add index in string format ({0} {1}...).

## Architecture

### Module Hierarchy

The codebase is organized into layered modules with clear dependency relationships:

```
CE-Main (executable)
    ↓
CE-Core (rendering engine)
    ↓
CE-GAPI (abstract graphics API)
    ├── CE-GAPI_DX12 (DirectX 12 backend)
    └── CE-GAPI_Metal (Metal backend)
    ↓
CE-Platform (OS abstraction)
    ↓
CE-Base (foundation) + CE-Logger
```

**CE-Base** (`Source/Base/`): Foundation library providing math (Vector, Matrix), types (SharedPtr, UniquePtr, CubeString), utilities (Event, Flags, Allocator), and platform defines.

**CE-Logger** (`Source/Logger/`): Logging and assertion framework used by all modules.

**CE-Platform** (`Source/Platform/`): Platform abstraction for Window management, input handling (keyboard/mouse), memory allocation, dynamic library loading (DLib), and cursor control. Has platform-specific implementations in `Public/Windows/` and `Public/MacOS/`.

**CE-GAPI** (`Source/GraphicsAPI/GAPI/`): Abstract graphics API interface defining Buffer, Texture, CommandList, Fence, Pipeline, Sampler, Shader, SwapChain, etc.

**CE-GAPI_Base** (`Source/GraphicsAPI/GAPI_Base/`): Shared backend utilities including SlangHelper (shader compilation) and ShaderParameterReflection (constant buffer introspection).

**CE-GAPI_DX12** (`Source/GraphicsAPI/GAPI_DX12/`): Complete DirectX 12 implementation with DX12Device, DX12CommandListManager, DX12DescriptorManager, DX12MemoryAllocator, etc.

**CE-GAPI_Metal** (`Source/GraphicsAPI/GAPI_Metal/`): Metal API implementation with MetalDevice, MetalShaderCompiler, MetalTimestampManager, etc. Written in Objective-C++ (.mm files) with ARC enabled.

**CE-Core** (`Source/Core/`): High-level rendering engine with Engine (lifecycle), Renderer (rendering pipeline), resource managers (ShaderManager, TextureManager, SamplerManager), rendering components (Material, Mesh, RenderObject), and support systems (CameraSystem, ModelLoaderSystem, StatsSystem).

**CE-Main** (`Source/Main/`): Entry point executable with platform-specific main functions (WinMain for Windows, main for macOS).

### Graphics API Abstraction (GAPI)

The GAPI system uses a **polymorphic plugin architecture** with runtime backend selection:

1. **Dynamic Loading**: Renderer loads GAPI backend via `Platform::LoadDLib()` at runtime
2. **Factory Pattern**: Each backend exports a `CreateGAPI()` function
3. **Backend Selection**: Determined by `GAPIName` enum (DX12 or Metal)

```cpp
// Renderer.cpp snippet
if (gAPIName == GAPIName::DX12)
    dLibName = "CE-GAPI_DX12.dll";  // Windows
else if (gAPIName == GAPIName::Metal)
    dLibName = "CE-GAPI_Metal.dylib";  // macOS

auto createGAPIFunc = reinterpret_cast<CreateGAPIFunction>(
    mGAPI_DLib->GetFunction("CreateGAPI")
);
mGAPI = SharedPtr<GAPI>(createGAPIFunc());
```

This design allows compiling both backends and selecting at runtime, or omitting unused backends entirely.

### Platform Abstraction

Platform code uses a **base class + using alias** pattern:

- `Public/Platform.h` defines `BasePlatform` with static interface functions (all with `NOT_IMPLEMENTED()` bodies)
- Platform-specific classes (e.g., `MacOSPlatform`, `WindowsPlatform`) inherit from the base class and hide base methods with their own implementations
- Platform headers define `using Platform = MacOSPlatform;` (or `WindowsPlatform`), selected via `#if defined(CUBE_PLATFORM_MACOS)` / `CUBE_PLATFORM_WINDOWS` at the bottom of `Public/Platform.h`
- The same pattern applies to other platform classes: `BaseDLib`/`DLib`, `BaseDebug`/`Debug`, `BaseFileSystem`/`FileSystem`
- Platform-specific code in `Public/Windows/` (C++) and `Public/MacOS/` (Objective-C++)
- **ODR padding for macOS classes with member variables**: When a macOS platform class holds Objective-C types (e.g., `NSString*`), those members are only visible under `#ifdef __OBJC__`. To prevent ODR (One Definition Rule) violations, a matching `void*` padding member must be provided in the `#else` branch so the class layout is identical in both Objective-C++ and plain C++ translation units. See `MacOSFilePath` for an example.

Key differences:
- **Windows**: Event loop in main thread, supports DX12, uses Win32 APIs
- **macOS**: Event loop in system thread (init/shutdown deferred), supports Metal, uses AppKit/Cocoa, requires ARC for Objective-C++

The `EngineInitializeInfo.runInitializeAndShutdownInLoopFunction` flag handles the threading difference.

### Shader Parameter System

CubeEngine uses **compile-time reflection** for type-safe constant buffer binding:

```cpp
CUBE_BEGIN_SHADER_PARAMETER_LIST(MyParameterList)
    CUBE_SHADER_PARAMETER(Matrix4x4, worldMatrix)
    CUBE_SHADER_PARAMETER(Vector4, color)
CUBE_END_SHADER_PARAMETER_LIST
```

This macro DSL generates:
1. Struct with typed fields
2. Reflection metadata (offsets, sizes, names)
3. Helper functions for GPU buffer binding

The `ShaderParameterListManager` pools constant buffers with multi-buffering (configurable `numGPUSync`) to avoid CPU/GPU sync stalls.

### Rendering Pipeline Flow

```
1. Engine::StartLoop()
2. Platform event loop starts
3. Engine::OnLoop() called each frame
4. Renderer::RenderAndPresent()
   ├→ GAPI::BeginRenderingFrame()
   ├→ GAPI::OnBeforeRender()
   ├→ Record commands (set render targets, pipelines, draw calls)
   ├→ GAPI::OnAfterRender()
   ├→ GAPI::OnBeforePresent()
   ├→ CommandList::Submit()
   ├→ GAPI::OnAfterPresent()
   └→ GAPI::EndRenderingFrame()
5. Platform::Shutdown() on window close
```

## Key Implementation Details

### Memory Management

- Use `SharedPtr<T>` and `UniquePtr<T>` from `Source/Base/Public/EngineBase.h` instead of raw pointers
- Platform-specific aligned allocation via `Platform::AllocAligned()` and `Platform::FreeAligned()`
- DX12 backend uses D3D12MemoryAllocator for GPU memory
- Metal backend uses automatic Metal resource management

### Precompiled Headers

Most modules use precompiled headers for faster builds, except:
- Platform module (macOS Objective-C++ incompatibility)
- GAPI_Metal module (Objective-C++ incompatibility)

### Platform-Specific Code

When adding platform-specific functionality:
1. Add platform check: `#if defined(CUBE_PLATFORM_WINDOWS)` or `#if defined(CUBE_PLATFORM_MACOS)`
2. For Platform module: implement in `Public/Windows/WindowsPlatform_*.cpp` or `Public/MacOS/MacOSPlatform_*.mm`
3. For macOS Objective-C++ files: use `.mm` extension and ensure ARC is enabled

### Shader Development

Shaders are written in Slang (HLSL-like syntax) in `Resources/Shaders/`:

- `Main.slang` - Primary vertex/pixel shaders
- `GenerateMipmaps.slang` - Mipmap generation compute shader
- `Light.slang` - Lighting calculations
- `Material.slang` - Material parameter definitions

The Slang compiler (via `SlangHelper`) cross-compiles to:
- DXIL for DirectX 12
- MSL (Metal Shading Language) for Metal

Shader compilation happens at runtime with caching via ShaderManager.

### Resource Loading

Model assets are not checked into the repository. Use provided scripts to fetch sample assets:

```bash
# From Resources/Models directory
./FetchDefaultModels.sh      # Fetch default models
./FetchglTFSampleAssets.sh   # Fetch glTF sample assets
```

Models are loaded via:
- tinygltf for glTF 2.0 format
- tinyobjloader for OBJ format

### Debugging

**Windows (DX12)**:
- Use PIX for Windows for GPU debugging
- WinPixEventRuntime is linked for GPU event markers
- Visual Studio natvis file: `Resources/natvis/CubeEngine.natvis`

**macOS (Metal)**:
- Use Xcode's Metal Debugger
- In Debug builds, `MTL_CAPTURE_ENABLED=1` is set for GPU frame capture
- Metal validation layer enabled in Debug builds

### Important Constraints

1. **macOS Thread Model**: Engine initialization and shutdown must occur in the platform loop thread. This is automatically handled when `runInitializeAndShutdownInLoopFunction=true`.

2. **Graphics API Availability**: DX12 backend only compiles on Windows; Metal backend only compiles on macOS. The build system handles this automatically.

3. **Shader Reflection**: The shader parameter system requires matching struct layouts between CPU (C++) and GPU (shader). Use the `CUBE_SHADER_PARAMETER` macros to ensure alignment.

4. **Multi-GPU Synchronization**: The `numGPUSync` parameter (typically 2-3) controls resource buffering. Increase if encountering GPU stalls; decrease for lower memory usage.

5. **ARC Requirement**: All Objective-C++ code in the macOS platform and Metal backend must be ARC-compatible. Don't manually retain/release Objective-C objects.

## Common File Locations

| Purpose | Path |
|---------|------|
| Main entry point | `Source/Main/main.cpp` |
| Engine interface | `Source/Core/Public/Engine.h` |
| Renderer implementation | `Source/Core/Private/Renderer/Renderer.h` |
| GAPI interface | `Source/GraphicsAPI/GAPI/Public/GAPI.h` |
| Platform interface | `Source/Platform/Public/Platform.h` |
| Math types | `Source/Base/Public/Math/` |
| Shader parameters DSL | `Source/Core/Public/Renderer/ShaderParameter.h` |
| DX12 implementation | `Source/GraphicsAPI/GAPI_DX12/Private/` |
| Metal implementation | `Source/GraphicsAPI/GAPI_Metal/Private/` |
| Shaders | `Resources/Shaders/` |
