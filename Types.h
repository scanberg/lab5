#ifndef TYPES_H
#define TYPES_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
//#include <glm/gtc/random.hpp>
#include <glm/gtx/random.hpp>
#include <omp.h>

#ifndef MAXFLOAT
#define MAXFLOAT 1e+37f
#endif

#ifndef PI
#define PI 3.1415926536f
#endif


using glm::vec2;
using glm::vec3;
using glm::vec4;

using glm::uvec2;
using glm::uvec3;
using glm::uvec4;

using glm::ivec2;
using glm::ivec3;
using glm::ivec4;

using glm::mat3;
using glm::mat4;

using glm::quat;


typedef unsigned char 	        u8;
typedef unsigned short 	        u16;
typedef unsigned int 	        u32;
typedef unsigned long	        u64;

typedef signed char		        i8;
typedef signed short 	        i16;
typedef signed int		        i32;
typedef signed long		        i64;

typedef glm::detail::float16	f16;
typedef float		 	        f32;
typedef double			        f64;

#endif
