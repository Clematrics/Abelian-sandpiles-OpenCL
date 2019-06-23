
#include <chrono>
#include <iostream>
#include <thread>

#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"

#include "imgui.h"
#include "imgui-SFML.h"

#include "application_log.hpp"
#include "sandpiles.hpp"

uint32_t width = 63;
uint32_t height = 63;
sandpile sp(width, height);
sf::Texture texture;
bool continue_sim = false;

void showSandpileControl() {
	static int new_width = width, new_height = height;
	static int x = width / 2, y = height / 2, quantity = 1;
	if (ImGui::CollapsingHeader("Grid")) {
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
		ImGui::InputInt("width", &new_width);	new_width = new_width < 0 ? 0 : new_width;
		ImGui::InputInt("height", &new_height);	new_height = new_height < 0 ? 0 : new_height;

		if (!sp.isBusy() && ImGui::Button("Apply size")) {
			width = new_width;
			height = new_height;
			sp.setSize(width, height);
			texture.create(width, height);
			x = width / 2; y = height / 2;
		}

		ImGui::PopItemWidth();
	}

	if (ImGui::CollapsingHeader("Sand")) {

		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
		ImGui::InputInt("x", &x); x = x < 0 ? 0 : x >= width ? width : x;
		ImGui::InputInt("y", &y); y = y < 0 ? 0 : y >= height ? height : y;
		ImGui::InputInt("Quantity", &quantity);

		if (ImGui::Button("Add sand")) {
			sp.addSand(x, y, quantity);
		}
		if (ImGui::Button("Add sand everywhere")) {
			sp.addSandEverywhere(quantity);
		}
		if (!sp.isBusy() && ImGui::Button("Reset grid")) {
			sp.reset();
		}

		// if (!sp.isBusy() && ImGui::Button("Update")) {
		// 	sp.update();
		// }

		ImGui::PopItemWidth();
	}

	if (ImGui::CollapsingHeader("Rule")) {
		constexpr int max_size_rule = 7;
		static int rule_width = 1, rule_height = 1;
		// static grid rule { { 0, 1, 0 }, { 1, 0, 1 }, { 0, 1, 0 } };

		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
		ImGui::SliderInt("rule - half width", &rule_width, 1, max_size_rule);
		ImGui::SliderInt("rule - half height", &rule_height, 1, max_size_rule);
		ImGui::PopItemWidth();

		// rule.resize(2 * rule_width + 1, std::vector<uint64_t>(2 * rule_height + 1, 0));
		// for(auto& row : rule)
		// 	row.resize(2 * rule_height + 1, 0);
		// rule[rule_width][rule_height] = 0;

		// ImGui::PushItemWidth(40);
		// for(int j = 0; j < 2 * rule_height + 1; ++j) {
		// 	for(int i = 0; i < 2 * rule_width + 1; ++i) {
		// 		ImGui::PushID(i * (2 * max_size_rule + 1) + j);
		// 		if (i == rule_width && j == rule_height)
		// 			ImGui::Dummy(ImVec2(40, 20));
		// 		else
		// 			ImGui::InputScalar("", ImGuiDataType_U64, &rule[i][j]);
		// 		ImGui::PopID();
		// 		if (i < (2 * rule_width + 1) - 1) ImGui::SameLine();
		// 	}
		// }
		// ImGui::PopItemWidth();

		// if (!sp.isBusy() && ImGui::Button("Change expansion rule")) {
		// 	sp.setRule(rule);
		// }
	}

	if (ImGui::CollapsingHeader("Worker status")) {
		std::string running = sp.isBusy() ? "Worker is running" : "Worker is not running";
		ImGui::Text(running.c_str());

		if(continue_sim && ImGui::Button("Interrupt"))
			continue_sim = false;
		if(!continue_sim && ImGui::Button("Continue"))
			continue_sim = true;

		ImGui::Text(std::string("Time to compute an update : " + std::to_string(sp.dur.count()) + "µs").c_str());
		ImGui::Text(std::string("Time to download the image : " + std::to_string(sp.dur_d.count()) + "µs").c_str());
	}
}

int main() {
	bool running = true;
	sf::RenderWindow window(sf::VideoMode(720, 720), "Sandpiles");
	ImGui::SFML::Init(window);
	ImGui::GetIO().FontGlobalScale = 1.5f;

	texture.create(width, height);

	try {
		sf::Clock delta;
		while (running) {
			sf::Event event;
			while (window.pollEvent(event)) {
				if (event.type == event.Resized) {
					window.setView(sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y)));
				}
				ImGui::SFML::ProcessEvent(event);
				running = event.type != event.Closed;
			}

			window.clear();

			texture.update(sp.getImage());
			if (continue_sim && !sp.isBusy())
				sp.update();
			sf::Sprite sprite;
			sprite.setTexture(texture);
			auto [window_width, window_height] = window.getSize();
			auto min_side = window_width < window_height ? window_width : window_height;
			sprite.setScale(min_side / sp.width, min_side / sp.height);
			window.draw(sprite);

			ImGui::SFML::Update(window, delta.restart());
			showSandpileControl();
			showDebugConsole();
			ImGui::SFML::Render(window);

			window.display();
		}

		ImGui::SFML::Shutdown();
		window.close();
	}
	catch (std::exception& e) {
		// logError(e.what());
		std::cout << e.what() << '\n';
		std::cin >> width;
	}
	return 0;
}