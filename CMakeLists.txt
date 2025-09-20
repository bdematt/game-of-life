cmake_minimum_required(VERSION 4.0)
project(index)

if(EMSCRIPTEN)
    set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(COMPILE_WARNING_AS_ERROR ON)

# Your executable
add_executable(
    index
    src/main.cpp
    src/webgpu-utils.cpp
    src/Life.cpp
    src/WebGPUContext.cpp
    src/RenderPipeline.cpp
    src/ComputePipeline.cpp
    src/Geometry.cpp
)

# Create dist directory for web assets
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/dist)

# Emscripten-specific settings
set_target_properties(index PROPERTIES
    SUFFIX ".html"
    LINK_FLAGS "-s WASM=1 -s USE_WEBGL2=1 --shell-file ${CMAKE_SOURCE_DIR}/src/index.html"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist
)

# Use the modern emdawnwebgpu port instead of the deprecated -sUSE_WEBGPU
set(emdawnwebgpu_flags "--use-port=emdawnwebgpu")

# Add compile options for emdawnwebgpu
target_compile_options(index PRIVATE
    ${emdawnwebgpu_flags}
)

# Emscripten link options with modern emdawnwebgpu port
target_link_options(index PRIVATE
    ${emdawnwebgpu_flags}          # Use modern emdawnwebgpu port
    -sASYNCIFY=1                   # Required for async WebGPU operations
    -sEXPORTED_FUNCTIONS=['_main'] # Export main function
    -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']
    -sALLOW_MEMORY_GROWTH=1        # Allow memory growth
    -sINITIAL_MEMORY=67108864      # 64MB initial memory
    -sMAXIMUM_MEMORY=134217728     # 128MB max memory
    -O2                            # Optimize for performance
)