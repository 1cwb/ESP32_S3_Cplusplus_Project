#include <mutex>
#include <condition_variable>
#include <queue>

template<typename T>
class MsgQueue
{
public:
    void sendQueue(T data)
    {
        std::unique_lock<std::mutex> lock(mtx_);
        q_.push_back(data);
        cdv_.notify_one();  //唤醒一个阻塞的线程
    }
    T getQueueData() {
        std::unique_lock<std::mutex> lock(mtx_);
        while (q_.empty())
        {
            cdv_.wait(lock);
        }
        auto ret = std::move(q_.front());
        q_.pop_front();
        return ret;
    }
    size_t size() {
        std::unique_lock<std::mutex> lock(mtx_);
        return q_.size();
    }
private:
    std::deque<T> q_;
    std::mutex mtx_;
    std::condition_variable cdv_;
};