#include <Purezento/Runtime/runtime.hpp>

#include "skip_list.hpp"

#include <chrono>
#include <map>

#define MAX_NODE 50

const auto window_width = 1280;
const auto window_height = 720;
const auto font_size = 12;

const auto red_color = purezento::color(1, 0, 0, 1);
const auto green_color = purezento::color(0, 0.7f, 0, 1);

auto total_insert = 0.0;
auto total_erase = 0.0;
auto total_find = 0.0;
auto total_min = 0.0;
auto total_max = 0.0;

using time_clock = std::chrono::high_resolution_clock;

std::shared_ptr<skip_list<int, std::string>> list;

auto runtime = std::make_shared<purezento::runtime>(
	purezento::runtime_startup(
		"SkipListDemo",
		purezento::font("./Resources/Consola.ttf", 12),
		window_width, window_height)
	);

bool is_number(const std::string& str) {
	for (const auto& c : str) if (c < '0' || c > '9') return false;
	return true;
}

void update_skip_list_graph(
	const std::shared_ptr<purezento::render>& render,
	const std::shared_ptr<void>& ctx)
{
	render->draw_elements["skip_list"].clear();

	const auto text_space = font_size * 6;
	const auto border = 10.f;
	const auto width = window_width - runtime->config()->config_window_info().size.x - border * 2 - text_space;
	const auto height = window_height - runtime->config()->console_window_info().size.y - border * 2;

	if (list == nullptr || list->size() == 0) return;
	
	const auto box_height = height / MAX_LEVEL;
	const auto list_size = std::max(list->size(), static_cast<size_t>(10));
	const auto box_width = width / list_size;
	const auto radius = std::min(box_width, box_height) * 0.40f;

	// add text of layer information
	for (size_t index = 0; index < MAX_LEVEL; index++) {
		const auto real_y = MAX_LEVEL - index - 1;
		
		render->draw_elements["skip_list"].push_back(
			purezento::draw_element::text(
				purezento::text(
					"layer " + std::to_string(real_y) + " : ",
					purezento::vec2(border, border + index * box_height + radius - font_size * 0.5f)
				)
			)
		);
	}

	std::map<int, size_t> key_position;

	size_t count = 0;
	
	for (auto it = list->head(); it != list->tail(); it = it->mNext[0]) {
		if (it != list->head()) key_position.insert({ it->mKey, count++ });
	}

	for (size_t y = 0; y < MAX_LEVEL; y++) {
		for (auto it = list->head(); it != list->tail(); it = it->mNext[y]) {
			if (it == list->head()) continue;

			const auto real_x = key_position[it->mKey];
			const auto real_y = MAX_LEVEL - y - 1;
			
			render->draw_elements["skip_list"].push_back(
				purezento::draw_element::circle_fill(
					purezento::circle(
						purezento::vec2(
							border + text_space + real_x * box_width + radius,
							border + real_y * box_height + radius),
						radius
					),
					purezento::color(0.95f, 0, 0, 1)
				)
			);

			const auto x_offset = std::to_string(it->mKey).length() * font_size * 0.25f;
			
			render->draw_elements["skip_list"].push_back(
				purezento::draw_element::text(
					purezento::text(
						std::to_string(it->mKey),
						purezento::vec2(
							border + text_space + real_x * box_width + radius - x_offset,
							border + real_y * box_height + radius - font_size * 0.25f
						)
					)
				)
			);
		}
	}
}

void insert_command(
	const std::vector<std::string>& arguments,
	const std::shared_ptr<purezento::console>& console,
	const std::shared_ptr<void>& ctx)
{
	// when the command is invalid
	if (arguments.size() == 1 || !is_number(arguments[1])) {
		console->texts.push_back(
			purezento::console_text(
				"error : the command is invalid.",
				red_color
			)
		);

		return;
	}

	// when the key is existed
	if (list->find(std::stoi(arguments[1])) != "-1") {
		console->texts.push_back(
			purezento::console_text(
				"error : the key is existed.",
				red_color
			)
		);

		return;
	}

	if (list->size() == MAX_NODE) {
		console->texts.push_back(
			purezento::console_text(
				"warning : the size of list is greater than the limit of list. if you insert element again, the graph may be wrong.",
				red_color
			)
		);
	}

	const auto start = time_clock::now();
	list->insert(std::stoi(arguments[1]), arguments[1]);
	const auto end = time_clock::now();

	const auto duration = std::chrono::duration_cast<
		std::chrono::duration<double>>(end - start);

	total_insert = total_insert + duration.count();
	
	console->texts.push_back(
		purezento::console_text(
			"insert successful!",
			green_color
		)
	);

	console->texts.push_back(
		purezento::console_text(
			"time used : " + std::to_string(duration.count()) + "ms, total used : " + std::to_string(total_insert) + "ms.",
			green_color
		)
	);
}

