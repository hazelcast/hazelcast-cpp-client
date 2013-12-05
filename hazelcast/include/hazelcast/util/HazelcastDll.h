#ifdef WIN32
	#ifdef HAZELCAST_EXPORTS
		#define HAZELCAST_API __declspec(dllexport)
	#else
		#define HAZELCAST_API __declspec(dllimport)
	#endif	
#else
	#define HAZELCAST_API
 #endif