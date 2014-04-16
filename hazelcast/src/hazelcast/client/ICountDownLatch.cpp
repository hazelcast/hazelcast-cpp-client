#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/countdownlatch/AwaitRequest.h"
#include "hazelcast/client/countdownlatch/CountDownRequest.h"
#include "hazelcast/client/countdownlatch/GetCountRequest.h"
#include "hazelcast/client/countdownlatch/SetCountRequest.h"

namespace hazelcast {
    namespace client {

        ICountDownLatch::ICountDownLatch(const std::string &instanceName, spi::ClientContext *context)
        :DistributedObject("hz:impl:atomicLongService", instanceName, context) {
            serialization::pimpl::Data keyData = getContext().getSerializationService().toData<std::string>(&instanceName);
            partitionId = getPartitionId(keyData);
        };

        bool ICountDownLatch::await(long timeoutInMillis) {
            countdownlatch::AwaitRequest *request = new countdownlatch::AwaitRequest(getName(), timeoutInMillis);
            return *(invoke<bool>(request, partitionId));
        };

        void ICountDownLatch::countDown() {
            countdownlatch::CountDownRequest *request = new countdownlatch::CountDownRequest(getName());
            invoke<serialization::pimpl::Void>(request, partitionId);
        };

        int ICountDownLatch::getCount() {
            countdownlatch::GetCountRequest *request = new countdownlatch::GetCountRequest(getName());
            boost::shared_ptr<int> response = invoke<int>(request, partitionId);
            return *response;

        };

        bool ICountDownLatch::trySetCount(int count) {
            countdownlatch::SetCountRequest *request = new countdownlatch::SetCountRequest(getName(), count);
            boost::shared_ptr<bool> response = invoke<bool>(request, partitionId);
            return *response;
        };

        void ICountDownLatch::onDestroy() {

        };
    }
}
