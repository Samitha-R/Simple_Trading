#ifndef BUFFER_H
#define BUFFER_H

#include "TypeDef.h"
#include <atomic>
#include <vector>
#include <thread>

template<typename T> class SWMRArray
{
private:
    struct BufferItem
    {
        BufferItem() {}
        BufferItem(const BufferItem& c) {
            assign(c);
        }

        T data_;
        std::atomic<std::size_t> updateCount_ = 0;
    private:
        void assign(const BufferItem& c) {
            std::size_t updateCount1;
            std::size_t updateCount2;
            do {
                updateCount1 = c.updateCount_.load(std::memory_order_acquire);
                data_ = c.data_;
                updateCount_.store(updateCount1,std::memory_order_release);
                updateCount2 = c.updateCount_.load(std::memory_order_acquire);
            } while ((updateCount1 != updateCount2) && (updateCount2 %2));

        }
    };

public:
    SWMRArray(std::size_t size) : buffer_(size) {}
public:
    void update(int index, const T& data);
    bool getData(int index, T& data) const;
private:
    std::vector<BufferItem> buffer_;
};

template<typename T> void SWMRArray<T>::update(int index, const T& data)
{
    buffer_[index].updateCount_.fetch_add(1, std::memory_order_acq_rel);
    buffer_[index].data_ = data;
    buffer_[index].updateCount_.fetch_add(1, std::memory_order_acq_rel);
}

template<typename T> bool SWMRArray<T>::getData(int index, T& data) const
{
    std::size_t updateCount1;
    std::size_t updateCount2;
    do {
        updateCount1 = buffer_[index].updateCount_.load(std::memory_order_acquire);

        if (updateCount1 == 0)
            return false;

        data = buffer_[index].data_;

        updateCount2 = buffer_[index].updateCount_.load(std::memory_order_acquire);
    } while ((updateCount1 != updateCount2) && (updateCount2 %2));
    return true;
}

/*
template<typename T> class MWMRQueue {
private:
    struct Status {
        Status () : writerIn_(0), readerIn_(0), dataValid_ (0) {}
        char writerIn_ : 2;
        char readerIn_ : 2;
        char dataValid_ : 4;
    };
public:
    MWMRQueue(std::size_t size) : size_(size) {
        array_ = new T[size];
        status_ = new std::atomic<Status>[size];
        writeIndex_.store(0);
        readIndex_.store(0);
    }
    ~MWMRQueue() {
        delete[] array_;
        delete[] status_;
    }
public:
    bool push(const T& data);
    bool pop(T& data);

private:
    inline std::size_t incrementIndex(std::size_t index) {
    return ++index % size_;
}
private:
   std::size_t size_;
   std::atomic<int> writeIndex_;
   std::atomic<int> readIndex_ ;
   std::atomic<Status> *status_;
   T* array_;
};

template<typename T> bool MWMRQueue<T>::push(const T& data)
{
    while (true) {

        auto wi = writeIndex_.load();
        auto wistat = status_[wi].load();

        if (wistat.writerIn_ ) {
            continue;
        }

        if (wistat.dataValid_ == 1)
            return false;

        auto wistatnew = wistat;
        wistatnew.writerIn_ = true;

        if (!status_[wi].compare_exchange_strong(wistat, wistatnew))
            continue;

        auto nextWi = incrementIndex(wi);
        writeIndex_.store(nextWi);

        try {
            array_[wi] = data;
        } catch (...) {
            wistat.writerIn_ = false;
            wistat.dataValid_ = 2; //corrupted
            status_[wi].store(wistat);
            return false;
        }
        wistat.writerIn_ = false;
        wistat.dataValid_ = 1;
        status_[wi].store(wistat);


        return true;

    }

}


template<typename T> bool MWMRQueue<T>::pop(T& data)
{
    while (true) {

        auto ri = readIndex_.load();
        auto ristat = status_[ri].load();

        if (ristat.readerIn_) {
           continue;   // wait till reader inside update the read poinnter
        }

        if (ristat.dataValid_ == 0) {
            return false;  // data is not written yet
        }

        auto ristatnew = ristat;
        ristatnew.readerIn_ = true;

        if (!status_[ri].compare_exchange_strong(ristat, ristatnew)) // another reader has occupied the place. wait till it updtes the next pointer
            continue;


        if (ristat.dataValid_ == 2) { // ignore corrupted data
            ristat.readerIn_ = false;
            status_[ri].store(ristat);
            auto nextri = incrementIndex(ri);
            readIndex_.store(nextri);
            continue;
        }

        try {
            data = array_[ri]; // reading fail
        } catch (...) {
            ristat.readerIn_ = false;
            status_[ri].store(ristat);
            return false;
        }

        auto nextri = incrementIndex(ri);
        readIndex_.store(nextri);
        ristat.readerIn_ = false;
        ristat.dataValid_ = 0;
        status_[ri].store(ristat);
        return true;

    }

}
*/

#endif