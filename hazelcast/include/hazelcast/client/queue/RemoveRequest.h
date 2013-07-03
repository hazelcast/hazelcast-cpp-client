//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_REMOVE_REQUEST
#define HAZELCAST_QUEUE_REMOVE_REQUEST

#include "../serialization/Data.h"
#include "QueuePortableHook.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            class RemoveRequest : public Portable {
            public:

                RemoveRequest(const std::string& name, serialization::Data& data)
                :name(name)
                , data(data) {

                };

                int getFactoryId() const {
                    return queue::QueuePortableHook::F_ID;
                }

                int getClassId() const {
                    return queue::QueuePortableHook::REMOVE;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", name);
                    serialization::BufferedDataOutput *out = writer.getRawDataOutput();
                    data.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    name = reader.readUTF("n");
                    serialization::BufferedDataInput *in = reader.getRawDataInput();
                    data.readData(*in);
                };
            private:
                serialization::Data& data;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_REMOVE_REQUEST
