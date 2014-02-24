//
// Created by sancar koyunlu on 20/02/14.
//

#include "hazelcast/client/map/DataEntryView.h"
#include "hazelcast/client/map/DataSerializableHook.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        namespace map{
                int DataEntryView::getFactoryId() const {
                    return map::DataSerializableHook::F_ID;
                };
                int DataEntryView::getClassId() const {
                    return map::DataSerializableHook::ENTRY_VIEW;
                };
                void DataEntryView::readData(serialization::ObjectDataInput &in) {
                    key = *(in.readObject<serialization::pimpl::Data>());
                    value = *(in.readObject<serialization::pimpl::Data>());
                    cost = in.readLong();
                    creationTime = in.readLong();
                    expirationTime = in.readLong();
                    hits = in.readLong();
                    lastAccessTime = in.readLong();
                    lastStoredTime = in.readLong();
                    lastUpdateTime = in.readLong();
                    version = in.readLong();
                };
        }
    }
}
