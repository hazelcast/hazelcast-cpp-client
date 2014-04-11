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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <sys/time.h>
#endif

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


        long getCurrentTimeMillis() {
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
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
			return 1L;
#else
			timeval time;
            ::gettimeofday(&time, NULL);
            return long(time.tv_sec) * 1000 + long(time.tv_usec / 1000);
#endif

        }

		void sleep(int seconds){
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
			Sleep(seconds * 1000);
#else
			::sleep(seconds);
#endif
		}
    }
}


