/********************************************************************************//**
\file      OVR_Math.h
\brief     Implementation of 3D primitives such as vectors, matrices.
\copyright Copyright 2014-2016 Oculus VR, LLC All Rights reserved.
*************************************************************************************/

#ifndef OVR_Math_h
#define OVR_Math_h


// This file is intended to be independent of the rest of LibOVR and LibOVRKernel and thus 
// has no #include dependencies on either.

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include "../OVR_CAPI.h" // Currently required due to a dependence on the ovrFovPort_ declaration.

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable: 4127) // conditional expression is constant
#endif


#if defined(_MSC_VER)
    #define OVRMath_sprintf sprintf_s
#else
    #define OVRMath_sprintf snprintf
#endif


//-------------------------------------------------------------------------------------
// ***** OVR_MATH_ASSERT
//
// Independent debug break implementation for OVR_Math.h.

#if !defined(OVR_MATH_DEBUG_BREAK)
    #if defined(_DEBUG)
        #if defined(_MSC_VER)
            #define OVR_MATH_DEBUG_BREAK __debugbreak()
        #else
            #define OVR_MATH_DEBUG_BREAK __builtin_trap()
        #endif
    #else
        #define OVR_MATH_DEBUG_BREAK ((void)0)
    #endif
#endif


//-------------------------------------------------------------------------------------
// ***** OVR_MATH_ASSERT
//
// Independent OVR_MATH_ASSERT implementation for OVR_Math.h.

#if !defined(OVR_MATH_ASSERT)
    #if defined(_DEBUG)
        #define OVR_MATH_ASSERT(p) if (!(p)) { OVR_MATH_DEBUG_BREAK; }
    #else
        #define OVR_MATH_ASSERT(p) ((void)0)
    #endif
#endif


//-------------------------------------------------------------------------------------
// ***** OVR_MATH_STATIC_ASSERT
//
// Independent OVR_MATH_ASSERT implementation for OVR_Math.h.

#if !defined(OVR_MATH_STATIC_ASSERT)
    #if defined(__cplusplus) && ((defined(_MSC_VER) && (defined(_MSC_VER) >= 1600)) || defined(__GXX_EXPERIMENTAL_CXX0X__) || (__cplusplus >= 201103L))
        #define OVR_MATH_STATIC_ASSERT static_assert
    #else
        #if !defined(OVR_SA_UNUSED)
            #if defined(__GNUC__) || defined(__clang__)
                #define OVR_SA_UNUSED __attribute__((unused))
            #else
                #define OVR_SA_UNUSED
            #endif
            #define OVR_SA_PASTE(a,b) a##b
            #define OVR_SA_HELP(a,b)  OVR_SA_PASTE(a,b)
        #endif

        #define OVR_MATH_STATIC_ASSERT(expression, msg) typedef char OVR_SA_HELP(compileTimeAssert, __LINE__) [((expression) != 0) ? 1 : -1] OVR_SA_UNUSED
    #endif
#endif



namespace OVR {

template<class T>
const T OVRMath_Min(const T a, const T b)
{ return (a < b) ? a : b; }

template<class T>
const T OVRMath_Max(const T a, const T b)
{ return (b < a) ? a : b; }

template<class T>
void OVRMath_Swap(T& a, T& b) 
{  T temp(a); a = b; b = temp; }


//-------------------------------------------------------------------------------------
// ***** Constants for 3D world/axis definitions.

// Definitions of axes for coordinate and rotation conversions.
enum Axis
{
    Axis_X = 0, Axis_Y = 1, Axis_Z = 2
};

// RotateDirection describes the rotation direction around an axis, interpreted as follows:
//  CW  - Clockwise while looking "down" from positive axis towards the origin.
//  CCW - Counter-clockwise while looking from the positive axis towards the origin,
//        which is in the negative axis direction.
//  CCW is the default for the RHS coordinate system. Oculus standard RHS coordinate
//  system defines Y up, X right, and Z back (pointing out from the screen). In this
//  system Rotate_CCW around Z will specifies counter-clockwise rotation in XY plane.
enum RotateDirection
{
    Rotate_CCW = 1,
    Rotate_CW  = -1 
};

// Constants for right handed and left handed coordinate systems
enum HandedSystem
{
    Handed_R = 1, Handed_L = -1
};

// AxisDirection describes which way the coordinate axis points. Used by WorldAxes.
enum AxisDirection
{
    Axis_Up    =  2,
    Axis_Down  = -2,
    Axis_Right =  1,
    Axis_Left  = -1,
    Axis_In    =  3,
    Axis_Out   = -3
};

struct WorldAxes
{
    AxisDirection XAxis, YAxis, ZAxis;

    WorldAxes(AxisDirection x, AxisDirection y, AxisDirection z)
        : XAxis(x), YAxis(y), ZAxis(z) 
    { OVR_MATH_ASSERT(abs(x) != abs(y) && abs(y) != abs(z) && abs(z) != abs(x));}
};

} // namespace OVR


//------------------------------------------------------------------------------------//
// ***** C Compatibility Types

// These declarations are used to support conversion between C types used in
// LibOVR C interfaces and their C++ versions. As an example, they allow passing
// Vector3f into a function that expects ovrVector3f.

typedef struct ovrQuatf_ ovrQuatf;
typedef struct ovrQuatd_ ovrQuatd;
typedef struct ovrSizei_ ovrSizei;
typedef struct ovrSizef_ ovrSizef;
typedef struct ovrSized_ ovrSized;
typedef struct ovrRecti_ ovrRecti;
typedef struct ovrVector2i_ ovrVector2i;
typedef struct ovrVector2f_ ovrVector2f;
typedef struct ovrVector2d_ ovrVector2d;
typedef struct ovrVector3f_ ovrVector3f;
typedef struct ovrVector3d_ ovrVector3d;
typedef struct ovrVector4f_ ovrVector4f;
typedef struct ovrVector4d_ ovrVector4d;
typedef struct ovrMatrix2f_ ovrMatrix2f;
typedef struct ovrMatrix2d_ ovrMatrix2d;
typedef struct ovrMatrix3f_ ovrMatrix3f;
typedef struct ovrMatrix3d_ ovrMatrix3d;
typedef struct ovrMatrix4f_ ovrMatrix4f;
typedef struct ovrMatrix4d_ ovrMatrix4d;
typedef struct ovrPosef_ ovrPosef;
typedef struct ovrPosed_ ovrPosed;
typedef struct ovrPoseStatef_ ovrPoseStatef;
typedef struct ovrPoseStated_ ovrPoseStated;

namespace OVR {

// Forward-declare our templates.
template<class T> class Quat;
template<class T> class Size;
template<class T> class Rect;
template<class T> class Vector2;
template<class T> class Vector3;
template<class T> class Vector4;
template<class T> class Matrix2;
template<class T> class Matrix3;
template<class T> class Matrix4;
template<class T> class Pose;
template<class T> class PoseState;

// CompatibleTypes::Type is used to lookup a compatible C-version of a C++ class.
template<class C>
struct CompatibleTypes
{    
    // Declaration here seems necessary for MSVC; specializations are
    // used instead.
    typedef struct {} Type;
};

// Specializations providing CompatibleTypes::Type value.
template<> struct CompatibleTypes<Quat<float> >     { typedef ovrQuatf Type; };
template<> struct CompatibleTypes<Quat<double> >    { typedef ovrQuatd Type; };
template<> struct CompatibleTypes<Matrix2<float> >  { typedef ovrMatrix2f Type; };
template<> struct CompatibleTypes<Matrix2<double> > { typedef ovrMatrix2d Type; };
template<> struct CompatibleTypes<Matrix3<float> >  { typedef ovrMatrix3f Type; };
template<> struct CompatibleTypes<Matrix3<double> > { typedef ovrMatrix3d Type; };
template<> struct CompatibleTypes<Matrix4<float> >  { typedef ovrMatrix4f Type; };
template<> struct CompatibleTypes<Matrix4<double> > { typedef ovrMatrix4d Type; };
template<> struct CompatibleTypes<Size<int> >       { typedef ovrSizei Type; };
template<> struct CompatibleTypes<Size<float> >     { typedef ovrSizef Type; };
template<> struct CompatibleTypes<Size<double> >    { typedef ovrSized Type; };
template<> struct CompatibleTypes<Rect<int> >       { typedef ovrRecti Type; };
template<> struct CompatibleTypes<Vector2<int> >    { typedef ovrVector2i Type; };
template<> struct CompatibleTypes<Vector2<float> >  { typedef ovrVector2f Type; };
template<> struct CompatibleTypes<Vector2<double> > { typedef ovrVector2d Type; };
template<> struct CompatibleTypes<Vector3<float> >  { typedef ovrVector3f Type; };
template<> struct CompatibleTypes<Vector3<double> > { typedef ovrVector3d Type; };
template<> struct CompatibleTypes<Vector4<float> >  { typedef ovrVector4f Type; };
template<> struct CompatibleTypes<Vector4<double> > { typedef ovrVector4d Type; };
template<> struct CompatibleTypes<Pose<float> >     { typedef ovrPosef Type; };
template<> struct CompatibleTypes<Pose<double> >    { typedef ovrPosed Type; };

//------------------------------------------------------------------------------------//
// ***** Math
//
// Math class contains constants and functions. This class is a template specialized
// per type, with Math<float> and Math<double> being distinct.
template<class T>
class Math
{  
public:
    // By default, support explicit conversion to float. This allows Vector2<int> to
    // compile, for example.
    typedef float OtherFloatType;

    static int Tolerance() { return 0; }  // Default value so integer types compile
};


//------------------------------------------------------------------------------------//
// ***** double constants
#define MATH_DOUBLE_PI              3.14159265358979323846
#define MATH_DOUBLE_TWOPI           (2*MATH_DOUBLE_PI)
#define MATH_DOUBLE_PIOVER2         (0.5*MATH_DOUBLE_PI)
#define MATH_DOUBLE_PIOVER4         (0.25*MATH_DOUBLE_PI)
#define MATH_FLOAT_MAXVALUE             (FLT_MAX) 

#define MATH_DOUBLE_RADTODEGREEFACTOR (360.0 / MATH_DOUBLE_TWOPI)
#define MATH_DOUBLE_DEGREETORADFACTOR (MATH_DOUBLE_TWOPI / 360.0)

#define MATH_DOUBLE_E               2.71828182845904523536
#define MATH_DOUBLE_LOG2E           1.44269504088896340736
#define MATH_DOUBLE_LOG10E          0.434294481903251827651
#define MATH_DOUBLE_LN2             0.693147180559945309417
#define MATH_DOUBLE_LN10            2.30258509299404568402

#define MATH_DOUBLE_SQRT2           1.41421356237309504880
#define MATH_DOUBLE_SQRT1_2         0.707106781186547524401

#define MATH_DOUBLE_TOLERANCE       1e-12   // a default number for value equality tolerance: about 4500*Epsilon;
#define MATH_DOUBLE_SINGULARITYRADIUS 1e-12 // about 1-cos(.0001 degree), for gimbal lock numerical problems    

//------------------------------------------------------------------------------------//
// ***** float constants
#define MATH_FLOAT_PI               float(MATH_DOUBLE_PI)
#define MATH_FLOAT_TWOPI            float(MATH_DOUBLE_TWOPI)
#define MATH_FLOAT_PIOVER2          float(MATH_DOUBLE_PIOVER2)
#define MATH_FLOAT_PIOVER4          float(MATH_DOUBLE_PIOVER4)

#define MATH_FLOAT_RADTODEGREEFACTOR float(MATH_DOUBLE_RADTODEGREEFACTOR)
#define MATH_FLOAT_DEGREETORADFACTOR float(MATH_DOUBLE_DEGREETORADFACTOR)

#define MATH_FLOAT_E                float(MATH_DOUBLE_E)
#define MATH_FLOAT_LOG2E            float(MATH_DOUBLE_LOG2E)
#define MATH_FLOAT_LOG10E           float(MATH_DOUBLE_LOG10E)
#define MATH_FLOAT_LN2              float(MATH_DOUBLE_LN2)
#define MATH_FLOAT_LN10             float(MATH_DOUBLE_LN10)

#define MATH_FLOAT_SQRT2            float(MATH_DOUBLE_SQRT2)
#define MATH_FLOAT_SQRT1_2          float(MATH_DOUBLE_SQRT1_2)

#define MATH_FLOAT_TOLERANCE        1e-5f   // a default number for value equality tolerance: 1e-5, about 84*EPSILON;
#define MATH_FLOAT_SINGULARITYRADIUS 1e-7f  // about 1-cos(.025 degree), for gimbal lock numerical problems   



// Single-precision Math constants class.
template<>
class Math<float>
{
public:
     typedef double OtherFloatType;

    static inline float Tolerance()         { return MATH_FLOAT_TOLERANCE; }; // a default number for value equality tolerance
    static inline float SingularityRadius() { return MATH_FLOAT_SINGULARITYRADIUS; };    // for gimbal lock numerical problems    
};

// Double-precision Math constants class
template<>
class Math<double>
{
public:
    typedef float OtherFloatType;

    static inline double Tolerance()         { return MATH_DOUBLE_TOLERANCE; }; // a default number for value equality tolerance
    static inline double SingularityRadius() { return MATH_DOUBLE_SINGULARITYRADIUS; };    // for gimbal lock numerical problems    
};

typedef Math<float>  Mathf;
typedef Math<double> Mathd;

// Conversion functions between degrees and radians
// (non-templated to ensure passing int arguments causes warning)
inline float  RadToDegree(float rad)         { return rad * MATH_FLOAT_RADTODEGREEFACTOR; }
inline double RadToDegree(double rad)        { return rad * MATH_DOUBLE_RADTODEGREEFACTOR; }

inline float  DegreeToRad(float deg)         { return deg * MATH_FLOAT_DEGREETORADFACTOR; }
inline double DegreeToRad(double deg)        { return deg * MATH_DOUBLE_DEGREETORADFACTOR; }

// Square function
template<class T>
inline T Sqr(T x) { return x*x; }

// Sign: returns 0 if x == 0, -1 if x < 0, and 1 if x > 0
template<class T>
inline T Sign(T x) { return (x != T(0)) ? (x < T(0) ? T(-1) : T(1)) : T(0); }

// Numerically stable acos function
inline float Acos(float x)   { return (x > 1.0f) ? 0.0f : (x < -1.0f) ? MATH_FLOAT_PI : acosf(x); }
inline double Acos(double x) { return (x > 1.0) ? 0.0 : (x < -1.0) ? MATH_DOUBLE_PI : acos(x); }

// Numerically stable asin function
inline float Asin(float x)   { return (x > 1.0f) ? MATH_FLOAT_PIOVER2 : (x < -1.0f) ? -MATH_FLOAT_PIOVER2 : asinf(x); }
inline double Asin(double x) { return (x > 1.0) ? MATH_DOUBLE_PIOVER2 : (x < -1.0) ? -MATH_DOUBLE_PIOVER2 : asin(x); }

#if defined(_MSC_VER)
    inline int isnan(double x) { return ::_isnan(x); }
#elif !defined(isnan) // Some libraries #define isnan.
    inline int isnan(double x) { return ::isnan(x); }
#endif

template<class T>
class Quat;


//-------------------------------------------------------------------------------------
// ***** Vector2<>

// Vector2f (Vector2d) represents a 2-dimensional vector or point in space,
// consisting of coordinates x and y

template<class T>
class Vector2
{
public:
    typedef T ElementType;
    static const size_t ElementCount = 2;

    T x, y;

    Vector2() : x(0), y(0) { }
    Vector2(T x_, T y_) : x(x_), y(y_) { }
    explicit Vector2(T s) : x(s), y(s) { }
    explicit Vector2(const Vector2<typename Math<T>::OtherFloatType> &src)
        : x((T)src.x), y((T)src.y) { }

    static Vector2 Zero() { return Vector2(0, 0); }

    // C-interop support.
    typedef  typename CompatibleTypes<Vector2<T> >::Type CompatibleType;

    Vector2(const CompatibleType& s) : x(s.x), y(s.y) {  }

    operator const CompatibleType& () const
    {
        OVR_MATH_STATIC_ASSERT(sizeof(Vector2<T>) == sizeof(CompatibleType), "sizeof(Vector2<T>) failure");
        return reinterpret_cast<const CompatibleType&>(*this);
    }

        
    bool     operator== (const Vector2& b) const  { return x == b.x && y == b.y; }
    bool     operator!= (const Vector2& b) const  { return x != b.x || y != b.y; }
             
    Vector2  operator+  (const Vector2& b) const  { return Vector2(x + b.x, y + b.y); }
    Vector2& operator+= (const Vector2& b)        { x += b.x; y += b.y; return *this; }
    Vector2  operator-  (const Vector2& b) const  { return Vector2(x - b.x, y - b.y); }
    Vector2& operator-= (const Vector2& b)        { x -= b.x; y -= b.y; return *this; }
    Vector2  operator- () const                   { return Vector2(-x, -y); }

    // Scalar multiplication/division scales vector.
    Vector2  operator*  (T s) const               { return Vector2(x*s, y*s); }
    Vector2& operator*= (T s)                     { x *= s; y *= s; return *this; }

    Vector2  operator/  (T s) const               { T rcp = T(1)/s;
                                                    return Vector2(x*rcp, y*rcp); }
    Vector2& operator/= (T s)                     { T rcp = T(1)/s;
                                                    x *= rcp; y *= rcp;
                                                    return *this; }

    static Vector2  Min(const Vector2& a, const Vector2& b) { return Vector2((a.x < b.x) ? a.x : b.x,
                                                                             (a.y < b.y) ? a.y : b.y); }
    static Vector2  Max(const Vector2& a, const Vector2& b) { return Vector2((a.x > b.x) ? a.x : b.x,
                                                                             (a.y > b.y) ? a.y : b.y); }

    Vector2 Clamped(T maxMag) const
    {
        T magSquared = LengthSq();
        if (magSquared <= Sqr(maxMag))
            return *this;
        else
            return *this * (maxMag / sqrt(magSquared));
    }

