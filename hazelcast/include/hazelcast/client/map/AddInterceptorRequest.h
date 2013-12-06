//
// Created by sancar koyunlu on 8/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AddInterceptorRequest
#define HAZELCAST_AddInterceptorRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/map/PortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            template<typename Interceptor>
            class HAZELCAST_API AddInterceptorRequest : impl::PortableRequest{
            public:
                AddInterceptorRequest(const std::string name, Interceptor& interceptor)
                :name(name)
                , interceptor(interceptor) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::ADD_INTERCEPTOR;
                }

                void writePortable(serialization::PortableWriter & writer) const {
                    writer.writeUTF("n", name);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    out.writeObject<Interceptor>(&interceptor);
                }

            private:
                std::string name;
                Interceptor& interceptor;
            };
        };
    }
}

#endif //HAZELCAST_AddInterceptorRequest
