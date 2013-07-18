//#include "serializationTest.h"
#include "deneme.h"
#include "mapTest.h"


GTEST_API_ int main(int argc, char **argv) {
//    try{
//        testSpeed();
//    } catch (hazelcast::client::exception::IException & exception) {
//        std::cout << exception.what() << std::endl;
//    } catch(std::exception& s){
//        std::cout << s.what() << std::endl;
//    }
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
//    return deneme::init();
};

