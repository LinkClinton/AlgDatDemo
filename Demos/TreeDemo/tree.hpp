#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <memory>
#include <set>

struct tree_node {
	int key = 0;

	std::set<int> children;

	tree_node() = default;

	tree_node(const int key) : key(key) {}
};

struct binary_tree_node {
	int key = 0, child = 0, brother = 0;

	binary_tree_node() = default;

	binary_tree_node(const int key, const int child, const int brother) :
		key(key), child(child), brother(brother) {}
};

class binary_tree;
class tree;

class tree {
public:
	tree(size_t size, const std::vector<int>& roots);

	void insert(int father, int child);

	void remove(int father, int child);

	void link(int father_root, int child_root);

	void print();

	bool in(int node);
	
	auto to_binary_tree()->std::shared_ptr<binary_tree>;
public:
	// 0 indicate the virtual root
	std::vector<tree_node> nodes;
private:
	void search(int node, int& output);

	bool search_in(int node, int target);
	
	void shift_to_fit(size_t target);
};

class binary_tree {
public:
	binary_tree(size_t size, int root);

	void insert(int father, int left_child, int right_child);

	void print();

	auto to_tree()->std::shared_ptr<tree>;
public:
	std::vector<binary_tree_node> nodes;

	int root;

private:
	void search(const int node, int& output);

	void shift_to_fit(size_t target);
};

inline tree::tree(size_t size, const std::vector<int>& roots)
{
	nodes = std::vector<tree_node>(size + 1);

	for (size_t index = 0; index < nodes.size(); index++)
		nodes[index].key = static_cast<int>(index);

	for (size_t index = 0; index < roots.size(); index++)
		nodes[0].children.insert(roots[index]);
}

inline void tree::insert(int father, int child)
{
	if (father == -1) father = 0;

	shift_to_fit(std::max(father, child) + 1);

	nodes[father].key = father;
	nodes[child].key = child;

	nodes[father].children.insert(child);
}

inline void tree::remove(int father, int child)
{
	if (father == -1) father = 0;

	for (const auto& new_root : nodes[child].children)
		nodes[0].children.insert(new_root);

	nodes[child].children.clear();
	nodes[child].key = -1;

	nodes[father].children.erase(child);
}

inline void tree::link(int father_root, int child_root)
{
	shift_to_fit(std::max(father_root, child_root) + 1);

	nodes[father_root].key = father_root;
	nodes[child_root].key = child_root;

	nodes[0].children.erase(child_root);
	nodes[father_root].children.insert(child_root);
}

inline void tree::print() {
	for (const auto& child : nodes[0].children) {
		int output = 0;

		search(child, output);

		std::cout << output << " ";
	}

	std::cout << std::endl;
}

inline bool tree::in(int node)
{
	return search_in(0, node);
}

inline auto tree::to_binary_tree() -> std::shared_ptr<binary_tree>
{
	auto tree = std::make_shared<binary_tree>(nodes.size() - 1, *nodes[0].children.begin());

	for (size_t index = 0; index < nodes.size(); index++)
		tree->nodes[index].key = nodes[index].key;

	for (size_t index = 0; index < nodes.size(); index++) {
		if (nodes[index].children.empty()) continue;

		tree->nodes[index].child = *nodes[index].children.begin();

		for (auto it = nodes[index].children.begin(); it != nodes[index].children.end(); ++it) {
			auto next_it = it; ++next_it;

			if (next_it != nodes[index].children.end())
				tree->nodes[*it].brother = *next_it;
		}
	}

	return tree;
}

inline void tree::search(int node, int& output)
{
	output = output ^ nodes[node].key;

	for (const auto& child : nodes[node].children)
		search(child, output);
}

inline bool tree::search_in(int node, int target)
{
	if (node == target) return true;

	for (const auto& child : nodes[node].children) 
		if (search_in(child, target)) return true;

	return false;
}

inline void tree::shift_to_fit(size_t target)
{
	for (size_t index = nodes.size(); index < target; index++) {
		nodes.push_back(tree_node(static_cast<int>(index)));
	}
}

inline binary_tree::binary_tree(size_t size, int root)
{
	nodes = std::vector<binary_tree_node>(size + 1);

	for (size_t index = 0; index < nodes.size(); index++)
		nodes[index].key = static_cast<int>(index);

	this->root = root;
}

inline void binary_tree::insert(int father, int left_child, int right_child)
{
	shift_to_fit(std::max(father, std::max(left_child, right_child)) + 1);

	nodes[father].key = father;

	if (left_child != -1) nodes[left_child].key = left_child;
	if (right_child != -1) nodes[right_child].key = right_child;

	if (left_child != -1 && left_child == root) root = father;
	if (right_child != -1 && right_child == root) root = father;

	if (left_child != -1) {

		if (nodes[father].child == 0) {
			nodes[father].child = left_child;
		}
		else {
			auto child = nodes[father].child;

			while (nodes[child].brother != 0) {
				child = nodes[child].brother;
			}

			nodes[child].brother = left_child;
		}
	}

	if (right_child != -1) {
		if (nodes[father].brother == 0) {
			nodes[father].brother = right_child;
		}
		else {
			auto brother = nodes[father].brother;

			while (nodes[brother].brother != 0)
				brother = nodes[brother].brother;

			nodes[brother].brother = right_child;
		}
	}
}

inline void binary_tree::print()
{
	int output = 0;

	search(root, output);

	std::cout << output << std::endl;
}

inline auto binary_tree::to_tree() -> std::shared_ptr<tree>
{
	auto tree = std::make_shared<::tree>(nodes.size() - 1, std::vector<int>());

	for (size_t index = 0; index < nodes.size(); index++)
		tree->nodes[index].key = nodes[index].key;

	std::vector<bool> is_root(nodes.size(), true);

	for (size_t index = 1; index < nodes.size(); index++) {
		auto child = nodes[index].child;

		while (child != 0) {
			tree->nodes[index].children.insert(child);

			is_root[child] = false;

			child = nodes[child].brother;
		}
	}

	for (size_t index = 1; index < is_root.size(); index++) {
		if (is_root[index] == false || nodes[index].key == -1) continue;

		tree->nodes[0].children.insert(static_cast<int>(index));
	}

	return tree;
}

inline void binary_tree::search(const int node, int& output)
{
	if (node == 0) return;

	output = output ^ nodes[node].key;

	search(nodes[node].child, output);
	search(nodes[node].brother, output);
}

inline void binary_tree::shift_to_fit(size_t target)
{
	for (size_t index = nodes.size(); index < target; index++) {
		nodes.push_back(binary_tree_node(static_cast<int>(index), 0, 0));
	}
}