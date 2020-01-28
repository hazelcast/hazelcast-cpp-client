/**
 * Autogenerated by Thrift Compiler (0.13.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef remote-controller_TYPES_H
#define remote-controller_TYPES_H

#include <iosfwd>

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/TBase.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <functional>
#include <memory>




struct Lang {
  enum type {
    JAVASCRIPT = 1,
    GROOVY = 2,
    PYTHON = 3,
    RUBY = 4
  };
};

extern const std::map<int, const char*> _Lang_VALUES_TO_NAMES;

std::ostream& operator<<(std::ostream& out, const Lang::type& val);

std::string to_string(const Lang::type& val);

class Cluster;

class Member;

class Response;

class ServerException;

typedef struct _Cluster__isset {
  _Cluster__isset() : id(false) {}
  bool id :1;
} _Cluster__isset;

class Cluster : public virtual ::apache::thrift::TBase {
 public:

  Cluster(const Cluster&);
  Cluster& operator=(const Cluster&);
  Cluster() : id() {
  }

  virtual ~Cluster() noexcept;
  std::string id;

  _Cluster__isset __isset;

  void __set_id(const std::string& val);

  bool operator == (const Cluster & rhs) const
  {
    if (!(id == rhs.id))
      return false;
    return true;
  }
  bool operator != (const Cluster &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Cluster & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(Cluster &a, Cluster &b);

std::ostream& operator<<(std::ostream& out, const Cluster& obj);

typedef struct _Member__isset {
  _Member__isset() : uuid(false), host(false), port(false) {}
  bool uuid :1;
  bool host :1;
  bool port :1;
} _Member__isset;

class Member : public virtual ::apache::thrift::TBase {
 public:

  Member(const Member&);
  Member& operator=(const Member&);
  Member() : uuid(), host(), port(0) {
  }

  virtual ~Member() noexcept;
  std::string uuid;
  std::string host;
  int32_t port;

  _Member__isset __isset;

  void __set_uuid(const std::string& val);

  void __set_host(const std::string& val);

  void __set_port(const int32_t val);

  bool operator == (const Member & rhs) const
  {
    if (!(uuid == rhs.uuid))
      return false;
    if (!(host == rhs.host))
      return false;
    if (!(port == rhs.port))
      return false;
    return true;
  }
  bool operator != (const Member &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Member & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(Member &a, Member &b);

std::ostream& operator<<(std::ostream& out, const Member& obj);

typedef struct _Response__isset {
  _Response__isset() : success(false), message(false), result(false) {}
  bool success :1;
  bool message :1;
  bool result :1;
} _Response__isset;

class Response : public virtual ::apache::thrift::TBase {
 public:

  Response(const Response&);
  Response& operator=(const Response&);
  Response() : success(0), message(), result() {
  }

  virtual ~Response() noexcept;
  bool success;
  std::string message;
  std::string result;

  _Response__isset __isset;

  void __set_success(const bool val);

  void __set_message(const std::string& val);

  void __set_result(const std::string& val);

  bool operator == (const Response & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    if (!(message == rhs.message))
      return false;
    if (!(result == rhs.result))
      return false;
    return true;
  }
  bool operator != (const Response &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Response & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(Response &a, Response &b);

std::ostream& operator<<(std::ostream& out, const Response& obj);

typedef struct _ServerException__isset {
  _ServerException__isset() : message(false) {}
  bool message :1;
} _ServerException__isset;

class ServerException : public ::apache::thrift::TException {
 public:

  ServerException(const ServerException&);
  ServerException& operator=(const ServerException&);
  ServerException() : message() {
  }

  virtual ~ServerException() noexcept;
  std::string message;

  _ServerException__isset __isset;

  void __set_message(const std::string& val);

  bool operator == (const ServerException & rhs) const
  {
    if (!(message == rhs.message))
      return false;
    return true;
  }
  bool operator != (const ServerException &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ServerException & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
  mutable std::string thriftTExceptionMessageHolder_;
  const char* what() const noexcept;
};

void swap(ServerException &a, ServerException &b);

std::ostream& operator<<(std::ostream& out, const ServerException& obj);



#endif
