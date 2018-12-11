# multitype_threadsafe_queue
Thread-safe FIFO passing generic self-dispatching commands of multiple types

Single FIFO capable of secure commands passing between threads. Each command type can have any number of fields of any type independently of other commands. Different types of commands are stored in the same FIFO thus receiving order is guaranteed to be the same as transmitting order.

Received commands are used directly to call proper handlers in the Receiver class in the receiver thread context. Since command is identified by it's type, simple method overloading does the job. No extensive if...else statements, no command dispatching maps, no dynamic (or any other) casting.

#### Example of command creating, sending and handling reception

```cpp
using CmdExample = commands::Cmd<1, Receiver, std::vector<std::string>, double, std::string>;
```

Command ```CmdExample``` has three fields identified by the indexes 0..2
```
field<0>: std::vector<std::string>
field<1>: double
field<2>: std::string
```

```Receiver``` is the class that implements handlers for received commands.

```cpp
void Receiver::handleCommand(const CmdExample& cmd) {
    //Read the double field
    auto x = cmd.getMember<1>();

    //Other stuff...
}
```

The first integral parameter of ```commands::Cmd``` template is the command ID. It's used only to ensure that different commands have different types (even if all the other template parameters are the same). ID itself isn't used for command identification or dispatching. That is being done exclusively upon the command type.

Constructor of ```CmdExample``` takes 3 parameters, for instance:

```cpp
auto* ptr = new CmdExample({"foo", "bar", "baz", "whatever"}, 42.0, "Hello");
```

A convenient ```send``` method template provided by TX interface can be used for command sending with no need of creating CmdExample object

```cpp
mqueue::MQueueImpl<Receiver> mq;
auto* const ifc = mq.getTxInterface();

ifc->send<CmdExample>({"foo", "bar", "baz", "whatever"}, 42.0, "Hello");
```
