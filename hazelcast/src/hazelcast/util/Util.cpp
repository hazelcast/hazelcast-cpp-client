//
// Created by sancar koyunlu on 5/3/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//


#include "Util.h"
#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"
#include "Data.h"
#include <boost/thread.hpp>

namespace hazelcast {
    namespace util {
        std::string to_string(int value) {
            std::stringstream s;
            s << value;
            return s.str();
        };

        int getThreadId() {
            return hash_value(boost::this_thread::get_id());;
        };

        void writeNullableData(client::serialization::ObjectDataOutput& out, const client::serialization::Data *data) {
            if (data != NULL) {
                out.writeBoolean(true);
                data->writeData(out);
            } else {
                // null
                out.writeBoolean(false);
            }
        };

        void readNullableData(client::serialization::ObjectDataInput & in, client::serialization::Data *data) {
            bool isNotNull = in.readBoolean();
            if (isNotNull)
                data->readData(in);
        };


        long getCurrentTimeMillis() {
            return boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
        }
    }
}

