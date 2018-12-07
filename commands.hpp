#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <iostream>
#include <functional>
#include <tuple>

namespace commands {

    template<typename TReceiver>
    struct CmdBase {
        CmdBase() = default;
        virtual ~CmdBase() = default;
        virtual void handleCommand(TReceiver&) const = 0;
    };

    template<std::size_t cmdID, typename TReceiver, typename... TArgs>
    struct Cmd : public CmdBase<TReceiver> {
        std::tuple<TArgs...> members;
        Cmd(const TArgs&... args) : members(args...) { }
        ~Cmd() = default;

        void handleCommand(TReceiver& rx) const override {
            rx.handleCommand(std::ref(*this));
        }

        template<std::size_t N>
        const auto getMember() const {
            return std::get<N>(members);
        }

        static constexpr auto getSize() {
            return std::int32_t(std::tuple_size<std::tuple<TArgs...>>::value);
        }

        static constexpr std::size_t ID = cmdID;
    };

    class Printer {

        Printer() = delete;

        template<std::int32_t N, typename TCmd>
        struct printer_ {
            static void print(const TCmd& cmd) {
                std::cout << "  field<" << N << ">: " << std::get<N>(cmd.members) << std::endl;
                printer_<N-1, TCmd>::print(cmd);
            }
        };

        template<typename TCmd>
        struct printer_<-1, TCmd> {
            static void print(const TCmd&) {
                std::cout << std::endl;
            }
        };

        public:
        template<typename TCmd>
        static void printCmd(const TCmd& cmd) {
            std::cout << "Cmd ID: "
                      << TCmd::ID
                      << ", type hash: 0x"
                      << std::hex
                      << typeid(TCmd).hash_code()
                      << std::dec
                      << ", members cnt: "
                      << TCmd::getSize()
                      << std::endl;
            printer_<TCmd::getSize()-1, TCmd>::print(cmd);
        }
    };

}

#endif
