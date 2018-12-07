#include <iostream>
#include <thread>
#include "mqueue_impl.hpp"

namespace {

    class Example {

        using CmdEmpty       = commands::Cmd<__COUNTER__, Example>;
        using CmdTemperature = commands::Cmd<__COUNTER__, Example, std::string, double, std::string>;
        using CmdVoltage     = commands::Cmd<__COUNTER__, Example, std::string, double, std::string, std::string, std::uint32_t>;
        using CmdExit        = commands::Cmd<__COUNTER__, Example, std::int32_t, std::string>;

        std::thread m_th;
        mqueue::MQueueImpl<Example> m_mq;

        //worker thead example
        void run(mqueue::MQueueTxInterface<decltype(m_mq)> * const ifc) {
            printTID(__func__);

            for(std::size_t i = 0; i < 5; ++i) {
                auto exampleT = double(23) + double(i) / 10;
                auto exampleV = -double(i) + double(i) / 10;
                auto idx = i % 3;
                if(idx == 0) {
                    ifc->send<CmdTemperature>("Sensor 1", exampleT, "degC");
                } else if(idx == 1) {
                    ifc->send<CmdVoltage>("Value", exampleV, "mV", "Interval", i + 2);
                } else {
                    ifc->send<CmdEmpty>();
                }
            }

            ifc->send<CmdExit>(EXIT_SUCCESS, "Bye");
        }

        public:
        void handleCommand(const CmdTemperature& cmd) {
            commands::Printer::printCmd(cmd);
        }

        void handleCommand(const CmdVoltage& cmd) {
            commands::Printer::printCmd(cmd);
        }

        void handleCommand(const CmdEmpty& cmd) {
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
            auto* const ifc = m_mq.getRxInterface();
            while(1) {
                ifc->receiveB(std::ref(*this));
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
