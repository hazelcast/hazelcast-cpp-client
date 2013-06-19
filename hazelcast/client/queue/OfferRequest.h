//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_OFFER_REQUEST
#define HAZELCAST_QUEUE_OFFER_REQUEST

#include "../serialization/Data.h"
#include "PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            class OfferRequest {
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

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return queue::PortableHook::F_ID;
                }

                int getClassId() const {
                    return queue::PortableHook::OFFER;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                    writer["t"] << timeoutInMillis;
                    writer << data;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                    reader["t"] >> timeoutInMillis;
                    reader >> data;
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
