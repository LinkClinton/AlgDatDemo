#include <Purezento/Runtime/runtime.hpp>

#include <Purezento/Extensions/ImGui/imgui.hpp>

#include "tree.hpp"

enum class mode : unsigned {
	binary_tree,
	tree
};

const auto window_width = 1280;
const auto window_height = 720;
const auto font_size = 12;

const auto red_color = purezento::color(1, 0, 0, 1);
const auto green_color = purezento::color(0, 0.7f, 0, 1);

auto runtime = std::make_shared<purezento::runtime>(
	purezento::runtime_startup(
		"TreeDemo",
		purezento::font("./Resources/Consola.ttf", 12),
		window_width, window_height)
	);

std::shared_ptr<binary_tree> global_binary_tree;
std::shared_ptr<tree> global_tree;

auto global_mode = mode::tree;
auto global_node = 0;

namespace value_check {

	bool is_number(const std::string& str) {
		for (const auto& c : str) if (c < '0' || c >'9') return false;

		return true;
	}
	
}

namespace message {

	void report_invalid_command(const std::shared_ptr<purezento::console>& console)
	{
		console->texts.push_back({ "error : the arguments of command are invalid.", red_color });
	}

	void report_invalid_tree(const std::shared_ptr<purezento::console>& console)
	{
		console->texts.push_back({ "error : please build a tree before this command.", red_color });
	}
}

void tree_node_config(
	const std::shared_ptr<purezento::config>& config, 
	const std::shared_ptr<void>& ctx)
{
	auto tree_size = 0;

	if (global_mode == mode::tree) tree_size = global_tree == nullptr ? 0 : static_cast<int>(global_tree->nodes.size() - 1);
	else tree_size = (global_binary_tree == nullptr) ? 0 : static_cast<int>(global_binary_tree->nodes.size() - 1);

	// limit the identity of node
	global_node = std::min(global_node, tree_size);
	global_node = std::max(global_node, 1);

	const auto current_node_name = global_tree == nullptr ? "empty" : std::to_string(global_node);

	if (ImGui::BeginCombo("Node", current_node_name.c_str())) {

		if (global_tree != nullptr) {

			for (auto index = 1; index < global_tree->nodes.size(); index++) {
				const auto selected = (global_node == index);

				if (ImGui::Selectable(std::to_string(index).c_str(), selected))
					global_node = index;
				if (selected) ImGui::SetItemDefaultFocus();
			}

		}

		ImGui::EndCombo();
	}
	
	if (global_mode == mode::tree) {

		static std::vector<std::string> children_name;
		static std::vector<const char*> children_name_ptr;
		static int current_children = 0;

		children_name_ptr.clear();
		children_name.clear();
		
		if (global_tree != nullptr) {

			for (const auto& it : global_tree->nodes[global_node].children) {
				children_name.push_back(std::to_string(it));
				children_name_ptr.push_back(children_name[children_name.size() - 1].c_str());
			}
		}

		ImGui::ListBox("Children", &current_children, children_name_ptr.data(),
			static_cast<int>(children_name_ptr.size()), 4);
		
	}else {
		const auto children_name = (global_tree == nullptr || global_binary_tree->nodes[global_node].child == 0) ? "empty" :
			std::to_string(global_binary_tree->nodes[global_node].child);
		const auto brother_name = (global_tree == nullptr || global_binary_tree->nodes[global_node].brother == 0) ? "empty" :
			std::to_string(global_binary_tree->nodes[global_node].brother);

		ImGui::LabelText("Children", children_name.c_str());
		ImGui::LabelText("Brother", brother_name.c_str());
	}
}

void build_command(
	const std::vector<std::string>& arguments,
	const std::shared_ptr<purezento::console>& console,
	const std::shared_ptr<void>& ctx)
{
	// build tree tree_size
	if (arguments.size() == 3) {

		// arguments[1] should be "tree" and arguments[2] should be number(positive)
		if (arguments[1] != "tree" || !value_check::is_number(arguments[2])) {
			message::report_invalid_command(console);

			return;
		}

		global_tree = std::make_shared<tree>(std::stoi(arguments[2]), std::vector<int>());

		for (int index = 1; index < global_tree->nodes.size(); index++)
			global_tree->nodes[0].children.insert(index);

		global_mode = mode::tree;

		console->texts.push_back({
			"build tree successfully", green_color
		});

		return;
	}

	// build binary_tree tree_size root
	if (arguments.size() == 4) {

		// arguments[1] should be "binary_tree", arguments[2] and arguments[3] should be number(positive)
		if (arguments[1] != "binary_tree" || !value_check::is_number(arguments[2]) || !value_check::is_number(arguments[3])) {
			message::report_invalid_command(console);

			return;
		}

		const auto size_of_tree = std::stoi(arguments[2]);
		const auto root_of_tree = std::stoi(arguments[3]);

		// limit the root of tree
		if (root_of_tree > size_of_tree) {
			console->texts.push_back({
				"error : the root of tree can not greater than size of tree", red_color
			});

			return;
		}
		
		global_binary_tree = std::make_shared<binary_tree>(size_of_tree, root_of_tree);

		for (int index = 1; index < global_binary_tree->nodes.size(); index++) {
			if (index == root_of_tree) continue;

			if (index + 1 == root_of_tree && root_of_tree != size_of_tree)
				global_binary_tree->nodes[index].brother = static_cast<int>(index) + 2;

			if (index + 1 != root_of_tree && index != size_of_tree)
				global_binary_tree->nodes[index].brother = static_cast<int>(index) + 1;
		}

		global_binary_tree->nodes[root_of_tree].brother = 1;
		
		global_mode = mode::binary_tree;

		console->texts.push_back({
			"build binary tree successfully", green_color
			});

		return;
	}

	// invalid arguments
	message::report_invalid_command(console);

	return;
}

void transform_command(
	const std::vector<std::string>& arguments,
	const std::shared_ptr<purezento::console>& console,
	const std::shared_ptr<void>& ctx)
{
	if (arguments.size() != 1) {

		console->texts.push_back({
			"error : the arguments of command are invalid.",
			red_color
		});

		return;
	}

	if (global_mode == mode::tree) {

		if (global_tree == nullptr) {
			message::report_invalid_tree(console);

			return;
		}

		global_binary_tree = global_tree->to_binary_tree();

		global_mode = mode::binary_tree;
		
		
	}else {

		if (global_binary_tree == nullptr) {
			message::report_invalid_tree(console);

			return;
		}

		global_tree = global_binary_tree->to_tree();

		global_mode = mode::tree;
	}

	console->texts.push_back({ "transform successfully", green_color });
}

int main() {
	runtime->config()->callbacks["tree_node"] = purezento::config_callback(tree_node_config, nullptr);

	runtime->console()->callbacks["build"] = purezento::command_callback(build_command, nullptr);
	runtime->console()->callbacks["transform"] = purezento::command_callback(transform_command, nullptr);
	
	runtime->run_loop();
}