    // Compare two vectors for equality with tolerance. Returns true if vectors match withing tolerance.
    bool IsEqual(const Vector2& b, T tolerance =Math<T>::Tolerance()) const
    {
        return (fabs(b.x-x) <= tolerance) &&
               (fabs(b.y-y) <= tolerance);
    }
    bool Compare(const Vector2& b, T tolerance = Math<T>::Tolerance()) const 
    {
        return IsEqual(b, tolerance);
    }

    // Access element by index
    T& operator[] (int idx)
    {
        OVR_MATH_ASSERT(0 <= idx && idx < 2);
        return *(&x + idx);
    }
    const T& operator[] (int idx) const
    {
        OVR_MATH_ASSERT(0 <= idx && idx < 2);
        return *(&x + idx);
    }

    // Entry-wise product of two vectors
    Vector2    EntrywiseMultiply(const Vector2& b) const    { return Vector2(x * b.x, y * b.y);}


    // Multiply and divide operators do entry-wise math. Used Dot() for dot product.
    Vector2  operator*  (const Vector2& b) const        { return Vector2(x * b.x,  y * b.y); }
    Vector2  operator/  (const Vector2& b) const        { return Vector2(x / b.x,  y / b.y); }

    // Dot product
    // Used to calculate angle q between two vectors among other things,
    // as (A dot B) = |a||b|cos(q).
    T        Dot(const Vector2& b) const                 { return x*b.x + y*b.y; }

    // Returns the angle from this vector to b, in radians.
    T       Angle(const Vector2& b) const        
    { 
        T div = LengthSq()*b.LengthSq();
        OVR_MATH_ASSERT(div != T(0));
        T result = Acos((this->Dot(b))/sqrt(div));
        return result;
    }

    // Return Length of the vector squared.
    T       LengthSq() const                     { return (x * x + y * y); }

    // Return vector length.
    T       Length() const                       { return sqrt(LengthSq()); }

    // Returns squared distance between two points represented by vectors.
    T       DistanceSq(const Vector2& b) const   { return (*this - b).LengthSq(); }

    // Returns distance between two points represented by vectors.
    T       Distance(const Vector2& b) const     { return (*this - b).Length(); }

    // Determine if this a unit vector.
    bool    IsNormalized() const                 { return fabs(LengthSq() - T(1)) < Math<T>::Tolerance(); }

    // Normalize, convention vector length to 1.    
    void    Normalize()                          
    {
        T s = Length();
        if (s != T(0))
            s = T(1) / s;
        *this *= s;
    }

    // Returns normalized (unit) version of the vector without modifying itself.
    Vector2 Normalized() const                   
    { 
        T s = Length();
        if (s != T(0))
            s = T(1) / s;
        return *this * s; 
    }

    // Linearly interpolates from this vector to another.
    // Factor should be between 0.0 and 1.0, with 0 giving full value to this.
    Vector2 Lerp(const Vector2& b, T f) const    { return *this*(T(1) - f) + b*f; }

    // Projects this vector onto the argument; in other words,
    // A.Project(B) returns projection of vector A onto B.
    Vector2 ProjectTo(const Vector2& b) const    
    { 
        T l2 = b.LengthSq();
        OVR_MATH_ASSERT(l2 != T(0));
        return b * ( Dot(b) / l2 ); 
    }
    
    // returns true if vector b is clockwise from this vector
    bool IsClockwise(const Vector2& b) const
    {
        return (x * b.y - y * b.x) < 0;
    }
};


typedef Vector2<float>  Vector2f;
typedef Vector2<double> Vector2d;
typedef Vector2<int>    Vector2i;

typedef Vector2<float>  Point2f;
typedef Vector2<double> Point2d;
typedef Vector2<int>    Point2i;

//-------------------------------------------------------------------------------------
// ***** Vector3<> - 3D vector of {x, y, z}

//
// Vector3f (Vector3d) represents a 3-dimensional vector or point in space,
// consisting of coordinates x, y and z.

template<class T>
class Vector3
{
public:
    typedef T ElementType;
    static const size_t ElementCount = 3;

    T x, y, z;

    // FIXME: default initialization of a vector class can be very expensive in a full-blown
    // application.  A few hundred thousand vector constructions is not unlikely and can add
    // up to milliseconds of time on processors like the PS3 PPU.
    Vector3() : x(0), y(0), z(0) { }
    Vector3(T x_, T y_, T z_ = 0) : x(x_), y(y_), z(z_) { }
    explicit Vector3(T s) : x(s), y(s), z(s) { }
    explicit Vector3(const Vector3<typename Math<T>::OtherFloatType> &src)
        : x((T)src.x), y((T)src.y), z((T)src.z) { }

    static Vector3 Zero() { return Vector3(0, 0, 0); }

    // C-interop support.
    typedef  typename CompatibleTypes<Vector3<T> >::Type CompatibleType;

    Vector3(const CompatibleType& s) : x(s.x), y(s.y), z(s.z) {  }

    operator const CompatibleType& () const
    {
        OVR_MATH_STATIC_ASSERT(sizeof(Vector3<T>) == sizeof(CompatibleType), "sizeof(Vector3<T>) failure");
        return reinterpret_cast<const CompatibleType&>(*this);
    }

    bool     operator== (const Vector3& b) const  { return x == b.x && y == b.y && z == b.z; }
    bool     operator!= (const Vector3& b) const  { return x != b.x || y != b.y || z != b.z; }
             
    Vector3  operator+  (const Vector3& b) const  { return Vector3(x + b.x, y + b.y, z + b.z); }
    Vector3& operator+= (const Vector3& b)        { x += b.x; y += b.y; z += b.z; return *this; }
    Vector3  operator-  (const Vector3& b) const  { return Vector3(x - b.x, y - b.y, z - b.z); }
    Vector3& operator-= (const Vector3& b)        { x -= b.x; y -= b.y; z -= b.z; return *this; }
    Vector3  operator- () const                   { return Vector3(-x, -y, -z); }

    // Scalar multiplication/division scales vector.
    Vector3  operator*  (T s) const               { return Vector3(x*s, y*s, z*s); }
    Vector3& operator*= (T s)                     { x *= s; y *= s; z *= s; return *this; }

    Vector3  operator/  (T s) const               { T rcp = T(1)/s;
                                                    return Vector3(x*rcp, y*rcp, z*rcp); }
    Vector3& operator/= (T s)                     { T rcp = T(1)/s;
                                                    x *= rcp; y *= rcp; z *= rcp;
                                                    return *this; }

    static Vector3  Min(const Vector3& a, const Vector3& b)
    {
        return Vector3((a.x < b.x) ? a.x : b.x,
                       (a.y < b.y) ? a.y : b.y,
                       (a.z < b.z) ? a.z : b.z);
    }
    static Vector3  Max(const Vector3& a, const Vector3& b)
    { 
        return Vector3((a.x > b.x) ? a.x : b.x,
                       (a.y > b.y) ? a.y : b.y,
                       (a.z > b.z) ? a.z : b.z);
    }        

    Vector3 Clamped(T maxMag) const
    {
        T magSquared = LengthSq();
        if (magSquared <= Sqr(maxMag))
            return *this;
        else
            return *this * (maxMag / sqrt(magSquared));
    }

    // Compare two vectors for equality with tolerance. Returns true if vectors match withing tolerance.
    bool IsEqual(const Vector3& b, T tolerance = Math<T>::Tolerance()) const
    {
        return (fabs(b.x-x) <= tolerance) && 
               (fabs(b.y-y) <= tolerance) && 
               (fabs(b.z-z) <= tolerance);
    }
    bool Compare(const Vector3& b, T tolerance = Math<T>::Tolerance()) const
    {
        return IsEqual(b, tolerance);
    }

    T& operator[] (int idx)
    {
        OVR_MATH_ASSERT(0 <= idx && idx < 3);
        return *(&x + idx);
    }

    const T& operator[] (int idx) const
    {
        OVR_MATH_ASSERT(0 <= idx && idx < 3);
        return *(&x + idx);
    }

    // Entrywise product of two vectors
    Vector3    EntrywiseMultiply(const Vector3& b) const    { return Vector3(x * b.x, 
                                                                         y * b.y, 
                                                                         z * b.z);}

    // Multiply and divide operators do entry-wise math
    Vector3  operator*  (const Vector3& b) const        { return Vector3(x * b.x, 
                                                                         y * b.y, 
                                                                         z * b.z); }

    Vector3  operator/  (const Vector3& b) const        { return Vector3(x / b.x, 
                                                                         y / b.y, 
                                                                         z / b.z); }


    // Dot product
    // Used to calculate angle q between two vectors among other things,
    // as (A dot B) = |a||b|cos(q).
     T      Dot(const Vector3& b) const          { return x*b.x + y*b.y + z*b.z; }

    // Compute cross product, which generates a normal vector.
    // Direction vector can be determined by right-hand rule: Pointing index finder in
    // direction a and middle finger in direction b, thumb will point in a.Cross(b).
    Vector3 Cross(const Vector3& b) const        { return Vector3(y*b.z - z*b.y,
                                                                  z*b.x - x*b.z,
                                                                  x*b.y - y*b.x); }

    // Returns the angle from this vector to b, in radians.
    T       Angle(const Vector3& b) const 
    {
        T div = LengthSq()*b.LengthSq();
        OVR_MATH_ASSERT(div != T(0));
        T result = Acos((this->Dot(b))/sqrt(div));
        return result;
    }

    // Return Length of the vector squared.
    T       LengthSq() const                     { return (x * x + y * y + z * z); }

    // Return vector length.
    T       Length() const                       { return (T)sqrt(LengthSq()); }

    // Returns squared distance between two points represented by vectors.
    T       DistanceSq(Vector3 const& b) const         { return (*this - b).LengthSq(); }

    // Returns distance between two points represented by vectors.
    T       Distance(Vector3 const& b) const     { return (*this - b).Length(); }
    
    bool    IsNormalized() const                 { return fabs(LengthSq() - T(1)) < Math<T>::Tolerance(); }

    // Normalize, convention vector length to 1.    
    void    Normalize()                          
    {
        T s = Length();
        if (s != T(0))
            s = T(1) / s;
        *this *= s;
    }

    // Returns normalized (unit) version of the vector without modifying itself.
    Vector3 Normalized() const                   
    { 
        T s = Length();
        if (s != T(0))
            s = T(1) / s;
        return *this * s;
    }

    // Linearly interpolates from this vector to another.
    // Factor should be between 0.0 and 1.0, with 0 giving full value to this.
    Vector3 Lerp(const Vector3& b, T f) const    { return *this*(T(1) - f) + b*f; }

    // Projects this vector onto the argument; in other words,
    // A.Project(B) returns projection of vector A onto B.
    Vector3 ProjectTo(const Vector3& b) const    
    { 
        T l2 = b.LengthSq();
        OVR_MATH_ASSERT(l2 != T(0));
        return b * ( Dot(b) / l2 ); 
    }

    // Projects this vector onto a plane defined by a normal vector
    Vector3 ProjectToPlane(const Vector3& normal) const { return *this - this->ProjectTo(normal); }
};

typedef Vector3<float>  Vector3f;
typedef Vector3<double> Vector3d;
typedef Vector3<int32_t>  Vector3i;
    
OVR_MATH_STATIC_ASSERT((sizeof(Vector3f) == 3*sizeof(float)), "sizeof(Vector3f) failure");
OVR_MATH_STATIC_ASSERT((sizeof(Vector3d) == 3*sizeof(double)), "sizeof(Vector3d) failure");
OVR_MATH_STATIC_ASSERT((sizeof(Vector3i) == 3*sizeof(int32_t)), "sizeof(Vector3i) failure");

typedef Vector3<float>   Point3f;
typedef Vector3<double>  Point3d;
typedef Vector3<int32_t>  Point3i;


//-------------------------------------------------------------------------------------
// ***** Vector4<> - 4D vector of {x, y, z, w}

//
// Vector4f (Vector4d) represents a 3-dimensional vector or point in space,
// consisting of coordinates x, y, z and w.

template<class T>
class Vector4
{
public:
    typedef T ElementType;
    static const size_t ElementCount = 4;

    T x, y, z, w;

    // FIXME: default initialization of a vector class can be very expensive in a full-blown
    // application.  A few hundred thousand vector constructions is not unlikely and can add
    // up to milliseconds of time on processors like the PS3 PPU.
    Vector4() : x(0), y(0), z(0), w(0) { }
    Vector4(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) { }
    explicit Vector4(T s) : x(s), y(s), z(s), w(s) { }
    explicit Vector4(const Vector3<T>& v, const T w_=T(1)) : x(v.x), y(v.y), z(v.z), w(w_) { }
    explicit Vector4(const Vector4<typename Math<T>::OtherFloatType> &src)
        : x((T)src.x), y((T)src.y), z((T)src.z), w((T)src.w) { }

    static Vector4 Zero() { return Vector4(0, 0, 0, 0); }

    // C-interop support.
    typedef  typename CompatibleTypes< Vector4<T> >::Type CompatibleType;

    Vector4(const CompatibleType& s) : x(s.x), y(s.y), z(s.z), w(s.w) {  }

    operator const CompatibleType& () const
    {
        OVR_MATH_STATIC_ASSERT(sizeof(Vector4<T>) == sizeof(CompatibleType), "sizeof(Vector4<T>) failure");
        return reinterpret_cast<const CompatibleType&>(*this);
    }

    Vector4& operator= (const Vector3<T>& other)  { x=other.x; y=other.y; z=other.z; w=1; return *this; }
    bool     operator== (const Vector4& b) const  { return x == b.x && y == b.y && z == b.z && w == b.w; }
    bool     operator!= (const Vector4& b) const  { return x != b.x || y != b.y || z != b.z || w != b.w; }
             
    Vector4  operator+  (const Vector4& b) const  { return Vector4(x + b.x, y + b.y, z + b.z, w + b.w); }
    Vector4& operator+= (const Vector4& b)        { x += b.x; y += b.y; z += b.z; w += b.w; return *this; }
    Vector4  operator-  (const Vector4& b) const  { return Vector4(x - b.x, y - b.y, z - b.z, w - b.w); }
    Vector4& operator-= (const Vector4& b)        { x -= b.x; y -= b.y; z -= b.z; w -= b.w; return *this; }
    Vector4  operator- () const                   { return Vector4(-x, -y, -z, -w); }

    // Scalar multiplication/division scales vector.
    Vector4  operator*  (T s) const               { return Vector4(x*s, y*s, z*s, w*s); }
    Vector4& operator*= (T s)                     { x *= s; y *= s; z *= s; w *= s;return *this; }

    Vector4  operator/  (T s) const               { T rcp = T(1)/s;
                                                    return Vector4(x*rcp, y*rcp, z*rcp, w*rcp); }
    Vector4& operator/= (T s)                     { T rcp = T(1)/s;
                                                    x *= rcp; y *= rcp; z *= rcp; w *= rcp;
                                                    return *this; }

    static Vector4  Min(const Vector4& a, const Vector4& b)
    {
        return Vector4((a.x < b.x) ? a.x : b.x,
                       (a.y < b.y) ? a.y : b.y,
                       (a.z < b.z) ? a.z : b.z,
                       (a.w < b.w) ? a.w : b.w);
    }
    static Vector4  Max(const Vector4& a, const Vector4& b)
    { 
        return Vector4((a.x > b.x) ? a.x : b.x,
                       (a.y > b.y) ? a.y : b.y,
                       (a.z > b.z) ? a.z : b.z,
                       (a.w > b.w) ? a.w : b.w);
    }        

    Vector4 Clamped(T maxMag) const
    {
        T magSquared = LengthSq();
        if (magSquared <= Sqr(maxMag))
            return *this;
        else
            return *this * (maxMag / sqrt(magSquared));
    }

    // Compare two vectors for equality with tolerance. Returns true if vectors match withing tolerance.
    bool IsEqual(const Vector4& b, T tolerance = Math<T>::Tolerance()) const
    {
        return (fabs(b.x-x) <= tolerance) && 
               (fabs(b.y-y) <= tolerance) && 
               (fabs(b.z-z) <= tolerance) &&
               (fabs(b.w-w) <= tolerance);
    }
    bool Compare(const Vector4& b, T tolerance = Math<T>::Tolerance()) const
    {
        return IsEqual(b, tolerance);
    }
    
    T& operator[] (int idx)
    {
        OVR_MATH_ASSERT(0 <= idx && idx < 4);
        return *(&x + idx);
    }

    const T& operator[] (int idx) const
    {
        OVR_MATH_ASSERT(0 <= idx && idx < 4);
        return *(&x + idx);
    }

    // Entry wise product of two vectors
    Vector4    EntrywiseMultiply(const Vector4& b) const    { return Vector4(x * b.x, 
                                                                         y * b.y, 
                                                                         z * b.z,
                                                                         w * b.w);}

    // Multiply and divide operators do entry-wise math
    Vector4  operator*  (const Vector4& b) const        { return Vector4(x * b.x, 
                                                                         y * b.y, 
                                                                         z * b.z,
                                                                         w * b.w); }

    Vector4  operator/  (const Vector4& b) const        { return Vector4(x / b.x, 
                                                                         y / b.y, 
                                                                         z / b.z,
                                                                         w / b.w); }


    // Dot product
    T       Dot(const Vector4& b) const          { return x*b.x + y*b.y + z*b.z + w*b.w; }

    // Return Length of the vector squared.
    T       LengthSq() const                     { return (x * x + y * y + z * z + w * w); }

    // Return vector length.
    T       Length() const                       { return sqrt(LengthSq()); }
    
    bool    IsNormalized() const                 { return fabs(LengthSq() - T(1)) < Math<T>::Tolerance(); }

    // Normalize, convention vector length to 1.    
    void    Normalize()                          
    {
        T s = Length();
        if (s != T(0))
            s = T(1) / s;
        *this *= s;
    }

    // Returns normalized (unit) version of the vector without modifying itself.
    Vector4 Normalized() const                   
    { 
        T s = Length();
        if (s != T(0))
            s = T(1) / s;
        return *this * s;
    }

    // Linearly interpolates from this vector to another.
    // Factor should be between 0.0 and 1.0, with 0 giving full value to this.
    Vector4 Lerp(const Vector4& b, T f) const    { return *this*(T(1) - f) + b*f; }
};

typedef Vector4<float>  Vector4f;
typedef Vector4<double> Vector4d;
typedef Vector4<int>    Vector4i;


//-------------------------------------------------------------------------------------
// ***** Bounds3

// Bounds class used to describe a 3D axis aligned bounding box.

template<class T>
class Bounds3
{
public:
    Vector3<T>    b[2];

    Bounds3()
    {
    }

    Bounds3( const Vector3<T> & mins, const Vector3<T> & maxs )
{
        b[0] = mins;
        b[1] = maxs;
    }

    void Clear()
    {
        b[0].x = b[0].y = b[0].z = Math<T>::MaxValue;
        b[1].x = b[1].y = b[1].z = -Math<T>::MaxValue;
    }

    void AddPoint( const Vector3<T> & v )
    {
        b[0].x = (b[0].x < v.x ? b[0].x : v.x);
        b[0].y = (b[0].y < v.y ? b[0].y : v.y);
        b[0].z = (b[0].z < v.z ? b[0].z : v.z);
        b[1].x = (v.x < b[1].x ? b[1].x : v.x);
        b[1].y = (v.y < b[1].y ? b[1].y : v.y);
        b[1].z = (v.z < b[1].z ? b[1].z : v.z);
    }

    const Vector3<T> & GetMins() const { return b[0]; }
    const Vector3<T> & GetMaxs() const { return b[1]; }

    Vector3<T> & GetMins() { return b[0]; }
    Vector3<T> & GetMaxs() { return b[1]; }
};

typedef Bounds3<float>    Bounds3f;
typedef Bounds3<double>    Bounds3d;


//-------------------------------------------------------------------------------------
// ***** Size

// Size class represents 2D size with Width, Height components.
// Used to describe distentions of render targets, etc.

template<class T>
class Size
{
public:
    T   w, h;

    Size()              : w(0), h(0)   { }
    Size(T w_, T h_)    : w(w_), h(h_) { }
    explicit Size(T s)  : w(s), h(s)   { }
    explicit Size(const Size<typename Math<T>::OtherFloatType> &src)
        : w((T)src.w), h((T)src.h) { }

    // C-interop support.
    typedef  typename CompatibleTypes<Size<T> >::Type CompatibleType;

    Size(const CompatibleType& s) : w(s.w), h(s.h) {  }

    operator const CompatibleType& () const
    {
        OVR_MATH_STATIC_ASSERT(sizeof(Size<T>) == sizeof(CompatibleType), "sizeof(Size<T>) failure");
        return reinterpret_cast<const CompatibleType&>(*this);
    }

    bool     operator== (const Size& b) const  { return w == b.w && h == b.h; }
    bool     operator!= (const Size& b) const  { return w != b.w || h != b.h; }
             
    Size  operator+  (const Size& b) const  { return Size(w + b.w, h + b.h); }
    Size& operator+= (const Size& b)        { w += b.w; h += b.h; return *this; }
    Size  operator-  (const Size& b) const  { return Size(w - b.w, h - b.h); }
    Size& operator-= (const Size& b)        { w -= b.w; h -= b.h; return *this; }
    Size  operator- () const                { return Size(-w, -h); }
    Size  operator*  (const Size& b) const  { return Size(w * b.w, h * b.h); }
    Size& operator*= (const Size& b)        { w *= b.w; h *= b.h; return *this; }
    Size  operator/  (const Size& b) const  { return Size(w / b.w, h / b.h); }
    Size& operator/= (const Size& b)        { w /= b.w; h /= b.h; return *this; }

    // Scalar multiplication/division scales both components.
    Size  operator*  (T s) const            { return Size(w*s, h*s); }
    Size& operator*= (T s)                  { w *= s; h *= s; return *this; }    
    Size  operator/  (T s) const            { return Size(w/s, h/s); }
    Size& operator/= (T s)                  { w /= s; h /= s; return *this; }

    static Size Min(const Size& a, const Size& b)  { return Size((a.w  < b.w)  ? a.w  : b.w,
                                                                 (a.h < b.h) ? a.h : b.h); }
    static Size Max(const Size& a, const Size& b)  { return Size((a.w  > b.w)  ? a.w  : b.w,
                                                                 (a.h > b.h) ? a.h : b.h); }
    
    T       Area() const                    { return w * h; }

    inline  Vector2<T> ToVector() const     { return Vector2<T>(w, h); }
};


typedef Size<int>       Sizei;
typedef Size<unsigned>  Sizeu;
typedef Size<float>     Sizef;
typedef Size<double>    Sized;



//-----------------------------------------------------------------------------------
// ***** Rect

// Rect describes a rectangular area for rendering, that includes position and size.
template<class T>
class Rect
{
public:
    T x, y;
    T w, h;

    Rect() { }
    Rect(T x1, T y1, T w1, T h1)                   : x(x1), y(y1), w(w1), h(h1) { }    
    Rect(const Vector2<T>& pos, const Size<T>& sz) : x(pos.x), y(pos.y), w(sz.w), h(sz.h) { }
    Rect(const Size<T>& sz)                        : x(0), y(0), w(sz.w), h(sz.h) { }
    
    // C-interop support.
    typedef  typename CompatibleTypes<Rect<T> >::Type CompatibleType;

    Rect(const CompatibleType& s) : x(s.Pos.x), y(s.Pos.y), w(s.Size.w), h(s.Size.h) {  }

    operator const CompatibleType& () const
    {
        OVR_MATH_STATIC_ASSERT(sizeof(Rect<T>) == sizeof(CompatibleType), "sizeof(Rect<T>) failure");
        return reinterpret_cast<const CompatibleType&>(*this);
    }

    Vector2<T> GetPos() const                { return Vector2<T>(x, y); }
    Size<T>    GetSize() const               { return Size<T>(w, h); }
    void       SetPos(const Vector2<T>& pos) { x = pos.x; y = pos.y; }
    void       SetSize(const Size<T>& sz)    { w = sz.w; h = sz.h; }

    bool operator == (const Rect& vp) const
    { return (x == vp.x) && (y == vp.y) && (w == vp.w) && (h == vp.h); }
    bool operator != (const Rect& vp) const
    { return !operator == (vp); }
};

typedef Rect<int> Recti;


//-------------------------------------------------------------------------------------//
// ***** Quat
//
// Quatf represents a quaternion class used for rotations.
// 
// Quaternion multiplications are done in right-to-left order, to match the
// behavior of matrices.


template<class T>
class Quat
{
public:
    typedef T ElementType;
    static const size_t ElementCount = 4;

    // x,y,z = axis*sin(angle), w = cos(angle)
    T x, y, z, w;    

    Quat() : x(0), y(0), z(0), w(1) { }
    Quat(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) { }
    explicit Quat(const Quat<typename Math<T>::OtherFloatType> &src)
        : x((T)src.x), y((T)src.y), z((T)src.z), w((T)src.w)
    {
        // NOTE: Converting a normalized Quat<float> to Quat<double>
        // will generally result in an un-normalized quaternion.
        // But we don't normalize here in case the quaternion
        // being converted is not a normalized rotation quaternion.
    }

    typedef  typename CompatibleTypes<Quat<T> >::Type CompatibleType;

    // C-interop support.
    Quat(const CompatibleType& s) : x(s.x), y(s.y), z(s.z), w(s.w) { }

    operator CompatibleType () const
    {
        CompatibleType result;
        result.x = x;
        result.y = y;
        result.z = z;
        result.w = w;
        return result;
    }

    // Constructs quaternion for rotation around the axis by an angle.
    Quat(const Vector3<T>& axis, T angle)
    {
        // Make sure we don't divide by zero. 
        if (axis.LengthSq() == T(0))
        {
            // Assert if the axis is zero, but the angle isn't
            OVR_MATH_ASSERT(angle == T(0));
            x = y = z = T(0); w = T(1);
            return;
        }

        Vector3<T> unitAxis = axis.Normalized();
        T          sinHalfAngle = sin(angle * T(0.5));

        w = cos(angle * T(0.5));
        x = unitAxis.x * sinHalfAngle;
        y = unitAxis.y * sinHalfAngle;
        z = unitAxis.z * sinHalfAngle;
    }

    // Constructs quaternion for rotation around one of the coordinate axis by an angle.
    Quat(Axis A, T angle, RotateDirection d = Rotate_CCW, HandedSystem s = Handed_R)
    {
        T sinHalfAngle = s * d *sin(angle * T(0.5));
        T v[3];
        v[0] = v[1] = v[2] = T(0);
        v[A] = sinHalfAngle;

        w = cos(angle * T(0.5));
        x = v[0];
        y = v[1];
        z = v[2];
    }

    Quat operator-() { return Quat(-x, -y, -z, -w); }   // unary minus

    static Quat Identity() { return Quat(0, 0, 0, 1); }

    // Compute axis and angle from quaternion
    void GetAxisAngle(Vector3<T>* axis, T* angle) const
    {
        if ( x*x + y*y + z*z > Math<T>::Tolerance() * Math<T>::Tolerance() ) {
            *axis  = Vector3<T>(x, y, z).Normalized();
            *angle = 2 * Acos(w);
            if (*angle > ((T)MATH_DOUBLE_PI)) // Reduce the magnitude of the angle, if necessary
            {
                *angle = ((T)MATH_DOUBLE_TWOPI) - *angle;
                *axis = *axis * (-1);
            }
        }
        else 
        {
            *axis = Vector3<T>(1, 0, 0);
            *angle= T(0);
        }
    }

    // Convert a quaternion to a rotation vector, also known as
    // Rodrigues vector, AxisAngle vector, SORA vector, exponential map.
    // A rotation vector describes a rotation about an axis:
    // the axis of rotation is the vector normalized,
    // the angle of rotation is the magnitude of the vector.
    Vector3<T> ToRotationVector() const
    {
        OVR_MATH_ASSERT(IsNormalized() || LengthSq() == 0);
        T s = T(0);
        T sinHalfAngle = sqrt(x*x + y*y + z*z);
        if (sinHalfAngle > T(0))
        {
            T cosHalfAngle = w;
            T halfAngle = atan2(sinHalfAngle, cosHalfAngle);

            // Ensure minimum rotation magnitude
            if (cosHalfAngle < 0)
                halfAngle -= T(MATH_DOUBLE_PI);

            s = T(2) * halfAngle / sinHalfAngle;
        }
        return Vector3<T>(x*s, y*s, z*s);
    }

    // Faster version of the above, optimized for use with small rotations, where rotation angle ~= sin(angle)
    inline OVR::Vector3<T> FastToRotationVector() const
    {
        OVR_MATH_ASSERT(IsNormalized());
        T s;
        T sinHalfSquared = x*x + y*y + z*z;
        if (sinHalfSquared < T(.0037)) // =~ sin(7/2 degrees)^2
        {
            // Max rotation magnitude error is about .062% at 7 degrees rotation, or about .0043 degrees
            s = T(2) * Sign(w);
        }
        else
        {
            T sinHalfAngle = sqrt(sinHalfSquared);
            T cosHalfAngle = w;
            T halfAngle = atan2(sinHalfAngle, cosHalfAngle);

            // Ensure minimum rotation magnitude
            if (cosHalfAngle < 0)
                halfAngle -= T(MATH_DOUBLE_PI);

            s = T(2) * halfAngle / sinHalfAngle;
        }
        return Vector3<T>(x*s, y*s, z*s);
    }

    // Given a rotation vector of form unitRotationAxis * angle,
    // returns the equivalent quaternion (unitRotationAxis * sin(angle), cos(Angle)).
    static Quat FromRotationVector(const Vector3<T>& v)
    {
        T angleSquared = v.LengthSq();
        T s = T(0);
        T c = T(1);
        if (angleSquared > T(0))
        {
            T angle = sqrt(angleSquared);
            s = sin(angle * T(0.5)) / angle;    // normalize
            c = cos(angle * T(0.5));
        }
        return Quat(s*v.x, s*v.y, s*v.z, c);
    }

    // Faster version of above, optimized for use with small rotation magnitudes, where rotation angle =~ sin(angle).
    // If normalize is false, small-angle quaternions are returned un-normalized.
    inline static Quat FastFromRotationVector(const OVR::Vector3<T>& v, bool normalize = true)
    {
        T s, c;
        T angleSquared = v.LengthSq();
        if (angleSquared < T(0.0076))   // =~ (5 degrees*pi/180)^2
        {
            s = T(0.5);
            c = T(1.0);
            // Max rotation magnitude error (after normalization) is about .064% at 5 degrees rotation, or .0032 degrees
            if (normalize && angleSquared > 0)
            {
                // sin(angle/2)^2 ~= (angle/2)^2 and cos(angle/2)^2 ~= 1
                T invLen = T(1) / sqrt(angleSquared * T(0.25) + T(1)); // normalize
                s = s * invLen;
                c = c * invLen;
            }
        }
        else
        {
            T angle = sqrt(angleSquared);
            s = sin(angle * T(0.5)) / angle;
            c = cos(angle * T(0.5));
        }
        return Quat(s*v.x, s*v.y, s*v.z, c);
    }

    // Constructs the quaternion from a rotation matrix
    explicit Quat(const Matrix4<T>& m)
    {
        T trace = m.M[0][0] + m.M[1][1] + m.M[2][2];

        // In almost all cases, the first part is executed.
        // However, if the trace is not positive, the other
        // cases arise.
        if (trace > T(0)) 
        {
            T s = sqrt(trace + T(1)) * T(2); // s=4*qw
            w = T(0.25) * s;
            x = (m.M[2][1] - m.M[1][2]) / s;
            y = (m.M[0][2] - m.M[2][0]) / s;
            z = (m.M[1][0] - m.M[0][1]) / s; 
        } 
        else if ((m.M[0][0] > m.M[1][1])&&(m.M[0][0] > m.M[2][2])) 
        {
            T s = sqrt(T(1) + m.M[0][0] - m.M[1][1] - m.M[2][2]) * T(2);
            w = (m.M[2][1] - m.M[1][2]) / s;
            x = T(0.25) * s;
            y = (m.M[0][1] + m.M[1][0]) / s;
            z = (m.M[2][0] + m.M[0][2]) / s;
        } 
        else if (m.M[1][1] > m.M[2][2]) 
        {
            T s = sqrt(T(1) + m.M[1][1] - m.M[0][0] - m.M[2][2]) * T(2); // S=4*qy
            w = (m.M[0][2] - m.M[2][0]) / s;
            x = (m.M[0][1] + m.M[1][0]) / s;
            y = T(0.25) * s;
            z = (m.M[1][2] + m.M[2][1]) / s;
        } 
        else 
        {
            T s = sqrt(T(1) + m.M[2][2] - m.M[0][0] - m.M[1][1]) * T(2); // S=4*qz
            w = (m.M[1][0] - m.M[0][1]) / s;
            x = (m.M[0][2] + m.M[2][0]) / s;
            y = (m.M[1][2] + m.M[2][1]) / s;
            z = T(0.25) * s;
        }
        OVR_MATH_ASSERT(IsNormalized());    // Ensure input matrix is orthogonal
    }

    // Constructs the quaternion from a rotation matrix
    explicit Quat(const Matrix3<T>& m)
    {
        T trace = m.M[0][0] + m.M[1][1] + m.M[2][2];

        // In almost all cases, the first part is executed.
        // However, if the trace is not positive, the other
        // cases arise.
        if (trace > T(0)) 
        {
            T s = sqrt(trace + T(1)) * T(2); // s=4*qw
            w = T(0.25) * s;
            x = (m.M[2][1] - m.M[1][2]) / s;
            y = (m.M[0][2] - m.M[2][0]) / s;
            z = (m.M[1][0] - m.M[0][1]) / s; 
        } 
        else if ((m.M[0][0] > m.M[1][1])&&(m.M[0][0] > m.M[2][2])) 
        {
            T s = sqrt(T(1) + m.M[0][0] - m.M[1][1] - m.M[2][2]) * T(2);
            w = (m.M[2][1] - m.M[1][2]) / s;
            x = T(0.25) * s;
            y = (m.M[0][1] + m.M[1][0]) / s;
            z = (m.M[2][0] + m.M[0][2]) / s;
        } 
        else if (m.M[1][1] > m.M[2][2]) 
        {
            T s = sqrt(T(1) + m.M[1][1] - m.M[0][0] - m.M[2][2]) * T(2); // S=4*qy
            w = (m.M[0][2] - m.M[2][0]) / s;
            x = (m.M[0][1] + m.M[1][0]) / s;
            y = T(0.25) * s;
            z = (m.M[1][2] + m.M[2][1]) / s;
        } 
        else 
        {
            T s = sqrt(T(1) + m.M[2][2] - m.M[0][0] - m.M[1][1]) * T(2); // S=4*qz
            w = (m.M[1][0] - m.M[0][1]) / s;
            x = (m.M[0][2] + m.M[2][0]) / s;
            y = (m.M[1][2] + m.M[2][1]) / s;
            z = T(0.25) * s;
        }
        OVR_MATH_ASSERT(IsNormalized());    // Ensure input matrix is orthogonal
    }

    bool operator== (const Quat& b) const   { return x == b.x && y == b.y && z == b.z && w == b.w; }
    bool operator!= (const Quat& b) const   { return x != b.x || y != b.y || z != b.z || w != b.w; }

    Quat  operator+  (const Quat& b) const  { return Quat(x + b.x, y + b.y, z + b.z, w + b.w); }
    Quat& operator+= (const Quat& b)        { w += b.w; x += b.x; y += b.y; z += b.z; return *this; }
    Quat  operator-  (const Quat& b) const  { return Quat(x - b.x, y - b.y, z - b.z, w - b.w); }
    Quat& operator-= (const Quat& b)        { w -= b.w; x -= b.x; y -= b.y; z -= b.z; return *this; }

    Quat  operator*  (T s) const            { return Quat(x * s, y * s, z * s, w * s); }
    Quat& operator*= (T s)                  { w *= s; x *= s; y *= s; z *= s; return *this; }
    Quat  operator/  (T s) const            { T rcp = T(1)/s; return Quat(x * rcp, y * rcp, z * rcp, w *rcp); }
    Quat& operator/= (T s)                  { T rcp = T(1)/s; w *= rcp; x *= rcp; y *= rcp; z *= rcp; return *this; }

