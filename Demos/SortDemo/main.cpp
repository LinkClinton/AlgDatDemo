#include "competition_tree.hpp"
#include "virtual_memory.hpp"

#include <chrono>

using element = int;

std::vector<element> copy_from_memory(virtual_memory<element>& memory, size_t begin, size_t end) {
	std::vector<element> elements;

	for (size_t index = begin; index < end; index++)
		elements.push_back(memory[index]);

	return elements;
}

void merge_sort(virtual_memory<element>& memory, memory_usage& usage, size_t lhs, size_t rhs, size_t k) {
	if (lhs == rhs) return;
	
	const auto size = rhs - lhs;

	// when the size of block is less than memory size, we can use competition tree to sort them
	if (size <= memory.memory_size()) {
		// read block from disk
		memory.read_block(lhs);
		
		competition_tree<element> tree(copy_from_memory(memory, lhs, rhs));

		for (size_t index = 0; index < tree.size(); index++) {
			// use competition tree to get the top value, and write it to memory
			memory[lhs + index] = tree.top_value();

			tree.replay();
		}

		// write block to disk
		memory.write_back();

		return;
	}

	// when the size of block is not less than memory size
	// we will divide the memory into k group memory
	const auto group_size = (size % k == 0) ? (size / k) : ((size / k) + 1);

	std::vector<size_t> group_begin(k);
	std::vector<size_t> group_end(k);

	// sort groups
	for (size_t index = 0; index < k; index++) {
		group_begin[index] = std::min(index * group_size + lhs, rhs);
		group_end[index] = std::min((index + 1) * group_size + lhs, rhs);

		merge_sort(memory, usage, group_begin[index], group_end[index], k);
	}

	std::vector<element> init_values(k);

	for (size_t index = 0; index < k; index++) {
		// when the group is empty, we set the max value
		if (group_begin[index] == group_end[index]) {
			init_values[index] = std::numeric_limits<element>::max();

			continue;
		}

		// read the first element in group
		// we use only use one cache, so when we change the group we need read a new block
		// and we do not need write block, because we do not change anything
		memory.read_block(group_begin[index]);
		
		init_values[index] = memory[group_begin[index]++];
	}

	// build competition tree to sort groups
	competition_tree<element> tree(init_values);
		
	// because we can not modify the memory before we finished to sort the groups
	// we will use a temp memory to store the result
	virtual_memory<element> temp_memory(size, memory.memory_size());

	for (size_t index = 0; index < size; index++) {
		const auto end = std::min(temp_memory.position() + temp_memory.memory_size(), temp_memory.disk_size());
		const auto group_id = tree.top_id();
		
		// when the index is out of block, we need write block to disk and read a new block from disk
		if (index < temp_memory.position() || index >= end) {
			temp_memory.write_back();
			
			temp_memory.read_block(index);
		}

		temp_memory[index] = tree.top_value();

		// when the group is not empty, we will read it from memory
		// virtual_memory::read_with_cache() will read data from cache
		// when the data is not in the cache, we will read it from disk
		// notice : the old block will not write back when we read a new block
		if (group_begin[group_id] != group_end[group_id])
			tree.replay(memory.read_with_cache(group_begin[group_id]++));
		else
			tree.replay();
	}

	// write the last block to disk
	temp_memory.write_back();

	// now, copy the temp_memory to memory
	// virtual_memory::write_with_cache() will write data to cache
	// when the data is not in the cache, we will read it from disk
	// notice : the old block will write back when we read a new block
	for (size_t index = 0; index < size; index++) {
		memory.write_with_cache(lhs + index, temp_memory.read_with_cache(index));
	}

	// write the last block to disk
	memory.write_back();
	
	// we also need record the usage of temp memory
	usage = usage + temp_memory.usage();
}

/*
 * input format : program_name file_name k memory_size output_file
 * default value: k = 2, memory_size = 1024, output_file = file_name
 */
int main(int argc, char** argv) {
	std::ios::sync_with_stdio(false);
	
	if (argc < 2) {
		std::cout << "Error : the arguments of program is invalid." << std::endl;

		return 0;
	}

	std::string file_name = std::string(argv[1]);
	std::string output = file_name;

	size_t memory_size = 1024;
	size_t k = 2;
	
	if (argc >= 3) k = std::stoul(argv[2]);
	if (argc >= 4) memory_size = std::stoul(argv[3]);
	if (argc >= 5) output = std::string(argv[4]);

	virtual_memory<element> memory(file_name, memory_size);
	memory_usage usage;

	const auto start = std::chrono::high_resolution_clock::now();
	
	merge_sort(memory, usage, 0, memory.disk_size(), k);

	const auto end = std::chrono::high_resolution_clock::now();
	
	usage = usage + memory.usage();

	memory.output(output);
	
	std::cout << "Info : sort finished." << std::endl;
	std::cout << "Info : disk read count : " << usage.read_count << "." << std::endl;
	std::cout << "Info : disk write count : " << usage.write_count << "." << std::endl;
	std::cout << "Info : time cost " << std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count() << "s." << std::endl;
}