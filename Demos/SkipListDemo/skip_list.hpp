#pragma once

#include <random>
#include <memory>

#define MAX_LEVEL 10

template<typename Key, typename Value>
struct skip_node {
	std::shared_ptr<skip_node> mNext[MAX_LEVEL];

	Value mValue;
	Key mKey;

	skip_node(const Key& key, const Value& value) :
		mValue(value), mKey(key) {}
};

template<typename Key, typename Value>
class skip_list {
public:
	skip_list() {
		mHead = std::make_shared<skip_node<Key, Value>>(std::numeric_limits<Key>::min(), Value());
		mTail = std::make_shared<skip_node<Key, Value>>(std::numeric_limits<Key>::max(), Value());

		for (size_t index = 0; index < MAX_LEVEL; index++)
			mHead->mNext[index] = mTail;

		mSize = 0;
	}

	void insert(const Key& key, const Value& value) {
		auto node = mHead;
		auto layer = random_level();

		std::shared_ptr<skip_node<Key, Value>> needUpdate[MAX_LEVEL];

		for (auto level = MAX_LEVEL - 1; level >= 0; level--) {
			while (key > node->mNext[level]->mKey&& node->mNext[level] != mTail) node = node->mNext[level];

			needUpdate[level] = node;
		}

		const auto newNode = std::make_shared<skip_node<Key, Value>>(key, value);

		for (auto level = layer; level >= 0; level--) {
			newNode->mNext[level] = needUpdate[level]->mNext[level];
			needUpdate[level]->mNext[level] = newNode;
		}

		mSize++;
	}

	auto erase(const Key& key) -> bool {
		auto node = mHead;

		std::shared_ptr<skip_node<Key, Value>> needUpdate[MAX_LEVEL];

		for (auto level = MAX_LEVEL - 1; level >= 0; level--) {
			while (key > node->mNext[level]->mKey&& node->mNext[level] != mTail) node = node->mNext[level];

			needUpdate[level] = node;
		}

		auto result = false;

		for (auto level = MAX_LEVEL - 1; level >= 0; level--) {
			if (needUpdate[level]->mNext[level]->mKey == key) {
				result = true;

				needUpdate[level]->mNext[level] = needUpdate[level]->mNext[level]->mNext[level];
			}
		}

		mSize--;

		return result;
	}

	auto find(const Key& key) const -> Value {
		auto node = mHead;

		for (auto level = MAX_LEVEL - 1; level >= 0; level--) {
			while (key > node->mNext[level]->mKey&& node->mNext[level] != mTail) node = node->mNext[level];

			if (key == node->mNext[level]->mKey) return node->mNext[level]->mValue;
		}

		return "-1";
	}

	auto min() const -> Key {
		return mHead->mNext[0]->mKey;
	}

	auto max() const -> Key {
		auto node = mHead;

		for (auto level = MAX_LEVEL - 1; level >= 0; level--) {
			while (node->mNext[level] != mTail) node = node->mNext[level];
		}

		return node->mKey;
	}

	auto size() const noexcept -> size_t { return mSize; }

	auto head() const noexcept -> std::shared_ptr<skip_node<Key, Value>> { return mHead; }

	auto tail() const noexcept -> std::shared_ptr<skip_node<Key, Value>> { return mTail; }
private:
	static auto random_level() -> int {
		static std::default_random_engine random;
		static std::uniform_int_distribution<int> vRange(0, 10000);

		int level = 0;

		while (vRange(random) % 2 && level < MAX_LEVEL - 1) level++;

		return level;
	}
private:
	std::shared_ptr<skip_node<Key, Value>> mHead;
	std::shared_ptr<skip_node<Key, Value>> mTail;

	size_t mSize;
};