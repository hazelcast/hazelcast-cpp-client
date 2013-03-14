#ifndef HAZELCAST_GENERAL_COMMANDS
#define HAZELCAST_GENERAL_COMMANDS

#include "Command.h"
#include "../serialization/DataInput.h"
#include "../serialization/DataOutput.h"
#include <iostream>
#include <stdexcept>

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace GeneralCommands {

                static std::string NEWLINE = "\r\n";
                static std::string SPACE = " ";

                class AuthCommand : public Command {
                public:

                    AuthCommand(std::string name, std::string password) : name(name), password(password) {

                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        std::string command = "AUTH";
                        command += SPACE + name + SPACE + password + NEWLINE;
                        dataOutput.write(command.c_str(), command.length());
                    };

                    void readHeaderLine(std::string line) {
                        if (line.compare("OK")) {
                            throw hazelcast::client::HazelcastException("wrong name or password");
                        }

                    };

                    void readSizeLine(std::string sizeInBytes) {

                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {

                    };

                    int nResults() {
                        return 0;
                    };

                    int resultSize(int i) {
                        return 0;
                    };
                private:
                    std::string name;
                    std::string password;
                };

            }
        }
    }
}

#endif /* HAZELCAST_GENERAL_COMMANDS */