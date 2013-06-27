#include "IAtomicLong.h"
#include "atomiclong/AddAndGetRequest.h"
#include "atomiclong/CompareAndSetRequest.h"
#include "atomiclong/GetAndAddRequest.h"
#include "atomiclong/GetAndSetRequest.h"
#include "atomiclong/SetRequest.h"


namespace hazelcast {
    namespace client {


        IAtomicLong::IAtomicLong() {

        };

        void IAtomicLong::init(const std::string& instanceName, spi::ClientContext *clientContext) {
            this->context = clientContext;
            this->instanceName = instanceName;
            key = context->getSerializationService().toData(instanceName);
        };

        std::string IAtomicLong::getName() const {
            return instanceName;
        };

        long IAtomicLong::addAndGet(long delta) {
            atomiclong::AddAndGetRequest request(instanceName, delta);
            return invoke<long>(request);
        };

        bool IAtomicLong::compareAndSet(long expect, long update) {
            atomiclong::CompareAndSetRequest request(instanceName, expect, update);
            return invoke<bool>(request);
        };

        long IAtomicLong::decrementAndGet() {
            return addAndGet(-1);
        };

        long IAtomicLong::get() {
            return getAndAdd(0);
        };

        long IAtomicLong::getAndAdd(long delta) {
            atomiclong::GetAndAddRequest request(instanceName, delta);
            return invoke<long>(request);
        };

        long IAtomicLong::getAndSet(long newValue) {
            atomiclong::GetAndSetRequest request(instanceName, newValue);
            return invoke<long>(request);
        };

        long IAtomicLong::incrementAndGet() {
            return addAndGet(1);
        };

        long IAtomicLong::getAndIncrement() {
            return getAndAdd(1);
        };

        void IAtomicLong::set(long newValue) {
            atomiclong::SetRequest request(instanceName, newValue);
            invoke<bool>(request);
        };


    }
}