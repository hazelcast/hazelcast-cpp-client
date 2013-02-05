#ifndef HAZELCAST_MAP_COMMANDS
#define HAZELCAST_MAP_COMMANDS

#include "../Array.h"
#include "Command.h"
#include "../serialization/DataInput.h"
#include "../serialization/DataOutput.h"
#include <iostream>
#include <stdexcept>
#include <utility>

namespace hazelcast{
namespace client{
namespace protocol{
namespace MapCommands{    

static std::string NEWLINE = "\r\n";
static std::string SPACE = " ";
    

class ContainsKeyCommand : public Command{
public:
    ContainsKeyCommand(std::string instanceName, hazelcast::client::serialization::Data key ):instanceName(instanceName)
                                                                                     ,key(key)
    {    
    };
    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
        std::string command = "MCONTAINSKEY";
        command += SPACE + instanceName + SPACE + "#1" + NEWLINE;
        dataOutput.write(command.c_str(),0,command.length());

        char integerBuffer[5];
        int integerBufferSize = sprintf(integerBuffer,"%d",key.totalSize());
        dataOutput.write(integerBuffer, 0,integerBufferSize);

        dataOutput.write(NEWLINE.c_str(),0,NEWLINE.length());

        key.writeData(dataOutput);
        
        dataOutput.write(NEWLINE.c_str(),0,NEWLINE.length());
    };
    void readHeaderLine(std::string line){
        containsKey = line.compare("OK true") ? false  : true;
    };
    void readSizeLine(std::string line) {
    };
    void readResult(hazelcast::client::serialization::DataInput& dataInput){
    };
    int nResults(){
       return 0;  
    };
    int resultSize(int i){
    };
    bool get(){
        return containsKey;
    };
private:
    std::string instanceName;
    hazelcast::client::serialization::Data key;
    bool containsKey;
}; 

class ContainsValueCommand : public Command{
public:
    ContainsValueCommand(std::string instanceName, hazelcast::client::serialization::Data key ):instanceName(instanceName)
                                                                                     ,key(key)
    {    
    };
    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
        std::string command = "MCONTAINSVALUE";
        command += SPACE + instanceName + SPACE + "#1" + NEWLINE;
        dataOutput.write(command.c_str(),0,command.length());

        char integerBuffer[5];
        int integerBufferSize = sprintf(integerBuffer,"%d",key.totalSize());
        dataOutput.write(integerBuffer, 0,integerBufferSize);

        dataOutput.write(NEWLINE.c_str(),0,NEWLINE.length());

        key.writeData(dataOutput);
        
        dataOutput.write(NEWLINE.c_str(),0,NEWLINE.length());
    };
    void readHeaderLine(std::string line){
        containsValue = line.compare("OK true") ? false  : true;
    };
    void readSizeLine(std::string line) {
    };
    void readResult(hazelcast::client::serialization::DataInput& dataInput){
    };
    int nResults(){
       return 0;  
    };
    int resultSize(int i){
    };
    bool get(){
        return containsValue;
    };
private:
    std::string instanceName;
    hazelcast::client::serialization::Data key;
    bool containsValue;
};

class PutCommand : public Command{
public:
    PutCommand(std::string instanceName,
               hazelcast::client::serialization::Data key , 
               hazelcast::client::serialization::Data value,
               long ttl)
                         :instanceName(instanceName)
                         ,key(key)
                         ,value(value)
                         ,ttl(ttl)
    {    
    };
    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
        std::string command = "MPUT";
        command += SPACE + instanceName + SPACE;
        char integerBuffer[10];
        int integerBufferSize;
        
        integerBufferSize = sprintf(integerBuffer,"%li",ttl);
        command.append(integerBuffer,integerBufferSize);
        command += SPACE;
            
        command += "#2" + NEWLINE;    

        integerBufferSize = sprintf(integerBuffer,"%d",key.totalSize());
        command.append(integerBuffer,integerBufferSize);
        command += SPACE;

        integerBufferSize = sprintf(integerBuffer,"%d",value.totalSize());
        command.append(integerBuffer,integerBufferSize);
        
        command += NEWLINE;