void erase_command(
	const std::vector<std::string>& arguments,
	const std::shared_ptr<purezento::console>& console,
	const std::shared_ptr<void>& ctx)
{
	// when the command is invalid
	if (arguments.size() == 1 || !is_number(arguments[1])) {
		console->texts.push_back(
			purezento::console_text(
				"error : the command is invalid.",
				red_color
			)
		);

		return;
	}

	// when the key is not existed
	if (list->size() == 0 || list->find(std::stoi(arguments[1])) == "-1") {
		console->texts.push_back(
			purezento::console_text(
				"error : the key is not existed.",
				red_color
			)
		);

		return;
	}

	const auto start = time_clock::now();
	list->erase(std::stoi(arguments[1]));
	const auto end = time_clock::now();
	
	const auto duration = std::chrono::duration_cast<
		std::chrono::duration<double>>(end - start);

	total_erase = total_erase + duration.count();

	console->texts.push_back(
		purezento::console_text(
			"erase successful!",
			green_color
		)
	);

	console->texts.push_back(
		purezento::console_text(
			"time used : " + std::to_string(duration.count()) + "ms, total used : " + std::to_string(total_erase) + "ms.",
			green_color
		)
	);
}

void find_command(
	const std::vector<std::string>& arguments,
	const std::shared_ptr<purezento::console>& console,
	const std::shared_ptr<void>& ctx)
{
	// when the command is invalid
	if (arguments.size() == 1 || !is_number(arguments[1])) {
		console->texts.push_back(
			purezento::console_text(
				"error : the command is invalid.",
				red_color
			)
		);

		return;
	}

	// when the key is not existed
	if (list->size() == 0 || list->find(std::stoi(arguments[1])) == "-1") {
		console->texts.push_back(
			purezento::console_text(
				"error : the key is not existed.",
				red_color
			)
		);

		return;
	}

	const auto start = time_clock::now();
	const auto key = list->find(std::stoi(arguments[1]));
	const auto end = time_clock::now();

	const auto duration = std::chrono::duration_cast<
		std::chrono::duration<double>>(end - start);

	total_find = total_find + duration.count();
	
	console->texts.push_back(
		purezento::console_text(
			"success, the key is existed!",
			green_color
		)
	);

	console->texts.push_back(
		purezento::console_text(
			"time used : " + std::to_string(duration.count()) + "ms, total used : " + std::to_string(total_find) + "ms.",
			green_color
		)
	);
}

void min_command(
	const std::vector<std::string>& arguments,
	const std::shared_ptr<purezento::console>& console,
	const std::shared_ptr<void>& ctx)
{
	// when the key is not existed
	if (list->size() == 0) {
		console->texts.push_back(
			purezento::console_text(
				"error : the key is not existed.",
				red_color
			)
		);

		return;
	}

	const auto start = time_clock::now();
	const auto min_key = list->min();
	const auto end = time_clock::now();

	const auto duration = std::chrono::duration_cast<
		std::chrono::duration<double>>(end - start);
	
	total_min = total_min + duration.count();

	console->texts.push_back(
		purezento::console_text(
			"success, the min key is " + std::to_string(min_key) + ".",
			green_color
		)
	);

	console->texts.push_back(
		purezento::console_text(
			"time used : " + std::to_string(duration.count()) + "ms, total used : " + std::to_string(total_min) + "ms.",
			green_color
		)
	);
}

void max_command(
	const std::vector<std::string>& arguments,
	const std::shared_ptr<purezento::console>& console,
	const std::shared_ptr<void>& ctx)
{
	// when the key is not existed
	if (list->size() == 0) {
		console->texts.push_back(
			purezento::console_text(
				"error : the key is not existed.",
				red_color
			)
		);

		return;
	}

	const auto start = time_clock::now();
	const auto max_key = list->max();
	const auto end = time_clock::now();

	const auto duration = std::chrono::duration_cast<
		std::chrono::duration<double>>(end - start);

	total_max = total_max + duration.count();
	
	console->texts.push_back(
		purezento::console_text(
			"success, the max key is " + std::to_string(max_key) + ".",
			green_color
		)
	);

	console->texts.push_back(
		purezento::console_text(
			"time used : " + std::to_string(duration.count()) + "ms, total used : " + std::to_string(total_max) + "ms.",
			green_color
		)
	);
}

int main() {
	list = std::make_shared<skip_list<int, std::string>>();
	
	runtime->render()->callbacks["update_skip_list_graph"] = purezento::render_callback(update_skip_list_graph, nullptr);

	runtime->console()->callbacks["insert"] = purezento::command_callback(insert_command, nullptr);
	runtime->console()->callbacks["erase"] = purezento::command_callback(erase_command, nullptr);
	runtime->console()->callbacks["find"] = purezento::command_callback(find_command, nullptr);
	runtime->console()->callbacks["max"] = purezento::command_callback(max_command, nullptr);
	runtime->console()->callbacks["min"] = purezento::command_callback(min_command, nullptr);

	runtime->run_loop();
}