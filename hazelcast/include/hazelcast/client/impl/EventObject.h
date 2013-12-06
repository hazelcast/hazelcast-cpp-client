//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_EVENT_OBJECT
#define HAZELCAST_EVENT_OBJECT

#include "hazelcast/util/HazelcastDll.h"
#include <string>
/**
 * <p>
 * The root class from which all event state objects shall be derived.
 * <p>
 */
namespace hazelcast {
    namespace client {
        namespace impl {
            class HAZELCAST_API EventObject {
            public:

                EventObject() {

                };

                EventObject(const std::string& source)
                : source(source) {

                };

                virtual ~EventObject() {

                };

                virtual std::string getSource() const {
                    return source;
                };

            private:
                std::string source;
            };

        }
    }
}

#endif //__EventObject_H_
