#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

struct rule_unit {
	int x, y;
	uint quantity;
};

kernel void update_grid(uint width, uint height, uint rule_total, uint rule_size, global read_write struct rule_unit* rule, global read_write uint* input_grid, global read_write uint* temp_grid) {
	size_t x = get_global_id(0);
	size_t y = get_global_id(1);

	if (x < width && y < height) {
		uint input_sand = input_grid[y * width + x];
		if (input_sand < rule_total) {
			atomic_add(&temp_grid[y * width + x], input_sand);
			return;
		}
		uint factor = input_sand / rule_total;
		for (uint index = 0; index < rule_size; index++) {
			int x_ = x + rule[index].x;
			int y_ = y + rule[index].y;
			if (x_ >= 0 && y_ >= 0 && x_ < width && y_ < height)
				atomic_add(&temp_grid[y_ * width + x_], rule[index].quantity * factor);
		}

		atomic_add(&temp_grid[y * width + x], input_sand % rule_total);
	}
}