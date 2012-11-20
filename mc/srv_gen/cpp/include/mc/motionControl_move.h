/* Auto-generated by genmsg_cpp for file /home/user/workspaces/ros/Armed-turtlebot/mc/srv/motionControl_move.srv */
#ifndef MC_SERVICE_MOTIONCONTROL_MOVE_H
#define MC_SERVICE_MOTIONCONTROL_MOVE_H
#include <string>
#include <vector>
#include <map>
#include <ostream>
#include "ros/serialization.h"
#include "ros/builtin_message_traits.h"
#include "ros/message_operations.h"
#include "ros/time.h"

#include "ros/macros.h"

#include "ros/assert.h"

#include "ros/service_traits.h"




namespace mc
{
template <class ContainerAllocator>
struct motionControl_moveRequest_ {
  typedef motionControl_moveRequest_<ContainerAllocator> Type;

  motionControl_moveRequest_()
  : linear(0.0)
  , angular(0.0)
  {
  }

  motionControl_moveRequest_(const ContainerAllocator& _alloc)
  : linear(0.0)
  , angular(0.0)
  {
  }

  typedef double _linear_type;
  double linear;

  typedef double _angular_type;
  double angular;


  typedef boost::shared_ptr< ::mc::motionControl_moveRequest_<ContainerAllocator> > Ptr;
  typedef boost::shared_ptr< ::mc::motionControl_moveRequest_<ContainerAllocator>  const> ConstPtr;
  boost::shared_ptr<std::map<std::string, std::string> > __connection_header;
}; // struct motionControl_moveRequest
typedef  ::mc::motionControl_moveRequest_<std::allocator<void> > motionControl_moveRequest;

typedef boost::shared_ptr< ::mc::motionControl_moveRequest> motionControl_moveRequestPtr;
typedef boost::shared_ptr< ::mc::motionControl_moveRequest const> motionControl_moveRequestConstPtr;


template <class ContainerAllocator>
struct motionControl_moveResponse_ {
  typedef motionControl_moveResponse_<ContainerAllocator> Type;

  motionControl_moveResponse_()
  {
  }

  motionControl_moveResponse_(const ContainerAllocator& _alloc)
  {
  }


  typedef boost::shared_ptr< ::mc::motionControl_moveResponse_<ContainerAllocator> > Ptr;
  typedef boost::shared_ptr< ::mc::motionControl_moveResponse_<ContainerAllocator>  const> ConstPtr;
  boost::shared_ptr<std::map<std::string, std::string> > __connection_header;
}; // struct motionControl_moveResponse
typedef  ::mc::motionControl_moveResponse_<std::allocator<void> > motionControl_moveResponse;

typedef boost::shared_ptr< ::mc::motionControl_moveResponse> motionControl_moveResponsePtr;
typedef boost::shared_ptr< ::mc::motionControl_moveResponse const> motionControl_moveResponseConstPtr;

struct motionControl_move
{

typedef motionControl_moveRequest Request;
typedef motionControl_moveResponse Response;
Request request;
Response response;

typedef Request RequestType;
typedef Response ResponseType;
}; // struct motionControl_move
} // namespace mc

namespace ros
{
namespace message_traits
{
template<class ContainerAllocator> struct IsMessage< ::mc::motionControl_moveRequest_<ContainerAllocator> > : public TrueType {};
template<class ContainerAllocator> struct IsMessage< ::mc::motionControl_moveRequest_<ContainerAllocator>  const> : public TrueType {};
template<class ContainerAllocator>
struct MD5Sum< ::mc::motionControl_moveRequest_<ContainerAllocator> > {
  static const char* value() 
  {
    return "144a16e4d6b53a0dbadc2e617460a173";
  }

  static const char* value(const  ::mc::motionControl_moveRequest_<ContainerAllocator> &) { return value(); } 
  static const uint64_t static_value1 = 0x144a16e4d6b53a0dULL;
  static const uint64_t static_value2 = 0xbadc2e617460a173ULL;
};

template<class ContainerAllocator>
struct DataType< ::mc::motionControl_moveRequest_<ContainerAllocator> > {
  static const char* value() 
  {
    return "mc/motionControl_moveRequest";
  }

  static const char* value(const  ::mc::motionControl_moveRequest_<ContainerAllocator> &) { return value(); } 
};

template<class ContainerAllocator>
struct Definition< ::mc::motionControl_moveRequest_<ContainerAllocator> > {
  static const char* value() 
  {
    return "float64 linear\n\
float64 angular\n\
\n\
";
  }

  static const char* value(const  ::mc::motionControl_moveRequest_<ContainerAllocator> &) { return value(); } 
};

template<class ContainerAllocator> struct IsFixedSize< ::mc::motionControl_moveRequest_<ContainerAllocator> > : public TrueType {};
} // namespace message_traits
} // namespace ros


namespace ros
{
namespace message_traits
{
template<class ContainerAllocator> struct IsMessage< ::mc::motionControl_moveResponse_<ContainerAllocator> > : public TrueType {};
template<class ContainerAllocator> struct IsMessage< ::mc::motionControl_moveResponse_<ContainerAllocator>  const> : public TrueType {};
template<class ContainerAllocator>
struct MD5Sum< ::mc::motionControl_moveResponse_<ContainerAllocator> > {
  static const char* value() 
  {
    return "d41d8cd98f00b204e9800998ecf8427e";
  }

  static const char* value(const  ::mc::motionControl_moveResponse_<ContainerAllocator> &) { return value(); } 
  static const uint64_t static_value1 = 0xd41d8cd98f00b204ULL;
  static const uint64_t static_value2 = 0xe9800998ecf8427eULL;
};

template<class ContainerAllocator>
struct DataType< ::mc::motionControl_moveResponse_<ContainerAllocator> > {
  static const char* value() 
  {
    return "mc/motionControl_moveResponse";
  }

  static const char* value(const  ::mc::motionControl_moveResponse_<ContainerAllocator> &) { return value(); } 
};

template<class ContainerAllocator>
struct Definition< ::mc::motionControl_moveResponse_<ContainerAllocator> > {
  static const char* value() 
  {
    return "\n\
\n\
";
  }

  static const char* value(const  ::mc::motionControl_moveResponse_<ContainerAllocator> &) { return value(); } 
};

template<class ContainerAllocator> struct IsFixedSize< ::mc::motionControl_moveResponse_<ContainerAllocator> > : public TrueType {};
} // namespace message_traits
} // namespace ros

namespace ros
{
namespace serialization
{

template<class ContainerAllocator> struct Serializer< ::mc::motionControl_moveRequest_<ContainerAllocator> >
{
  template<typename Stream, typename T> inline static void allInOne(Stream& stream, T m)
  {
    stream.next(m.linear);
    stream.next(m.angular);
  }

  ROS_DECLARE_ALLINONE_SERIALIZER;
}; // struct motionControl_moveRequest_
} // namespace serialization
} // namespace ros


namespace ros
{
namespace serialization
{

template<class ContainerAllocator> struct Serializer< ::mc::motionControl_moveResponse_<ContainerAllocator> >
{
  template<typename Stream, typename T> inline static void allInOne(Stream& stream, T m)
  {
  }

  ROS_DECLARE_ALLINONE_SERIALIZER;
}; // struct motionControl_moveResponse_
} // namespace serialization
} // namespace ros

namespace ros
{
namespace service_traits
{
template<>
struct MD5Sum<mc::motionControl_move> {
  static const char* value() 
  {
    return "144a16e4d6b53a0dbadc2e617460a173";
  }

  static const char* value(const mc::motionControl_move&) { return value(); } 
};

template<>
struct DataType<mc::motionControl_move> {
  static const char* value() 
  {
    return "mc/motionControl_move";
  }

  static const char* value(const mc::motionControl_move&) { return value(); } 
};

template<class ContainerAllocator>
struct MD5Sum<mc::motionControl_moveRequest_<ContainerAllocator> > {
  static const char* value() 
  {
    return "144a16e4d6b53a0dbadc2e617460a173";
  }

  static const char* value(const mc::motionControl_moveRequest_<ContainerAllocator> &) { return value(); } 
};

template<class ContainerAllocator>
struct DataType<mc::motionControl_moveRequest_<ContainerAllocator> > {
  static const char* value() 
  {
    return "mc/motionControl_move";
  }

  static const char* value(const mc::motionControl_moveRequest_<ContainerAllocator> &) { return value(); } 
};

template<class ContainerAllocator>
struct MD5Sum<mc::motionControl_moveResponse_<ContainerAllocator> > {
  static const char* value() 
  {
    return "144a16e4d6b53a0dbadc2e617460a173";
  }

  static const char* value(const mc::motionControl_moveResponse_<ContainerAllocator> &) { return value(); } 
};

template<class ContainerAllocator>
struct DataType<mc::motionControl_moveResponse_<ContainerAllocator> > {
  static const char* value() 
  {
    return "mc/motionControl_move";
  }

  static const char* value(const mc::motionControl_moveResponse_<ContainerAllocator> &) { return value(); } 
};

} // namespace service_traits
} // namespace ros

#endif // MC_SERVICE_MOTIONCONTROL_MOVE_H

