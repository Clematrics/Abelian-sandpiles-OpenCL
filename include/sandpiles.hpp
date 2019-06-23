#pragma once

#include <chrono>
#include <list>
#include <memory>
#include <vector>

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl2.hpp>

#include "application_log.hpp"

struct rule_unit {
	int32_t x, y;
	uint32_t quantity;
};

class sandpile {
public:
	sandpile(uint32_t width, uint32_t height);
	~sandpile();

	bool isBusy();

	// grid functions
	void setSize(uint32_t width, uint32_t height);
	void reset();

	// rule functions
	void setRule(const std::vector<rule_unit>& rule);

	// sand functions
	void addSand(uint32_t x, uint32_t y, uint32_t quantity);
	void addSandEverywhere(uint32_t quantity);
	void update();

	// get image
	uint8_t* getImage();

private:
	std::list<std::unique_ptr<cl::UserEvent>> events;
	void beginTask(const std::string& taskName);
	void endTask(const std::string& taskName);

public:
	uint32_t width, height;

	std::chrono::time_point<std::chrono::steady_clock> start;
	std::chrono::microseconds dur { 1000 };
	std::chrono::time_point<std::chrono::steady_clock> start_d;
	std::chrono::microseconds dur_d;
private:
	std::unique_ptr<uint32_t[]> rawGrid;
	std::unique_ptr<uint8_t[]> lastImage;

	bool startedCompute;

	// rule variables
	std::vector<rule_unit> rule;
	uint32_t rule_total;


	// OpenCL variables
	cl::Context context;
	cl::CommandQueue queue;
	cl::Kernel kernelUpdate;
	cl::Kernel kernelAdd;
	cl::Kernel kernelAddEverywhere;
	cl::Buffer ruleBuffer;
	cl::Buffer inputBuffer;
	cl::Buffer tempBuffer;
};