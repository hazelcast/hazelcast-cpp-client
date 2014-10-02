//
// Created by sancar koyunlu on 30/09/14.
//

#include "hazelcast/client/proxy/IQueueImpl.h"
#include "hazelcast/client/queue/OfferRequest.h"
#include "hazelcast/client/queue/PollRequest.h"
#include "hazelcast/client/queue/RemainingCapacityRequest.h"
#include "hazelcast/client/queue/RemoveRequest.h"
#include "hazelcast/client/queue/ContainsRequest.h"
#include "hazelcast/client/queue/DrainRequest.h"
#include "hazelcast/client/queue/PeekRequest.h"
#include "hazelcast/client/queue/SizeRequest.h"
#include "hazelcast/client/queue/CompareAndRemoveRequest.h"
#include "hazelcast/client/queue/AddAllRequest.h"
#include "hazelcast/client/queue/ClearRequest.h"
#include "hazelcast/client/queue/IteratorRequest.h"
#include "hazelcast/client/queue/AddListenerRequest.h"
#include "hazelcast/client/queue/RemoveListenerRequest.h"
#include "hazelcast/client/impl/PortableCollection.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/impl/ServerException.h"
#include "hazelcast/client/spi/ServerListenerService.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            IQueueImpl::IQueueImpl(const std::string& instanceName, spi::ClientContext *context)
            : ProxyImpl("hz:impl:queueService", instanceName, context) {
                serialization::pimpl::Data data = context->getSerializationService().toData<std::string>(&instanceName);
                partitionId = getPartitionId(data);
            }

            std::string IQueueImpl::addItemListener(impl::BaseEventHandler *itemEventHandler, bool includeValue) {
                queue::AddListenerRequest *request = new queue::AddListenerRequest(getName(), includeValue);
                return listen(request, itemEventHandler);
            }

            bool IQueueImpl::removeItemListener(const std::string& registrationId) {
                queue::RemoveListenerRequest *request = new queue::RemoveListenerRequest(getName(), registrationId);
                return stopListening(request, registrationId);
            }

            bool IQueueImpl::offer(const serialization::pimpl::Data& element, long timeoutInMillis) {
                queue::OfferRequest *request = new queue::OfferRequest(getName(), element, timeoutInMillis);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            serialization::pimpl::Data IQueueImpl::poll(long timeoutInMillis) {
                queue::PollRequest *request = new queue::PollRequest(getName(), timeoutInMillis);
                return invoke(request, partitionId);
            }

            int IQueueImpl::remainingCapacity() {
                queue::RemainingCapacityRequest *request = new queue::RemainingCapacityRequest(getName());
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, int)
                return *result;
            }

            bool IQueueImpl::remove(const serialization::pimpl::Data& element) {
                queue::RemoveRequest *request = new queue::RemoveRequest(getName(), element);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            bool IQueueImpl::contains(const serialization::pimpl::Data& element) {
                std::vector<serialization::pimpl::Data> list(1);
                list[0] = element;
                queue::ContainsRequest *request = new queue::ContainsRequest(getName(), list);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            std::vector<serialization::pimpl::Data> IQueueImpl::drainTo(int maxElements) {
                queue::DrainRequest *request = new queue::DrainRequest(getName(), maxElements);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, impl::PortableCollection);
                return result->getCollection();
            }


            serialization::pimpl::Data IQueueImpl::peek() {
                queue::PeekRequest *request = new queue::PeekRequest(getName());
                return invoke(request, partitionId);
            }

            int IQueueImpl::size() {
                queue::SizeRequest *request = new queue::SizeRequest(getName());
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, int)
                return *result;
            }

            std::vector<serialization::pimpl::Data> IQueueImpl::toArray() {
                queue::IteratorRequest *request = new queue::IteratorRequest(getName());
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, impl::PortableCollection)
                return result->getCollection();
            }

            bool IQueueImpl::containsAll(const std::vector<serialization::pimpl::Data>& elements) {
                queue::ContainsRequest *request = new queue::ContainsRequest(getName(), elements);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            bool IQueueImpl::addAll(const std::vector<serialization::pimpl::Data>& elements) {
                queue::AddAllRequest *request = new queue::AddAllRequest(getName(), elements);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            bool IQueueImpl::removeAll(const std::vector<serialization::pimpl::Data>& elements) {
                queue::CompareAndRemoveRequest *request = new queue::CompareAndRemoveRequest(getName(), elements, false);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            bool IQueueImpl::retainAll(const std::vector<serialization::pimpl::Data>& elements) {
                queue::CompareAndRemoveRequest *request = new queue::CompareAndRemoveRequest(getName(), elements, true);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            void IQueueImpl::clear() {
                queue::ClearRequest *request = new queue::ClearRequest(getName());
                invoke(request, partitionId);
            }

        }
    }
}