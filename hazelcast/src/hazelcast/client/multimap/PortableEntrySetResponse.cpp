//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/PortableEntrySetResponse.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            PortableEntrySetResponse::PortableEntrySetResponse() {

            };

            const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > >& PortableEntrySetResponse::getEntrySet() const {
                return entrySet;
            };

            int PortableEntrySetResponse::getFactoryId() const {
                return collection::CollectionPortableHook::F_ID;
            };

            int PortableEntrySetResponse::getClassId() const {
                return MultiMapPortableHook::ENTRY_SET_RESPONSE;
            };

            void PortableEntrySetResponse::readPortable(serialization::PortableReader& reader) {
                int size = reader.readInt("s");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                for (int i = 0; i < size; ++i) {
                    serialization::pimpl::Data keyData;
                    keyData.readData(in);

                    serialization::pimpl::Data valueData;
                    valueData.readData(in);

                    entrySet.push_back(std::make_pair(keyData, valueData));
                }
            };
        };

    }
}



