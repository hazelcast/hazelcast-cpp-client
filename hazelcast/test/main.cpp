//#include "deneme.h"
//#include "SimpleMapTest.h"
//#include "CountDownLatch.h"
//#include "HazelcastInstanceFactory.h"
#include "ClientSerializationTest.h"
//#include "ClientMapTest.h"
#include "testUtil.h"

//int testSpeed() {
//    SimpleMapTest s(SERVER_ADDRESS, SERVER_PORT);
//    s.run();
//    return 0;
//};
//
//void testCountDownLatchTestThread(hazelcast::util::CountDownLatch& countDownLatch, int time) {
//    try{
//        std::cout << "Sleeping for " << time << " seconds " << std::endl;
//        boost::this_thread::sleep(boost::posix_time::seconds(time));
//        countDownLatch.countDown();
//        std::cout << "CountDown" << std::endl;
//        std::cout.flush();
//    } catch(std::exception& e){
//        std::cout << e.what() << std::endl;
//        std::cout.flush();
//    } catch(...){
//        std::cout << "testCountDownLatch unexpected exception" << std::endl;
//        std::cout.flush();
//    }
//}
//
//void sampleCountDownLatchUsage() {
//    hazelcast::util::CountDownLatch countDownLatch(2);
//    boost::thread x1(testCountDownLatchTestThread, boost::ref(countDownLatch), 2);
//    boost::thread x2(testCountDownLatchTestThread, boost::ref(countDownLatch), 3);
//    std::cout << "waiting for others to wakeup" << std::endl;
//    std::cout.flush();
//    countDownLatch.await();
//    std::cout << "everyBody up" << std::endl;
//    std::cout.flush();
//}

int main(int argc, char **argv) {
    hazelcast::client::test::ClientSerializationTest serializationTest;
    serializationTest.executeTests();
//    hazelcast::client::test::HazelcastInstanceFactory factory;
//    test::ClientMapTest mapTest(factory);
//    mapTest.executeTests();
    //QueueTest queueTest;
    //queueTest.executeTests();
    //IAtomicLongTest atomTest;
    //atomTest.executeTests();
//    IdGeneratorTest generatorTest;
//    generatorTest.executeTests();
    return 0;

//    sampleCountDownLatchUsage();
//    return testSpeed();
//    return deneme::init();
};

