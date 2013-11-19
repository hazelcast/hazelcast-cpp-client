//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnCollectionRequest.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/Data.h"


namespace hazelcast {
    namespace client {
        namespace collection {
            TxnCollectionRequest::TxnCollectionRequest(const std::string& name)
            :name(name), data(NULL) {

            };

            TxnCollectionRequest::TxnCollectionRequest(const std::string& name, serialization::Data *data)
            :name(name), data(data) {

            };

            int TxnCollectionRequest::getFactoryId() const {
                return CollectionPortableHook::F_ID;
            };

            void TxnCollectionRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                util::writeNullableData(out, data);
            };
        }
    }
}