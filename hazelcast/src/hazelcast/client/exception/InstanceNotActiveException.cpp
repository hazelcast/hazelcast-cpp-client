//
// Created by sancar koyunlu on 27/10/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/exception/InstanceNotActiveException.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/Address.h"

namespace hazelcast {
    namespace client {
        namespace exception {

            InstanceNotActiveException::InstanceNotActiveException(const Address &address)
            :IException(address.getHost() + ":" + util::to_string(address.getPort()), "HazelcastInstanceNotActiveException") {

            }

            InstanceNotActiveException::InstanceNotActiveException(const std::string &source)
            :IException(source, "HazelcastInstanceNotActiveException") {

            }

            InstanceNotActiveException::~InstanceNotActiveException() throw() {

            }

            char const *InstanceNotActiveException::what() const throw() {
                return IException::what();
            }
        }
    }
}