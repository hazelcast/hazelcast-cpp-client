//
// Created by sancar koyunlu on 8/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AddInterceptorRequest
#define HAZELCAST_AddInterceptorRequest

#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/map/PortableHook.h"
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


namespace hazelcast {
    namespace client {
        namespace map {
            template<typename Interceptor>
            class AddInterceptorRequest : impl::ClientRequest {
            public:
                AddInterceptorRequest(const std::string name, Interceptor &interceptor)
                :name(name)
                , interceptor(interceptor) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::ADD_INTERCEPTOR;
                }

                void write(serialization::PortableWriter &writer) const {
                    writer.writeUTF("n", &name);
                    serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                    out.writeObject<Interceptor>(&interceptor);
                }

            private:
                std::string name;
                Interceptor interceptor;
            };
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_AddInterceptorRequest

