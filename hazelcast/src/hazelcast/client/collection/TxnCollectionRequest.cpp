//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnCollectionRequest.h"
#include "CollectionPortableHook.h"
#include "PortableWriter.h"
#include "Data.h"


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