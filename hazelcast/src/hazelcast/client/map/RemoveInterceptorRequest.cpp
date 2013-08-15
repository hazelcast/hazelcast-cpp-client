//
// Created by sancar koyunlu on 8/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "RemoveInterceptorRequest.h"
#include "PortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

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

            void RemoveInterceptorRequest::writePortable(serialization::PortableWriter & writer) const {
                writer.writeUTF("n", name);
                writer.writeUTF("id", id);
            }

            void RemoveInterceptorRequest::readPortable(serialization::PortableReader & reader) {
                name = reader.readUTF("n");
                id = reader.readUTF("id");
            }

        }
    }
}
