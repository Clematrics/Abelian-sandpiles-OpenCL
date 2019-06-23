kernel void add_everywhere(uint width, uint height, uint quantity, global write_only uint* input_grid, global write_only uint* output_grid) {
	size_t x = get_global_id(0);
	size_t y = get_global_id(1);

	if (x >= width || y >= height)
		return;

	input_grid[y * width + x] += quantity;
	output_grid[y * width + x] += quantity;
}