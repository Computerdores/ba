/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
#pragma once

// licensed under MIT from https://github.com/fastflow/fastflow/

#include <cassert>
#include <cstdint>
#include <memory>

#include "utils.h"

#ifdef _MSC_VER
#pragma warning(disable : 4200)  // nonstandard extension used : zero-sized array in struct/union
#define abort() __debugbreak(), (abort)()
#endif

#define INLINE static __inline
#define NOINLINE
#if !defined(CACHE_LINE_SIZE)
#define CACHE_LINE_SIZE 128
#endif

INLINE void *aligned_malloc(const size_t sz) {
    void *mem;
    if (posix_memalign(&mem, CACHE_LINE_SIZE, sz)) {
        return nullptr;
    }
    return mem;
}

INLINE void aligned_free(void *mem) { free(mem); }

INLINE void atomic_addr_store_release(void *volatile *addr, void *val) {
    __asm __volatile("" ::: "memory");
    addr[0] = val;
}

INLINE void *atomic_addr_load_acquire(void *volatile *addr) {
    void *val;
    val = addr[0];
    __asm __volatile("" ::: "memory");
    return val;
}

namespace queues {
template <typename T = u64>
class ff_queue {
  public:
    ff_queue(ff_queue const &) = delete;

    ff_queue(const size_t bucket_size, const size_t max_bucket_count)
        : bucket_size_(bucket_size), max_bucket_count_(max_bucket_count) {
        bucket_count_ = 0;
        bucket_t *bucket = alloc_bucket(bucket_size_);
        head_pos_ = bucket->data;
        tail_pos_ = bucket->data;
        tail_end_ = bucket->data + bucket_size_;
        tail_next_ = nullptr;
        tail_bucket_ = bucket;
        last_bucket_ = bucket;
        *reinterpret_cast<void **>(head_pos_) = reinterpret_cast<void *>(1);
    }

    void operator=(ff_queue const &) = delete;

    ~ff_queue() {
        bucket_t *bucket = last_bucket_;
        while (bucket != nullptr) {
            bucket_t *next_bucket = bucket->next;
            aligned_free(bucket);
            bucket = next_bucket;
        }
    }

    bool enqueue(T item) {
        T *slot = static_cast<T *>(enqueue_prepare(sizeof(T)));
        if (!slot) return false;
        *slot = item;
        enqueue_commit();
        return true;
    }

    void *enqueue_prepare(const size_t sz) {
        assert(reinterpret_cast<uintptr_t>(tail_pos_) % sizeof(void *) == 0);
        if (const size_t msg_size = (sz + sizeof(void *) - 1 & ~(sizeof(void *) - 1)) + sizeof(void *);
            static_cast<size_t>(tail_end_ - tail_pos_) >= msg_size + sizeof(void *)) {
            tail_next_ = tail_pos_ + msg_size;
            return tail_pos_ + sizeof(void *);
        }
        return enqueue_prepare_slow(sz);
    }

    void enqueue_commit() {
        *reinterpret_cast<std::uint8_t *volatile *>(tail_next_) = reinterpret_cast<std::uint8_t *>(1);
        atomic_addr_store_release(reinterpret_cast<void *volatile *>(tail_pos_), tail_next_);
        tail_pos_ = tail_next_;
    }

    std::optional<T> dequeue() {
        T *slot = static_cast<T *>(dequeue_prepare());
        if (!slot) return std::nullopt;
        T out = *slot;
        dequeue_commit();
        return out;
    }

    void *dequeue_prepare() {
        assert(reinterpret_cast<uintptr_t>(head_pos_) % sizeof(void *) == 0);
        void *next = atomic_addr_load_acquire(reinterpret_cast<void *volatile *>(head_pos_));
        if ((reinterpret_cast<uintptr_t>(next) & 1) == 0) {
            u8 *msg = head_pos_ + sizeof(void *);
            return msg;
        }
        if ((reinterpret_cast<uintptr_t>(next) & ~1) == 0) {
            return nullptr;
        }
        atomic_addr_store_release(reinterpret_cast<void *volatile *>(&head_pos_),
                                  reinterpret_cast<std::uint8_t *>(reinterpret_cast<uintptr_t>(next) & ~1));
        return dequeue_prepare();
    }

    void dequeue_commit() {
        u8 *next = *reinterpret_cast<std::uint8_t *volatile *>(head_pos_);
        assert(next != nullptr);
        atomic_addr_store_release(reinterpret_cast<void *volatile *>(&head_pos_), next);
    }

  private:
    struct bucket_t {
        bucket_t *next;
        size_t size;
        u8 data[0];
    };

    u8 *volatile head_pos_;

    u8 pad_[CACHE_LINE_SIZE] = {};

    u8 *tail_pos_;
    u8 *tail_end_;
    u8 *tail_next_;
    bucket_t *tail_bucket_;
    bucket_t *last_bucket_;
    size_t const bucket_size_;
    size_t const max_bucket_count_;
    size_t bucket_count_;

    bucket_t *alloc_bucket(const size_t sz) {
        const auto bucket = static_cast<bucket_t *>(aligned_malloc(sizeof(bucket_t) + sz));
        if (bucket == nullptr) throw std::bad_alloc();
        bucket->next = nullptr;
        bucket->size = sz;
        bucket_count_ += 1;
        return bucket;
    }

    NOINLINE void *enqueue_prepare_slow(const size_t sz) {
        size_t bucket_size = bucket_size_;
        if (bucket_size < sz + 2 * sizeof(void *)) bucket_size = sz + 2 * sizeof(void *);

        bucket_t *bucket = nullptr;
        u8 *head_pos =
            static_cast<std::uint8_t *>(atomic_addr_load_acquire(reinterpret_cast<void *volatile *>(&head_pos_)));
        while (head_pos < last_bucket_->data || head_pos >= last_bucket_->data + last_bucket_->size) {
            bucket = last_bucket_;
            last_bucket_ = bucket->next;
            bucket->next = nullptr;
            assert(last_bucket_ != nullptr);

            if (bucket->size < bucket_size ||
                bucket_count_ > max_bucket_count_ &&
                    (head_pos < last_bucket_->data || head_pos >= last_bucket_->data + last_bucket_->size)) {
                aligned_free(bucket);
                bucket = nullptr;
                continue;
            }
            break;
        }

        if (bucket == nullptr) bucket = alloc_bucket(bucket_size);
        *reinterpret_cast<void *volatile *>(bucket->data) = reinterpret_cast<void *>(1);
        atomic_addr_store_release(reinterpret_cast<void *volatile *>(tail_pos_),
                                  reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(bucket->data) | 1));
        tail_pos_ = bucket->data;
        tail_end_ = tail_pos_ + bucket_size;
        tail_bucket_->next = bucket;
        tail_bucket_ = bucket;
        return enqueue_prepare(sz);
    }
};
}  // namespace queues
