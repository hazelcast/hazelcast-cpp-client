#ifndef HAZELCAST_SERIALIZATION_CONSTRAINTS
#define HAZELCAST_SERIALIZATION_CONSTRAINTS

#include <vector>
#include <string>

typedef unsigned char byte;

namespace hazelcast {
    namespace client {
        namespace serialization {
            //TODO ?????
            template<class T> struct Is_Portable {
                static void constraints(T& t) {
                    int s = t.getFactoryId();
                    s = t.getClassId();
//                    static EmptyPortableWriter ew;
//                    static EmptyPortableReader er;
//                    t.writePortable(ew);
//                    t.readPortable(er);

                }

                static void const_constraints(const T& t) {
//                    static EmptyPortableWriter ew;
//                    t.writePortable(ew);
                    int s = t.getFactoryId();
                    s = t.getClassId();
                }

                Is_Portable() {
                    void(*p)(T&) = constraints;
                    void(*cp)(const T&) = const_constraints;
                }
            };

            template<class T> struct Is_DataSerializable {
                static void constraints(T& t) {
                    int s = t.getFactoryId();
                    s = t.getClassId();
//                    ObjectDataOutput o;
                    std::vector<byte> a;
//                    ObjectDataInput i(a);
//                    t.writeData(o);
//                    t.readData(i);
                }

                static void const_constraints(T& t) {
                    int s = t.getFactoryId();
                    s = t.getClassId();
//                    ObjectDataOutput o;
//                    t.writeData(o);
                }

                Is_DataSerializable() {
                    void(*p)(T&) = constraints;
                    void(*cp)(T&) = const_constraints;
                }
            };

        }
    }
}

#endif