#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/semaphore/InitRequest.h"
#include "hazelcast/client/semaphore/AcquireRequest.h"
#include "hazelcast/client/semaphore/AvailableRequest.h"
#include "hazelcast/client/semaphore/DrainRequest.h"
#include "hazelcast/client/semaphore/ReduceRequest.h"
#include "hazelcast/client/semaphore/ReleaseRequest.h"


namespace hazelcast {
    namespace client {

        ISemaphore::ISemaphore() {

        };

        void ISemaphore::init(const std::string& instanceName, spi::ClientContext *clientContext) {
            this->context = clientContext;
            this->instanceName = instanceName;
            key = context->getSerializationService().toData<std::string>(&instanceName);
        };


        bool ISemaphore::init(int permits) {
            checkNegative(permits);
            semaphore::InitRequest request(instanceName, permits);
            return invoke<bool>(request);
        };

        void ISemaphore::acquire() {
            acquire(1);
        };

        void ISemaphore::acquire(int permits) {
            checkNegative(permits);
            semaphore::AcquireRequest request(instanceName, permits, -1);
            invoke<bool>(request);
        };

        int ISemaphore::availablePermits() {
            semaphore::AvailableRequest request(instanceName);
            return invoke<int>(request);
        };

        int ISemaphore::drainPermits() {
            semaphore::DrainRequest request(instanceName);
            return invoke<int>(request);
        };

        void ISemaphore::reducePermits(int reduction) {
            checkNegative(reduction);
            semaphore::ReduceRequest request(instanceName, reduction);
            invoke<bool>(request);
        };

        void ISemaphore::release() {
            release(1);
        };

        void ISemaphore::release(int permits) {
            semaphore::ReleaseRequest request(instanceName, permits);
            invoke<bool>(request);
        };

        bool ISemaphore::tryAcquire() {
            return tryAcquire(1);
        };

        bool ISemaphore::tryAcquire(int permits) {
            checkNegative(permits);
            try {
                return tryAcquire(permits, 0);
            } catch (HazelcastException&/*InterruptedException*/ e) {
                return false;
            }
        };

        bool ISemaphore::tryAcquire(long timeoutInMillis) {
            return tryAcquire(1, timeoutInMillis);
        };

        bool ISemaphore::tryAcquire(int permits, long timeoutInMillis) {
            checkNegative(permits);
            semaphore::AcquireRequest request(instanceName, permits, timeoutInMillis);
            return invoke<bool>(request);;
        };

        void ISemaphore::checkNegative(int permits) {
            if (permits < 0) {
                throw HazelcastException/*IllegalArgumentException*/("Permits cannot be negative!");
            }
        };


    }
}