//
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_SERVER_ERROR
#define HAZELCAST_SERVER_ERROR

#include "ProtocolConstants.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace protocol {


            class HazelcastServerError {

            public:
                HazelcastServerError();

                virtual ~HazelcastServerError();

                virtual char const *what() const;

                std::string message;

                std::string details;

                int type;
            };
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace serialization {
            inline int getFactoryId(const hazelcast::client::protocol::HazelcastServerError& ar) {
                return hazelcast::client::protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;
            }

            inline int getClassId(const hazelcast::client::protocol::HazelcastServerError& ar) {
                return hazelcast::client::protocol::ProtocolConstants::HAZELCAST_SERVER_ERROR_ID;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const hazelcast::client::protocol::HazelcastServerError& data) {
                writer["m"] << data.message;
                writer["d"] << data.details;
                writer["t"] << data.type;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, hazelcast::client::protocol::HazelcastServerError& data) {
                reader["m"] >> data.message;
                reader["d"] >> data.details;
                reader["t"] >> data.type;
            };

        }
    }
}

#endif //HAZELCAST_SERVER_ERROR
