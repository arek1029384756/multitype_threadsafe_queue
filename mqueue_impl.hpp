#ifndef MQUEUE_IMPL
#define MQUEUE_IMPL

#include <list>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "mqueue_interface.hpp"

namespace mqueue {

    template<typename TUser>
    class MQueueImpl : public MQueueTxInterface<TUser, MQueueImpl<TUser>>,
                       public MQueueRxInterface<TUser> {
        using queue_type = std::list<std::unique_ptr<const commands::CmdBase<TUser>>>;

        std::mutex m_mtx;
        std::condition_variable m_cv;
        queue_type m_sharedQueue;
        queue_type m_localQueue;

        void shared2Local() {
            while(!m_sharedQueue.empty()) {
                auto& cptr = m_sharedQueue.front();
                m_localQueue.push_back(std::move(cptr));
                m_sharedQueue.pop_front();
            }
        }

        void readB() {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.wait(lock, [this]{ return !m_sharedQueue.empty(); } );
            shared2Local();
        }

        void readNB() {
            std::unique_lock<std::mutex> lock(m_mtx);
            shared2Local();
        }

        void serviceCommands(TUser& rx) {
            while(!m_localQueue.empty()) {
                auto& cptr = m_localQueue.front();
                cptr->handleCommand(rx);
                m_localQueue.pop_front();
            }
        }

        public:
        void send(const commands::CmdBase<TUser> * const cmd) /* static polymorphism */ {
            {
                std::unique_lock<std::mutex> lock(m_mtx);
                m_sharedQueue.emplace_back(cmd);
            }
            m_cv.notify_one();
        }

        void receiveB(TUser& rx) override {
            readB();
            serviceCommands(rx);
        }

        void receiveNB(TUser& rx) override {
            readNB();
            serviceCommands(rx);
        }

        MQueueTxInterface<TUser, MQueueImpl<TUser>> * getTxInterface() {
            return this;
        }

        MQueueRxInterface<TUser> * getRxInterface() {
            return this;
        }
 
    };

}

#endif
