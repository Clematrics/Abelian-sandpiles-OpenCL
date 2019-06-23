#include "sandpiles.hpp"

#include <numeric>
#include <fstream>

const std::string resetTask { "reset" };
const std::string addTask { "add" };
const std::string addEverywhereTask { "add everywhere" };
const std::string updateTask { "update" };

void sandpile::beginTask(const std::string& taskName) {
	events.push_back(std::make_unique<cl::UserEvent>(context));
	if (taskName == updateTask) {
		start = std::chrono::high_resolution_clock::now();
	}
}
void sandpile::endTask(const std::string& taskName) {
	queue.flush();
	if (taskName == updateTask)
		events.back().get()->setCallback(CL_COMPLETE,
			[](cl_event, cl_int, void* data) {
				sandpile* sp = (sandpile*)data;
				sp->dur = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - sp->start);
				// std::string* taskName = (std::string*)data;
				// logDebug("fin de " + *taskName);
			}, this);

	// delete finished events
	events.erase(std::remove_if(events.begin(), events.end(), [](std::unique_ptr<cl::UserEvent>& event) { return event->getInfo<CL_EVENT_COMMAND_EXECUTION_STATUS>() == CL_COMPLETE; }), events.end());
}

sandpile::sandpile(uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;
	uint32_t platform_id = 0, device_id = 0;
	try
	{
		// Query for platforms
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		logDebug( std::to_string(platforms.size()) + " OpenCl plateforms were found :");
		for (cl::Platform& platform : platforms)
			logDebug( platform.getInfo<CL_PLATFORM_NAME>() );

		// Get a list of devices on this platform
		std::vector<cl::Device> devices;
		platforms[platform_id].getDevices( CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU, &devices); // Select the platform.
		logDebug( std::to_string(devices.size()) + " devices were found :");
		for (cl::Device& device : devices)
			logDebug( device.getInfo<CL_DEVICE_NAME>() );

		// Create a context
		context = cl::Context(devices);

		// Create a command queue
		queue = cl::CommandQueue( context, devices[device_id] );   // Select the device.

		std::ifstream source1("update.cl");
		std::string kernel_code( std::istreambuf_iterator<char>(source1), (std::istreambuf_iterator<char>()));
		std::ifstream source2("add.cl");
		std::string kernel_code_add( std::istreambuf_iterator<char>(source2), (std::istreambuf_iterator<char>()));
		std::ifstream source3("addEverywhere.cl");
		std::string kernel_code_add_everywhere( std::istreambuf_iterator<char>(source3), (std::istreambuf_iterator<char>()));

		// Make programs from the source code
		cl::Program program1(context, kernel_code);
		cl::Program program2(context, kernel_code_add);
		cl::Program program3(context, kernel_code_add_everywhere);

		// Build the program for the devices
		program1.build(devices);
		program2.build(devices);
		program3.build(devices);

		// Make kernel
		kernelUpdate = cl::Kernel(program1, "update_grid");
		kernelAdd = cl::Kernel(program2, "add");
		kernelAddEverywhere = cl::Kernel(program3, "add_everywhere");

		setSize(width, height);
		setRule({ { -1, 0, 1 }, { 1, 0, 1 }, { 0, -1, 1 }, { 0, 1, 1 } });
	}
	catch(cl::Error err) {
		logError(std::string(err.what()) + " (" + std::to_string(err.err()) + ")" );
	}
}

sandpile::~sandpile() {
}

bool sandpile::isBusy() {
	return std::any_of(events.begin(), events.end(), [](std::unique_ptr<cl::UserEvent>& event) { return event->getInfo<CL_EVENT_COMMAND_EXECUTION_STATUS>() != CL_COMPLETE; });
}

void sandpile::setSize(uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;

	rawGrid = std::make_unique<uint32_t[]>(width * height);
	lastImage = std::make_unique<uint8_t[]>(width * height * 4);

	cl_int err;
	// Create new memory buffers
	inputBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, width * height * sizeof(uint32_t), nullptr, &err);
	tempBuffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, width * height * sizeof(uint32_t), nullptr, &err);
	reset();
}

void sandpile::reset() {
	beginTask(resetTask);
	queue.enqueueFillBuffer<uint32_t>(inputBuffer, 0, 0, width * height * sizeof(uint32_t));
	queue.enqueueFillBuffer<uint32_t>(tempBuffer, 0, 0, width * height * sizeof(uint32_t), nullptr, events.back().get());
	// queue.enqueueCopyBuffer(inputBuffer, outputBuffer, 0, 0, width * height * sizeof(uint32_t), nullptr, events.back().get());
	endTask(resetTask);
}

