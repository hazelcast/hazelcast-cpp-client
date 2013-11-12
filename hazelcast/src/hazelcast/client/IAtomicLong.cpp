#include "hazelcast/client/IAtomicLong.h"
#include "hazelcast/client/atomiclong/AddAndGetRequest.h"
#include "hazelcast/client/atomiclong/CompareAndSetRequest.h"
#include "hazelcast/client/atomiclong/GetAndAddRequest.h"
#include "hazelcast/client/atomiclong/GetAndSetRequest.h"
#include "hazelcast/client/atomiclong/SetRequest.h"

namespace hazelcast {
    namespace client {


        IAtomicLong::IAtomicLong(const std::string &instanceName, spi::ClientContext *context)
        :DistributedObject("hz:impl:atomicLongService", instanceName, context)
        , key(context->getSerializationService().toData<std::string>(&instanceName)) {
        };

        long IAtomicLong::addAndGet(long delta) {
            atomiclong::AddAndGetRequest request(getName(), delta);
            return invoke<long>(request);
        };

        bool IAtomicLong::compareAndSet(long expect, long update) {
            atomiclong::CompareAndSetRequest request(getName(), expect, update);
            return invoke<bool>(request);
        };

        long IAtomicLong::decrementAndGet() {
            return addAndGet(-1);
        };

        long IAtomicLong::get() {
            return getAndAdd(0);
        };

        long IAtomicLong::getAndAdd(long delta) {
            atomiclong::GetAndAddRequest request(getName(), delta);
            return invoke<long>(request);
        };

        long IAtomicLong::getAndSet(long newValue) {
            atomiclong::GetAndSetRequest request(getName(), newValue);
            return invoke<long>(request);
        };

        long IAtomicLong::incrementAndGet() {
            return addAndGet(1);
        };

        long IAtomicLong::getAndIncrement() {
            return getAndAdd(1);
        };

        void IAtomicLong::set(long newValue) {
            atomiclong::SetRequest request(getName(), newValue);
            invoke<bool>(request);
        };

        void IAtomicLong::onDestroy() {
        };


    }
}