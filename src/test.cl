#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

kernel void update_grid(uint update_number, uint width, uint height, uint rule_total, uint rule_size, global read_only uint3* rule, global read_only uint* input_grid, global uint* temp_grid, global write_only uint* output_grid) {
	size_t x = get_global_size(0);
	size_t y = get_global_size(1);

	if (x >= width || y >= height)
		return;

	for (uint update_count = 0; update_count < update_number; update_count++) {
		uint input_sand = input_grid[x * width + y];
		atomic_xchg(&temp_grid[x * width + y], input_sand % rule_total);
		uint factor = input_sand / rule_total;
		for (uint index = 0; index < rule_size; index++) {
			uint x_ = x + rule[index].x;
			uint y_ = y + rule[index].y;
			if (x_ >= 0 && y_ >= 0 && x_ < width && y_ < height)
				atomic_add(&output_grid[x_ * width + y_], rule[index].z * factor);
		}

		atomic_add(&output_grid[x * width + y], temp_grid[x * width + y]);
	}
}