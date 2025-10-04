
/**
  Vertex Shader Input/Output Structs
*/
struct VertexInput {
  @location(0) pos: vec2f,
  @builtin(instance_index) instance: u32,
};
struct VertexOutput {
  @builtin(position) pos: vec4f,
  @location(0) cell: vec2f,
};

/**
  Bindings
*/
@group(0) @binding(0) var<uniform> grid: vec2f; // The grid dimensions (UNIFORM_ARRAY)
@group(0) @binding(1) var<storage> cellStateIn: array<u32>;
@group(0) @binding(2) var<storage, read_write> cellStateOut: array<u32>;

@vertex
fn vertexMain(input: VertexInput) -> VertexOutput  {
  /**
    Convert instance index to cell position
  */
  // Get instance index as float
  let i = f32(input.instance);
  // Convert to cell coordinates (x,y)
  let cell = vec2f(i % grid.x, floor(i / grid.x));
  // Get cell state (0 or 1)
  let state = f32(cellStateIn[input.instance]);

  let cellOffset = cell / grid * 2;
  let gridPos = (input.pos*state+1) / grid - 1 + cellOffset;

  var output: VertexOutput;
  output.pos = vec4f(gridPos, 0, 1);
  output.cell = cell;
  return output;
}

struct FragInput {
  @location(0) cell: vec2f,
};

@fragment
fn fragmentMain(input: VertexOutput) -> @location(0) vec4f {
  let c = input.cell / grid;
  return vec4f(c, 1-c.x, 1);
}

fn cellIndex(cell: vec2u) -> u32 {
  return (cell.y % u32(grid.y)) * u32(grid.x) +
         (cell.x % u32(grid.x));
}

fn cellActive(x: u32, y: u32) -> u32 {
  return cellStateIn[cellIndex(vec2(x, y))];
}

// Default to 8, but dynamically overriden in computer pipeline
override WORKGROUP_SIZE: u32 = 8;

@compute
@workgroup_size(WORKGROUP_SIZE, WORKGROUP_SIZE)
fn computeMain(@builtin(global_invocation_id) cell: vec3u) {
  let activeNeighbors = cellActive(cell.x+1, cell.y+1) +
                        cellActive(cell.x+1, cell.y) +
                        cellActive(cell.x+1, cell.y-1) +
                        cellActive(cell.x, cell.y-1) +
                        cellActive(cell.x-1, cell.y-1) +
                        cellActive(cell.x-1, cell.y) +
                        cellActive(cell.x-1, cell.y+1) +
                        cellActive(cell.x, cell.y+1);
  let i = cellIndex(cell.xy);
  switch activeNeighbors {
    case 2: { // Active cells with 2 neighbors stay active.
      cellStateOut[i] = cellStateIn[i];
    }
    case 3: { // Cells with 3 neighbors become or stay active.
      cellStateOut[i] = 1;
    }
    default: { // Cells with < 2 or > 3 neighbors become inactive.
      cellStateOut[i] = 0;
    }
  }
}