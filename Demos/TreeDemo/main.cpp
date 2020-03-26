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

	void report_invalid_mode(const std::shared_ptr<purezento::console>& console)
	{
		console->texts.push_back({ "error : the mode of tree is not support this command.", red_color });
	}

	void report_invalid_node(const std::shared_ptr<purezento::console>& console)
	{
		console->texts.push_back({ "error : the node is not a tree node.", red_color });
	}
}

namespace tree_info {

	auto has_tree() -> bool
	{
		if (global_mode == mode::tree) return global_tree != nullptr;
		if (global_mode == mode::binary_tree) return global_binary_tree != nullptr;

		return false;
	}
	
	auto size() -> size_t
	{
		if (global_mode == mode::tree) return !has_tree() ? 0 : global_tree->nodes.size() - 1;
		if (global_mode == mode::binary_tree) return !has_tree() ? 0 : global_binary_tree->nodes.size() - 1;

		return 0;
	}

	bool legal_node(int node) {
		return node > 0 && node <= size();
	}

	bool is_root(int node) {
		if (global_mode == mode::tree) {
			if (!has_tree()) return false;

			return global_tree->nodes[0].children.find(node) != global_tree->nodes[0].children.end();
		}

		if (global_mode == mode::binary_tree) {
			if (!has_tree()) return false;

			return global_binary_tree->root == node;
		}

		return false;
	}
}

namespace node_window {

	std::vector<bool> windows;
	std::vector<int> nodes;

	// info of tree
	std::vector<std::vector<std::string>> windows_listbox;
	std::vector<int> windows_child;

