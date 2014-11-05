//
// Created by sancar koyunlu on 01/10/14.
//

#include "hazelcast/client/proxy/ITopicImpl.h"
#include "hazelcast/client/topic/PublishRequest.h"
#include "hazelcast/client/topic/AddMessageListenerRequest.h"
#include "hazelcast/client/topic/RemoveMessageListenerRequest.h"
#include "hazelcast/client/topic/TopicEventHandler.h"
#include "hazelcast/client/spi/ServerListenerService.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            ITopicImpl::ITopicImpl(const std::string& instanceName, spi::ClientContext *context)
            : proxy::ProxyImpl("hz:impl:topicService", instanceName, context) {
                partitionId = getPartitionId(toData(instanceName));
            }

            void ITopicImpl::publish(const serialization::pimpl::Data& data) {
                topic::PublishRequest *request = new topic::PublishRequest(getName(), data);
                invoke(request, partitionId);
            }


            std::string ITopicImpl::addMessageListener(impl::BaseEventHandler *topicEventHandler) {
                topic::AddMessageListenerRequest *request = new topic::AddMessageListenerRequest(getName());
                return listen(request, partitionId, topicEventHandler);
            }

            bool ITopicImpl::removeMessageListener(const std::string& registrationId) {
                topic::RemoveMessageListenerRequest *request = new topic::RemoveMessageListenerRequest(getName(), registrationId);
                return stopListening(request, registrationId);
            }
        }
    }
}

