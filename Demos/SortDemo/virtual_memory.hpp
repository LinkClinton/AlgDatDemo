#pragma once

#include <algorithm>
#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <string>

/*
 * virtual memory is a class that simulate the disk to simplify model of sort
 * when it created, it will load the disk file to memory as virtual disk(like memory map without virtual memory)
 * we can access element in the block we read(we use seek to set the first element we will read to block)
 * read data from block or write data to block do not need access disk, but read/write block need access the disk.
 */

struct memory_usage {
    size_t write_count = 0;
    size_t read_count = 0;

    memory_usage() = default;

    memory_usage operator+(const memory_usage& rhs) const noexcept;
};

inline memory_usage memory_usage::operator+(const memory_usage& rhs) const noexcept
{
    return {
        write_count + rhs.write_count,
        read_count + rhs.read_count
    };
}

template <typename T>
class virtual_memory final {
public:
    using value_type = T;
public:
    explicit virtual_memory(const std::string& file_name, size_t memory_size);

    explicit virtual_memory(size_t disk_size, size_t memory_size);
	
	~virtual_memory() = default;
	
    bool read_block(size_t position);

    void write_back();

    auto access(size_t index) -> value_type&;

    auto read_with_cache(size_t index) -> const value_type&;

    void write_with_cache(size_t index, const value_type& value);

    void output(const std::string& file_name = "") const noexcept;
	
    auto memory() const noexcept -> const std::vector<value_type>&;

    size_t position() const noexcept;
	
    size_t memory_size() const noexcept;
	
    size_t disk_size() const noexcept;

    memory_usage usage() const noexcept;
    
    value_type& operator[](size_t index);
private:
    std::vector<value_type> mMemory;
    std::vector<value_type> mDisk;

    std::string mFileName;

    memory_usage mDiskUsage;
	
    size_t mPosition = 0;
};

template <typename T>
virtual_memory<T>::virtual_memory(const std::string& file_name, size_t memory_size) :
	mMemory(memory_size), mFileName(file_name)
{
    std::ifstream stream(mFileName);

	if (!stream.is_open()) {
        std::cout << "Error : the file is invalid." << std::endl;
		
        return;
	}

	if (mMemory.size() == 0) {
        std::cout << "Error : the block size can not be zero." << std::endl;

        return;
	}
	
    value_type value;

	while (stream >> value) mDisk.push_back(value);

    stream.close();
}

template <typename T>
virtual_memory<T>::virtual_memory(size_t disk_size, size_t memory_size) :
	mMemory(memory_size), mDisk(disk_size), mFileName("")
{	
}

template <typename T>
bool virtual_memory<T>::read_block(size_t position)
{
	if (position >= mDisk.size()) {
        std::cout << "Error : the position is out of file." << std::endl;

        return false;
	}

    mPosition = position;

    const auto end = std::min(mPosition + mMemory.size(), mDisk.size());

    for (size_t index = mPosition; index < end; index++)
        mMemory[index - mPosition] = mDisk[index];

    mDiskUsage.read_count++;

    return true;
}

template <typename T>
void virtual_memory<T>::write_back()
{
    const auto end = std::min(mPosition + mMemory.size(), mDisk.size());

    for (size_t index = mPosition; index < end; index++)
        mDisk[index] = mMemory[index - mPosition];

    mDiskUsage.write_count++;
}

template <typename T>
auto virtual_memory<T>::access(size_t index) -> value_type& 
{
    const auto end = std::min(mPosition + mMemory.size(), mDisk.size());

	if (index < mPosition || index >= end) {
        std::cout << "Warning : access a value that is not in memory, we will write old block to disk and read new block from disk." << std::endl;

        write_back();
		
        read_block(index);
	}

    return mMemory[index - mPosition];
}

template <typename T>
auto virtual_memory<T>::read_with_cache(size_t index) -> const value_type& 
{
    const auto end = std::min(mPosition + mMemory.size(), mDisk.size());

    if (index < mPosition || index >= end) read_block(index);
    
    return mMemory[index - mPosition];
}

template <typename T>
void virtual_memory<T>::write_with_cache(size_t index, const value_type& value)
{
    const auto end = std::min(mPosition + mMemory.size(), mDisk.size());

    if (index < mPosition || index >= end) {
        write_back();
    	
        read_block(index);
    }

    mMemory[index - mPosition] = value;
}

template <typename T>
void virtual_memory<T>::output(const std::string& file_name) const noexcept
{
    std::ofstream stream;

    if (file_name == "") stream.open(mFileName);
    else stream.open(file_name);

    for (const auto& value : mDisk) stream << value << std::endl;
	
    stream.close();
}

template <typename T>
auto virtual_memory<T>::memory() const noexcept -> const std::vector<value_type>& 
{
    return mMemory;
}

template <typename T>
size_t virtual_memory<T>::position() const noexcept
{
    return mPosition;
}

template <typename T>
size_t virtual_memory<T>::memory_size() const noexcept
{
    return mMemory.size();
}

template <typename T>
size_t virtual_memory<T>::disk_size() const noexcept
{
    return mDisk.size();
}

template <typename T>
memory_usage virtual_memory<T>::usage() const noexcept
{
    return mDiskUsage;
}

template <typename T>
typename virtual_memory<T>::value_type& virtual_memory<T>::operator[](size_t index)
{
    return access(index);
}
