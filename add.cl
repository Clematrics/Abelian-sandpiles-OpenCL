kernel void add(uint width, uint height, uint x, uint y, uint quantity, global write_only uint* input_grid, global write_only uint* output_grid) {
	input_grid[y * width + x] += quantity;
	output_grid[y * width + x] += quantity;
}