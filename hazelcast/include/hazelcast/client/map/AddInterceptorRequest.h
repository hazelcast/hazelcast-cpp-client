//
// Created by sancar koyunlu on 8/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AddInterceptorRequest
#define HAZELCAST_AddInterceptorRequest

#include "Portable.h"
#include "PortableWriter.h"
#include "PortableReader.h"
#include "PortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            template<typename Interceptor>
            class AddInterceptorRequest {
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

                void readPortable(serialization::PortableReader & reader) {
                    name = reader.readUTF("n");
                    serialization::ObjectDataInput& in = reader.getRawDataInput();
                    interceptor = in.readObject<Interceptor>();
                }

            private:
                std::string name;
                Interceptor& interceptor;
            };
        };
    }
}
}

#endif //HAZELCAST_AddInterceptorRequest
