#ifndef MQUEUE_INTERFACE
#define MQUEUE_INTERFACE

#include <list>
#include <memory>
#include "commands.hpp"

namespace mqueue {

    template<typename TUser>
    class MQueueTxInterface {
        public:
        virtual void send(const commands::CmdBase<TUser> * const) = 0;
    };

    template<typename TUser>
    class MQueueRxInterface {
        public:
        using queue_type = std::list<std::unique_ptr<const commands::CmdBase<TUser>>>;

        virtual void receiveB(queue_type&) = 0;
        virtual void receiveNB(queue_type&) = 0;
    };

}

#endif
