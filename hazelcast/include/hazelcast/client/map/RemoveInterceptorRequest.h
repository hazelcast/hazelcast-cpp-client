//
// Created by sancar koyunlu on 8/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_RemoveInterceptorRequest
#define HAZELCAST_RemoveInterceptorRequest

#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class RemoveInterceptorRequest : public Portable {
            public:
                RemoveInterceptorRequest(const std::string&, const std::string&);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter & writer) const;

                void readPortable(serialization::PortableReader & reader);

            private:
                std::string name;
                std::string id;
            };
        }
    }
}

#endif //HAZELCAST_RemoveInterceptorRequest
