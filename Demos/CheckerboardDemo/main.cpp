#include <Purezento/Extensions/ImGui/imgui.hpp>
#include <Purezento/Runtime/runtime.hpp>

#include <iostream>
#include <cstring>
#include <cassert>
#include <vector>
#include <queue>
#include <array>
#include <set>

using identity = int;

const auto window_width = 1280;
const auto window_height = 720;
const auto font_size = 12;

const auto red_color = purezento::color(1, 0, 0, 1);
const auto green_color = purezento::color(0, 0.7f, 0, 1);

auto runtime = std::make_shared<purezento::runtime>(
	purezento::runtime_startup(
		"CheckerboardDemo",
		purezento::font("./Resources/Consola.ttf", font_size),
		window_width, window_height)
	);


std::array<purezento::color, 4> ColorTable = {
	purezento::color(1, 0, 0, 1),
	purezento::color(0, 1, 0, 1),
	purezento::color(0, 0, 1, 1),
	purezento::color(1, 1, 0, 1)
};

template <typename T>
struct range_t {
	T begin, end;

	range_t() = default;

	range_t(const T& begin, const T& end) : begin(begin), end(end) {}

	T length() const noexcept { return end - begin; }
};

template <typename T>
struct point_t {
	T x, y;

	point_t() = default;

	point_t(const T& x, const T& y) : x(x), y(y) {}
};

using range = range_t<int>;
using point = point_t<int>;

class checkerboard {
public:
	explicit checkerboard(int k);

	~checkerboard() = default;

	void build(int x, int y);

	identity color(int x, int y) const;

	identity id(int x, int y) const;
	
	int size() const noexcept;
private:
	void push_triangles(
		const range& range_x, const range& range_y, int x, int y, identity& identity);

	void push_graph();

	bool push_colors(identity x, size_t count);

	bool check(identity block, identity color) const;
	
	int block_index(int x, int y) const;

	std::vector<identity> mIdentity;
	std::vector<identity> mColors;
	std::vector<std::vector<point>> mBlocks;

	std::vector<std::set<identity>> mGraph;
	std::vector<identity> mBlockColors;
	
	int mSize;
};

checkerboard::checkerboard(int k)
{
	assert(k < 10);
	
	mSize = 1 << k;
}

void checkerboard::build(int x, int y)
{
	identity id = 0;

	mIdentity = std::vector<identity>(mSize * mSize, 0);
	mColors = std::vector<identity>(mSize * mSize, -1);
	mBlocks = std::vector<std::vector<point>>((mSize * mSize - 1) / 3 + 1);
	mGraph = std::vector<std::set<identity>>(mBlocks.size());
	mBlockColors = std::vector<identity>(mBlocks.size(), -1);
	
	push_triangles(range(0, mSize), range(0, mSize), x, y, id);
	push_graph();
	push_colors(1, 1);

	for (size_t index = 0; index < mColors.size(); index++) { 
		mColors[index] = mBlockColors[mIdentity[index]];
	}
}

identity checkerboard::color(int x, int y) const
{
	return mColors[block_index(x, y)];
}

identity checkerboard::id(int x, int y) const
{
	return mIdentity[block_index(x, y)];
}

int checkerboard::size() const noexcept
{
	return mSize;
}

void checkerboard::push_triangles(const range& range_x, const range& range_y, int x, int y, identity& identity)
{
	if (range_x.length() == 2 && range_y.length() == 2) {
		identity++;

		std::vector<point> block;
		
		for (auto px = range_x.begin; px < range_x.end; px++) {
			for (auto py = range_y.begin; py < range_y.end; py++) {
				if (px != x || py != y) {
					mIdentity[block_index(px, py)] = identity;
					block.push_back(point(px, py));
				}
			}
		}

		mBlocks[identity] = block;

		return;
	}

	const auto center_x = (range_x.begin + range_x.end) / 2;
	const auto center_y = (range_y.begin + range_y.end) / 2;

	identity++;
	
	mIdentity[block_index(center_x - 1, center_y - 1)] = identity;
	mIdentity[block_index(center_x - 1, center_y - 0)] = identity;
	mIdentity[block_index(center_x - 0, center_y - 1)] = identity;
	mIdentity[block_index(center_x - 0, center_y - 0)] = identity;

	auto x_offset = 0;
	auto y_offset = 0;

	if (x < center_x) x_offset = 1;
	if (y < center_y) y_offset = 1;

	mIdentity[block_index(center_x - x_offset, center_y - y_offset)] = 0;

	std::vector<point> block;

	for (auto offset_x = 0; offset_x < 2; offset_x++)
		for (auto offset_y = 0; offset_y < 2; offset_y++)
			if (offset_x != x_offset || offset_y != y_offset)
				block.push_back(point(center_x - offset_x, center_y - offset_y));

	mBlocks[identity] = block;
	
	std::array<int, 4> block_x = {
		center_x - 1, center_x - 1,
		center_x - 0, center_x - 0
	};
	
	std::array<int, 4> block_y = {
		center_y - 1, center_y - 0,
		center_y - 1, center_y - 0
	};

	if (x_offset == 0 && y_offset == 0) block_x[3] = x, block_y[3] = y;
	if (x_offset == 0 && y_offset == 1) block_x[2] = x, block_y[2] = y;
	if (x_offset == 1 && y_offset == 0) block_x[1] = x, block_y[1] = y;
	if (x_offset == 1 && y_offset == 1) block_x[0] = x, block_y[0] = y;

	std::array<range, 4> range_divide_x = {
		range(range_x.begin, center_x),
		range(range_x.begin, center_x),
		range(center_x, range_x.end),
		range(center_x, range_x.end)
	};
	
	std::array<range, 4> range_divide_y = {
	range(range_y.begin, center_y),
	range(center_y, range_y.end),
	range(range_y.begin, center_y),
	range(center_y, range_y.end)
	};

	for (size_t index = 0; index < 4; index++)
		push_triangles(range_divide_x[index], range_divide_y[index], block_x[index], block_y[index], identity);
}

