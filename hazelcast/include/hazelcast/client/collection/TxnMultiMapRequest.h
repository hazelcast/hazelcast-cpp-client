//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnMultiMapRequest
#define HAZELCAST_TxnMultiMapRequest

#include "Portable.h"
#include "CollectionPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace collection {
            class TxnMultiMapRequest : public Portable {
            public:
                TxnMultiMapRequest(const std::string& name);

                int getFactoryId() const;


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", name);
                };


                void readPortable(serialization::PortableReader& reader) {
                    name = reader.readUTF("n");
                };
            private:
                std::string name;
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapRequest
