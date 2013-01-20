//
//  SerializationConstants.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_SerializationConstants_h
#define Server_SerializationConstants_h

class SerializationConstants{
public:
    static int ID ;
    
    // !!! NEVER CHANGE THE ORDER OF SERIALIZERS !!!
    
    static int const CONSTANT_TYPE_PORTABLE = -2;
    
    static int const CONSTANT_TYPE_DATA = -3;
    
    static int const CONSTANT_TYPE_BYTE = -4;
    
    static int const CONSTANT_TYPE_BOOLEAN = -5;
    
    static int const CONSTANT_TYPE_CHAR = -6;
    
    static int const CONSTANT_TYPE_SHORT = -7;
    
    static int const CONSTANT_TYPE_INTEGER = -8;
    
    static int const CONSTANT_TYPE_LONG = -9;
    
    static int const CONSTANT_TYPE_FLOAT = -10;
    
    static int const CONSTANT_TYPE_DOUBLE = -11;
    
    static int const CONSTANT_TYPE_STRING = -12;
    
    static int const CONSTANT_TYPE_BYTE_ARRAY = -13;
    
    static int const CONSTANT_TYPE_CHAR_ARRAY = -14;
    
    static int const CONSTANT_TYPE_SHORT_ARRAY = -15;
    
    static int const CONSTANT_TYPE_INTEGER_ARRAY = -16;
    
    static int const CONSTANT_TYPE_LONG_ARRAY = -17;
    
    static int const CONSTANT_TYPE_FLOAT_ARRAY = -18;
    
    static int const CONSTANT_TYPE_DOUBLE_ARRAY = -19;
    
    static int const CONSTANT_SERIALIZERS_LENGTH = -(-19) - 1;
    
    // ------------------------------------------------------------
    
    static int const DEFAULT_TYPE_CLASS = -20;
    
    static int const DEFAULT_TYPE_DATE = -21;
    
    static int const DEFAULT_TYPE_BIG_INTEGER = -22;
    
    static int const DEFAULT_TYPE_OBJECT = -23;
    
    static int const DEFAULT_TYPE_EXTERNALIZABLE = -24;
private:
    SerializationConstants() {};
};


#endif
