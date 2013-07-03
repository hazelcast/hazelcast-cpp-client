//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_OFFER_REQUEST
#define HAZELCAST_QUEUE_OFFER_REQUEST

#include "../serialization/Data.h"
#include "QueuePortableHook.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            class OfferRequest : public Portable {
            public:
                OfferRequest(const std::string& name, serialization::Data& data, long timeout)
                :name(name)
                , timeoutInMillis(timeout)
                , data(data) {

                };

                OfferRequest(const std::string& name, serialization::Data& data)
                :name(name)
                , timeoutInMillis(0)
                , data(data) {

                };

                int getFactoryId() const {
                    return queue::QueuePortableHook::F_ID;
                }

                int getClassId() const {
                    return queue::QueuePortableHook::OFFER;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeLong("t", timeoutInMillis);
                    serialization::BufferedDataOutput *out = writer.getRawDataOutput();
                    data.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    name = reader.readUTF("n");
                    timeoutInMillis = reader.readUTF("t");
                    serialization::BufferedDataInput *in = reader.getRawDataInput();
                    data.readData(*in);
                };
            private:
                serialization::Data& data;
                std::string name;
                long timeoutInMillis;
            };
        }
    }
}

#endif //HAZELCAST_OFFER_REQUEST
