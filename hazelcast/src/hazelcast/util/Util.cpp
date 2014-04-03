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
#include <sstream>
#include <sys/time.h>

namespace hazelcast {
    namespace util {
        std::string to_string(int value) {
            std::stringstream s;
            s << value;
            return s.str();
        }

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


        long getCurrentTimeMillis() {
                timeval time;
                ::gettimeofday(&time, NULL);
                return long(time.tv_sec) * 1000  +  long(time.tv_usec / 1000);

//                    if (UseFakeTimers) {
//                        return fake_time++;
//                    } else {
//                        FILETIME wt;
//                        GetSystemTimeAsFileTime(&wt);
//                        return windows_to_java_time(wt);
//                    }

//            windows_to_java_time
//            jlong a = jlong_from(wt.dwHighDateTime, wt.dwLowDateTime);
//            return (a - offset()) / 10000;

        }
    }
}