    // Compare two quats for equality within tolerance. Returns true if quats match withing tolerance.
    bool IsEqual(const Quat& b, T tolerance = Math<T>::Tolerance()) const
    {
        return Abs(Dot(b)) >= T(1) - tolerance;
    }

    static T Abs(const T v)                 { return (v >= 0) ? v : -v; }

    // Get Imaginary part vector
    Vector3<T> Imag() const                 { return Vector3<T>(x,y,z); }

    // Get quaternion length.
    T       Length() const                  { return sqrt(LengthSq()); }

    // Get quaternion length squared.
    T       LengthSq() const                { return (x * x + y * y + z * z + w * w); }

    // Simple Euclidean distance in R^4 (not SLERP distance, but at least respects Haar measure)
    T       Distance(const Quat& q) const    
    { 
        T d1 = (*this - q).Length();
        T d2 = (*this + q).Length(); // Antipodal point check
        return (d1 < d2) ? d1 : d2;
    }

    T       DistanceSq(const Quat& q) const
    {
        T d1 = (*this - q).LengthSq();
        T d2 = (*this + q).LengthSq(); // Antipodal point check
        return (d1 < d2) ? d1 : d2;
    }

    T       Dot(const Quat& q) const
    {
        return x * q.x + y * q.y + z * q.z + w * q.w;
    }

    // Angle between two quaternions in radians
    T Angle(const Quat& q) const
    {
        return T(2) * Acos(Abs(Dot(q)));
    }

    // Angle of quaternion
    T Angle() const
    {
        return T(2) * Acos(Abs(w));
    }

    // Normalize
    bool    IsNormalized() const            { return fabs(LengthSq() - T(1)) < Math<T>::Tolerance(); }

    void    Normalize()
    {
        T s = Length();
        if (s != T(0))
            s = T(1) / s;
        *this *= s;
    }

    Quat    Normalized() const
    { 
        T s = Length();
        if (s != T(0))
            s = T(1) / s;
        return *this * s;
    }

    inline void EnsureSameHemisphere(const Quat& o)
    {
        if (Dot(o) < T(0))
        {
            x = -x;
            y = -y;
            z = -z;
            w = -w;
        }
    }

    // Returns conjugate of the quaternion. Produces inverse rotation if quaternion is normalized.
    Quat    Conj() const                    { return Quat(-x, -y, -z, w); }

    // Quaternion multiplication. Combines quaternion rotations, performing the one on the 
    // right hand side first.
    Quat  operator* (const Quat& b) const   { return Quat(w * b.x + x * b.w + y * b.z - z * b.y,
                                                          w * b.y - x * b.z + y * b.w + z * b.x,
                                                          w * b.z + x * b.y - y * b.x + z * b.w,
                                                          w * b.w - x * b.x - y * b.y - z * b.z); }
    const Quat& operator*= (const Quat& b)  { *this = *this * b;  return *this; }

    // 
    // this^p normalized; same as rotating by this p times.
    Quat PowNormalized(T p) const
    {
        Vector3<T> v;
        T          a;
        GetAxisAngle(&v, &a);
        return Quat(v, a * p);
    }

    // Compute quaternion that rotates v into alignTo: alignTo = Quat::Align(alignTo, v).Rotate(v).
    // NOTE: alignTo and v must be normalized.
    static Quat Align(const Vector3<T>& alignTo, const Vector3<T>& v)
    {
        OVR_MATH_ASSERT(alignTo.IsNormalized() && v.IsNormalized());
        Vector3<T> bisector = (v + alignTo);
        bisector.Normalize();
        T cosHalfAngle = v.Dot(bisector); // 0..1
        if (cosHalfAngle > T(0))
        {
            Vector3<T> imag = v.Cross(bisector);
            return Quat(imag.x, imag.y, imag.z, cosHalfAngle);
        }
        else
        {
            // cosHalfAngle == 0: a 180 degree rotation.
            // sinHalfAngle == 1, rotation axis is any axis perpendicular
            // to alignTo.  Choose axis to include largest magnitude components
            if (fabs(v.x) > fabs(v.y))
            {
                // x or z is max magnitude component
                // = Cross(v, (0,1,0)).Normalized();
                T invLen = sqrt(v.x*v.x + v.z*v.z);
                if (invLen > T(0))
                    invLen = T(1) / invLen;
                return Quat(-v.z*invLen, 0, v.x*invLen, 0);
            }
            else
            {
                // y or z is max magnitude component
                // = Cross(v, (1,0,0)).Normalized();
                T invLen = sqrt(v.y*v.y + v.z*v.z);
                if (invLen > T(0))
                    invLen = T(1) / invLen;
                return Quat(0, v.z*invLen, -v.y*invLen, 0);
            }
        }
    }

    // Decompose a quat into quat = swing * twist, where twist is a rotation about axis,
    // and swing is a rotation perpendicular to axis.
    Quat GetSwingTwist(const Vector3<T>& axis, Quat* twist) const
    {
        OVR_MATH_ASSERT(twist);
        OVR_MATH_ASSERT(axis.IsNormalized());

        // Create a normalized quaternion from projection of (x,y,z) onto axis
        T d = axis.Dot(Vector3<T>(x, y, z));
        *twist = Quat(axis.x*d, axis.y*d, axis.z*d, w);
        T len = twist->Length();
        if (len == 0)
            twist->w = T(1);    // identity
        else
            twist /= len;       // normalize

        return *this * twist.Inverted();
    }

    // Normalized linear interpolation of quaternions
    // NOTE: This function is a bad approximation of Slerp()
    // when the angle between the *this and b is large.
    // Use FastSlerp() or Slerp() instead.
    Quat Lerp(const Quat& b, T s) const
    {
        return (*this * (T(1) - s) + b * (Dot(b) < 0 ? -s : s)).Normalized();
    }

    // Spherical linear interpolation between rotations
    Quat Slerp(const Quat& b, T s) const
    {
        Vector3<T> delta = (b * this->Inverted()).ToRotationVector();
        return (FromRotationVector(delta * s) * *this).Normalized();    // normalize so errors don't accumulate
    }

    // Spherical linear interpolation: much faster for small rotations, accurate for large rotations. See FastTo/FromRotationVector
    Quat FastSlerp(const Quat& b, T s) const
    {
        Vector3<T> delta = (b * this->Inverted()).FastToRotationVector();
        return (FastFromRotationVector(delta * s, false) * *this).Normalized();
    }

    // Rotate transforms vector in a manner that matches Matrix rotations (counter-clockwise,
    // assuming negative direction of the axis). Standard formula: q(t) * V * q(t)^-1. 
    Vector3<T> Rotate(const Vector3<T>& v) const
    {
        OVR_MATH_ASSERT(isnan(w) || IsNormalized());

        // rv = q * (v,0) * q'
        // Same as rv = v + real * cross(imag,v)*2 + cross(imag, cross(imag,v)*2);

        // uv = 2 * Imag().Cross(v);
        T uvx = T(2) * (y*v.z - z*v.y);
        T uvy = T(2) * (z*v.x - x*v.z);
        T uvz = T(2) * (x*v.y - y*v.x);

        // return v + Real()*uv + Imag().Cross(uv);
        return Vector3<T>(v.x + w*uvx + y*uvz - z*uvy,
                          v.y + w*uvy + z*uvx - x*uvz,
                          v.z + w*uvz + x*uvy - y*uvx);
    }

    // Rotation by inverse of *this
    Vector3<T> InverseRotate(const Vector3<T>& v) const
    {
        OVR_MATH_ASSERT(IsNormalized());

        // rv = q' * (v,0) * q
        // Same as rv = v + real * cross(-imag,v)*2 + cross(-imag, cross(-imag,v)*2);
        //      or rv = v - real * cross(imag,v)*2 + cross(imag, cross(imag,v)*2);

        // uv = 2 * Imag().Cross(v);
        T uvx = T(2) * (y*v.z - z*v.y);
        T uvy = T(2) * (z*v.x - x*v.z);
        T uvz = T(2) * (x*v.y - y*v.x);

        // return v - Real()*uv + Imag().Cross(uv);
        return Vector3<T>(v.x - w*uvx + y*uvz - z*uvy,
                          v.y - w*uvy + z*uvx - x*uvz,
                          v.z - w*uvz + x*uvy - y*uvx);
    }
    
    // Inversed quaternion rotates in the opposite direction.
    Quat        Inverted() const
    {
        return Quat(-x, -y, -z, w);
    }

    Quat        Inverse() const
    {
        return Quat(-x, -y, -z, w);
    }

    // Sets this quaternion to the one rotates in the opposite direction.
    void        Invert()
    {
        *this = Quat(-x, -y, -z, w);
    }
    
    // Time integration of constant angular velocity over dt
    Quat TimeIntegrate(Vector3<T> angularVelocity, T dt) const
    {
        // solution is: this * exp( omega*dt/2 ); FromRotationVector(v) gives exp(v*.5).
        return (*this * FastFromRotationVector(angularVelocity * dt, false)).Normalized();
    }

    // Time integration of constant angular acceleration and velocity over dt
    // These are the first two terms of the "Magnus expansion" of the solution
    //
    //   o = o * exp( W=(W1 + W2 + W3+...) * 0.5 );
    //
    //  omega1 = (omega + omegaDot*dt)
    //  W1 = (omega + omega1)*dt/2              
    //  W2 = cross(omega, omega1)/12*dt^2 % (= -cross(omega_dot, omega)/12*dt^3)
    // Terms 3 and beyond are vanishingly small:
    //  W3 = cross(omega_dot, cross(omega_dot, omega))/240*dt^5 
    //
    Quat TimeIntegrate(Vector3<T> angularVelocity, Vector3<T> angularAcceleration, T dt) const
    {
        const Vector3<T>& omega = angularVelocity;
        const Vector3<T>& omegaDot = angularAcceleration;

        Vector3<T> omega1 = (omega + omegaDot * dt);
        Vector3<T> W = ( (omega + omega1) + omega.Cross(omega1) * (dt/T(6)) ) * (dt/T(2));

        // FromRotationVector(v) is exp(v*.5)
        return (*this * FastFromRotationVector(W, false)).Normalized();
    }

    // Decompose rotation into three rotations:
    // roll radians about Z axis, then pitch radians about X axis, then yaw radians about Y axis.
    // Call with nullptr if a return value is not needed.
    void GetYawPitchRoll(T* yaw, T* pitch, T* roll) const
    {
        return GetEulerAngles<Axis_Y, Axis_X, Axis_Z, Rotate_CCW, Handed_R>(yaw, pitch, roll);
    }

    // GetEulerAngles extracts Euler angles from the quaternion, in the specified order of
    // axis rotations and the specified coordinate system. Right-handed coordinate system
    // is the default, with CCW rotations while looking in the negative axis direction.
    // Here a,b,c, are the Yaw/Pitch/Roll angles to be returned.
    // Rotation order is c, b, a:
    // rotation c around axis A3
    // is followed by rotation b around axis A2
    // is followed by rotation a around axis A1
    // rotations are CCW or CW (D) in LH or RH coordinate system (S)
    // 
    template <Axis A1, Axis A2, Axis A3, RotateDirection D, HandedSystem S>
    void GetEulerAngles(T *a, T *b, T *c) const 
    {
        OVR_MATH_ASSERT(IsNormalized());
        OVR_MATH_STATIC_ASSERT((A1 != A2) && (A2 != A3) && (A1 != A3), "(A1 != A2) && (A2 != A3) && (A1 != A3)");

        T Q[3] = { x, y, z };  //Quaternion components x,y,z

        T ww  = w*w;
        T Q11 = Q[A1]*Q[A1];
        T Q22 = Q[A2]*Q[A2];
        T Q33 = Q[A3]*Q[A3];

        T psign = T(-1);
        // Determine whether even permutation
        if (((A1 + 1) % 3 == A2) && ((A2 + 1) % 3 == A3))
            psign = T(1);
        
        T s2 = psign * T(2) * (psign*w*Q[A2] + Q[A1]*Q[A3]);

        T singularityRadius = Math<T>::SingularityRadius();
        if (s2 < T(-1) + singularityRadius)
        { // South pole singularity
            if (a) *a = T(0);
            if (b) *b = -S*D*((T)MATH_DOUBLE_PIOVER2);
            if (c) *c = S*D*atan2(T(2)*(psign*Q[A1] * Q[A2] + w*Q[A3]), ww + Q22 - Q11 - Q33 );
        }
        else if (s2 > T(1) - singularityRadius)
        {  // North pole singularity
            if (a) *a = T(0);
            if (b) *b = S*D*((T)MATH_DOUBLE_PIOVER2);
            if (c) *c = S*D*atan2(T(2)*(psign*Q[A1] * Q[A2] + w*Q[A3]), ww + Q22 - Q11 - Q33);
        }
        else
        {
            if (a) *a = -S*D*atan2(T(-2)*(w*Q[A1] - psign*Q[A2] * Q[A3]), ww + Q33 - Q11 - Q22);
            if (b) *b = S*D*asin(s2);
            if (c) *c = S*D*atan2(T(2)*(w*Q[A3] - psign*Q[A1] * Q[A2]), ww + Q11 - Q22 - Q33);
        }      
    }

    template <Axis A1, Axis A2, Axis A3, RotateDirection D>
    void GetEulerAngles(T *a, T *b, T *c) const
    { GetEulerAngles<A1, A2, A3, D, Handed_R>(a, b, c); }

    template <Axis A1, Axis A2, Axis A3>
    void GetEulerAngles(T *a, T *b, T *c) const
    { GetEulerAngles<A1, A2, A3, Rotate_CCW, Handed_R>(a, b, c); }

    // GetEulerAnglesABA extracts Euler angles from the quaternion, in the specified order of
    // axis rotations and the specified coordinate system. Right-handed coordinate system
    // is the default, with CCW rotations while looking in the negative axis direction.
    // Here a,b,c, are the Yaw/Pitch/Roll angles to be returned.
    // rotation a around axis A1
    // is followed by rotation b around axis A2
    // is followed by rotation c around axis A1
    // Rotations are CCW or CW (D) in LH or RH coordinate system (S)
    template <Axis A1, Axis A2, RotateDirection D, HandedSystem S>
    void GetEulerAnglesABA(T *a, T *b, T *c) const
    {
        OVR_MATH_ASSERT(IsNormalized());
        OVR_MATH_STATIC_ASSERT(A1 != A2, "A1 != A2");

        T Q[3] = {x, y, z}; // Quaternion components

        // Determine the missing axis that was not supplied
        int m = 3 - A1 - A2;

        T ww = w*w;
        T Q11 = Q[A1]*Q[A1];
        T Q22 = Q[A2]*Q[A2];
        T Qmm = Q[m]*Q[m];

        T psign = T(-1);
        if ((A1 + 1) % 3 == A2) // Determine whether even permutation
        {
            psign = T(1);
        }

        T c2 = ww + Q11 - Q22 - Qmm;
        T singularityRadius = Math<T>::SingularityRadius();
        if (c2 < T(-1) + singularityRadius)
        { // South pole singularity
            if (a) *a = T(0);
            if (b) *b = S*D*((T)MATH_DOUBLE_PI);
            if (c) *c = S*D*atan2(T(2)*(w*Q[A1] - psign*Q[A2] * Q[m]),
                            ww + Q22 - Q11 - Qmm);
        }
        else if (c2 > T(1) - singularityRadius)
        {  // North pole singularity
            if (a) *a = T(0);
            if (b) *b = T(0);
            if (c) *c = S*D*atan2(T(2)*(w*Q[A1] - psign*Q[A2] * Q[m]),
                           ww + Q22 - Q11 - Qmm);
        }
        else
        {
            if (a) *a = S*D*atan2(psign*w*Q[m] + Q[A1] * Q[A2],
                           w*Q[A2] -psign*Q[A1]*Q[m]);
            if (b) *b = S*D*acos(c2);
            if (c) *c = S*D*atan2(-psign*w*Q[m] + Q[A1] * Q[A2],
                           w*Q[A2] + psign*Q[A1]*Q[m]);
        }
    }
};

typedef Quat<float>  Quatf;
typedef Quat<double> Quatd;

OVR_MATH_STATIC_ASSERT((sizeof(Quatf) == 4*sizeof(float)), "sizeof(Quatf) failure");
OVR_MATH_STATIC_ASSERT((sizeof(Quatd) == 4*sizeof(double)), "sizeof(Quatd) failure");

//-------------------------------------------------------------------------------------
// ***** Pose
//
// Position and orientation combined.
//
// This structure needs to be the same size and layout on 32-bit and 64-bit arch.
// Update OVR_PadCheck.cpp when updating this object.
template<class T>
class Pose
{
public:
    typedef typename CompatibleTypes<Pose<T> >::Type CompatibleType;

    Pose() { }
    Pose(const Quat<T>& orientation, const Vector3<T>& pos)
        : Rotation(orientation), Translation(pos) {  }
    Pose(const Pose& s)
        : Rotation(s.Rotation), Translation(s.Translation) {  }
    Pose(const Matrix3<T>& R, const Vector3<T>& t)
        : Rotation((Quat<T>)R), Translation(t) {  }
    Pose(const CompatibleType& s)
        : Rotation(s.Orientation), Translation(s.Position) {  }

    explicit Pose(const Pose<typename Math<T>::OtherFloatType> &s)
        : Rotation(s.Rotation), Translation(s.Translation)
    {
        // Ensure normalized rotation if converting from float to double
        if (sizeof(T) > sizeof(typename Math<T>::OtherFloatType))
            Rotation.Normalize();
    }

    static Pose Identity() { return Pose(Quat<T>(0, 0, 0, 1), Vector3<T>(0, 0, 0)); }

    void SetIdentity() { Rotation = Quat<T>(0, 0, 0, 1); Translation = Vector3<T>(0, 0, 0); }

    // used to make things obviously broken if someone tries to use the value
    void SetInvalid() { Rotation = Quat<T>(NAN, NAN, NAN, NAN); Translation = Vector3<T>(NAN, NAN, NAN); }

    bool IsEqual(const Pose&b, T tolerance = Math<T>::Tolerance()) const
    {
        return Translation.IsEqual(b.Translation, tolerance) && Rotation.IsEqual(b.Rotation, tolerance);
    }

    operator typename CompatibleTypes<Pose<T> >::Type () const
    {
        typename CompatibleTypes<Pose<T> >::Type result;
        result.Orientation = Rotation;
        result.Position = Translation;
        return result;
    }

    Quat<T>    Rotation;
    Vector3<T> Translation;
    
    OVR_MATH_STATIC_ASSERT((sizeof(T) == sizeof(double) || sizeof(T) == sizeof(float)), "(sizeof(T) == sizeof(double) || sizeof(T) == sizeof(float))");

    void ToArray(T* arr) const
    {
        T temp[7] =  { Rotation.x, Rotation.y, Rotation.z, Rotation.w, Translation.x, Translation.y, Translation.z };
        for (int i = 0; i < 7; i++) arr[i] = temp[i];
    }

    static Pose<T> FromArray(const T* v)
    {
        Quat<T> rotation(v[0], v[1], v[2], v[3]);
        Vector3<T> translation(v[4], v[5], v[6]);
        // Ensure rotation is normalized, in case it was originally a float, stored in a .json file, etc.
        return Pose<T>(rotation.Normalized(), translation);
    }

    Vector3<T> Rotate(const Vector3<T>& v) const
    {
        return Rotation.Rotate(v);
    }

    Vector3<T> InverseRotate(const Vector3<T>& v) const
    {
        return Rotation.InverseRotate(v);
    }

    Vector3<T> Translate(const Vector3<T>& v) const
    {
        return v + Translation;
    }

    Vector3<T> Transform(const Vector3<T>& v) const
    {
        return Rotate(v) + Translation;
    }

    Vector3<T> InverseTransform(const Vector3<T>& v) const
    {
        return InverseRotate(v - Translation);
    }


    Vector3<T> Apply(const Vector3<T>& v) const
    {
        return Transform(v);
    }

    Pose operator*(const Pose& other) const   
    {
        return Pose(Rotation * other.Rotation, Apply(other.Translation));
    }

    Pose Inverted() const   
    {
        Quat<T> inv = Rotation.Inverted();
        return Pose(inv, inv.Rotate(-Translation));
    }

    // Interpolation between two poses: translation is interpolated with Lerp(),
    // and rotations are interpolated with Slerp().
    Pose Lerp(const Pose& b, T s)
    {
        return Pose(Rotation.Slerp(b.Rotation, s), Translation.Lerp(b.Translation, s));
    }

    // Similar to Lerp above, except faster in case of small rotation differences.  See Quat<T>::FastSlerp.
    Pose FastLerp(const Pose& b, T s)
    {
        return Pose(Rotation.FastSlerp(b.Rotation, s), Translation.Lerp(b.Translation, s));
    }

    Pose TimeIntegrate(const Vector3<T>& linearVelocity, const Vector3<T>& angularVelocity, T dt) const
    {
        return Pose(
                (Rotation * Quat<T>::FastFromRotationVector(angularVelocity * dt, false)).Normalized(),
                Translation + linearVelocity * dt);
    }

    Pose TimeIntegrate(const Vector3<T>& linearVelocity, const Vector3<T>& linearAcceleration,
                       const Vector3<T>& angularVelocity, const Vector3<T>& angularAcceleration,
                       T dt) const
    {
        return Pose(Rotation.TimeIntegrate(angularVelocity, angularAcceleration, dt),
                    Translation + linearVelocity*dt + linearAcceleration*dt*dt * T(0.5));
    }
};

typedef Pose<float>  Posef;
typedef Pose<double> Posed;

OVR_MATH_STATIC_ASSERT((sizeof(Posed) == sizeof(Quatd) + sizeof(Vector3d)), "sizeof(Posed) failure");
OVR_MATH_STATIC_ASSERT((sizeof(Posef) == sizeof(Quatf) + sizeof(Vector3f)), "sizeof(Posef) failure");
    

//-------------------------------------------------------------------------------------
// ***** Matrix4
//
// Matrix4 is a 4x4 matrix used for 3d transformations and projections.
// Translation stored in the last column.
// The matrix is stored in row-major order in memory, meaning that values
// of the first row are stored before the next one.
//
// The arrangement of the matrix is chosen to be in Right-Handed 
// coordinate system and counterclockwise rotations when looking down
// the axis
//
// Transformation Order:
//   - Transformations are applied from right to left, so the expression
//     M1 * M2 * M3 * V means that the vector V is transformed by M3 first,
//     followed by M2 and M1. 
//
// Coordinate system: Right Handed
//
// Rotations: Counterclockwise when looking down the axis. All angles are in radians.
//    
//  | sx   01   02   tx |    // First column  (sx, 10, 20): Axis X basis vector.
//  | 10   sy   12   ty |    // Second column (01, sy, 21): Axis Y basis vector.
//  | 20   21   sz   tz |    // Third columnt (02, 12, sz): Axis Z basis vector.
//  | 30   31   32   33 |
//
//  The basis vectors are first three columns.

template<class T>
class Matrix4
{
public:
    typedef T ElementType;
    static const size_t Dimension = 4;

    T M[4][4];

    enum NoInitType { NoInit };

    // Construct with no memory initialization.
    Matrix4(NoInitType) { }

    // By default, we construct identity matrix.
    Matrix4()
    {
        M[0][0] = M[1][1] = M[2][2] = M[3][3] = T(1);
        M[0][1] = M[1][0] = M[2][3] = M[3][1] = T(0);
        M[0][2] = M[1][2] = M[2][0] = M[3][2] = T(0);
        M[0][3] = M[1][3] = M[2][1] = M[3][0] = T(0);
    }

    Matrix4(T m11, T m12, T m13, T m14,
            T m21, T m22, T m23, T m24,
            T m31, T m32, T m33, T m34,
            T m41, T m42, T m43, T m44)
    {
        M[0][0] = m11; M[0][1] = m12; M[0][2] = m13; M[0][3] = m14;
        M[1][0] = m21; M[1][1] = m22; M[1][2] = m23; M[1][3] = m24;
        M[2][0] = m31; M[2][1] = m32; M[2][2] = m33; M[2][3] = m34;
        M[3][0] = m41; M[3][1] = m42; M[3][2] = m43; M[3][3] = m44;
    }

    Matrix4(T m11, T m12, T m13,
            T m21, T m22, T m23,
            T m31, T m32, T m33)
    {
        M[0][0] = m11; M[0][1] = m12; M[0][2] = m13; M[0][3] = T(0);
        M[1][0] = m21; M[1][1] = m22; M[1][2] = m23; M[1][3] = T(0);
        M[2][0] = m31; M[2][1] = m32; M[2][2] = m33; M[2][3] = T(0);
        M[3][0] = T(0);   M[3][1] = T(0);   M[3][2] = T(0);   M[3][3] = T(1);
    }

    explicit Matrix4(const Matrix3<T>& m)
    {
        M[0][0] = m.M[0][0]; M[0][1] = m.M[0][1]; M[0][2] = m.M[0][2]; M[0][3] = T(0);
        M[1][0] = m.M[1][0]; M[1][1] = m.M[1][1]; M[1][2] = m.M[1][2]; M[1][3] = T(0);
        M[2][0] = m.M[2][0]; M[2][1] = m.M[2][1]; M[2][2] = m.M[2][2]; M[2][3] = T(0);
        M[3][0] = T(0);         M[3][1] = T(0);         M[3][2] = T(0);         M[3][3] = T(1);
    }

    explicit Matrix4(const Quat<T>& q)
    {
        OVR_MATH_ASSERT(q.IsNormalized());
        T ww = q.w*q.w;
        T xx = q.x*q.x;
        T yy = q.y*q.y;
        T zz = q.z*q.z;

        M[0][0] = ww + xx - yy - zz;       M[0][1] = 2 * (q.x*q.y - q.w*q.z); M[0][2] = 2 * (q.x*q.z + q.w*q.y); M[0][3] = T(0);
        M[1][0] = 2 * (q.x*q.y + q.w*q.z); M[1][1] = ww - xx + yy - zz;       M[1][2] = 2 * (q.y*q.z - q.w*q.x); M[1][3] = T(0);
        M[2][0] = 2 * (q.x*q.z - q.w*q.y); M[2][1] = 2 * (q.y*q.z + q.w*q.x); M[2][2] = ww - xx - yy + zz;       M[2][3] = T(0);
        M[3][0] = T(0);                       M[3][1] = T(0);                       M[3][2] = T(0);                       M[3][3] = T(1);
    }

    explicit Matrix4(const Pose<T>& p)
    {
        Matrix4 result(p.Rotation);
        result.SetTranslation(p.Translation);
        *this = result;
    }


    // C-interop support
    explicit Matrix4(const Matrix4<typename Math<T>::OtherFloatType> &src)
    {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                M[i][j] = (T)src.M[i][j];
    }

    // C-interop support.
    Matrix4(const typename CompatibleTypes<Matrix4<T> >::Type& s) 
    {
        OVR_MATH_STATIC_ASSERT(sizeof(s) == sizeof(Matrix4), "sizeof(s) == sizeof(Matrix4)");
        memcpy(M, s.M, sizeof(M));
    }

    operator typename CompatibleTypes<Matrix4<T> >::Type () const
    {
        typename CompatibleTypes<Matrix4<T> >::Type result;
        OVR_MATH_STATIC_ASSERT(sizeof(result) == sizeof(Matrix4), "sizeof(result) == sizeof(Matrix4)");
        memcpy(result.M, M, sizeof(M));
        return result;
    }

    void ToString(char* dest, size_t destsize) const
    {
        size_t pos = 0;
        for (int r=0; r<4; r++)
        {
            for (int c=0; c<4; c++)
            {
                pos += OVRMath_sprintf(dest+pos, destsize-pos, "%g ", M[r][c]);
            }
        }
    }

    static Matrix4 FromString(const char* src)
    {
        Matrix4 result;
        if (src)
        {
            for (int r = 0; r < 4; r++)
            {
                for (int c = 0; c < 4; c++)
                {
                    result.M[r][c] = (T)atof(src);
                    while (*src && *src != ' ')
                    {
                        src++;
                    }
                    while (*src && *src == ' ')
                    {
                        src++;
                    }
                }
            }
        }
        return result;
    }

    static Matrix4 Identity()  { return Matrix4(); }

    void SetIdentity()
    {
        M[0][0] = M[1][1] = M[2][2] = M[3][3] = T(1);
        M[0][1] = M[1][0] = M[2][3] = M[3][1] = T(0);
        M[0][2] = M[1][2] = M[2][0] = M[3][2] = T(0);
        M[0][3] = M[1][3] = M[2][1] = M[3][0] = T(0);
    }

    void SetXBasis(const Vector3<T>& v)
    {
        M[0][0] = v.x;
        M[1][0] = v.y;
        M[2][0] = v.z;
    }
    Vector3<T> GetXBasis() const
    {
        return Vector3<T>(M[0][0], M[1][0], M[2][0]);
    }

    void SetYBasis(const Vector3<T> & v)
    {
        M[0][1] = v.x;
        M[1][1] = v.y;
        M[2][1] = v.z;
    }
    Vector3<T> GetYBasis() const
    {
        return Vector3<T>(M[0][1], M[1][1], M[2][1]);
    }

    void SetZBasis(const Vector3<T> & v)
    {
        M[0][2] = v.x;
        M[1][2] = v.y;
        M[2][2] = v.z;
    }
    Vector3<T> GetZBasis() const
    {
        return Vector3<T>(M[0][2], M[1][2], M[2][2]);
    }

    bool operator== (const Matrix4& b) const
    {
        bool isEqual = true;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                isEqual &= (M[i][j] == b.M[i][j]);

        return isEqual;
    }

    Matrix4 operator+ (const Matrix4& b) const
    {
        Matrix4 result(*this);
        result += b;
        return result;
    }

    Matrix4& operator+= (const Matrix4& b)
    {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                M[i][j] += b.M[i][j];
        return *this;
    }

    Matrix4 operator- (const Matrix4& b) const
    {
        Matrix4 result(*this);
        result -= b;
        return result;
    }

    Matrix4& operator-= (const Matrix4& b)
    {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                M[i][j] -= b.M[i][j];
        return *this;
    }

    // Multiplies two matrices into destination with minimum copying.
    static Matrix4& Multiply(Matrix4* d, const Matrix4& a, const Matrix4& b)
    {
        OVR_MATH_ASSERT((d != &a) && (d != &b));
        int i = 0;
        do {
            d->M[i][0] = a.M[i][0] * b.M[0][0] + a.M[i][1] * b.M[1][0] + a.M[i][2] * b.M[2][0] + a.M[i][3] * b.M[3][0];
            d->M[i][1] = a.M[i][0] * b.M[0][1] + a.M[i][1] * b.M[1][1] + a.M[i][2] * b.M[2][1] + a.M[i][3] * b.M[3][1];
            d->M[i][2] = a.M[i][0] * b.M[0][2] + a.M[i][1] * b.M[1][2] + a.M[i][2] * b.M[2][2] + a.M[i][3] * b.M[3][2];
            d->M[i][3] = a.M[i][0] * b.M[0][3] + a.M[i][1] * b.M[1][3] + a.M[i][2] * b.M[2][3] + a.M[i][3] * b.M[3][3];
        } while((++i) < 4);

        return *d;
    }

    Matrix4 operator* (const Matrix4& b) const
    {
        Matrix4 result(Matrix4::NoInit);
        Multiply(&result, *this, b);
        return result;
    }

    Matrix4& operator*= (const Matrix4& b)
    {
        return Multiply(this, Matrix4(*this), b);
    }

    Matrix4 operator* (T s) const
    {
        Matrix4 result(*this);
        result *= s;
        return result;
    }

    Matrix4& operator*= (T s)
    {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                M[i][j] *= s;
        return *this;
    }


    Matrix4 operator/ (T s) const
    {
        Matrix4 result(*this);
        result /= s;
        return result;
    }

    Matrix4& operator/= (T s)
    {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                M[i][j] /= s;
        return *this;
    }

    Vector3<T> Transform(const Vector3<T>& v) const
    {
        const T rcpW = T(1) / (M[3][0] * v.x + M[3][1] * v.y + M[3][2] * v.z + M[3][3]);
        return Vector3<T>((M[0][0] * v.x + M[0][1] * v.y + M[0][2] * v.z + M[0][3]) * rcpW,
                          (M[1][0] * v.x + M[1][1] * v.y + M[1][2] * v.z + M[1][3]) * rcpW,
                          (M[2][0] * v.x + M[2][1] * v.y + M[2][2] * v.z + M[2][3]) * rcpW);
    }

    Vector4<T> Transform(const Vector4<T>& v) const
    {
        return Vector4<T>(M[0][0] * v.x + M[0][1] * v.y + M[0][2] * v.z + M[0][3] * v.w,
                          M[1][0] * v.x + M[1][1] * v.y + M[1][2] * v.z + M[1][3] * v.w,
                          M[2][0] * v.x + M[2][1] * v.y + M[2][2] * v.z + M[2][3] * v.w,
                          M[3][0] * v.x + M[3][1] * v.y + M[3][2] * v.z + M[3][3] * v.w);
    }

    Matrix4 Transposed() const
    {
        return Matrix4(M[0][0], M[1][0], M[2][0], M[3][0],
                        M[0][1], M[1][1], M[2][1], M[3][1],
                        M[0][2], M[1][2], M[2][2], M[3][2],
                        M[0][3], M[1][3], M[2][3], M[3][3]);
    }

    void     Transpose()
    {
        *this = Transposed();
    }


    T SubDet (const size_t* rows, const size_t* cols) const
    {
        return M[rows[0]][cols[0]] * (M[rows[1]][cols[1]] * M[rows[2]][cols[2]] - M[rows[1]][cols[2]] * M[rows[2]][cols[1]])
             - M[rows[0]][cols[1]] * (M[rows[1]][cols[0]] * M[rows[2]][cols[2]] - M[rows[1]][cols[2]] * M[rows[2]][cols[0]])
             + M[rows[0]][cols[2]] * (M[rows[1]][cols[0]] * M[rows[2]][cols[1]] - M[rows[1]][cols[1]] * M[rows[2]][cols[0]]);
    }

    T Cofactor(size_t I, size_t J) const
    {
        const size_t indices[4][3] = {{1,2,3},{0,2,3},{0,1,3},{0,1,2}};
        return ((I+J)&1) ? -SubDet(indices[I],indices[J]) : SubDet(indices[I],indices[J]);
    }

    T    Determinant() const
    {
        return M[0][0] * Cofactor(0,0) + M[0][1] * Cofactor(0,1) + M[0][2] * Cofactor(0,2) + M[0][3] * Cofactor(0,3);
    }

    Matrix4 Adjugated() const
    {
        return Matrix4(Cofactor(0,0), Cofactor(1,0), Cofactor(2,0), Cofactor(3,0), 
                        Cofactor(0,1), Cofactor(1,1), Cofactor(2,1), Cofactor(3,1), 
                        Cofactor(0,2), Cofactor(1,2), Cofactor(2,2), Cofactor(3,2),
                        Cofactor(0,3), Cofactor(1,3), Cofactor(2,3), Cofactor(3,3));
    }

    Matrix4 Inverted() const
    {
        T det = Determinant();
        OVR_MATH_ASSERT(det != 0);
        return Adjugated() * (T(1)/det);
    }

    void Invert()
    {
        *this = Inverted();
    }

    // This is more efficient than general inverse, but ONLY works
    // correctly if it is a homogeneous transform matrix (rot + trans)
    Matrix4 InvertedHomogeneousTransform() const
    {
        // Make the inverse rotation matrix
        Matrix4 rinv = this->Transposed();
        rinv.M[3][0] = rinv.M[3][1] = rinv.M[3][2] = T(0);
        // Make the inverse translation matrix
        Vector3<T> tvinv(-M[0][3],-M[1][3],-M[2][3]);
        Matrix4 tinv = Matrix4::Translation(tvinv);
        return rinv * tinv;  // "untranslate", then "unrotate"
    }

    // This is more efficient than general inverse, but ONLY works
    // correctly if it is a homogeneous transform matrix (rot + trans)
    void InvertHomogeneousTransform()
    {
        *this = InvertedHomogeneousTransform();
    }

    // Matrix to Euler Angles conversion
    // a,b,c, are the YawPitchRoll angles to be returned
    // rotation a around axis A1
    // is followed by rotation b around axis A2
    // is followed by rotation c around axis A3
    // rotations are CCW or CW (D) in LH or RH coordinate system (S)
    template <Axis A1, Axis A2, Axis A3, RotateDirection D, HandedSystem S>
    void ToEulerAngles(T *a, T *b, T *c) const
    {
        OVR_MATH_STATIC_ASSERT((A1 != A2) && (A2 != A3) && (A1 != A3), "(A1 != A2) && (A2 != A3) && (A1 != A3)");

        T psign = T(-1);
        if (((A1 + 1) % 3 == A2) && ((A2 + 1) % 3 == A3)) // Determine whether even permutation
            psign = T(1);
        
        T pm = psign*M[A1][A3];
        T singularityRadius = Math<T>::SingularityRadius();
        if (pm < T(-1) + singularityRadius)
        { // South pole singularity
            *a = T(0);
            *b = -S*D*((T)MATH_DOUBLE_PIOVER2);
            *c = S*D*atan2( psign*M[A2][A1], M[A2][A2] );
        }
        else if (pm > T(1) - singularityRadius)
        { // North pole singularity
            *a = T(0);
            *b = S*D*((T)MATH_DOUBLE_PIOVER2);
            *c = S*D*atan2( psign*M[A2][A1], M[A2][A2] );
        }
        else
        { // Normal case (nonsingular)
            *a = S*D*atan2( -psign*M[A2][A3], M[A3][A3] );
            *b = S*D*asin(pm);
            *c = S*D*atan2( -psign*M[A1][A2], M[A1][A1] );
        }
    }

    // Matrix to Euler Angles conversion
    // a,b,c, are the YawPitchRoll angles to be returned
    // rotation a around axis A1
    // is followed by rotation b around axis A2
    // is followed by rotation c around axis A1
    // rotations are CCW or CW (D) in LH or RH coordinate system (S)
    template <Axis A1, Axis A2, RotateDirection D, HandedSystem S>
    void ToEulerAnglesABA(T *a, T *b, T *c) const
    {        
         OVR_MATH_STATIC_ASSERT(A1 != A2, "A1 != A2");
  
        // Determine the axis that was not supplied
        int m = 3 - A1 - A2;

        T psign = T(-1);
        if ((A1 + 1) % 3 == A2) // Determine whether even permutation
            psign = T(1);

        T c2 = M[A1][A1];
        T singularityRadius = Math<T>::SingularityRadius();
        if (c2 < T(-1) + singularityRadius)
        { // South pole singularity
            *a = T(0);
            *b = S*D*((T)MATH_DOUBLE_PI);
            *c = S*D*atan2( -psign*M[A2][m],M[A2][A2]);
        }
        else if (c2 > T(1) - singularityRadius)
        { // North pole singularity
            *a = T(0);
            *b = T(0);
            *c = S*D*atan2( -psign*M[A2][m],M[A2][A2]);
        }
        else
        { // Normal case (nonsingular)
            *a = S*D*atan2( M[A2][A1],-psign*M[m][A1]);
            *b = S*D*acos(c2);
            *c = S*D*atan2( M[A1][A2],psign*M[A1][m]);
        }
    }
  
    // Creates a matrix that converts the vertices from one coordinate system
    // to another.
    static Matrix4 AxisConversion(const WorldAxes& to, const WorldAxes& from)
    {        
        // Holds axis values from the 'to' structure
        int toArray[3] = { to.XAxis, to.YAxis, to.ZAxis };

        // The inverse of the toArray
        int inv[4]; 
        inv[0] = inv[abs(to.XAxis)] = 0;
        inv[abs(to.YAxis)] = 1;
        inv[abs(to.ZAxis)] = 2;

        Matrix4 m(0,  0,  0, 
                  0,  0,  0,
                  0,  0,  0);

        // Only three values in the matrix need to be changed to 1 or -1.
        m.M[inv[abs(from.XAxis)]][0] = T(from.XAxis/toArray[inv[abs(from.XAxis)]]);
        m.M[inv[abs(from.YAxis)]][1] = T(from.YAxis/toArray[inv[abs(from.YAxis)]]);
        m.M[inv[abs(from.ZAxis)]][2] = T(from.ZAxis/toArray[inv[abs(from.ZAxis)]]);
        return m;
    } 


    // Creates a matrix for translation by vector
    static Matrix4 Translation(const Vector3<T>& v)
    {
        Matrix4 t;
        t.M[0][3] = v.x;
        t.M[1][3] = v.y;
        t.M[2][3] = v.z;
        return t;
    }

    // Creates a matrix for translation by vector
    static Matrix4 Translation(T x, T y, T z = T(0))
    {
        Matrix4 t;
        t.M[0][3] = x;
        t.M[1][3] = y;
        t.M[2][3] = z;
        return t;
    }

    // Sets the translation part
    void SetTranslation(const Vector3<T>& v)
    {
        M[0][3] = v.x;
        M[1][3] = v.y;
        M[2][3] = v.z;
    }

    Vector3<T> GetTranslation() const
    {
        return Vector3<T>( M[0][3], M[1][3], M[2][3] );
    }

    // Creates a matrix for scaling by vector
    static Matrix4 Scaling(const Vector3<T>& v)
    {
        Matrix4 t;
        t.M[0][0] = v.x;
        t.M[1][1] = v.y;
        t.M[2][2] = v.z;
        return t;
    }

    // Creates a matrix for scaling by vector
    static Matrix4 Scaling(T x, T y, T z)
    {
        Matrix4 t;
        t.M[0][0] = x;
        t.M[1][1] = y;
        t.M[2][2] = z;
        return t;
    }

    // Creates a matrix for scaling by constant
    static Matrix4 Scaling(T s)
    {
        Matrix4 t;
        t.M[0][0] = s;
        t.M[1][1] = s;
        t.M[2][2] = s;
        return t;
    }

    // Simple L1 distance in R^12
    T Distance(const Matrix4& m2) const           
    { 
        T d = fabs(M[0][0] - m2.M[0][0]) + fabs(M[0][1] - m2.M[0][1]);
        d += fabs(M[0][2] - m2.M[0][2]) + fabs(M[0][3] - m2.M[0][3]);
        d += fabs(M[1][0] - m2.M[1][0]) + fabs(M[1][1] - m2.M[1][1]);
        d += fabs(M[1][2] - m2.M[1][2]) + fabs(M[1][3] - m2.M[1][3]);
        d += fabs(M[2][0] - m2.M[2][0]) + fabs(M[2][1] - m2.M[2][1]);
        d += fabs(M[2][2] - m2.M[2][2]) + fabs(M[2][3] - m2.M[2][3]);
        d += fabs(M[3][0] - m2.M[3][0]) + fabs(M[3][1] - m2.M[3][1]);
        d += fabs(M[3][2] - m2.M[3][2]) + fabs(M[3][3] - m2.M[3][3]);
        return d; 
    }

    // Creates a rotation matrix rotating around the X axis by 'angle' radians.
    // Just for quick testing.  Not for final API.  Need to remove case.
    static Matrix4 RotationAxis(Axis A, T angle, RotateDirection d, HandedSystem s)
    {
        T sina = s * d *sin(angle);
        T cosa = cos(angle);
        
        switch(A)
        {
        case Axis_X:
            return Matrix4(1,  0,     0, 
                           0,  cosa,  -sina,
                           0,  sina,  cosa);
        case Axis_Y:
            return Matrix4(cosa,  0,   sina, 
                           0,     1,   0,
                           -sina, 0,   cosa);
        case Axis_Z:
            return Matrix4(cosa,  -sina,  0, 
                           sina,  cosa,   0,
                           0,     0,      1);
        default:
            return Matrix4();
        }
    }


    // Creates a rotation matrix rotating around the X axis by 'angle' radians.
    // Rotation direction is depends on the coordinate system:
    // RHS (Oculus default): Positive angle values rotate Counter-clockwise (CCW),
    //                        while looking in the negative axis direction. This is the
    //                        same as looking down from positive axis values towards origin.
    // LHS: Positive angle values rotate clock-wise (CW), while looking in the
    //       negative axis direction.
    static Matrix4 RotationX(T angle)
    {
        T sina = sin(angle);
        T cosa = cos(angle);
        return Matrix4(1,  0,     0, 
                       0,  cosa,  -sina,
                       0,  sina,  cosa);
    }

    // Creates a rotation matrix rotating around the Y axis by 'angle' radians.
    // Rotation direction is depends on the coordinate system:
    //  RHS (Oculus default): Positive angle values rotate Counter-clockwise (CCW),
    //                        while looking in the negative axis direction. This is the
    //                        same as looking down from positive axis values towards origin.
    //  LHS: Positive angle values rotate clock-wise (CW), while looking in the
    //       negative axis direction.
    static Matrix4 RotationY(T angle)
    {
        T sina = (T)sin(angle);
        T cosa = (T)cos(angle);
        return Matrix4(cosa,  0,   sina, 
                       0,     1,   0,
                       -sina, 0,   cosa);
    }

    // Creates a rotation matrix rotating around the Z axis by 'angle' radians.
    // Rotation direction is depends on the coordinate system:
    //  RHS (Oculus default): Positive angle values rotate Counter-clockwise (CCW),
    //                        while looking in the negative axis direction. This is the
    //                        same as looking down from positive axis values towards origin.
    //  LHS: Positive angle values rotate clock-wise (CW), while looking in the
    //       negative axis direction.
    static Matrix4 RotationZ(T angle)
    {
        T sina = sin(angle);
        T cosa = cos(angle);
        return Matrix4(cosa,  -sina,  0, 
                       sina,  cosa,   0,
                       0,     0,      1);
    }

    // LookAtRH creates a View transformation matrix for right-handed coordinate system.
    // The resulting matrix points camera from 'eye' towards 'at' direction, with 'up'
    // specifying the up vector. The resulting matrix should be used with PerspectiveRH
    // projection.
    static Matrix4 LookAtRH(const Vector3<T>& eye, const Vector3<T>& at, const Vector3<T>& up)
    {
        Vector3<T> z = (eye - at).Normalized();  // Forward
        Vector3<T> x = up.Cross(z).Normalized(); // Right
        Vector3<T> y = z.Cross(x);

        Matrix4 m(x.x,  x.y,  x.z,  -(x.Dot(eye)),
                  y.x,  y.y,  y.z,  -(y.Dot(eye)),
                  z.x,  z.y,  z.z,  -(z.Dot(eye)),
                  0,    0,    0,    1 );
        return m;
    }
    
    // LookAtLH creates a View transformation matrix for left-handed coordinate system.
    // The resulting matrix points camera from 'eye' towards 'at' direction, with 'up'
    // specifying the up vector. 
    static Matrix4 LookAtLH(const Vector3<T>& eye, const Vector3<T>& at, const Vector3<T>& up)
    {
        Vector3<T> z = (at - eye).Normalized();  // Forward
        Vector3<T> x = up.Cross(z).Normalized(); // Right
        Vector3<T> y = z.Cross(x);

        Matrix4 m(x.x,  x.y,  x.z,  -(x.Dot(eye)),
                  y.x,  y.y,  y.z,  -(y.Dot(eye)),
                  z.x,  z.y,  z.z,  -(z.Dot(eye)),
                  0,    0,    0,    1 ); 
        return m;
    }
    
    // PerspectiveRH creates a right-handed perspective projection matrix that can be
    // used with the Oculus sample renderer. 
    //  yfov   - Specifies vertical field of view in radians.
    //  aspect - Screen aspect ration, which is usually width/height for square pixels.
    //           Note that xfov = yfov * aspect.
    //  znear  - Absolute value of near Z clipping clipping range.
    //  zfar   - Absolute value of far  Z clipping clipping range (larger then near).
    // Even though RHS usually looks in the direction of negative Z, positive values
    // are expected for znear and zfar.
    static Matrix4 PerspectiveRH(T yfov, T aspect, T znear, T zfar)
    {
        Matrix4 m;
        T tanHalfFov = tan(yfov * T(0.5));

        m.M[0][0] = T(1) / (aspect * tanHalfFov);
        m.M[1][1] = T(1) / tanHalfFov;
        m.M[2][2] = zfar / (znear - zfar);
        m.M[3][2] = T(-1);
        m.M[2][3] = (zfar * znear) / (znear - zfar);
        m.M[3][3] = T(0);

        // Note: Post-projection matrix result assumes Left-Handed coordinate system,
        //       with Y up, X right and Z forward. This supports positive z-buffer values.
        // This is the case even for RHS coordinate input.
        return m;
    }
    
    // PerspectiveLH creates a left-handed perspective projection matrix that can be
    // used with the Oculus sample renderer. 
    //  yfov   - Specifies vertical field of view in radians.
    //  aspect - Screen aspect ration, which is usually width/height for square pixels.
    //           Note that xfov = yfov * aspect.
    //  znear  - Absolute value of near Z clipping clipping range.
    //  zfar   - Absolute value of far  Z clipping clipping range (larger then near).
    static Matrix4 PerspectiveLH(T yfov, T aspect, T znear, T zfar)
    {
        Matrix4 m;
        T tanHalfFov = tan(yfov * T(0.5));

        m.M[0][0] = T(1) / (aspect * tanHalfFov);
        m.M[1][1] = T(1) / tanHalfFov;
        //m.M[2][2] = zfar / (znear - zfar);
         m.M[2][2] = zfar / (zfar - znear);
        m.M[3][2] = T(-1);
        m.M[2][3] = (zfar * znear) / (znear - zfar);
        m.M[3][3] = T(0);

        // Note: Post-projection matrix result assumes Left-Handed coordinate system,    
        //       with Y up, X right and Z forward. This supports positive z-buffer values.
        // This is the case even for RHS coordinate input. 
        return m;
    }

    static Matrix4 Ortho2D(T w, T h)
    {
        Matrix4 m;
        m.M[0][0] = T(2.0)/w;
        m.M[1][1] = T(-2.0)/h;
        m.M[0][3] = T(-1.0);
        m.M[1][3] = T(1.0);
        m.M[2][2] = T(0);
        return m;
    }
};

typedef Matrix4<float>  Matrix4f;
typedef Matrix4<double> Matrix4d;

//-------------------------------------------------------------------------------------
// ***** Matrix3
//
// Matrix3 is a 3x3 matrix used for representing a rotation matrix.
// The matrix is stored in row-major order in memory, meaning that values
// of the first row are stored before the next one.
//
// The arrangement of the matrix is chosen to be in Right-Handed 
// coordinate system and counterclockwise rotations when looking down
// the axis
//
// Transformation Order:
//   - Transformations are applied from right to left, so the expression
//     M1 * M2 * M3 * V means that the vector V is transformed by M3 first,
//     followed by M2 and M1. 
//
// Coordinate system: Right Handed
//
// Rotations: Counterclockwise when looking down the axis. All angles are in radians.

template<class T>
class Matrix3
{
public:
    typedef T ElementType;
    static const size_t Dimension = 3;

    T M[3][3];

    enum NoInitType { NoInit };

    // Construct with no memory initialization.
    Matrix3(NoInitType) { }

    // By default, we construct identity matrix.
    Matrix3()
    {
        M[0][0] = M[1][1] = M[2][2] = T(1);
        M[0][1] = M[1][0] = M[2][0] = T(0);
        M[0][2] = M[1][2] = M[2][1] = T(0);
    }

    Matrix3(T m11, T m12, T m13,
            T m21, T m22, T m23,
            T m31, T m32, T m33)
    {
        M[0][0] = m11; M[0][1] = m12; M[0][2] = m13;
        M[1][0] = m21; M[1][1] = m22; M[1][2] = m23;
        M[2][0] = m31; M[2][1] = m32; M[2][2] = m33;
    }
    
    // Construction from X, Y, Z basis vectors
    Matrix3(const Vector3<T>& xBasis, const Vector3<T>& yBasis, const Vector3<T>& zBasis)
    {
        M[0][0] = xBasis.x; M[0][1] = yBasis.x; M[0][2] = zBasis.x;
        M[1][0] = xBasis.y; M[1][1] = yBasis.y; M[1][2] = zBasis.y;
        M[2][0] = xBasis.z; M[2][1] = yBasis.z; M[2][2] = zBasis.z;
    }

    explicit Matrix3(const Quat<T>& q)
    {
        OVR_MATH_ASSERT(q.IsNormalized());
        const T tx  = q.x+q.x,  ty  = q.y+q.y,  tz  = q.z+q.z;
        const T twx = q.w*tx,   twy = q.w*ty,   twz = q.w*tz;
        const T txx = q.x*tx,   txy = q.x*ty,   txz = q.x*tz;
        const T tyy = q.y*ty,   tyz = q.y*tz,   tzz = q.z*tz;
        M[0][0] = T(1) - (tyy + tzz);    M[0][1] = txy - twz;            M[0][2] = txz + twy;
        M[1][0] = txy + twz;            M[1][1] = T(1) - (txx + tzz);    M[1][2] = tyz - twx;
        M[2][0] = txz - twy;            M[2][1] = tyz + twx;            M[2][2] = T(1) - (txx + tyy);
    }
    
    inline explicit Matrix3(T s)
    {
        M[0][0] = M[1][1] = M[2][2] = s;
        M[0][1] = M[0][2] = M[1][0] = M[1][2] = M[2][0] = M[2][1] = T(0);
    }

    Matrix3(T m11, T m22, T m33)
    {
        M[0][0] = m11; M[0][1] = T(0); M[0][2] = T(0);
        M[1][0] = T(0); M[1][1] = m22; M[1][2] = T(0);
        M[2][0] = T(0); M[2][1] = T(0); M[2][2] = m33;
    }

    explicit Matrix3(const Matrix3<typename Math<T>::OtherFloatType> &src)
    {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                M[i][j] = (T)src.M[i][j];
    }

    // C-interop support.
    Matrix3(const typename CompatibleTypes<Matrix3<T> >::Type& s) 
    {
        OVR_MATH_STATIC_ASSERT(sizeof(s) == sizeof(Matrix3), "sizeof(s) == sizeof(Matrix3)");
        memcpy(M, s.M, sizeof(M));
    }

    operator const typename CompatibleTypes<Matrix3<T> >::Type () const
    {
        typename CompatibleTypes<Matrix3<T> >::Type result;
        OVR_MATH_STATIC_ASSERT(sizeof(result) == sizeof(Matrix3), "sizeof(result) == sizeof(Matrix3)");
        memcpy(result.M, M, sizeof(M));
        return result;
    }

    T  operator()(int i, int j) const { return M[i][j]; }
    T& operator()(int i, int j)       { return M[i][j]; }

    void ToString(char* dest, size_t destsize) const
    {
        size_t pos = 0;
        for (int r=0; r<3; r++)
        {
            for (int c=0; c<3; c++)
                pos += OVRMath_sprintf(dest+pos, destsize-pos, "%g ", M[r][c]);
        }
    }

    static Matrix3 FromString(const char* src)
    {
        Matrix3 result;
        if (src)
        {
            for (int r=0; r<3; r++)
            {
                for (int c=0; c<3; c++)
                {
                    result.M[r][c] = (T)atof(src);
                    while (*src && *src != ' ')
                        src++;
                    while (*src && *src == ' ')
                        src++;
                }
            }
        }
        return result;
    }

    static Matrix3 Identity()  { return Matrix3(); }

    void SetIdentity()
    {
        M[0][0] = M[1][1] = M[2][2] = T(1);
        M[0][1] = M[1][0] = M[2][0] = T(0);
        M[0][2] = M[1][2] = M[2][1] = T(0);
    }

    static Matrix3 Diagonal(T m00, T m11, T m22)
    {
        return Matrix3(m00, 0, 0,
            0, m11, 0,
            0, 0, m22);
    }
    static Matrix3 Diagonal(const Vector3<T>& v) { return Diagonal(v.x, v.y, v.z); }

    T Trace() const { return M[0][0] + M[1][1] + M[2][2]; }
    
    bool operator== (const Matrix3& b) const
    {
        bool isEqual = true;
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
                isEqual &= (M[i][j] == b.M[i][j]);
        }

        return isEqual;
    }

    Matrix3 operator+ (const Matrix3& b) const
    {
        Matrix3<T> result(*this);
        result += b;
        return result;
    }

    Matrix3& operator+= (const Matrix3& b)
    {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                M[i][j] += b.M[i][j];
        return *this;
    }

    void operator= (const Matrix3& b)
    {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                M[i][j] = b.M[i][j];
    }

    Matrix3 operator- (const Matrix3& b) const
    {
        Matrix3 result(*this);
        result -= b;
        return result;
    }

    Matrix3& operator-= (const Matrix3& b)
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
                M[i][j] -= b.M[i][j];
        }

        return *this;
    }

    // Multiplies two matrices into destination with minimum copying.
    static Matrix3& Multiply(Matrix3* d, const Matrix3& a, const Matrix3& b)
    {
        OVR_MATH_ASSERT((d != &a) && (d != &b));
        int i = 0;
        do {
            d->M[i][0] = a.M[i][0] * b.M[0][0] + a.M[i][1] * b.M[1][0] + a.M[i][2] * b.M[2][0];
            d->M[i][1] = a.M[i][0] * b.M[0][1] + a.M[i][1] * b.M[1][1] + a.M[i][2] * b.M[2][1];
            d->M[i][2] = a.M[i][0] * b.M[0][2] + a.M[i][1] * b.M[1][2] + a.M[i][2] * b.M[2][2];
        } while((++i) < 3);

        return *d;
    }

    Matrix3 operator* (const Matrix3& b) const
    {
        Matrix3 result(Matrix3::NoInit);
        Multiply(&result, *this, b);
        return result;
    }

    Matrix3& operator*= (const Matrix3& b)
    {
        return Multiply(this, Matrix3(*this), b);
    }

    Matrix3 operator* (T s) const
    {
        Matrix3 result(*this);
        result *= s;
        return result;
    }

    Matrix3& operator*= (T s)
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
                M[i][j] *= s;
        }

        return *this;
    }

    Vector3<T> operator* (const Vector3<T> &b) const
    {
        Vector3<T> result;
        result.x = M[0][0]*b.x + M[0][1]*b.y + M[0][2]*b.z;
        result.y = M[1][0]*b.x + M[1][1]*b.y + M[1][2]*b.z;
        result.z = M[2][0]*b.x + M[2][1]*b.y + M[2][2]*b.z;

        return result;
    }

    Matrix3 operator/ (T s) const
    {
        Matrix3 result(*this);
        result /= s;
        return result;
    }

    Matrix3& operator/= (T s)
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
                M[i][j] /= s;
        }

        return *this;
    }

    Vector2<T> Transform(const Vector2<T>& v) const
    {
        const T rcpZ = T(1) / (M[2][0] * v.x + M[2][1] * v.y + M[2][2]);
        return Vector2<T>((M[0][0] * v.x + M[0][1] * v.y + M[0][2]) * rcpZ,
                          (M[1][0] * v.x + M[1][1] * v.y + M[1][2]) * rcpZ);
    }

    Vector3<T> Transform(const Vector3<T>& v) const
    {
        return Vector3<T>(M[0][0] * v.x + M[0][1] * v.y + M[0][2] * v.z,
                          M[1][0] * v.x + M[1][1] * v.y + M[1][2] * v.z,
                          M[2][0] * v.x + M[2][1] * v.y + M[2][2] * v.z);
    }

    Matrix3 Transposed() const
    {
        return Matrix3(M[0][0], M[1][0], M[2][0],
                       M[0][1], M[1][1], M[2][1],
                       M[0][2], M[1][2], M[2][2]);
    }

    void     Transpose()
    {
        *this = Transposed();
    }


    T SubDet (const size_t* rows, const size_t* cols) const
    {
        return M[rows[0]][cols[0]] * (M[rows[1]][cols[1]] * M[rows[2]][cols[2]] - M[rows[1]][cols[2]] * M[rows[2]][cols[1]])
             - M[rows[0]][cols[1]] * (M[rows[1]][cols[0]] * M[rows[2]][cols[2]] - M[rows[1]][cols[2]] * M[rows[2]][cols[0]])
             + M[rows[0]][cols[2]] * (M[rows[1]][cols[0]] * M[rows[2]][cols[1]] - M[rows[1]][cols[1]] * M[rows[2]][cols[0]]);
    }

    
    // M += a*b.t()
    inline void Rank1Add(const Vector3<T> &a, const Vector3<T> &b)
    {
        M[0][0] += a.x*b.x;        M[0][1] += a.x*b.y;        M[0][2] += a.x*b.z;
        M[1][0] += a.y*b.x;        M[1][1] += a.y*b.y;        M[1][2] += a.y*b.z;
        M[2][0] += a.z*b.x;        M[2][1] += a.z*b.y;        M[2][2] += a.z*b.z;
    }

    // M -= a*b.t()
    inline void Rank1Sub(const Vector3<T> &a, const Vector3<T> &b)
    {
        M[0][0] -= a.x*b.x;        M[0][1] -= a.x*b.y;        M[0][2] -= a.x*b.z;
        M[1][0] -= a.y*b.x;        M[1][1] -= a.y*b.y;        M[1][2] -= a.y*b.z;
        M[2][0] -= a.z*b.x;        M[2][1] -= a.z*b.y;        M[2][2] -= a.z*b.z;
    }

    inline Vector3<T> Col(int c) const
    {
        return Vector3<T>(M[0][c], M[1][c], M[2][c]);
    }

    inline Vector3<T> Row(int r) const
    {
        return Vector3<T>(M[r][0], M[r][1], M[r][2]);
    }

    inline Vector3<T> GetColumn(int c) const
    {
        return Vector3<T>(M[0][c], M[1][c], M[2][c]);
    }

    inline Vector3<T> GetRow(int r) const
    {
        return Vector3<T>(M[r][0], M[r][1], M[r][2]);
    }

    inline void SetColumn(int c, const Vector3<T>& v)
    {
        M[0][c] = v.x;
        M[1][c] = v.y;
        M[2][c] = v.z;
    }

    inline void SetRow(int r, const Vector3<T>& v)
    {
        M[r][0] = v.x;
        M[r][1] = v.y;
        M[r][2] = v.z;
    }

    inline T Determinant() const
    {
        const Matrix3<T>& m = *this;
        T d; 

        d  = m.M[0][0] * (m.M[1][1]*m.M[2][2] - m.M[1][2] * m.M[2][1]);
        d -= m.M[0][1] * (m.M[1][0]*m.M[2][2] - m.M[1][2] * m.M[2][0]);
        d += m.M[0][2] * (m.M[1][0]*m.M[2][1] - m.M[1][1] * m.M[2][0]);

        return d;
    }
    
    inline Matrix3<T> Inverse() const
    {
        Matrix3<T> a;
        const  Matrix3<T>& m = *this;
        T d = Determinant();

        OVR_MATH_ASSERT(d != 0);
        T s = T(1)/d;

        a.M[0][0] = s * (m.M[1][1] * m.M[2][2] - m.M[1][2] * m.M[2][1]);   
        a.M[1][0] = s * (m.M[1][2] * m.M[2][0] - m.M[1][0] * m.M[2][2]);   
        a.M[2][0] = s * (m.M[1][0] * m.M[2][1] - m.M[1][1] * m.M[2][0]);   

        a.M[0][1] = s * (m.M[0][2] * m.M[2][1] - m.M[0][1] * m.M[2][2]);   
        a.M[1][1] = s * (m.M[0][0] * m.M[2][2] - m.M[0][2] * m.M[2][0]);   
        a.M[2][1] = s * (m.M[0][1] * m.M[2][0] - m.M[0][0] * m.M[2][1]);   
        
        a.M[0][2] = s * (m.M[0][1] * m.M[1][2] - m.M[0][2] * m.M[1][1]);   
        a.M[1][2] = s * (m.M[0][2] * m.M[1][0] - m.M[0][0] * m.M[1][2]);   
        a.M[2][2] = s * (m.M[0][0] * m.M[1][1] - m.M[0][1] * m.M[1][0]);   
        
        return a;
    }
    
    // Outer Product of two column vectors: a * b.Transpose()
    static Matrix3 OuterProduct(const Vector3<T>& a, const Vector3<T>& b)
    {
        return Matrix3(a.x*b.x, a.x*b.y, a.x*b.z,
                       a.y*b.x, a.y*b.y, a.y*b.z,
                       a.z*b.x, a.z*b.y, a.z*b.z);
    }

    // Vector cross product as a premultiply matrix:
    // L.Cross(R) = LeftCrossAsMatrix(L) * R
    static Matrix3 LeftCrossAsMatrix(const Vector3<T>& L)
    {
        return Matrix3(
            T(0), -L.z, +L.y,
            +L.z, T(0), -L.x,
            -L.y, +L.x, T(0));
    }

    // Vector cross product as a premultiply matrix:
    // L.Cross(R) = RightCrossAsMatrix(R) * L
    static Matrix3 RightCrossAsMatrix(const Vector3<T>& R)
    {
        return Matrix3(
            T(0), +R.z, -R.y,
            -R.z, T(0), +R.x,
            +R.y, -R.x, T(0));
    }

    // Angle in radians of a rotation matrix
    // Uses identity trace(a) = 2*cos(theta) + 1
    T Angle() const
    {
        return Acos((Trace() - T(1)) * T(0.5));
    }

    // Angle in radians between two rotation matrices
    T Angle(const Matrix3& b) const
    {
        // Compute trace of (this->Transposed() * b)
        // This works out to sum of products of elements.
        T trace = T(0);
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                trace += M[i][j] * b.M[i][j];
            }
        }
        return Acos((trace - T(1)) * T(0.5));
    }
};

typedef Matrix3<float>  Matrix3f;
typedef Matrix3<double> Matrix3d;

//-------------------------------------------------------------------------------------
// ***** Matrix2

template<class T>
class Matrix2
{
public:
    typedef T ElementType;
    static const size_t Dimension = 2;

    T M[2][2];

    enum NoInitType { NoInit };

    // Construct with no memory initialization.
    Matrix2(NoInitType) { }

    // By default, we construct identity matrix.
    Matrix2()
    {
        M[0][0] = M[1][1] = T(1);
        M[0][1] = M[1][0] = T(0);
    }

    Matrix2(T m11, T m12,
            T m21, T m22)
    {
        M[0][0] = m11; M[0][1] = m12;
        M[1][0] = m21; M[1][1] = m22;
    }

    // Construction from X, Y basis vectors
    Matrix2(const Vector2<T>& xBasis, const Vector2<T>& yBasis)
    {
        M[0][0] = xBasis.x; M[0][1] = yBasis.x;
        M[1][0] = xBasis.y; M[1][1] = yBasis.y;
    }

    explicit Matrix2(T s)
    {
        M[0][0] = M[1][1] = s;
        M[0][1] = M[1][0] = T(0);
    }

    Matrix2(T m11, T m22)
    {
        M[0][0] = m11; M[0][1] = T(0);
        M[1][0] = T(0);   M[1][1] = m22;
    }

    explicit Matrix2(const Matrix2<typename Math<T>::OtherFloatType> &src)
    {
        M[0][0] = T(src.M[0][0]); M[0][1] = T(src.M[0][1]);
        M[1][0] = T(src.M[1][0]); M[1][1] = T(src.M[1][1]);
    }

    // C-interop support
    Matrix2(const typename CompatibleTypes<Matrix2<T> >::Type& s)
    {
        OVR_MATH_STATIC_ASSERT(sizeof(s) == sizeof(Matrix2), "sizeof(s) == sizeof(Matrix2)");
        memcpy(M, s.M, sizeof(M));
    }

    operator const typename CompatibleTypes<Matrix2<T> >::Type() const
    {
        typename CompatibleTypes<Matrix2<T> >::Type result;
        OVR_MATH_STATIC_ASSERT(sizeof(result) == sizeof(Matrix2), "sizeof(result) == sizeof(Matrix2)");
        memcpy(result.M, M, sizeof(M));
        return result;
    }

    T  operator()(int i, int j) const { return M[i][j]; }
    T& operator()(int i, int j)       { return M[i][j]; }
    const T*  operator[](int i) const { return M[i]; }
    T*  operator[](int i)             { return M[i]; }

    static Matrix2 Identity()  { return Matrix2(); }

    void SetIdentity()
    {
        M[0][0] = M[1][1] = T(1);
        M[0][1] = M[1][0] = T(0);
    }

    static Matrix2 Diagonal(T m00, T m11)
    {
        return Matrix2(m00, m11);
    }
    static Matrix2 Diagonal(const Vector2<T>& v) { return Matrix2(v.x, v.y); }

    T Trace() const { return M[0][0] + M[1][1]; }

    bool operator== (const Matrix2& b) const
    {
        return M[0][0] == b.M[0][0] && M[0][1] == b.M[0][1] &&
               M[1][0] == b.M[1][0] && M[1][1] == b.M[1][1];
    }

    Matrix2 operator+ (const Matrix2& b) const
    {
        return Matrix2(M[0][0] + b.M[0][0], M[0][1] + b.M[0][1],
                       M[1][0] + b.M[1][0], M[1][1] + b.M[1][1]);
    }

    Matrix2& operator+= (const Matrix2& b)
    {
        M[0][0] += b.M[0][0]; M[0][1] += b.M[0][1];
        M[1][0] += b.M[1][0]; M[1][1] += b.M[1][1];
        return *this;
    }

    void operator= (const Matrix2& b)
    {
        M[0][0] = b.M[0][0]; M[0][1] = b.M[0][1];
        M[1][0] = b.M[1][0]; M[1][1] = b.M[1][1];
    }

    Matrix2 operator- (const Matrix2& b) const
    {
        return Matrix2(M[0][0] - b.M[0][0], M[0][1] - b.M[0][1],
                       M[1][0] - b.M[1][0], M[1][1] - b.M[1][1]);
    }

    Matrix2& operator-= (const Matrix2& b)
    {
        M[0][0] -= b.M[0][0]; M[0][1] -= b.M[0][1];
        M[1][0] -= b.M[1][0]; M[1][1] -= b.M[1][1];
        return *this;
    }

    Matrix2 operator* (const Matrix2& b) const
    {
        return Matrix2(M[0][0] * b.M[0][0] + M[0][1] * b.M[1][0], M[0][0] * b.M[0][1] + M[0][1] * b.M[1][1],
                       M[1][0] * b.M[0][0] + M[1][1] * b.M[1][0], M[1][0] * b.M[0][1] + M[1][1] * b.M[1][1]);
    }

    Matrix2& operator*= (const Matrix2& b)
    {
        *this = *this * b;
        return *this;
    }

    Matrix2 operator* (T s) const
    {
        return Matrix2(M[0][0] * s, M[0][1] * s,
                       M[1][0] * s, M[1][1] * s);
    }

    Matrix2& operator*= (T s)
    {
        M[0][0] *= s; M[0][1] *= s;
        M[1][0] *= s; M[1][1] *= s;
        return *this;
    }

    Matrix2 operator/ (T s) const
    {
        return *this * (T(1) / s);
    }

    Matrix2& operator/= (T s)
    {
        return *this *= (T(1) / s);
    }

    Vector2<T> operator* (const Vector2<T> &b) const
    {
        return Vector2<T>(M[0][0] * b.x + M[0][1] * b.y,
                          M[1][0] * b.x + M[1][1] * b.y);
    }

    Vector2<T> Transform(const Vector2<T>& v) const
    {
        return Vector2<T>(M[0][0] * v.x + M[0][1] * v.y,
                          M[1][0] * v.x + M[1][1] * v.y);
    }

    Matrix2 Transposed() const
    {
        return Matrix2(M[0][0], M[1][0],
                       M[0][1], M[1][1]);
    }

    void Transpose()
    {
        OVRMath_Swap(M[1][0], M[0][1]);
    }

    Vector2<T> GetColumn(int c) const
    {
        return Vector2<T>(M[0][c], M[1][c]);
    }

    Vector2<T> GetRow(int r) const
    {
        return Vector2<T>(M[r][0], M[r][1]);
    }

    void SetColumn(int c, const Vector2<T>& v)
    {
        M[0][c] = v.x;
        M[1][c] = v.y;
    }

    void SetRow(int r, const Vector2<T>& v)
    {
        M[r][0] = v.x;
        M[r][1] = v.y;
    }

    T Determinant() const
    {
        return M[0][0] * M[1][1] - M[0][1] * M[1][0];
    }

    Matrix2 Inverse() const
    {
        T rcpDet = T(1) / Determinant();
        return Matrix2( M[1][1] * rcpDet, -M[0][1] * rcpDet,
                       -M[1][0] * rcpDet,  M[0][0] * rcpDet);
    }

    // Outer Product of two column vectors: a * b.Transpose()
    static Matrix2 OuterProduct(const Vector2<T>& a, const Vector2<T>& b)
    {
        return Matrix2(a.x*b.x, a.x*b.y,
                       a.y*b.x, a.y*b.y);
    }

    // Angle in radians between two rotation matrices
    T Angle(const Matrix2& b) const
    {
        const Matrix2& a = *this;
        return Acos(a(0, 0)*b(0, 0) + a(1, 0)*b(1, 0));
    }
};

typedef Matrix2<float>  Matrix2f;
typedef Matrix2<double> Matrix2d;

//-------------------------------------------------------------------------------------

template<class T>
class SymMat3
{
private:
    typedef SymMat3<T> this_type;

public:
    typedef T Value_t;
    // Upper symmetric
    T v[6]; // _00 _01 _02 _11 _12 _22

    inline SymMat3() {}

    inline explicit SymMat3(T s)
    {
        v[0] = v[3] = v[5] = s;
        v[1] = v[2] = v[4] = T(0);
    }

    inline explicit SymMat3(T a00, T a01, T a02, T a11, T a12, T a22)
    {
        v[0] = a00; v[1] = a01; v[2] = a02;
        v[3] = a11; v[4] = a12;
        v[5] = a22;
    }

    // Cast to symmetric Matrix3
    operator Matrix3<T>() const
    {
        return Matrix3<T>(v[0], v[1], v[2],
                          v[1], v[3], v[4],
                          v[2], v[4], v[5]);
    }

    static inline int Index(unsigned int i, unsigned int j)
    {
        return (i <= j) ? (3*i - i*(i+1)/2 + j) : (3*j - j*(j+1)/2 + i);
    }

    inline T operator()(int i, int j) const { return v[Index(i,j)]; }
    
    inline T &operator()(int i, int j) { return v[Index(i,j)]; }

    inline this_type& operator+=(const this_type& b)
    {
        v[0]+=b.v[0];
        v[1]+=b.v[1];
        v[2]+=b.v[2];
        v[3]+=b.v[3];
        v[4]+=b.v[4];
        v[5]+=b.v[5];
        return *this;
    }

    inline this_type& operator-=(const this_type& b)
    {
        v[0]-=b.v[0];
        v[1]-=b.v[1];
        v[2]-=b.v[2];
        v[3]-=b.v[3];
        v[4]-=b.v[4];
        v[5]-=b.v[5];

        return *this;
    }

    inline this_type& operator*=(T s)
    {
        v[0]*=s;
        v[1]*=s;
        v[2]*=s;
        v[3]*=s;
        v[4]*=s;
        v[5]*=s;

        return *this;
    }
        
    inline SymMat3 operator*(T s) const
    {
        SymMat3 d;
        d.v[0] = v[0]*s; 
        d.v[1] = v[1]*s; 
        d.v[2] = v[2]*s; 
        d.v[3] = v[3]*s; 
        d.v[4] = v[4]*s; 
        d.v[5] = v[5]*s; 
                        
        return d;
    }

    // Multiplies two matrices into destination with minimum copying.
    static SymMat3& Multiply(SymMat3* d, const SymMat3& a, const SymMat3& b)
    {        
        // _00 _01 _02 _11 _12 _22

        d->v[0] = a.v[0] * b.v[0];
        d->v[1] = a.v[0] * b.v[1] + a.v[1] * b.v[3];
        d->v[2] = a.v[0] * b.v[2] + a.v[1] * b.v[4];
                    
        d->v[3] = a.v[3] * b.v[3];
        d->v[4] = a.v[3] * b.v[4] + a.v[4] * b.v[5];
                
        d->v[5] = a.v[5] * b.v[5];
    
        return *d;
    }
    
    inline T Determinant() const
    {
        const this_type& m = *this;
        T d; 

        d  = m(0,0) * (m(1,1)*m(2,2) - m(1,2) * m(2,1));
        d -= m(0,1) * (m(1,0)*m(2,2) - m(1,2) * m(2,0));
        d += m(0,2) * (m(1,0)*m(2,1) - m(1,1) * m(2,0));

        return d;
    }

    inline this_type Inverse() const
    {
        this_type a;
        const this_type& m = *this;
        T d = Determinant();

        OVR_MATH_ASSERT(d != 0);
        T s = T(1)/d;

        a(0,0) = s * (m(1,1) * m(2,2) - m(1,2) * m(2,1));   

        a(0,1) = s * (m(0,2) * m(2,1) - m(0,1) * m(2,2));   
        a(1,1) = s * (m(0,0) * m(2,2) - m(0,2) * m(2,0));   

        a(0,2) = s * (m(0,1) * m(1,2) - m(0,2) * m(1,1));   
        a(1,2) = s * (m(0,2) * m(1,0) - m(0,0) * m(1,2));   
        a(2,2) = s * (m(0,0) * m(1,1) - m(0,1) * m(1,0));   

        return a;
    }

    inline T Trace() const { return v[0] + v[3] + v[5]; }

    // M = a*a.t()
    inline void Rank1(const Vector3<T> &a)
    {
        v[0] = a.x*a.x; v[1] = a.x*a.y; v[2] = a.x*a.z;
        v[3] = a.y*a.y; v[4] = a.y*a.z;
        v[5] = a.z*a.z;
    }

    // M += a*a.t()
    inline void Rank1Add(const Vector3<T> &a)
    {
        v[0] += a.x*a.x; v[1] += a.x*a.y; v[2] += a.x*a.z;
        v[3] += a.y*a.y; v[4] += a.y*a.z;
        v[5] += a.z*a.z;
    }

    // M -= a*a.t()
    inline void Rank1Sub(const Vector3<T> &a)
    {
        v[0] -= a.x*a.x; v[1] -= a.x*a.y; v[2] -= a.x*a.z;
        v[3] -= a.y*a.y; v[4] -= a.y*a.z;
        v[5] -= a.z*a.z;
    }
};

typedef SymMat3<float>  SymMat3f;
typedef SymMat3<double> SymMat3d;

template<class T>
inline Matrix3<T> operator*(const SymMat3<T>& a, const SymMat3<T>& b)
{
    #define AJB_ARBC(r,c) (a(r,0)*b(0,c)+a(r,1)*b(1,c)+a(r,2)*b(2,c))
    return Matrix3<T>(
        AJB_ARBC(0,0), AJB_ARBC(0,1), AJB_ARBC(0,2),
        AJB_ARBC(1,0), AJB_ARBC(1,1), AJB_ARBC(1,2),
        AJB_ARBC(2,0), AJB_ARBC(2,1), AJB_ARBC(2,2));
    #undef AJB_ARBC
}

template<class T>
inline Matrix3<T> operator*(const Matrix3<T>& a, const SymMat3<T>& b)
{
    #define AJB_ARBC(r,c) (a(r,0)*b(0,c)+a(r,1)*b(1,c)+a(r,2)*b(2,c))
    return Matrix3<T>(
        AJB_ARBC(0,0), AJB_ARBC(0,1), AJB_ARBC(0,2),
        AJB_ARBC(1,0), AJB_ARBC(1,1), AJB_ARBC(1,2),
        AJB_ARBC(2,0), AJB_ARBC(2,1), AJB_ARBC(2,2));
    #undef AJB_ARBC
}

//-------------------------------------------------------------------------------------
// ***** Angle

// Cleanly representing the algebra of 2D rotations.
// The operations maintain the angle between -Pi and Pi, the same range as atan2.

template<class T>
class Angle
{
public:
    enum AngularUnits
    {
        Radians = 0,
        Degrees = 1
    };

    Angle() : a(0) {}
    
    // Fix the range to be between -Pi and Pi
    Angle(T a_, AngularUnits u = Radians) : a((u == Radians) ? a_ : a_*((T)MATH_DOUBLE_DEGREETORADFACTOR)) { FixRange(); }

    T    Get(AngularUnits u = Radians) const       { return (u == Radians) ? a : a*((T)MATH_DOUBLE_RADTODEGREEFACTOR); }
    void Set(const T& x, AngularUnits u = Radians) { a = (u == Radians) ? x : x*((T)MATH_DOUBLE_DEGREETORADFACTOR); FixRange(); }
    int Sign() const                               { if (a == 0) return 0; else return (a > 0) ? 1 : -1; }
    T   Abs() const                                { return (a >= 0) ? a : -a; }

    bool operator== (const Angle& b) const    { return a == b.a; }
    bool operator!= (const Angle& b) const    { return a != b.a; }
//    bool operator<  (const Angle& b) const    { return a < a.b; } 
//    bool operator>  (const Angle& b) const    { return a > a.b; } 
//    bool operator<= (const Angle& b) const    { return a <= a.b; } 
//    bool operator>= (const Angle& b) const    { return a >= a.b; } 
//    bool operator= (const T& x)               { a = x; FixRange(); }

    // These operations assume a is already between -Pi and Pi.
    Angle& operator+= (const Angle& b)        { a = a + b.a; FastFixRange(); return *this; }
    Angle& operator+= (const T& x)            { a = a + x; FixRange(); return *this; }
    Angle  operator+  (const Angle& b) const  { Angle res = *this; res += b; return res; }
    Angle  operator+  (const T& x) const      { Angle res = *this; res += x; return res; }
    Angle& operator-= (const Angle& b)        { a = a - b.a; FastFixRange(); return *this; }
    Angle& operator-= (const T& x)            { a = a - x; FixRange(); return *this; }
    Angle  operator-  (const Angle& b) const  { Angle res = *this; res -= b; return res; }
    Angle  operator-  (const T& x) const      { Angle res = *this; res -= x; return res; }
    
    T   Distance(const Angle& b)              { T c = fabs(a - b.a); return (c <= ((T)MATH_DOUBLE_PI)) ? c : ((T)MATH_DOUBLE_TWOPI) - c; }

private:

    // The stored angle, which should be maintained between -Pi and Pi
    T a;

    // Fixes the angle range to [-Pi,Pi], but assumes no more than 2Pi away on either side 
    inline void FastFixRange()
    {
        if (a < -((T)MATH_DOUBLE_PI))
            a += ((T)MATH_DOUBLE_TWOPI);
        else if (a > ((T)MATH_DOUBLE_PI))
            a -= ((T)MATH_DOUBLE_TWOPI);
    }

    // Fixes the angle range to [-Pi,Pi] for any given range, but slower then the fast method
    inline void FixRange()
    {
        // do nothing if the value is already in the correct range, since fmod call is expensive
        if (a >= -((T)MATH_DOUBLE_PI) && a <= ((T)MATH_DOUBLE_PI))
            return;
        a = fmod(a,((T)MATH_DOUBLE_TWOPI));
        if (a < -((T)MATH_DOUBLE_PI))
            a += ((T)MATH_DOUBLE_TWOPI);
        else if (a > ((T)MATH_DOUBLE_PI))
            a -= ((T)MATH_DOUBLE_TWOPI);
    }
};


typedef Angle<float>  Anglef;
typedef Angle<double> Angled;


//-------------------------------------------------------------------------------------
// ***** Plane

// Consists of a normal vector and distance from the origin where the plane is located.

template<class T>
class Plane
{
public:
    Vector3<T> N;
    T          D;

    Plane() : D(0) {}

    // Normals must already be normalized
    Plane(const Vector3<T>& n, T d) : N(n), D(d) {}
    Plane(T x, T y, T z, T d) : N(x,y,z), D(d) {}

    // construct from a point on the plane and the normal
    Plane(const Vector3<T>& p, const Vector3<T>& n) : N(n), D(-(p * n)) {}

    // Find the point to plane distance. The sign indicates what side of the plane the point is on (0 = point on plane).
    T TestSide(const Vector3<T>& p) const
    {
        return (N.Dot(p)) + D;
    }

    Plane<T> Flipped() const
    {
        return Plane(-N, -D);
    }

    void Flip()
    {
        N = -N;
        D = -D;
    }

    bool operator==(const Plane<T>& rhs) const
    {
        return (this->D == rhs.D && this->N == rhs.N);
    }
};

typedef Plane<float> Planef;
typedef Plane<double> Planed;




//-----------------------------------------------------------------------------------
// ***** ScaleAndOffset2D

struct ScaleAndOffset2D
{
    Vector2f Scale;
    Vector2f Offset;

    ScaleAndOffset2D(float sx = 0.0f, float sy = 0.0f, float ox = 0.0f, float oy = 0.0f)
        : Scale(sx, sy), Offset(ox, oy)        
    { }
};


//-----------------------------------------------------------------------------------
// ***** FovPort

// FovPort describes Field Of View (FOV) of a viewport.
// This class has values for up, down, left and right, stored in 
// tangent of the angle units to simplify calculations.
//
// As an example, for a standard 90 degree vertical FOV, we would 
// have: { UpTan = tan(90 degrees / 2), DownTan = tan(90 degrees / 2) }.
//
// CreateFromRadians/Degrees helper functions can be used to
// access FOV in different units.


// ***** FovPort

struct FovPort
{
    float UpTan;
    float DownTan;
    float LeftTan;
    float RightTan;

    FovPort ( float sideTan = 0.0f ) :
        UpTan(sideTan), DownTan(sideTan), LeftTan(sideTan), RightTan(sideTan) { }
    FovPort ( float u, float d, float l, float r ) :
        UpTan(u), DownTan(d), LeftTan(l), RightTan(r) { }

    // C-interop support: FovPort <-> ovrFovPort (implementation in OVR_CAPI.cpp).
    FovPort(const ovrFovPort &src)
        : UpTan(src.UpTan), DownTan(src.DownTan), LeftTan(src.LeftTan), RightTan(src.RightTan)
    { }    

    operator ovrFovPort () const
    {
        ovrFovPort result;
        result.LeftTan  = LeftTan;
        result.RightTan = RightTan;
        result.UpTan    = UpTan;
        result.DownTan  = DownTan;
        return result;
    }

    static FovPort CreateFromRadians(float horizontalFov, float verticalFov)
    {
        FovPort result;
        result.UpTan    = tanf (   verticalFov * 0.5f );
        result.DownTan  = tanf (   verticalFov * 0.5f );
        result.LeftTan  = tanf ( horizontalFov * 0.5f );
        result.RightTan = tanf ( horizontalFov * 0.5f );
        return result;
    }

    static FovPort CreateFromDegrees(float horizontalFovDegrees,
                                     float verticalFovDegrees)
    {
        return CreateFromRadians(DegreeToRad(horizontalFovDegrees),
                                 DegreeToRad(verticalFovDegrees));
    }

    //  Get Horizontal/Vertical components of Fov in radians.
    float GetVerticalFovRadians() const     { return atanf(UpTan)    + atanf(DownTan); }
    float GetHorizontalFovRadians() const   { return atanf(LeftTan)  + atanf(RightTan); }
    //  Get Horizontal/Vertical components of Fov in degrees.
    float GetVerticalFovDegrees() const     { return RadToDegree(GetVerticalFovRadians()); }
    float GetHorizontalFovDegrees() const   { return RadToDegree(GetHorizontalFovRadians()); }

    // Compute maximum tangent value among all four sides.
    float GetMaxSideTan() const
    {
        return OVRMath_Max(OVRMath_Max(UpTan, DownTan), OVRMath_Max(LeftTan, RightTan));
    }

    static ScaleAndOffset2D CreateNDCScaleAndOffsetFromFov ( FovPort tanHalfFov )
    {
        float projXScale = 2.0f / ( tanHalfFov.LeftTan + tanHalfFov.RightTan );
        float projXOffset = ( tanHalfFov.LeftTan - tanHalfFov.RightTan ) * projXScale * 0.5f;
        float projYScale = 2.0f / ( tanHalfFov.UpTan + tanHalfFov.DownTan );
        float projYOffset = ( tanHalfFov.UpTan - tanHalfFov.DownTan ) * projYScale * 0.5f;

        ScaleAndOffset2D result;
        result.Scale    = Vector2f(projXScale, projYScale);
        result.Offset   = Vector2f(projXOffset, projYOffset);
        // Hey - why is that Y.Offset negated?
        // It's because a projection matrix transforms from world coords with Y=up,
        // whereas this is from NDC which is Y=down.

        return result;
    }

    // Converts Fov Tan angle units to [-1,1] render target NDC space
    Vector2f TanAngleToRendertargetNDC(Vector2f const &tanEyeAngle)
    {  
        ScaleAndOffset2D eyeToSourceNDC = CreateNDCScaleAndOffsetFromFov(*this);
        return tanEyeAngle * eyeToSourceNDC.Scale + eyeToSourceNDC.Offset;
    }

    // Compute per-channel minimum and maximum of Fov.
    static FovPort Min(const FovPort& a, const FovPort& b)
    {   
        FovPort fov( OVRMath_Min( a.UpTan   , b.UpTan    ),   
                     OVRMath_Min( a.DownTan , b.DownTan  ),
                     OVRMath_Min( a.LeftTan , b.LeftTan  ),
                     OVRMath_Min( a.RightTan, b.RightTan ) );
        return fov;
    }

    static FovPort Max(const FovPort& a, const FovPort& b)
    {   
        FovPort fov( OVRMath_Max( a.UpTan   , b.UpTan    ),   
                     OVRMath_Max( a.DownTan , b.DownTan  ),
                     OVRMath_Max( a.LeftTan , b.LeftTan  ),
                     OVRMath_Max( a.RightTan, b.RightTan ) );
        return fov;
    }
};


} // Namespace OVR


#if defined(_MSC_VER)
    #pragma warning(pop)
#endif


#endif
