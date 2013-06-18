#ifndef HAZELCAST_ISET
#define HAZELCAST_ISET

#include <stdexcept>

namespace hazelcast {
    namespace client {


        template<typename E>
        class ISet {
        public:

            ISet(const std::string& instanceName, spi::ClientContext& clientContext)
            : instanceName(instanceName)
            , context(clientContext) {

            };

            int size();

            bool isEmpty();

            bool contains(E o);

            typename std::vector<E>::iterator iterator();

            std::vector<E> toArray();

            std::vector<E> toArray(std::vector<E> ts);

            bool add(E e);

            bool remove(E o);

            bool containsAll(std::vector<E> objects);

            bool addAll(std::vector<E> es);

            bool retainAll(std::vector<E> objects);

            bool removeAll(std::vector<E> objects);

            void clear();

        private:
            std::string instanceName;
            spi::ClientContext& context;
        };
    }
}

#endif /* HAZELCAST_ISET */