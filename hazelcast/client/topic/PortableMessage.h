//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PortableMessage
#define HAZELCAST_PortableMessage

#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "TopicPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace topic {
            class PortableMessage {
                PortableMessage(const serialization::Data& message, long publishTime, const std::string& uuid)
                : uuid(uuid)
                , message(message)
                , publishTime(publishTime) {

                };

                const serialization::Data& getMessage() const {
                    return message;
                };

                std::string getUuid() const {
                    return uuid;
                };

                long getPublishTime() const {
                    return publishTime;
                };

                int getFactoryId() const {
                    return TopicPortableHook::F_ID;
                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getClassId() const {
                    return TopicPortableHook::PORTABLE_MESSAGE;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["pt"] << publishTime;
                    writer["u"] << uuid;
                    writer << message;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["pt"] >> publishTime;
                    reader["u"] >> uuid;
                    reader >> message;
                };
            private:
                serialization::Data message;
                std::string uuid;
                long publishTime;
            };
        }
    }
}


#endif //HAZELCAST_PortableMessage
