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
            inline int getFactoryId(const protocol::HazelcastServerError& ar) {
                return protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;
            }

            inline int getClassId(const protocol::HazelcastServerError& ar) {
                return protocol::ProtocolConstants::HAZELCAST_SERVER_ERROR_ID;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const protocol::HazelcastServerError& data) {
                writer["m"] << data.message;
                writer["d"] << data.details;
                writer["t"] << data.type;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, protocol::HazelcastServerError& data) {
                reader["m"] >> data.message;
                reader["d"] >> data.details;
                reader["t"] >> data.type;
            };

        }
    }
}

#endif //HAZELCAST_SERVER_ERROR
