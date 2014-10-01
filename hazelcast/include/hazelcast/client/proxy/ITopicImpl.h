//
// Created by sancar koyunlu on 01/10/14.
//


#ifndef HAZELCAST_ITopicImpl
#define HAZELCAST_ITopicImpl


#include "hazelcast/client/proxy/ProxyImpl.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace proxy {
            class HAZELCAST_API ITopicImpl : public proxy::ProxyImpl {
            protected:
                ITopicImpl(const std::string& instanceName, spi::ClientContext *context);

                void publish(const serialization::pimpl::Data& data);

                std::string addMessageListener(impl::BaseEventHandler *topicEventHandler);

                bool removeMessageListener(const std::string& registrationId);

            private:
                int partitionId;
            };
        }
    }
}

#endif //HAZELCAST_ITopicImpl
