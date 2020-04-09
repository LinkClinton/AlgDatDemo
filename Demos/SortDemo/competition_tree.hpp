#pragma once

#include <algorithm>
#include <vector>

template <typename T, typename Compare = std::less<T>>
class competition_tree final {
public:
	using value_type = T;
	using identity = int;
public:
	explicit competition_tree(const std::vector<value_type>& values);

	void replay(const value_type& value = std::numeric_limits<value_type>::max());
	
	value_type top_value() const noexcept;

	identity top_id() const noexcept;

	size_t size() const noexcept;
private:
	identity winner(const identity& index0, const identity& index1) const noexcept;

	identity loser(const identity& index0, const identity& index1) const noexcept;
private:
	std::vector<value_type> mPool;

	std::vector<identity> mLoser;
	std::vector<identity> mWinner;

	Compare mCompare;
};

template <typename T, typename Compare>
competition_tree<T, Compare>::competition_tree(const std::vector<value_type>& values)
{
	mPool = values;

	size_t numNodes = 0;
	size_t layer = 1;

	while (layer < mPool.size()) {
		numNodes = numNodes + layer;

		layer = layer << 1;
	}

	numNodes = numNodes + ((mPool.size() - (layer >> 1)) << 1);
	
	mLoser = std::vector<identity>(numNodes + 1);
	mWinner = std::vector<identity>(numNodes + 1);

	for (size_t index = 0; index < mPool.size(); index++) {
		const auto node = numNodes - index;

		mLoser[node] = mWinner[node] = static_cast<identity>(index);
	}
	
	for (size_t node = numNodes; node > 1; node -= 2) {
		const auto parent = node >> 1;
		
		mWinner[parent] = winner(mWinner[node - 1], mWinner[node]);
		mLoser[parent] = loser(mWinner[node - 1], mWinner[node]);
	}
}

template <typename T, typename Compare>
void competition_tree<T, Compare>::replay(const value_type& value)
{
	mPool[mWinner[1]] = value;

	for (auto node = mWinner.size() - 1 - mWinner[1]; node > 1; node >>= 1) {
		const auto parent = node >> 1;

		// because we change the winner node, so the loser must be other node.
		mWinner[parent] = winner(mWinner[node], mLoser[parent]);
		mLoser[parent] = loser(mWinner[node], mLoser[parent]);
	}
}

template <typename T, typename Compare>
typename competition_tree<T, Compare>::value_type competition_tree<T, Compare>::top_value() const noexcept
{
	return mPool[mWinner[1]];
}

template <typename T, typename Compare>
typename competition_tree<T, Compare>::identity competition_tree<T, Compare>::top_id() const noexcept
{
	return mWinner[1];
}

template <typename T, typename Compare>
size_t competition_tree<T, Compare>::size() const noexcept
{
	return mPool.size();
}

template <typename T, typename Compare>
typename competition_tree<T, Compare>::identity competition_tree<T, Compare>::winner(
	const identity& index0, const identity& index1) const noexcept
{
	return mCompare(mPool[index0], mPool[index1]) ? index0 : index1;
}

template <typename T, typename Compare>
typename competition_tree<T, Compare>::identity competition_tree<T, Compare>::loser(
	const identity& index0, const identity& index1) const noexcept
{
	return mCompare(mPool[index0], mPool[index1]) ? index1 : index0;
}
