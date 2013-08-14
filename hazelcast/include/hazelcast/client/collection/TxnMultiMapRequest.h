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

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", name);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    name = reader.readUTF("n");
                };
            private:
                std::string name;
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapRequest
