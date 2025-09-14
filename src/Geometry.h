#pragma once
#include <webgpu/webgpu.h>
#include "WebGPUContext.h"

class Geometry
{
public:
    Geometry(WebGPUContext& context);
    ~Geometry();

    // Getters
    WGPUBuffer getVertexBuffer() const { return vertexBuffer; }
    uint32_t getVertexCount() const { return VERTEX_COUNT; }
    WGPUVertexBufferLayout getVertexBufferLayout() const { return VERTEX_BUFFER_LAYOUT; }
    uint64_t getSizeOfVertices() const { return sizeof(VERTICES); }

private:
    // Square vertex data
    static constexpr float VERTICES[] = {
        // X,    Y,
        -0.8f, -0.8f, // Top Left Triangle
         0.8f, -0.8f,
         0.8f,  0.8f,

        -0.8f, -0.8f, // Bottom Right Triangle
         0.8f,  0.8f,
        -0.8f,  0.8f,
    };
    static constexpr uint32_t VERTEX_COUNT = 6;

    // Descriptors, Layouts, Attributes
    static constexpr WGPUBufferDescriptor VERTEX_BUFFER_DESC = {
        .label = WGPUStringView{"Square Vertices", 15},
        .usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst,
        .size = sizeof(VERTICES),
        .mappedAtCreation = false
    };
    static constexpr WGPUVertexAttribute VERTEX_ATTRIBUTE = {
        .format = WGPUVertexFormat_Float32x2,
        .offset = 0,
        .shaderLocation = 0
    };
    static constexpr WGPUVertexBufferLayout VERTEX_BUFFER_LAYOUT = {
        .stepMode = WGPUVertexStepMode_Vertex,
        .arrayStride = 8,
        .attributeCount = 1,
        .attributes = &VERTEX_ATTRIBUTE
    };

    // WGPU Handles
    WGPUBuffer vertexBuffer = nullptr;

};