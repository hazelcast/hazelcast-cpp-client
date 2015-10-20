//
// Created by sancar koyunlu on 8/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/RemoveInterceptorRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace map {

            RemoveInterceptorRequest::RemoveInterceptorRequest(const std::string & name, const std::string & id)
            :name(name)
            , id(id) {

            }

            int RemoveInterceptorRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int RemoveInterceptorRequest::getClassId() const {
                return PortableHook::REMOVE_INTERCEPTOR;
            }

            void RemoveInterceptorRequest::write(serialization::PortableWriter & writer) const {
                writer.writeUTF("n", &name);
                writer.writeUTF("id", &id);
            }
        }
    }
}

