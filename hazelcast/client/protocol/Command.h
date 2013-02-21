#ifndef HAZELCAST_COMMAND
#define HAZELCAST_COMMAND

#include "../serialization/DataInput.h"
#include "../serialization/DataOutput.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace protocol {

            class Command {
            public:

                virtual void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) = 0;

                virtual void readHeaderLine(std::string dataInput) = 0;

                virtual void readSizeLine(std::string sizeInBytes) = 0;

                virtual void readResult(hazelcast::client::serialization::DataInput& dataInput) = 0;

                virtual int nResults() = 0;

                virtual int resultSize(int i) = 0;

                virtual ~Command() {
                };

            };


        }
    }
}


#endif /* HAZELCAST_COMMAND */