void checkerboard::push_graph()
{
	std::array<int, 4> x_offset = { 0, 0, 1, -1 };
	std::array<int, 4> y_offset = { 1, -1, 0, 0 };

	for (size_t block_id = 1; block_id < mBlocks.size(); block_id++) {
		for (const auto& point : mBlocks[block_id]) {
			for (size_t index = 0; index < 4; index++) {
				const auto new_x = point.x + x_offset[index];
				const auto new_y = point.y + y_offset[index];

				if (new_x < 0 || new_x >= mSize) continue;
				if (new_y < 0 || new_y >= mSize) continue;

				if (mIdentity[block_index(new_x, new_y)] == block_id || 
					mIdentity[block_index(new_x, new_y)] == 0) continue;
				
				if (mGraph[block_id].find(mIdentity[block_index(new_x, new_y)]) == mGraph[block_id].end()) {
					mGraph[block_id].insert(mIdentity[block_index(new_x, new_y)]);
				}
			}
		}
	}
}

bool checkerboard::push_colors(identity x, size_t count)
{
	if (x == mBlocks.size()) return true;
	
	for (identity color = 0; color < 3; color++) {
		mBlockColors[x] = color;

		if (!check(x, mBlockColors[x])) continue;

		if (push_colors(x + 1, count)) return true;
	}

	mBlockColors[x] = -1;
	
	return false;
}

bool checkerboard::check(identity block, identity color) const
{
	for (const auto& next : mGraph[block])
		if (mBlockColors[next] == color) return false;

	return true;
}


int checkerboard::block_index(int x, int y) const
{
	return x + y * mSize;
}

auto k = 3;
auto block_x = 0;
auto block_y = 0;

auto board = std::make_shared<checkerboard>(k);

void update_checkerboard_graph(
	const std::shared_ptr<purezento::render>& render,
	const std::shared_ptr<void>& ctx)
{
	render->draw_elements["board"].clear();

	const auto size = window_height * 0.7f;
	const auto block_size = size / board->size();
	const auto begin_x = (window_width - runtime->config()->config_window_info().size.x - size) * 0.5f;
	const auto begin_y = (window_height - runtime->config()->console_window_info().size.y - size) * 0.5f;

	for (auto x = 0; x < board->size(); x++) {
		for (auto y = 0; y < board->size(); y++) {
			const auto color = board->color(x, y);
			const auto position_x = begin_x + x * block_size;
			const auto position_y = begin_y + y * block_size;
			
			if (color == -1) continue;
			
			render->draw_elements["board"].push_back(
				purezento::draw_element::rectangle_fill(
					purezento::rectangle(position_x, position_y, position_x + block_size, position_y + block_size),
					ColorTable[color]
				)
			);

			render->draw_elements["board"].push_back(
				purezento::draw_element::text(
					purezento::text(std::to_string(board->id(x, y)), purezento::vec2(position_x, position_y))
				)
			);
		}
	}
}

void board_config(
	const std::shared_ptr<purezento::config>& config,
	const std::shared_ptr<void>& ctx)
{
	auto need_update = false;

	need_update = need_update ^ ImGui::InputInt("k", &k);
	need_update = need_update ^ ImGui::InputInt("x", &block_x);
	need_update = need_update ^ ImGui::InputInt("y", &block_y);

	k = std::min(std::max(k, 1), 6);

	block_x = std::min(std::max(block_x, 0), (1 << k) - 1);
	block_y = std::min(std::max(block_y, 0), (1 << k) - 1);

	if (need_update) {
		board = std::make_shared<checkerboard>(k);
		board->build(block_x, block_y);
	}
}

int main() {
	board->build(block_x, block_y);

	runtime->config()->callbacks["board_config"] = purezento::config_callback(board_config, nullptr);
	runtime->render()->callbacks["update_checkerboard_graph"] = purezento::render_callback(update_checkerboard_graph, nullptr);
	
	runtime->run_loop();
}
