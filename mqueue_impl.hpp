#ifndef MQUEUE_IMPL
#define MQUEUE_IMPL

#include <mutex>
#include <condition_variable>
#include "mqueue_interface.hpp"

namespace mqueue {

    template<typename TUser>
    class MQueueImpl : public MQueueTxInterface<TUser>,
                       public MQueueRxInterface<TUser> {

        std::mutex m_mtx;
        std::condition_variable m_cv;
        typename MQueueRxInterface<TUser>::queue_type m_sharedQueue;

        void readSharedQueue(typename MQueueRxInterface<TUser>::queue_type& rxQueue) {
            while(!m_sharedQueue.empty()) {
                auto& cptr = m_sharedQueue.front();
                rxQueue.push_back(std::move(cptr));
                m_sharedQueue.pop_front();
            }
        }

        void send(const commands::CmdBase<TUser> * const cmd) override {
            {
                std::unique_lock<std::mutex> lock(m_mtx);
                m_sharedQueue.emplace_back(cmd);
            }
            m_cv.notify_one();
        }

        void receiveB(typename MQueueRxInterface<TUser>::queue_type& rxQueue) override {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.wait(lock, [this]{ return !m_sharedQueue.empty(); } );
            readSharedQueue(rxQueue);
        }

        void receiveNB(typename MQueueRxInterface<TUser>::queue_type& rxQueue) override {
            std::unique_lock<std::mutex> lock(m_mtx);
            readSharedQueue(rxQueue);
        }
 
        public:
        MQueueTxInterface<TUser> * getTxInterface() {
            return this;
        }

        MQueueRxInterface<TUser> * getRxInterface() {
            return this;
        }
 
    };

}

#endif
