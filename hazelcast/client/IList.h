#ifndef HAZELCAST_ILIST
#define HAZELCAST_ILIST

#include <stdexcept>


namespace hazelcast {
    namespace client {


        template<typename E>
        class IList {
        public:

            IList(const std::string& instanceName, spi::ClientContext& clientContext)
            : instanceName(instanceName)
            , context(clientContext) {

            };

            int size();

            bool isEmpty();

            bool contains(E o);

            typename std::set<E>::iterator iterator();

            std::vector<E> toArray();

            std::vector<E> toArray(std::vector<E> ts);

            bool add(E e);

            bool remove(E o);

            bool containsAll(std::vector<E> objects);

            bool addAll(std::vector<E> es);

            bool addAll(int i, std::vector<E> es);

            bool removeAll(std::vector<E> objects);

            bool retainAll(std::vector<E> objects);

            void clear();

            bool equals(E o);

            int hashCode();

            E get(int i);

            E set(int i, E e);

            void add(int i, E e);

            E remove(int i);

            int indexOf(E o);

            int lastIndexOf(E o);

            typename std::set<E>::iterator listIterator();

            typename std::set<E>::iterator listIterator(int i);

            std::set<E> subList(int i, int i1);

        private:
            std::string instanceName;
            spi::ClientContext& context;
        };
    }
}

#endif /* HAZELCAST_ILIST */