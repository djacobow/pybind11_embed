#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

// A simple and hopefully correct generic thread-safe queue class

template<class T>
class TSafeQueue {
    public:
        TSafeQueue() = default;
        void Push(T e) {
            std::lock_guard<std::mutex> lock(mtx_);
            q_.push(e);
            cv_.notify_one();
        }

        // does not block on non-emptiness but will still block
        // to get the mutex
        bool PopNoBlock(T& e) {
            std::unique_lock<std::mutex> lock(mtx_);
            if (q_.empty()) {
                return false;
            }
            e = q_.front();
            q_.pop();
            return true;
        }

        // this blocks on the mutex *and* non-emptiness
        T PopBlock() {
            T elem;
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [this]{return !q_.empty();});
            if (!q_.empty()) {
                elem = q_.front();
                q_.pop();
            }
            return elem;
        }
    private:    
        std::mutex mtx_;
        std::condition_variable cv_;
        std::queue<T> q_;
};
