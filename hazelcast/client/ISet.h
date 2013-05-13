//#ifndef HAZELCAST_ISET
//#define HAZELCAST_ISET
//
//#include <stdexcept>
//
//namespace hazelcast {
//    namespace client {
//
//        class impl::ClientService;
//
//        template<typename E>
//        class ISet {
//        public:
//
//            ISet(std::string instanceName, impl::ClientService& clientService) : instanceName(instanceName)
//            , clientService(clientService) {
//
//            };
//
//            ISet(const ISet& rhs) : instanceName(rhs.instanceName)
//            , clientService(rhs.clientService) {
//            };
//
//            ~ISet() {
//
//            };
//
//            int size();
//
//            bool isEmpty();
//
//            bool contains(E o);
//
//            typename std::vector<E>::iterator iterator();
//
//            std::vector<E> toArray();
//
//            std::vector<E> toArray(std::vector<E> ts);
//
//            bool add(E e);
//
//            bool remove(E o);
//
//            bool containsAll(std::vector<E> objects);
//
//            bool addAll(std::vector<E> es);
//
//            bool retainAll(std::vector<E> objects);
//
//            bool removeAll(std::vector<E> objects);
//
//            void clear();
//
//        private:
//            std::string instanceName;
//            impl::ClientService& clientService;
//        };
//    }
//}
//
//#endif /* HAZELCAST_ISET */