#ifndef MQUEUE_IMPL
#define MQUEUE_IMPL

#include <list>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "mqueue_interface.hpp"

namespace mqueue {

    template<typename TReceiver>
    class MQueueImpl : public MQueueTxInterface<MQueueImpl<TReceiver>>,
                       public MQueueRxInterface<TReceiver> {
        using queue_type = std::list<std::unique_ptr<const commands::CmdBase<TReceiver>>>;

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

        void serviceCommands(TReceiver& rx) {
            while(!m_localQueue.empty()) {
                auto& cptr = m_localQueue.front();
                cptr->handleCommand(rx);
                m_localQueue.pop_front();
            }
        }

        public:
        void send(const commands::CmdBase<TReceiver> * const cmd) /* static polymorphism */ {
            {
                std::unique_lock<std::mutex> lock(m_mtx);
                m_sharedQueue.emplace_back(cmd);
            }
            m_cv.notify_one();
        }

        void receiveB(TReceiver& rx) override {
            readB();
            serviceCommands(rx);
        }

        void receiveNB(TReceiver& rx) override {
            readNB();
            serviceCommands(rx);
        }

        MQueueTxInterface<MQueueImpl<TReceiver>> * getTxInterface() {
            return this;
        }

        MQueueRxInterface<TReceiver> * getRxInterface() {
            return this;
        }
 
    };

}

#endif
