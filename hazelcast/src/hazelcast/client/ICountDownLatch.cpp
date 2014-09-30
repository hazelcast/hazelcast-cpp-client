#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/countdownlatch/AwaitRequest.h"
#include "hazelcast/client/countdownlatch/CountDownRequest.h"
#include "hazelcast/client/countdownlatch/GetCountRequest.h"
#include "hazelcast/client/countdownlatch/SetCountRequest.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

namespace hazelcast {
    namespace client {

        ICountDownLatch::ICountDownLatch(const std::string &instanceName, spi::ClientContext *context)
        :DistributedObject("hz:impl:atomicLongService", instanceName, context) {
            serialization::pimpl::Data keyData = getContext().getSerializationService().toData<std::string>(&instanceName);
            partitionId = getPartitionId(keyData);
        }

        bool ICountDownLatch::await(long timeoutInMillis) {
            countdownlatch::AwaitRequest *request = new countdownlatch::AwaitRequest(getName(), timeoutInMillis);
            serialization::pimpl::Data data = invoke(request, partitionId);
            DESERIALIZE(data , bool);
            return *result;
        }

        void ICountDownLatch::countDown() {
            countdownlatch::CountDownRequest *request = new countdownlatch::CountDownRequest(getName());
            invoke(request, partitionId);
        }

        int ICountDownLatch::getCount() {
            countdownlatch::GetCountRequest *request = new countdownlatch::GetCountRequest(getName());
            serialization::pimpl::Data data = invoke(request, partitionId);
            DESERIALIZE(data, int);
            return *result;
        }

        bool ICountDownLatch::trySetCount(int count) {
            countdownlatch::SetCountRequest *request = new countdownlatch::SetCountRequest(getName(), count);
            serialization::pimpl::Data data = invoke(request, partitionId);
            DESERIALIZE(data, bool);
            return *result;
        }

        void ICountDownLatch::onDestroy() {

        }
    }
}