void sandpile::setRule(const std::vector<rule_unit>& rule) {
	this->rule = rule;
	rule_total = std::accumulate(rule.begin(), rule.end(), 0, [](uint32_t last, rule_unit a){ return last + a.quantity; } );
	ruleBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, rule.size() * sizeof(rule_unit));
	queue.enqueueWriteBuffer(ruleBuffer, true, 0, rule.size() * sizeof(rule_unit), rule.data());
}

void sandpile::addSand(uint32_t x, uint32_t y, uint32_t quantity) {
	std::unique_ptr<uint32_t> ptr = std::make_unique<uint32_t>(quantity);
	beginTask(addTask);
	kernelAdd.setArg(0, width);
	kernelAdd.setArg(1, height);
	kernelAdd.setArg(2, x);
	kernelAdd.setArg(3, y);
	kernelAdd.setArg(4, quantity);
	kernelAdd.setArg(5, inputBuffer);
	kernelAdd.setArg(6, tempBuffer);
	queue.enqueueTask(kernelAdd, nullptr, events.back().get());
	// queue.enqueueWriteBuffer(inputBuffer, false, (y * width + x) * sizeof(uint32_t), sizeof(uint32_t), ptr.get(), nullptr);
	// queue.enqueueCopyBuffer(outputBuffer, inputBuffer, 0, 0, width * height * sizeof(uint32_t), nullptr, events.back().get());
	endTask(addTask);
}


void sandpile::addSandEverywhere(uint32_t quantity) {
	int global_width = (width / 32) * 32; global_width += width % 32 ? 32 : 0;
	int global_height = (height / 32) * 32; global_height += height % 32 ? 32 : 0;
	beginTask(addEverywhereTask);
	kernelAddEverywhere.setArg(0, width);
	kernelAddEverywhere.setArg(1, height);
	kernelAddEverywhere.setArg(2, quantity);
	kernelAddEverywhere.setArg(3, inputBuffer);
	kernelAddEverywhere.setArg(4, tempBuffer);

	queue.enqueueNDRangeKernel(kernelAddEverywhere, cl::NDRange(0, 0), cl::NDRange(global_width, global_height), cl::NullRange, nullptr, events.back().get());
	// queue.enqueueCopyBuffer(outputBuffer, inputBuffer, 0, 0, width * height * sizeof(uint32_t), nullptr, events.back().get());
	endTask(addEverywhereTask);
}

void sandpile::update() {
	if (startedCompute)
		return;
	startedCompute = true;

	size_t nb_update = 1;
	if (1000 / dur.count() > 0)
		nb_update = 1000 / dur.count();

	int global_width = (width / 32) * 32; global_width += width % 32 ? 32 : 0;
	int global_height = (height / 32) * 32; global_height += height % 32 ? 32 : 0;
	beginTask(updateTask);
	kernelUpdate.setArg<uint32_t>(0, width);
	kernelUpdate.setArg<uint32_t>(1, height);
	kernelUpdate.setArg<uint32_t>(2, rule_total);
	kernelUpdate.setArg<uint32_t>(3, rule.size() * sizeof(rule_unit));
	kernelUpdate.setArg(4, ruleBuffer);
	kernelUpdate.setArg(5, inputBuffer);
	kernelUpdate.setArg(6, tempBuffer);

	for (auto i = 0; i < nb_update; i++) {
		queue.enqueueFillBuffer<uint32_t>(tempBuffer, 0, 0, width * height * sizeof(uint32_t));
		queue.enqueueNDRangeKernel(kernelUpdate, cl::NDRange(0, 0), cl::NDRange(global_width, global_height));
		if (i < nb_update - 1)
			queue.enqueueCopyBuffer(tempBuffer, inputBuffer, 0, 0, width * height * sizeof(uint32_t));
	}
	queue.enqueueCopyBuffer(tempBuffer, inputBuffer, 0, 0, width * height * sizeof(uint32_t), nullptr, events.back().get());
	endTask(updateTask);
}

uint8_t* sandpile::getImage() {
	if (!isBusy()) {
		start_d = std::chrono::high_resolution_clock::now();
		queue.enqueueReadBuffer(inputBuffer, true, 0, width * height * sizeof(uint32_t), rawGrid.get());
		for (uint32_t i = 0; i < width * height; ++i) {
			uint32_t q = rawGrid[i];
			uint8_t value = 255 * (q % rule_total) / rule_total;
			lastImage[4 * i + 0] = value;
			lastImage[4 * i + 1] = value;
			lastImage[4 * i + 2] = value;
			lastImage[4 * i + 3] = 255;
		}
		startedCompute = false;
		dur_d = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_d);
	}
	return lastImage.get();
}