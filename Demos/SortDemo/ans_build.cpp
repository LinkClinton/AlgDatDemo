#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

int main() {
	std::ios::sync_with_stdio(false);

	std::vector<int> input;

	int value;

	while (std::cin >> value) input.push_back(value);

	std::sort(input.begin(), input.end());

	for (const auto& element : input) std::cout << element << std::endl;

	return 0;
}