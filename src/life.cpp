#include "Life.h"
#include <iostream>
#include <memory>
#include "Geometry.h"

Life::Life()
{
    std::cout << "🔧 Creating WebGPU Context..." << std::endl;
    context = std::make_unique<WebGPUContext>();

    geometry = std::make_unique<Geometry>(*context.get());
    pipeline = std::make_unique<RenderPipeline>(*context.get(), *geometry.get());
}



void Life::tick()
{
    pipeline->renderFrame(*context.get(), *geometry.get());
}