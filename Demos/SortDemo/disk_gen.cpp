#include <iostream>
#include <string>
#include <random>
#include <ctime>

int main(int argc, char** argv) {
	size_t count = 0;

	std::mt19937 rng(time(0));

	if (argc == 2) count = std::stoul(argv[1]);
	else {
		std::uniform_int_distribution<size_t> range(100, 1000000);

		count = range(rng);
	}

	std::uniform_int_distribution<int> range(1, std::numeric_limits<int>::max() - 1);

	for (size_t index = 0; index < count; index++) {
		std::cout << range(rng) << std::endl;
	}
}