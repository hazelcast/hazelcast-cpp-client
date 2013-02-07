#ifndef HAZELCAST_COMMAND
#define HAZELCAST_COMMAND

#include "../serialization/DataInput.h"
#include "../serialization/DataOutput.h"
#include <vector>

namespace hazelcast{
namespace client{
namespace protocol{
  
class Command{
public:
    virtual void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) = 0;
    virtual void readHeaderLine(std::string dataInput) = 0;
    virtual void readSizeLine(std::string sizeInBytes) = 0;
    virtual void readResult(hazelcast::client::serialization::DataInput& dataInput) = 0;
    virtual int nResults() = 0;
    virtual int resultSize(int i) = 0;
/*
    void setFlag(int flag) {
        this->flag = flag;
        isFlagSet = true;
    };

    int getFlag() const {
        return flag;
    };

    void setThreadId(int threadId) {
        this->threadId = threadId;
        isThreadIdSet = true;
    };

    int getThreadId() const {
        return threadId;
    }

    bool isFlagSet() const {
        return isFlagSet;
    }

    bool isThreadIdSet() const {
        return isThreadIdSet;
    };
private:
     bool isThreadIdSet = false;
     bool isFlagSet = false;
     int threadId;
     int flag;
*/
};


}}}


#endif /* HAZELCAST_COMMAND */