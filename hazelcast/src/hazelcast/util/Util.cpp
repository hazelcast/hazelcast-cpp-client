//
// Created by sancar koyunlu on 5/3/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//


#include "Util.h"
#include "ObjectDataOutput.h"
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
            std::stringstream s;
            s << boost::this_thread::get_id();
            int threadNumber;
            sscanf(s.str().c_str(), "%ix", &threadNumber);
            return threadNumber;
        };

        void writeNullableData(client::serialization::ObjectDataOutput& out, client::serialization::Data *data) {
            if (data != NULL) {
                out.writeBoolean(true);
                data->writeData(out);
            } else {
                // null
                out.writeBoolean(false);
            }
        };

        long getCurrentTimeMillis() {
            return boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
        }
    }
}