        dataOutput.write(command.c_str(),0,command.length());

        key.writeData(dataOutput);
        value.writeData(dataOutput);

        dataOutput.write(NEWLINE.c_str(),0,NEWLINE.length());
    };
    void readHeaderLine(std::string line){
        if(line.compare("OK #1"))
            throw std::domain_error("unexpected header of put return");
    };
    void readSizeLine(std::string line) {
        returnSize = atoi(line.c_str());
    };
    void readResult(hazelcast::client::serialization::DataInput& dataInput){
    };
    int nResults(){
       return 1;  
    };
    int resultSize(int i){
        return returnSize;
    };
private:
    std::string instanceName;
    hazelcast::client::serialization::Data key;
    hazelcast::client::serialization::Data value;
    long ttl;
    int returnSize;
}; 

class GetCommand : public Command{
public:
    GetCommand(std::string instanceName, hazelcast::client::serialization::Data key ):instanceName(instanceName)
                                                                                     ,key(key)
    {    
    };
    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
        std::string command = "MGET";
        command += SPACE + instanceName + SPACE + "#1" + NEWLINE;

        char integerBuffer[5];
        int integerBufferSize = sprintf(integerBuffer,"%d",key.totalSize());
        command.append(integerBuffer,integerBufferSize);
        command += NEWLINE;
        
        dataOutput.write(command.c_str(),0,command.length());
        
        key.writeData(dataOutput);
        
        dataOutput.write(NEWLINE.c_str(),0,NEWLINE.length());
    };
    void readHeaderLine(std::string line){
        if(line.compare("OK #1"))
            throw std::domain_error("unexpected header of get return");
    };
    void readSizeLine(std::string line) {
        int returnSize = atoi(line.c_str());
        resultSizes.push_back(returnSize);
    };
    void readResult(hazelcast::client::serialization::DataInput& dataInput){
        oldValue.readData(dataInput);
    };
    int nResults(){
       return 1;  
    };
    int resultSize(int i){
        return resultSizes[i];
    };
    hazelcast::client::serialization::Data get(){
      return oldValue;  
    };
private:
    std::string instanceName;
    hazelcast::client::serialization::Data key;
    std::vector<int> resultSizes;
    hazelcast::client::serialization::Data oldValue;
}; 

class RemoveCommand : public Command{
public:
    RemoveCommand(std::string instanceName, hazelcast::client::serialization::Data key ):instanceName(instanceName)
                                                                                     ,key(key)
    {    
    };
    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
        std::string command = "MREMOVE";
        command += SPACE + instanceName + SPACE + "#1" + NEWLINE;

        char integerBuffer[5];
        int integerBufferSize = sprintf(integerBuffer,"%d",key.totalSize());
        command.append(integerBuffer,integerBufferSize);
        command += NEWLINE;
        
        dataOutput.write(command.c_str(),0,command.length());
        
        key.writeData(dataOutput);
        
        dataOutput.write(NEWLINE.c_str(),0,NEWLINE.length());
    };
    void readHeaderLine(std::string line){
        if(line.compare("OK #1"))
            throw std::domain_error("unexpected header of remove return");
    };
    void readSizeLine(std::string line) {
    };
    void readResult(hazelcast::client::serialization::DataInput& dataInput){
    };
    int nResults(){
       return 0;  
    };
    int resultSize(int i){
        return 0;
    };
    hazelcast::client::serialization::Data get(){
    };
private:
    std::string instanceName;
    hazelcast::client::serialization::Data key;
}; 

class FlushCommand : public Command{
public:
    FlushCommand(std::string instanceName):instanceName(instanceName)
    {    
    };
    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
        std::string command = "MFLUSH";
        command += SPACE + instanceName + NEWLINE;
        dataOutput.write(command.c_str(),0,command.length());
    };
    void readHeaderLine(std::string line){
       if(line.compare("OK "))
            throw std::domain_error("unexpected header of containsKey return");
            
    };
    void readSizeLine(std::string line) {
    };
    void readResult(hazelcast::client::serialization::DataInput& dataInput){
    };
    int nResults(){
       return 0;  
    };
    int resultSize(int i){
    };
