//
// Created by sancar koyunlu on 15/05/14.
//


#ifndef HAZELCAST_TestHelperFunctions
#define HAZELCAST_TestHelperFunctions

#define ASSERT_EVENTUALLY( CONDITION, ... ) do{     \
            for(int i = 0 ; i < 60 ; i++ ) {        \
                hazelcast::util::sleep(2);                     \
                if(i == 59){                        \
                    CONDITION(__VA_ARGS__);         \
                }                                   \
                try{                                \
                    CONDITION(__VA_ARGS__);         \
                    break;                          \
                }catch(iTestException ignored){     \
                }                                   \
            }                                       \
      }while(0)                                     \


#endif //HAZELCAST_TestHelperFunctions
