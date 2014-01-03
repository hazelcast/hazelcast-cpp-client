//
// Created by sancar koyunlu on 03/01/14.
//

#include "EventHandlerWrapper.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            void EventHandlerWrapper::handle(const client::serialization::Data &data) {
                eventHandler->handle(data);
            }
        }
    }
}