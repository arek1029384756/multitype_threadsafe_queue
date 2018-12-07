#ifndef MQUEUE_INTERFACE
#define MQUEUE_INTERFACE

#include "commands.hpp"

namespace mqueue {

    template<typename Impl>
    class MQueueTxInterface {
        public:
        template<typename TCmd, typename... TArgs>
        void send(const TArgs&... args) {
            Impl* const p = static_cast<Impl*>(this);
            p->send(new TCmd(args...));
        }
    };

    template<typename TReceiver>
    class MQueueRxInterface {
        public:
        virtual void receiveB(TReceiver&) = 0;
        virtual void receiveNB(TReceiver&) = 0;
    };

}

#endif
