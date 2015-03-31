#include "testUtil.h"

namespace hazelcast {
    namespace client {
        namespace test {
            TestMainPortable getTestMainPortable() {
                byte byteArray[] = {0, 1, 2};
                std::vector<byte> bb(byteArray, byteArray + 3);
                char charArray[] = {'c', 'h', 'a', 'r'};
                std::vector<char> cc(charArray, charArray + 4);
                short shortArray[] = {3, 4, 5};
                std::vector<short> ss(shortArray, shortArray + 3);
                int integerArray[] = {9, 8, 7, 6};
                std::vector<int> ii(integerArray, integerArray + 4);
                long longArray[] = {0, 1, 5, 7, 9, 11};
                std::vector<long> ll(longArray, longArray + 6);
                float floatArray[] = {0.6543f, -3.56f, 45.67f};
                std::vector<float> ff(floatArray, floatArray + 3);
                double doubleArray[] = {456.456, 789.789, 321.321};
                std::vector<double> dd(doubleArray, doubleArray + 3);
                TestNamedPortable portableArray[5];
                for (int i = 0; i < 5; i++) {
                    std::string x = "named-portable-";
                    x.push_back('0' + i);
                    portableArray[i] = TestNamedPortable(x, i);
                }
                std::vector<TestNamedPortable> nn(portableArray, portableArray + 5);

                TestInnerPortable inner(bb, cc, ss, ii, ll, ff, dd, nn);
                TestMainPortable main((byte) 113, true, 'x', (short) -500, 56789, -50992225, 900.5678f,
                        -897543.3678909, "this is main portable object created for testing!", inner);
                return main;
            }
        }
    }
}
