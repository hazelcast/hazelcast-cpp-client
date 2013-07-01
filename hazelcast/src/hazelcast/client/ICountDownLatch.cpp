#include "ICountDownLatch.h"
#include "hazelcast/client/countdownlatch/AwaitRequest.h"
#include "hazelcast/client/countdownlatch/CountDownRequest.h"
#include "hazelcast/client/countdownlatch/GetCountRequest.h"
#include "hazelcast/client/countdownlatch/SetCountRequest.h"

namespace hazelcast {
    namespace client {

        ICountDownLatch::ICountDownLatch() {

        };

        void ICountDownLatch::init(const std::string& instanceName, spi::ClientContext *clientContext) {
            this->context = clientContext;
            this->instanceName = instanceName;
            key = context->getSerializationService().toData<std::string>(&instanceName);
        };

        bool ICountDownLatch::await(long timeoutInMillis) {
            countdownlatch::AwaitRequest request(instanceName, timeoutInMillis);
            return invoke<bool>(request);
        };

        void ICountDownLatch::countDown() {
            countdownlatch::CountDownRequest request(instanceName);
            invoke<bool>(request);
        };

        int ICountDownLatch::getCount() {
            countdownlatch::GetCountRequest request(instanceName);
            return invoke<int>(request);

        };

        bool ICountDownLatch::trySetCount(int count) {
            countdownlatch::SetCountRequest request(instanceName, count);
            return invoke<bool>(request);
        };


    }
}