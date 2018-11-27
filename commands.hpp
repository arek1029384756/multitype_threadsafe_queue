#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <iostream>
#include <functional>
#include <tuple>

namespace commands {

    template<typename TUser>
    struct CmdBase {
        CmdBase() = default;
        virtual ~CmdBase() = default;
        virtual void handleCommand(TUser&) const = 0;
    };

    template<std::size_t cmdID, typename TUser, typename... TArgs>
    struct Cmd : public CmdBase<TUser> {
        std::tuple<TArgs...> members;
        Cmd(const TArgs&... args) : members(args...) { }
        ~Cmd() = default;

        void handleCommand(TUser& user) const override {
            user.handleCommand(std::ref(*this));
        }

        template<std::size_t N>
        const auto getMember() const {
            return std::get<N>(members);
        }

        static constexpr typename std::tuple_size<std::tuple<TArgs...>>::value_type getSize() {
            return std::tuple_size<std::tuple<TArgs...>>::value;
        }

        static constexpr std::size_t ID = cmdID;
    };

    class Printer {

        Printer() = delete;

        template<std::size_t>
        struct s_ {};

        template<typename TCmd>
        static void printCmd(const TCmd& cmd, s_<0>) {
            std::cout << "  field<" << 0 << ">: " << std::get<0>(cmd.members)
                      << std::endl << std::endl;
        }

        template<std::size_t N, typename TCmd>
        static void printCmd(const TCmd& cmd, s_<N>) {
            std::cout << "  field<" << N << ">: " << std::get<N>(cmd.members) << std::endl;
            printCmd(cmd, s_<N-1>());
        }

        public:
        template<typename TCmd>
        static void printCmd(const TCmd& cmd) {
            std::cout << "Cmd ID: " << cmd.ID << ", members cnt: " << cmd.getSize() << std::endl;
            printCmd(cmd, s_<TCmd::getSize()-1>());
        }
    };

}

#endif
