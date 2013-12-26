#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/countdownlatch/AwaitRequest.h"
#include "hazelcast/client/countdownlatch/CountDownRequest.h"
#include "hazelcast/client/countdownlatch/GetCountRequest.h"
#include "hazelcast/client/countdownlatch/SetCountRequest.h"

namespace hazelcast {
    namespace client {

        ICountDownLatch::ICountDownLatch(const std::string &instanceName, spi::ClientContext *context)
        :DistributedObject("hz:impl:atomicLongService", instanceName, context)
        , key(context->getSerializationService().toData<std::string>(&instanceName)) {

        };

        bool ICountDownLatch::await(long timeoutInMillis) {
            countdownlatch::AwaitRequest request(getName(), timeoutInMillis);
            return *(invoke<bool>(request, key));
        };

        void ICountDownLatch::countDown() {
            countdownlatch::CountDownRequest request(getName());
            invoke<bool>(request, key);
        };

        int ICountDownLatch::getCount() {
            countdownlatch::GetCountRequest request(getName());
            boost::shared_ptr<int> response = invoke<int>(request, key);
            return *response;

        };

        bool ICountDownLatch::trySetCount(int count) {
            countdownlatch::SetCountRequest request(getName(), count);
            boost::shared_ptr<bool> response = invoke<bool>(request, key);
            return *response;
        };

        void ICountDownLatch::onDestroy() {

        };
    }
}