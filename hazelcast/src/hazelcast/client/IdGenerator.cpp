#include "hazelcast/client/IdGenerator.h"
#include "hazelcast/client/impl/IdGeneratorSupport.h"

namespace hazelcast {
    namespace client {

        IdGenerator::IdGenerator()
        : local(-1)
        , residue(BLOCK_SIZE) {

        };


        void IdGenerator::setIdGeneratorSupport(impl::IdGeneratorSupport *support) {
            this->support = support;
        };

        void IdGenerator::init(const std::string& instanceName, spi::ClientContext *clientContext) {
            this->context = clientContext;
            this->instanceName = instanceName;
            atomicLong.init(instanceName, clientContext);
        };


        bool IdGenerator::init(long id) {
            if (id <= 0) {
                return false;
            }
            long step = (id / BLOCK_SIZE);

            util::LockGuard lg(*(support->getLock(instanceName)));
            bool init = atomicLong.compareAndSet(0, step + 1);
            if (init) {
                local = step;
                residue = (id % BLOCK_SIZE) + 1;
            }
            return init;
        };

        long IdGenerator::newId() {
            int value = residue++;
            if (value >= BLOCK_SIZE) {
                util::LockGuard lg(*(support->getLock(instanceName)));
                value = residue;
                if (value >= BLOCK_SIZE) {
                    local = atomicLong.getAndIncrement();
                    residue = 0;
                }
                return newId();

            }
            return int(local) * BLOCK_SIZE + value;
        };
    }
}