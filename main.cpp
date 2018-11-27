#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include "commands.hpp"

namespace {

    class Example {

        using CmdTemperature = commands::Cmd<1, Example, std::string, double, std::string>;
        using CmdVoltage     = commands::Cmd<2, Example, std::string, double, std::string, std::string, std::uint32_t>;
        using CmdExit        = commands::Cmd<3, Example, std::int32_t, std::string>;

        std::thread m_th;
        std::mutex m_mtx;
        std::condition_variable m_cv;
        std::list<std::unique_ptr<const commands::CmdBase<Example>>> m_sharedQueue;

        void send(const commands::CmdBase<Example> * const cmd) {
            {
                std::unique_lock<std::mutex> lock(m_mtx);
                m_sharedQueue.emplace_back(cmd);
            }
            m_cv.notify_one();
        }

        void receive(decltype(m_sharedQueue)& rxQueue) {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.wait(lock, [this]{ return !m_sharedQueue.empty(); } );
            while(!m_sharedQueue.empty()) {
                auto& cptr = m_sharedQueue.front();
                rxQueue.push_back(std::move(cptr));
                m_sharedQueue.pop_front();
            }
        }

        //worker thead
        void run() {
            printTID(__func__);

            for(std::size_t i = 0; i < 5; ++i) {
                auto exampleT = double(23) + double(i) / 10;
                auto exampleV = -double(i) + double(i) / 10;
                if(i % 2) {
                    send(new CmdTemperature("Sensor 1", exampleT, "degC"));
                } else {
                    send(new CmdVoltage("Value", exampleV, "mV", "Interval", i + 2));
                }
            }

            send(new CmdExit(EXIT_SUCCESS, "Bye"));
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
            m_th = std::thread(std::bind(&Example::run, this));
        }

        void receiveCommands() {
            decltype(m_sharedQueue) myQueue;

            while(1) {
                receive(myQueue);
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
