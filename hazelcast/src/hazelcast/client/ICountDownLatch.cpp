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
            return invoke<bool>(request);
        };

        void ICountDownLatch::countDown() {
            countdownlatch::CountDownRequest request(getName());
            invoke<bool>(request);
        };

        int ICountDownLatch::getCount() {
            countdownlatch::GetCountRequest request(getName());
            return invoke<int>(request);

        };

        bool ICountDownLatch::trySetCount(int count) {
            countdownlatch::SetCountRequest request(getName(), count);
            return invoke<bool>(request);
        };

        void ICountDownLatch::onDestroy() {

        };
    }
}