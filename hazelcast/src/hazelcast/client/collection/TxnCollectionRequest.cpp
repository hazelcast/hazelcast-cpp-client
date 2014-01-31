//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/collection/TxnCollectionRequest.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace collection {
            TxnCollectionRequest::TxnCollectionRequest(const std::string &name)
            :name(name)
            , hasData(false) {

            };

            TxnCollectionRequest::TxnCollectionRequest(const std::string &name, serialization::Data &data)
            :name(name)
            , hasData(true)
            , data(data) {

            };

            int TxnCollectionRequest::getFactoryId() const {
                return CollectionPortableHook::F_ID;
            };

            void TxnCollectionRequest::write(serialization::PortableWriter &writer) const {
                BaseTxnRequest::write(writer);
                writer.writeUTF("n", name);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                out.writeBoolean(hasData);
                if (hasData) {
                    data.writeData(out);
                }
            };
        }
    }
}