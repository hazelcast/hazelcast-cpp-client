#include "CommandHandler.h"
#include "Command.h"

namespace hazelcast {
    namespace client {
        namespace protocol {

            CommandHandler::CommandHandler(Address address, hazelcast::client::serialization::SerializationService *serializationService)
            : socket(address)
            , serializationService(serializationService) {

            };

            void CommandHandler::start() {
                std::string command = "P01\r\n";
                socket.sendData(command.c_str(), command.length());
            };

            void CommandHandler::sendCommand(Command *const command) {
                using namespace hazelcast::client::serialization;
                DataOutput *dataOutput = serializationService->pop();
                command->writeCommand(*dataOutput);
                std::string sendBuffer = dataOutput->toString();
                serializationService->push(dataOutput);
                socket.sendData(sendBuffer.c_str(), sendBuffer.length());

                std::string headerLine = socket.readLine();
                command->readHeaderLine(headerLine);

                if (!command->nResults())
                    return;

                std::string sizeLine = socket.readLine();
                command->readSizeLine(sizeLine);


                for (int i = 0; i < command->nResults(); i++) {
                    int size = command->resultSize(i);
                    byte tempBuffer[size];
                    socket.recvData(tempBuffer, size);
                    std::vector<unsigned char> element(tempBuffer, tempBuffer + size);
                    DataInput resultInput(element, serializationService);
                    command->readResult(resultInput);

                }
                socket.readLine();

            };

        }
    }
}