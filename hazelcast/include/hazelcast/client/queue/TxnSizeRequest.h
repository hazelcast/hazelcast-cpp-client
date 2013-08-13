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


#endif //HAZELCAST_TxnSizeRequest
