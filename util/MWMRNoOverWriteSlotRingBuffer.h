#ifndef MWMR_NO_OVERWRITE_RING_BUFFER_H
#define MWMR_NO_OVERWRITE_RING_BUFFER_H

#include <memory>
#include <atomic>
#include <vector>

template<typename T> class MWMRNoOverWriteSlotRingBuffer
{
private:
    enum class DataStatus {NO_DATA = 0, DATA_VALID, DATA_WRITING, DATA_READING};
public:
    struct Slot {
    friend class MWMRNoOverWriteSlotRingBuffer<T>;
    public:
        Slot() { }
        Slot(const T& data) : data_(data) {}
        Slot(const Slot& t) : data_(t.data_) {
            dataStatus_.store(t.dataStatus_.load(std::memory_order_acquire), std::memory_order_release);
        }
        ~Slot() {}
    public:
        T& getData() { return data_; }
    private:
        T data_;
        std::atomic<DataStatus> dataStatus_ = {DataStatus::NO_DATA};
    };

public:
    MWMRNoOverWriteSlotRingBuffer(std::size_t size) : size_(size), array_(size) { }
    MWMRNoOverWriteSlotRingBuffer(std::size_t size, const T& t) : size_(size), array_(size, Slot(t)) { }

public:
    Slot* getWriteSlot();
    Slot* getReadSlot();
    void setWriteComplete(Slot *t);
    void setReadComplete(Slot *t);

private:
    inline std::size_t incrementIndex(std::size_t index) { return ++index % size_;}
private:
   std::size_t size_;
   std::atomic<int> writeIndex_ = {0 };
   std::atomic<int> readIndex_  = { 0 };
   std::vector<Slot> array_;
};

template<typename T> MWMRNoOverWriteSlotRingBuffer<T>::Slot* MWMRNoOverWriteSlotRingBuffer<T>::getWriteSlot()
{
    while(true) {
 
        auto wi = writeIndex_.load(std::memory_order_acquire);

        auto& slot = array_[wi];
        auto dataStatus = slot.dataStatus_.load(std::memory_order_acquire);

        if (dataStatus == DataStatus::DATA_WRITING)
            continue;

        if (dataStatus != DataStatus::NO_DATA)
            return nullptr;

        auto dataStatusCp = dataStatus;

        if (!slot.dataStatus_.compare_exchange_strong(dataStatusCp, DataStatus::DATA_WRITING, std::memory_order_acq_rel))
            continue;

        auto win = incrementIndex(wi);
        writeIndex_.store(win, std::memory_order_release);

        return &slot;

    }
}

template<typename T> void MWMRNoOverWriteSlotRingBuffer<T>::setWriteComplete(Slot *t)
{
    t->dataStatus_.store(DataStatus::DATA_VALID, std::memory_order_release);
}

template<typename T> MWMRNoOverWriteSlotRingBuffer<T>::Slot* MWMRNoOverWriteSlotRingBuffer<T>::getReadSlot()
{
    while(true) {

        auto ri = readIndex_.load(std::memory_order_acquire);

        auto &slot = array_[ri];
        auto dataStatus = slot.dataStatus_.load(std::memory_order_acquire);

        if (dataStatus == DataStatus::DATA_READING)
            continue;

        if (dataStatus != DataStatus::DATA_VALID)
            return nullptr;

        auto dataStatuscp = dataStatus;

        if (!slot.dataStatus_.compare_exchange_strong(dataStatuscp, DataStatus::DATA_READING, std::memory_order_acq_rel))
            continue;

        auto rin = incrementIndex(ri);
        readIndex_.store(rin, std::memory_order_release);

        return &slot;
    }
}

template<typename T> void MWMRNoOverWriteSlotRingBuffer<T>::setReadComplete(Slot *t)
{
    t->dataStatus_.store(DataStatus::NO_DATA, std::memory_order_release);
}

#endif