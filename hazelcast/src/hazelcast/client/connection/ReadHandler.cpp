//
// Created by sancar koyunlu on 24/12/13.
//

#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/serialization/DataAdapter.h"
#include "hazelcast/client/spi/ClusterService.h"
//#define BOOST_THREAD_PROVIDES_FUTURE

namespace hazelcast {
    namespace client {
        namespace connection {
            ReadHandler::ReadHandler(Connection &connection, spi::ClusterService &clusterService, int bufferSize )
            : connection(connection)
            , clusterService(clusterService)
            , buffer(bufferSize)
            , dataAdapter(NULL) {

            };

            void ReadHandler::handle() {
                if (!connection.live) {
                    return;
                }
                connection.lastRead = clock();
                buffer.readFrom(connection.getSocket());

                while (buffer.remainingData()) {
                    if (dataAdapter == NULL) {
                        dataAdapter = new serialization::DataAdapter();
                    }
                    bool complete = dataAdapter->readFrom(buffer);
                    if (complete) {
                        clusterService.handlePacket(connection.getEndpoint(), dataAdapter->getData());
                        delete dataAdapter;
                    } else {
                        assert(buffer.remainingData() == 0);
                        break;
                    }
                }
            };
        }
    }
}
