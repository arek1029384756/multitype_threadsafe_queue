#include <iostream>
#include <thread>
#include "mqueue_impl.hpp"

namespace {

    class Example {

        using CmdTemperature = commands::Cmd<1, Example, std::string, double, std::string>;
        using CmdVoltage     = commands::Cmd<2, Example, std::string, double, std::string, std::string, std::uint32_t>;
        using CmdExit        = commands::Cmd<3, Example, std::int32_t, std::string>;

        std::thread m_th;
        mqueue::MQueueImpl<Example> m_mq;

        //worker thead
        void run(mqueue::MQueueTxInterface<Example> * const ifc) {
            printTID(__func__);

            for(std::size_t i = 0; i < 5; ++i) {
                auto exampleT = double(23) + double(i) / 10;
                auto exampleV = -double(i) + double(i) / 10;
                if(i % 2) {
                    ifc->send(new CmdTemperature("Sensor 1", exampleT, "degC"));
                } else {
                    ifc->send(new CmdVoltage("Value", exampleV, "mV", "Interval", i + 2));
                }
            }

            ifc->send(new CmdExit(EXIT_SUCCESS, "Bye"));
        }

        public:
        void handleCommand(const CmdTemperature& cmd) {
            commands::Printer::printCmd(cmd);
        }

        void handleCommand(const CmdVoltage& cmd) {
            commands::Printer::printCmd(cmd);
        }

        void handleCommand(const CmdExit& cmd) {
            commands::Printer::printCmd(cmd);
            std::cout << "Leaving with message: '" << cmd.getMember<1>() << "'" << std::endl;
            std::exit(cmd.getMember<0>());
        }

        void startThread() {
            m_th = std::thread(std::bind(&Example::run, this, m_mq.getTxInterface()));
        }

        void receiveCommands() {
            typename mqueue::MQueueRxInterface<Example>::queue_type myQueue;

            auto* const ifc = m_mq.getRxInterface();
            while(1) {
                ifc->receiveB(myQueue);
                while(!myQueue.empty()) {
                    auto& cptr = myQueue.front();
                    cptr->handleCommand(std::ref(*this));
                    myQueue.pop_front();
                }
            }
        }

        void join() {
            if(m_th.joinable()) {
                m_th.join();
            }
        }

        inline static void printTID(const std::string& fname) {
            std::cout << fname << "()\tTID: 0x"
                      << std::hex << std::this_thread::get_id() << std::dec << std::endl;
        }
    };

}

int main() {
    Example::printTID(__func__);

    Example mtq;

    mtq.startThread();
    mtq.receiveCommands();
    mtq.join();

    return EXIT_FAILURE;
}
