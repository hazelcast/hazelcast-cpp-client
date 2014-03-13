#if defined(__SIZEOF_INT128__)
#  define BOOST_HAS_INT128
#endif

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
	#ifdef HAZELCAST_EXPORTS
		#define HAZELCAST_API __declspec(dllexport)
	#else
		#define HAZELCAST_API __declspec(dllimport)
	#endif	
#else
	#define HAZELCAST_API
 #endif
namespace hazelcast{
    typedef unsigned char byte;
}