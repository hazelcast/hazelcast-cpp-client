#include "ICountDownLatch.h"
#include "countdownlatch/AwaitRequest.h"
#include "countdownlatch/CountDownRequest.h"
#include "countdownlatch/GetCountRequest.h"
#include "countdownlatch/SetCountRequest.h"

namespace hazelcast {
    namespace client {

        ICountDownLatch::ICountDownLatch() {

        };

        void ICountDownLatch::init(const std::string& instanceName, spi::ClientContext *clientContext) {
            this->context = clientContext;
            this->instanceName = instanceName;
            key = context->getSerializationService().toData(instanceName);
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