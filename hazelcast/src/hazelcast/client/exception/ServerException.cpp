//
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/exception/ServerException.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            ServerException::ServerException() {

            }

            ServerException::~ServerException() throw() {
            }

            char const *ServerException::what() const throw() {
                std::string error = message + ":" + details;
                return error.c_str();
            }

            int ServerException::getFactoryId() const {
                return protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;
            }

            int ServerException::getClassId() const {
                return protocol::ProtocolConstants::HAZELCAST_SERVER_ERROR_ID;
            }

            void ServerException::readPortable(serialization::PortableReader &reader) {
                message = reader.readUTF("m");
                details = reader.readUTF("d");
                type = reader.readInt("t");
            };

        }
    }
}
