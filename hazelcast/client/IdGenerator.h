//#ifndef HAZELCAST_ID_GENERATOR
//#define HAZELCAST_ID_GENERATOR
//
//#include <string>
//#include "ClientService.h"
//
//namespace hazelcast {
//    namespace client {
//
//        class impl::ClientService;
//
//        class IdGenerator {
//        public:
//
//            IdGenerator(std::string instanceName, impl::ClientService& clientService);
//
//            IdGenerator(const IdGenerator& rhs);
//
//            ~IdGenerator();
//
//            std::string getName() const;
//
//            /**
//             * Try to initialize this IdGenerator instance with given id
//             *
//             * @return true if initialization success
//             */
//            bool init(long id);
//
//            /**
//             * Generates and returns cluster-wide unique id.
//             * Generated ids are guaranteed to be unique for the entire cluster
//             * as long as the cluster is live. If the cluster restarts then
//             * id generation will start from 0.
//             *
//             * @return cluster-wide new unique id
//             */
//            long newId();
//
//        private:
//            std::string instanceName;
//            impl::ClientService& clientService;
//        };
//    }
//}
//
//#endif /* HAZELCAST_ID_GENERATOR */