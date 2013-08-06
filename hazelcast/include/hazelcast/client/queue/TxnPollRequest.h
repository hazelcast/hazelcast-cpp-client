//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnPollRequest
#define HAZELCAST_TxnPollRequest

#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class TxnPollRequest : public Portable {
            public:
                TxnPollRequest(const std::string& name);

                int getFactoryId() const;

                int getClassId() const;

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeLong("t", timeout);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    name = reader.readUTF("n");
                    timeout = reader.readLong("t");
                };
            private:
                std::string name;
                long timeout;
            };
        }
    }
}


#endif //HAZELCAST_TxnPollRequest
