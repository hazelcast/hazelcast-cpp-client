//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/ClearRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace map {
            ClearRequest::ClearRequest(const std::string& name)
            :name(name) {

            };

            int ClearRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int ClearRequest::getClassId() const {
                return PortableHook::CLEAR;
            }


            void ClearRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
            };

            bool ClearRequest::isRetryable() const{
                return true;
            }

        }
    }
}


