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
            boost::shared_ptr<long> response = invoke<long>(request);
            return *response;
        };

        bool IAtomicLong::compareAndSet(long expect, long update) {
            atomiclong::CompareAndSetRequest request(getName(), expect, update);
            boost::shared_ptr<bool> response = invoke<bool>(request);
            return *response;
        };

        long IAtomicLong::decrementAndGet() {
            return addAndGet(-1);
        };

        long IAtomicLong::get() {
            return getAndAdd(0);
        };

        long IAtomicLong::getAndAdd(long delta) {
            atomiclong::GetAndAddRequest request(getName(), delta);
            boost::shared_ptr<long> response = invoke<long>(request);
            return *response;
        };

        long IAtomicLong::getAndSet(long newValue) {
            atomiclong::GetAndSetRequest request(getName(), newValue);
            boost::shared_ptr<long> response = invoke<long>(request);
            return *response;
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