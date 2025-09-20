#pragma once

#include <webgpu/webgpu.h>
#include <memory>
#include "WebGPUContext.h"
#include "Geometry.h"
#include "RenderPipeline.h"

class Life
{
public:

    class InitializationError : public std::runtime_error {
        public:
            InitializationError(const std::string& msg) 
                : std::runtime_error("Initialization failed: " + msg) {}
    };
    class RuntimeError : public std::runtime_error {
        public:
            RuntimeError(const std::string& msg) 
                : std::runtime_error("Encountered an unexpected runtime error: " + msg) {}
    };
    Life();
    void tick();

private:
    // WebGPU Context
    std::unique_ptr<WebGPUContext> context;

    // Component Classes
    std::unique_ptr<Geometry> geometry; 
    std::unique_ptr<RenderPipeline> pipeline;    
};