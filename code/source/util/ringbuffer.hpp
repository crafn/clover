#ifndef CLOVER_UTIL_RINGBUFFER_HPP
#define CLOVER_UTIL_RINGBUFFER_HPP

#include "build.hpp"
#include "dyn_array.hpp"
// .tpp
#include "ensure.hpp"

/// @todo Use util::Atomic
#include <atomic>

namespace clover {
namespace util {

/// FIFO mutexless circular buffer
template <typename T>
class RingBuffer {
public:
	RingBuffer(SizeType size=0);
	RingBuffer(const RingBuffer&);
	RingBuffer(RingBuffer&& other):RingBuffer(other){}

	RingBuffer& operator=(const RingBuffer&)= default;
	RingBuffer& operator=(RingBuffer&&)= default;

	/// Resize buffer. Don't call when there's a thread reading or writing
	void resize(SizeType s);
	SizeType size() const;

	/// How much there's free space
	SizeType getFreeCount() const;
	bool isEmpty() const;

	/// How much there's used space
	SizeType getUsedCount() const;
	bool isFull() const;

	/// Writes data at the end of buffer
	void write(const util::DynArray<T>& data);
	void write(const util::DynArray<T>& data, SizeType begin, SizeType end);

	/// Reads data at the start of buffer
	util::DynArray<T> read(SizeType read_count);
	void read(util::DynArray<T>& target, SizeType read_count);

private:
	util::DynArray<T> buffer;
	std::atomic<SizeType> readIndex, writeIndex, usedCount;
};

#include "ringbuffer.tpp"

} // util
} // clover

#endif // CLOVER_UTIL_RINGBUFFER_HPP