private:
    std::string instanceName;
}; 

class GetAllCommand : public Command{
public:
    typedef std::vector<hazelcast::client::serialization::Data> DataSet;
    GetAllCommand(std::string instanceName, std::vector<hazelcast::client::serialization::Data> keySet ):instanceName(instanceName)
                                                                                     ,keySet(keySet)
    {    
    };
    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
        std::string command = "MGETALL";
        command += SPACE + instanceName + SPACE + "#";
        
        char integerBuffer[5];
        int integerBufferSize = sprintf(integerBuffer,"%d",(int)keySet.size());
        command.append(integerBuffer,integerBufferSize);
        command += NEWLINE;
        
        for(DataSet::const_iterator it = keySet.begin() ; it != keySet.end(); it++){ 
            char integerBuffer[5];
            int integerBufferSize = sprintf(integerBuffer,"%d",it->totalSize());
            command.append(integerBuffer,integerBufferSize);
            
            command += SPACE;
        }
        command.erase(command.end()-1);
        command += NEWLINE;
        
        dataOutput.write(command.c_str(),0,command.length());
        for(DataSet::const_iterator it = keySet.begin() ; it != keySet.end(); it++){ 
            it->writeData(dataOutput);
        }
        dataOutput.write(NEWLINE.c_str(),0,NEWLINE.length());
    };
    void readHeaderLine(std::string line){
        int pos = line.find_first_of('#');
        std::string ok = line.substr(0,pos);
        if(ok.compare("OK "))
            throw std::domain_error("unexpected header of getAll return");
        std::string sizeStr = line.substr(pos + 1,line.length() - pos);
        nReturnedResults = std::atoi(sizeStr.c_str());
        nReturnedResults /= 2;
    };
    void readSizeLine(std::string line) {
        int beg = 0 , end = 0 , middle = 0;
        do
        {
          middle = line.find_first_of(" ",beg);
          end = line.find_first_of(" ",middle + 1);
          
          std::string keySize = line.substr(beg ,middle - beg);
          int a = atoi(keySize.c_str());
          keySizes.push_back(a);
          
          std::string valueSize = line.substr(middle ,end - middle);
          int b = atoi(valueSize.c_str());
          valueSizes.push_back(b);
          
          beg = end;
          beg++;
          
        }while (end != std::string::npos);

    };
    void readResult(hazelcast::client::serialization::DataInput& dataInput){
        using namespace hazelcast::client::serialization;
        Data key;
        key.readData(dataInput);
        keys.push_back(key);
        
        Data value;
        value.readData(dataInput);
        values.push_back(value);

    };
    int nResults(){
       return nReturnedResults;  
    };
    int resultSize(int i){
        return keySizes[i] + valueSizes[i];
    };

    DataSet getKeys(){
      return keys;  
    };
    DataSet getValues(){
      return values;  
    };
private:
    std::string instanceName;
    DataSet keySet;
    int nReturnedResults;
    std::vector<int> keySizes;
    std::vector<int> valueSizes;
    DataSet keys;
    DataSet values;
};

class TryRemoveCommand : public Command{
public:
    TryRemoveCommand(std::string instanceName, hazelcast::client::serialization::Data key, long timeoutInMillis )
                                                                        :instanceName(instanceName)
                                                                        ,key(key)
                                                                        ,timeoutInMillis(timeoutInMillis)
    {    
    };
    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
        std::string command = "MTRYREMOVE";
        command += SPACE + instanceName + SPACE;

        char integerBuffer[5];
        int integerBufferSize = sprintf(integerBuffer,"%li",timeoutInMillis);
        command.append(integerBuffer,integerBufferSize);
        
        command += SPACE + "#1" + NEWLINE;
        
        integerBuffer[5];
        integerBufferSize = sprintf(integerBuffer,"%d",key.totalSize());
        command.append(integerBuffer,integerBufferSize);

        command += NEWLINE;
        
        dataOutput.write(command.c_str(),0,command.length());

        key.writeData(dataOutput);
        
