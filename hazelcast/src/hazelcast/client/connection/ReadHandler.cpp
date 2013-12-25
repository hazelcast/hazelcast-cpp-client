//
// Created by sancar koyunlu on 24/12/13.
//

#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/serialization/DataAdapter.h"
#include <boost/thread/future.hpp>

namespace hazelcast {
    namespace client {
        namespace connection {
            ReadHandler::ReadHandler(Connection &connection, int bufferSize)
            : connection(connection)
            , buffer(bufferSize)
            , dataAdapter(NULL) {

            };


            void ReadHandler::handle() {
                if (!connection.live) {
                    return;
                }
                connection.lastRead = clock();
                buffer.readFrom(connection.getSocket());

                while (buffer.remaining()) {
                    if (dataAdapter == NULL) {
                        dataAdapter = new serialization::DataAdapter();
                    }
                    bool complete = dataAdapter->readFrom(buffer);
                    if (complete) {
//                        dataAdapter->getData();
//                        serialization::Data &data = dataAdapter->getData();
//                        data.isServerError(); Set exception to future if error
                        //TODO find related future in ????
                        boost::future<int> a;
                    } else {
                        assert(buffer.remaining() == 0);
                        break;
                    }
                }
            };
        }
    }
}
