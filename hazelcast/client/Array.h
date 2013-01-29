/* 
 * File:   Array.h
 * Author: msk
 *
 * Created on January 20, 2013, 2:35 PM
 */

#ifndef HAZELCAST_ARRAY
#define	HAZELCAST_ARRAY

#include <memory>
#include <iostream>

namespace hazelcast{ 
namespace client{
namespace serialization{
    
template <typename T>
class Array{
public:
    
    Array():buffer(NULL),len(0){
    };
    Array(int size){
        len = size;
        if(size < 0)
            throw "illegal argument";
        if(size == 0)
            buffer = NULL;
        else
            buffer = new T[size];
    };
    Array(int size, T* b){
        init(size,b);
    };
    Array(const Array<T>& rhs){
        init(rhs.len, rhs.buffer );
    };
    
    ~Array(){
        delete [] buffer;
    };
    T& operator[](int i) const{
        return buffer[i];
    };
    const Array<T>& operator=(const Array<T>& rhs){
        delete [] buffer;
        init(rhs.len, rhs.buffer);
        return *this;
    };
    
    bool operator==(const Array<T>& m) const{
        if(this == &m)
            return true;
        if(len != m.len)
            return false;
        for(int i = 0; i < len ; i++){
            if(!(m[i] == buffer[i]))
                return false;
        }
        return true;
    };
    bool operator!=(const Array<T>& m) const{
        return !(*this == m);
    };
    friend std::ostream& operator<<(std::ostream &strm, const Array<T> &a) {
        strm << "Array( ";
        for(int i = 0; i < a.length() ; i++){
            strm << a[i] << " ";
        }
        strm << ")";
        return strm;
    };
    int length() const{ return len; };
    
    
    T* buffer;
private:
    void init(int size, T* b){
        len = size;
        buffer = new T[len];
        for(int i = 0; i < len; i++){
            buffer[i] = b[i];
        }
    };
    int len;
};

}}}
#endif	/* HAZELCAST_ARRAY */