	void draw_window(const std::shared_ptr<void>& ctx, float delta)
	{
		const auto identity = std::static_pointer_cast<int>(ctx);
		const auto tree_size = static_cast<int>(tree_info::size());

		nodes[*identity] = std::min(nodes[*identity], tree_size);
		nodes[*identity] = std::max(nodes[*identity], 1);

		bool state = windows[*identity];

		ImGui::Begin(("node_window" + std::to_string(*identity)).c_str(), &state);

		const auto node_name = tree_info::has_tree() ? std::to_string(nodes[*identity]) : "empty";

		if (ImGui::BeginCombo("Node", node_name.c_str())) {

			if (tree_info::has_tree()) {

				for (auto index = 1; index <= tree_size; index++) {
					const auto selected = (nodes[*identity] == index);

					if (ImGui::Selectable(std::to_string(index).c_str(), selected))
						nodes[*identity] = index;
					if (selected) ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		if (global_mode == mode::tree) {

			windows_listbox[*identity].clear();

			std::vector<const char*> windows_listbox_ptr;

			if (tree_info::has_tree()) {

				for (const auto& it : global_tree->nodes[nodes[*identity]].children) {
					windows_listbox[*identity].push_back("Node " + std::to_string(it));
					windows_listbox_ptr.push_back(windows_listbox[*identity][windows_listbox[*identity].size() - 1].c_str());
				}
			}

			ImGui::ListBox("Children", &windows_child[*identity], windows_listbox_ptr.data(),
				static_cast<int>(windows_listbox_ptr.size()), 4);
		}else {

			const auto children_name = (tree_info::has_tree() && global_binary_tree->nodes[nodes[*identity]].child != 0) ? 
				"Node " + std::to_string(global_binary_tree->nodes[nodes[*identity]].child) : "Empty";
			const auto brother_name = (tree_info::has_tree() && global_binary_tree->nodes[nodes[*identity]].brother != 0)?
				"Node " + std::to_string(global_binary_tree->nodes[nodes[*identity]].brother) : "Empty";

			ImGui::LabelText("Children", children_name.c_str());
			ImGui::LabelText("Brother", brother_name.c_str());
		}

		const auto is_root = tree_info::is_root(nodes[*identity]) ? "True" : "False";
		
		ImGui::LabelText("IsRoot", is_root);
		
		ImGui::End();

		windows[*identity] = state;
	}
	
	void add_window() {
		windows_listbox.push_back(std::vector<std::string>());
		windows_child.push_back(0);
		
		windows.push_back(true);
		nodes.push_back(0);

		runtime->render()->draw_elements["windows"].push_back(
			purezento::draw_element::draw_call(
				purezento::draw_call(draw_window, std::make_shared<int>(static_cast<int>(windows.size() - 1)))
			));
	}

}

namespace help_window {

	void draw_window(const std::shared_ptr<void>& ctx, float delta)
	{
		ImGui::Begin("Help");

		const auto green_color = ImVec4(0, 0.7f, 0, 1.f);

		std::vector<std::string> commands = {
			"build binary_tree size root",
			"build tree size",
			"transform",
			"insert father node",
			"remove father node",
			"link father root",
			"insert_child father child",
			"insert_brother father child"
		};
		std::vector<std::string> tooltips = {
			"build a binary tree\nsize : the size of tree\nroot : the root of tree",
			"build a tree\nsize : the size of tree",
			"transform binary tree to tree or tree to binary tree",
			"insert node to father(tree command)\nfather : node in the tree\nnode : any node",
			"remove node from father(tree command)\nfather : any node\nnode : the child of father",
			"link a root of tree to a node in tree(tree command)\nfather : a node in the tree\nroot : a root of tree",
			"insert a node as child(binary tree command)\nfather : any node that does not have child\nchild : any node",
			"insert a node as brother(binary tree command)\nfather : any node that does not have brother\nchild : any node"
		};
		
		ImGui::Text("Support Commands");

		for (size_t index = 0; index < commands.size(); index++) {
			ImGui::BulletText(commands[index].c_str());
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(tooltips[index].c_str());
		}
		
		ImGui::End();
	}
	
}

void tree_node_config(
	const std::shared_ptr<purezento::config>& config, 
	const std::shared_ptr<void>& ctx)
{
	if (ImGui::Button("New Node Window")) 
		node_window::add_window();
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

void insert_command(
	const std::vector<std::string>& arguments,
	const std::shared_ptr<purezento::console>& console,
	const std::shared_ptr<void>& ctx)
{
	if (global_mode == mode::binary_tree) {
		message::report_invalid_mode(console);

		return;
	}
	
	// insert father child
	if (arguments.size() == 3) {

		if (!value_check::is_number(arguments[1]) || !value_check::is_number(arguments[2])) {
			message::report_invalid_command(console);

			return;
		}

		const auto father = std::stoi(arguments[1]);
		const auto child = std::stoi(arguments[2]);

		if (!tree_info::legal_node(father)) {
			message::report_invalid_node(console);

			return;
		}
		
		if (!global_tree->in(father)) {
			console->texts.push_back({ "error : the father node is not in tree.", red_color });

			return;
		}

		if (global_tree->in(child)) {
			console->texts.push_back({ "error : the child node is in tree.", red_color });

			return;
		}

		global_tree->insert(father, child);

		console->texts.push_back({ "insert node successfully.", green_color });
		
		return;
	}

	message::report_invalid_command(console);
}

void remove_command(
	const std::vector<std::string>& arguments,
	const std::shared_ptr<purezento::console>& console,
	const std::shared_ptr<void>& ctx)
{
	if (global_mode == mode::binary_tree) {
		message::report_invalid_mode(console);

		return;
	}
	
	// remove father child
	if (arguments.size() == 3) {
		
		if (!value_check::is_number(arguments[1]) || !value_check::is_number(arguments[2])) {
			message::report_invalid_command(console);

			return;
		}

		const auto father = std::stoi(arguments[1]);
		const auto child = std::stoi(arguments[2]);

		if (!tree_info::legal_node(father) || !tree_info::legal_node(child)) {
			message::report_invalid_node(console);

			return;
		}
		
		if (!global_tree->in(father)) {
			console->texts.push_back({ "error : the father node is not in tree.", red_color });

			return;
		}

		if (child > tree_info::size() || global_tree->nodes[father].children.find(child) == global_tree->nodes[father].children.end()) {
			console->texts.push_back({ "error : the node is not the child of father.", red_color });

			return;
		}

		global_tree->remove(father, child);

		console->texts.push_back({ "remove node successfully.", green_color });
		
		return;
	}

	message::report_invalid_command(console);
}

void link_command(
	const std::vector<std::string>& arguments,
	const std::shared_ptr<purezento::console>& console,
	const std::shared_ptr<void>& ctx)
{
	if (global_mode == mode::binary_tree) {
		message::report_invalid_mode(console);

		return;
	}
	
	// link father_node child_root
	if (arguments.size() == 3) {

		if (!value_check::is_number(arguments[1]) || !value_check::is_number(arguments[2])) {
			message::report_invalid_command(console);

			return;
		}

		const auto father = std::stoi(arguments[1]);
		const auto child = std::stoi(arguments[2]);

		if (!tree_info::legal_node(father) || !tree_info::legal_node(child)) {
			message::report_invalid_node(console);

			return;
		}
		
		if (!tree_info::is_root(child)) {
			console->texts.push_back({ "error : the node is the root of tree.", red_color });

			return;
		}

		global_tree->link(father, child);

		console->texts.push_back({ "link root successfully.", green_color });

		return;
	}

	message::report_invalid_command(console);
}

void insert_child_command(
	const std::vector<std::string>& arguments,
	const std::shared_ptr<purezento::console>& console,
	const std::shared_ptr<void>& ctx)
{
	if (global_mode == mode::tree) {
		message::report_invalid_mode(console);

		return;
	}

	// insert_child father child
	if (arguments.size() == 3) {

		if (!value_check::is_number(arguments[1]) || !value_check::is_number(arguments[2])) {
			message::report_invalid_command(console);

			return;
		}

		const auto father = std::stoi(arguments[1]);
		const auto child = std::stoi(arguments[2]);

		if (!tree_info::legal_node(father) || !tree_info::legal_node(child)) {
			message::report_invalid_node(console);

			return;
		}

		if (global_binary_tree->nodes[father].child != 0) {
			console->texts.push_back({ "error : the child of node is existed.", red_color });

			return;
		}

		global_binary_tree->insert(father, child, -1);

		console->texts.push_back({ "insert child successfully.", green_color });

		return;
	}

	message::report_invalid_command(console);
}

void insert_brother_command(
	const std::vector<std::string>& arguments,
	const std::shared_ptr<purezento::console>& console,
	const std::shared_ptr<void>& ctx)
{
	if (global_mode == mode::tree) {
		message::report_invalid_mode(console);

		return;
	}

	// insert_brother father child
	if (arguments.size() == 3) {

		if (!value_check::is_number(arguments[1]) || !value_check::is_number(arguments[2])) {
			message::report_invalid_command(console);

			return;
		}

		const auto father = std::stoi(arguments[1]);
		const auto child = std::stoi(arguments[2]);

		if (!tree_info::legal_node(father) || !tree_info::legal_node(child)) {
			message::report_invalid_node(console);

			return;
		}

		if (global_binary_tree->nodes[father].brother != 0) {
			console->texts.push_back({ "error : the brother of node is existed.", red_color });

			return;
		}

		global_binary_tree->insert(father, -1, child);

		console->texts.push_back({ "insert brother successfully.", green_color });

		return;
	}

	message::report_invalid_command(console);
}

int main() {
	runtime->config()->callbacks["tree"] = purezento::config_callback(tree_node_config, nullptr);

	runtime->render()->draw_elements["windows"].push_back(
		purezento::draw_element::draw_call(
			purezento::draw_call(help_window::draw_window, nullptr)
		));
	
	runtime->console()->callbacks["build"] = purezento::command_callback(build_command, nullptr);
	runtime->console()->callbacks["transform"] = purezento::command_callback(transform_command, nullptr);
	runtime->console()->callbacks["insert"] = purezento::command_callback(insert_command, nullptr);
	runtime->console()->callbacks["remove"] = purezento::command_callback(remove_command, nullptr);
	runtime->console()->callbacks["link"] = purezento::command_callback(link_command, nullptr);
	runtime->console()->callbacks["insert_child"] = purezento::command_callback(insert_child_command, nullptr);
	runtime->console()->callbacks["insert_brother"] = purezento::command_callback(insert_brother_command, nullptr);
	
	runtime->run_loop();
}