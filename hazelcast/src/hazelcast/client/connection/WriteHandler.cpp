//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/WriteHandler.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/OListener.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/AddWriteSocketTask.h"
#include "hazelcast/client/serialization/DataAdapter.h"
//#define BOOST_THREAD_PROVIDES_FUTURE

namespace hazelcast {
    namespace client {
        namespace connection {
            WriteHandler::WriteHandler(Connection &connection, OListener &oListener, int bufferSize)
            : connection(connection)
            , oListener(oListener)
            , buffer(bufferSize)
            , initialized(false) {

            };


            void WriteHandler::enqueueData(const serialization::Data &data) {
                serialization::DataAdapter *adapter = new serialization::DataAdapter(data);
                oListener.addTask(new AddWriteSocketTask(connection.getSocket()));
                writeQueue.offer(adapter);
            }

            void WriteHandler::handle() {
                if (!connection.live) {
                    return;
                }
                connection.lastWrite = clock();
                if (!initialized) {
                    initialized = true;
                    buffer.write(protocol::ProtocolConstants::PROTOCOL);
                }

                if (lastData == NULL) {
                    lastData = writeQueue.poll();
                    if (lastData == NULL && buffer.remainingData() == 0) {
                        return;
                    }
                }
                while (lastData != NULL) {
                    bool complete = lastData->writeTo(buffer);
                    if (complete) {
                        delete lastData;
                        lastData = writeQueue.poll();
                    } else {
                        break;
                    }
                }
                buffer.writeTo(connection.getSocket());

            };
        }
    }
}
