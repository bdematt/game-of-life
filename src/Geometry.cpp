#include <iostream>
#include "Geometry.h"
#include "Life.h"

Geometry::Geometry(WebGPUContext& context)
{
    vertexBuffer = wgpuDeviceCreateBuffer(context.getDevice(), &VERTEX_BUFFER_DESC);
    if (!vertexBuffer) throw Life::InitializationError("Failed to create instance");

    wgpuQueueWriteBuffer(context.getQueue(), vertexBuffer, 0, VERTICES, sizeof(VERTICES));
}
Geometry::~Geometry()
{
    if (vertexBuffer) wgpuBufferRelease(vertexBuffer);
}
