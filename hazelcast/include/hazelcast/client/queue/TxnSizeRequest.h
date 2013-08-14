//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnSizeRequest
#define HAZELCAST_TxnSizeRequest

#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class TxnSizeRequest : public Portable {
            public:
                TxnSizeRequest(const std::string& name);

                int getFactoryId() const;

                int getClassId() const;

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


#endif //HAZELCAST_TxnSizeRequest
