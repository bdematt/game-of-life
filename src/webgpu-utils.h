#pragma once
#include <webgpu/webgpu.h>

/**
 * Utility function to get a WebGPU adapter synchronously
 */
WGPUAdapter requestAdapterSync(WGPUInstance instance, WGPURequestAdapterOptions const * options);

/**
 * Utility function to get a WebGPU device synchronously
 */
WGPUDevice requestDeviceSync(WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor);

/**
 * Utility function to create a WebGPU surface
 */
WGPUSurface createSurface(WGPUInstance instance);