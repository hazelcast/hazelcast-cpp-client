#ifndef HAZELCAST_ID_GENERATOR
#define HAZELCAST_ID_GENERATOR

#include <string>

namespace hazelcast {
    namespace client {
        namespace spi {
            class ClientContext;
        }

        class IdGenerator {
        public:

            IdGenerator(const std::string& instanceName, spi::ClientContext& clientContext);

            /**
             * Try to initialize this IdGenerator instance with given id
             *
             * @return true if initialization success
             */
            bool init(long id);

            /**
             * Generates and returns cluster-wide unique id.
             * Generated ids are guaranteed to be unique for the entire cluster
             * as long as the cluster is live. If the cluster restarts then
             * id generation will start from 0.
             *
             * @return cluster-wide new unique id
             */
            long newId();

        private:
            std::string instanceName;
            spi::ClientContext& context;
        };
    }
}

#endif /* HAZELCAST_ID_GENERATOR */