        dataOutput.write(NEWLINE.c_str(),0,NEWLINE.length());
    };
    void readHeaderLine(std::string line){
       if(!line.compare("OK #1"))
           success = true;
       else if(!line.compare("OK timeout"))
           success = false;
       else
            throw std::domain_error("unexpected header of containsKey return");
            
    };
    void readSizeLine(std::string line) {
    };
    void readResult(hazelcast::client::serialization::DataInput& dataInput){
    };
    int nResults(){
       return 0;  
    };
    int resultSize(int i){
    };
    bool get(){
        return success;
    };
private:
    std::string instanceName;
    hazelcast::client::serialization::Data key;
    long timeoutInMillis;
    bool success;
}; 

class TryPutCommand : public Command{
public:
    TryPutCommand(std::string instanceName, 
                  hazelcast::client::serialization::Data key , 
                  hazelcast::client::serialization::Data value,
                  long timeoutInMillis)
                                :instanceName(instanceName)
                                ,key(key)
                                ,value(value)
                                ,timeoutInMillis(timeoutInMillis)
    {    
    };
    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
        std::string command = "MTRYPUT";
        command += SPACE + instanceName + SPACE + "#2" + NEWLINE;
        dataOutput.write(command.c_str(),0,command.length());

        char integerBuffer[10];
        int integerBufferSize = sprintf(integerBuffer,"%d",key.totalSize());
        dataOutput.write(integerBuffer, 0,integerBufferSize);

        dataOutput.write(SPACE.c_str(),0,SPACE.length());

        integerBufferSize = sprintf(integerBuffer,"%d",value.totalSize());
        dataOutput.write(integerBuffer, 0,integerBufferSize);

        dataOutput.write(NEWLINE.c_str(),0,NEWLINE.length());

        key.writeData(dataOutput);
        value.writeData(dataOutput);

        dataOutput.write(NEWLINE.c_str(),0,NEWLINE.length());
    };
    void readHeaderLine(std::string line){
        success = line.compare("OK true") ? false  : true;
    };
    void readSizeLine(std::string line) {
    };
    void readResult(hazelcast::client::serialization::DataInput& dataInput){
    };
    int nResults(){
       return 0;  
    };
    int resultSize(int i){
    };
    bool get(){
      return success;  
    };
private:
    std::string instanceName;
    hazelcast::client::serialization::Data key;
    hazelcast::client::serialization::Data value;
    bool success;
    long timeoutInMillis;
}; 

class PutTransientCommand : public Command{
public:
    PutTransientCommand(std::string instanceName,
               hazelcast::client::serialization::Data key , 
               hazelcast::client::serialization::Data value,
               long ttl)
                         :instanceName(instanceName)
                         ,key(key)
                         ,value(value)
                         ,ttl(ttl)
    {    
    };
    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
        std::string command = "MPUTTRANSIENT";
        command += SPACE + instanceName + SPACE;
        char integerBuffer[10];
        int integerBufferSize;
        
        integerBufferSize = sprintf(integerBuffer,"%li",ttl);
        command.append(integerBuffer,integerBufferSize);
        command += SPACE;
            
        command += "#2" + NEWLINE;    

        integerBufferSize = sprintf(integerBuffer,"%d",key.totalSize());
        command.append(integerBuffer,integerBufferSize);
        command += SPACE;

        integerBufferSize = sprintf(integerBuffer,"%d",value.totalSize());
        command.append(integerBuffer,integerBufferSize);
        
        command += NEWLINE;

        dataOutput.write(command.c_str(),0,command.length());

        key.writeData(dataOutput);
        value.writeData(dataOutput);

        dataOutput.write(NEWLINE.c_str(),0,NEWLINE.length());
    };
    void readHeaderLine(std::string line){
        if(line.compare("OK"))
            throw std::domain_error("unexpected header of putTransient return");
    };
    void readSizeLine(std::string line) {
    };
    void readResult(hazelcast::client::serialization::DataInput& dataInput){
    };
    int nResults(){
       return 0;  
    };
    int resultSize(int i){
    };
private:
    std::string instanceName;
    hazelcast::client::serialization::Data key;
    hazelcast::client::serialization::Data value;
    long ttl;
}; 
}}}}

#endif /* HAZELCAST_MAP_COMMANDS */