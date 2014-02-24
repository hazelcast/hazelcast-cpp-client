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
#include <boost/thread.hpp>

namespace hazelcast {
    namespace util {
        std::string to_string(int value) {
            std::stringstream s;
            s << value;
            return s.str();
        };

        long getThreadId() {
            return hash_value(boost::this_thread::get_id());
        };

        void writeNullableData(client::serialization::ObjectDataOutput &out, const client::serialization::pimpl::Data *data) {
            if (data != NULL) {
                out.writeBoolean(true);
                data->writeData(out);
            } else {
                // null
                out.writeBoolean(false);
            }
        };

        void readNullableData(client::serialization::ObjectDataInput &in, client::serialization::pimpl::Data *data) {
            bool isNotNull = in.readBoolean();
            if (isNotNull)
                data->readData(in);
        };


        long getCurrentTimeMillis() {
            return boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
        }
    }
}

