//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/collection/TxnCollectionRequest.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace collection {
            TxnCollectionRequest::TxnCollectionRequest(const std::string& name)
            : name(name)
            , hasData(false) {

            }

            TxnCollectionRequest::TxnCollectionRequest(const std::string& name, const serialization::pimpl::Data& data)
            : name(name)
            , hasData(true)
            , data(data) {

            }

            int TxnCollectionRequest::getFactoryId() const {
                return CollectionPortableHook::F_ID;
            }

            void TxnCollectionRequest::write(serialization::PortableWriter& writer) const {
                BaseTxnRequest::write(writer);
                writer.writeUTF("n", name);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                if (hasData) {
                    out.writeData(&data);
                } else{
                    out.writeData(NULL);
                }
            }
        }
    }
}
