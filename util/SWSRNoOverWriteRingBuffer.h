#ifndef SWSR_NO_OVER_WRITE_RING_BUFFER_H
#define SWSR_NO_OVER_WRITE_RING_BUFFER_H

#include <memory>
#include <atomic>

template<typename T> class SWSRNoOverWriteRingBuffer {
public:
    SWSRNoOverWriteRingBuffer(std::size_t size);
    ~SWSRNoOverWriteRingBuffer();
    bool write(const T &data);
    bool read(T &data);
    bool empty();

private:
    inline void incrementPointer(T*& ptr) {
        ptr++;
        if (ptr == array_ + size_) {
            ptr = array_;
        }
    }

private:
    std::size_t size_;
    T* array_;
    T* writePtr_;
    T* readPtr_;
    std::atomic<std::size_t> count_;
};

template<typename T> SWSRNoOverWriteRingBuffer<T>::SWSRNoOverWriteRingBuffer(std::size_t size) : size_(size)
{
    if (size_ <= 0) {
        throw std::invalid_argument("Size must be greater than zero.");
    }
    array_ = new T[size_];
    writePtr_ =  array_;
    readPtr_ = array_;
    count_.store(0, std::memory_order_release);
}

template<typename T> SWSRNoOverWriteRingBuffer<T>::~SWSRNoOverWriteRingBuffer()
{
    delete[] array_;
}

template<typename T> bool SWSRNoOverWriteRingBuffer<T>::write(const T &data)
{
    if (count_.load(std::memory_order_acquire) == size_)
        return false;

    *writePtr_ = data;
    incrementPointer(writePtr_);
    count_.fetch_add(1, std::memory_order_acq_rel);
    return true;
}

template<typename T> bool SWSRNoOverWriteRingBuffer<T>::read(T &data) {

    if (count_.load(std::memory_order_acquire) == 0)
        return false;

    auto readPtrTmp = readPtr_;

    data = std::move(*readPtr_);
    incrementPointer(readPtr_);
    count_.fetch_sub(1, std::memory_order_acq_rel);
    return true;
}

template<typename T> bool SWSRNoOverWriteRingBuffer<T>::empty() {
    return (count_.load(std::memory_order_acquire) == 0);
}

#endif