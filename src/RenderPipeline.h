#pragma once
#include <webgpu/webgpu.h>
#include <memory>
#include "WebGPUContext.h"

class RenderPipeline
{
public:
    RenderPipeline(WebGPUContext& context);
    ~RenderPipeline();

    // Getters
    WGPUShaderModule getCellShaderModule() const { return cellShaderModule; }

    WGPURenderPipeline create();
    
private:
    static const char* SHADER_CODE;
    WGPUShaderModule cellShaderModule = nullptr;

};