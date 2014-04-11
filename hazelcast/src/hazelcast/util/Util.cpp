//
// Created by sancar koyunlu on 5/3/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//


#include "hazelcast/util/Util.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/util/Thread.h"

namespace hazelcast {
    namespace util {

        long getThreadId() {
            return util::Thread::getThreadID();
        }

        void writeNullableData(client::serialization::ObjectDataOutput &out, const client::serialization::pimpl::Data *data) {
            if (data != NULL) {
                out.writeBoolean(true);
                data->writeData(out);
            } else {
                // null
                out.writeBoolean(false);
            }
        }

        void readNullableData(client::serialization::ObjectDataInput &in, client::serialization::pimpl::Data *data) {
            bool isNotNull = in.readBoolean();
            if (isNotNull)
                data->readData(in);
        }


    }
}


