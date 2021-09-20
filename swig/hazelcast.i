%module hazelcast

%include <typemaps.i>

%apply int* OUTPUT {int *client_id}; // int *result is output
%apply long* OUTPUT {long *value}; // int *result is output

%{
#include "hazelcast/hazelcastc.h"
%}

%include "../../include/hazelcast/hazelcastc.h"
