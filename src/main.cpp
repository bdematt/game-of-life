#include <emscripten.h>
#include <emscripten/html5.h>
#include <iostream>
#include <webgpu/webgpu.h>
#define WEBGPU_CPP_IMPLEMENTATION
#include "webgpu.hpp"

void main_loop() {
}

int main() {

    wgpu::Instance instance {};

    wgpu::RequestAdapterOptions adapterOptions {};
    adapterOptions.setDefault();
    wgpu::Adapter adapter = instance.requestAdapter(adapterOptions);

    wgpu::DeviceDescriptor deviceDesc {};
    deviceDesc.setDefault();
    wgpu::Device device = adapter.requestDevice(deviceDesc);

    wgpu::SurfaceDescriptor surfaceDesc {};
    surfaceDesc.setDefault();
    wgpu::Surface surface = instance.createSurface(surfaceDesc);

    std::cout << "🚀 Starting WebGPU application..." << std::endl;


    emscripten_set_main_loop(main_loop, 0, 1);
    return 0;
}