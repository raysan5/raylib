#include "raymath.h"

void rl_tgempty(void);

#if __STDC_VERSION__ < 201112L
#warning _Generic is a C11 directive and requires at least C11 compatibility.
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - TGVector
//----------------------------------------------------------------------------------

#define RLEquals(a, b) _Generic((a), \
	Vector2: _Generic((b),           \
		Vector2: Vector2Equals,      \
		default: rl_tgempty          \
	),                               \
	Vector3: _Generic((b),           \
		Vector3: Vector3Equals,      \
		default: rl_tgempty          \
	),                               \
	Vector4: _Generic((b),           \
		Vector4: Vector4Equals,      \
		default: rl_tgempty          \
	),                               \
	default:   _Generic((b),         \
		default:   FloatEquals       \
	)                                \
) (a, b)

#define RLClamp(val, min, max) _Generic((val), \
	Vector2: _Generic((min),                   \
		Vector2:   _Generic((max),             \
			Vector2: Vector2Clamp,             \
			default: rl_tgempty                \
		),                                     \
		default: _Generic((max),               \
			default: Vector2ClampValue         \
		)                                      \
	),                                         \
	Vector3: _Generic((min),                   \
		Vector3:   _Generic((max),             \
			Vector3: Vector3Clamp,             \
			default:   rl_tgempty              \
		),                                     \
		default: _Generic((max),               \
			default: Vector3ClampValue         \
		)                                      \
	),                                         \
	default:   _Generic((min),                 \
		default:   _Generic((max),             \
			default: Clamp                     \
		)                                      \
	)                                          \
) (val, min, max)

#define RLLerp(a, b, amount)  _Generic((a), \
	Vector2:  _Generic((b),                 \
		Vector2: Vector2Lerp,               \
		default: rl_tgempty                 \
	),                                      \
	Vector3:  _Generic((b),                 \
		Vector3: Vector3Lerp,               \
		default: rl_tgempty                 \
	),                                      \
	Vector4:  _Generic((b),                 \
		Vector4: Vector4Lerp,               \
		default: rl_tgempty                 \
	),                                      \
	default: _Generic((b),                  \
		default: Lerp                       \
	)                                       \
) (a, b, amount)

#define VectorZero(v) _Generic((v), \
	Vector2: Vector2Zero,           \
	Vector3: Vector3Zero,           \
	Vector4: Vector4Zero            \
) ()

#define VectorOne(v) _Generic((v), \
	Vector2: Vector2One,           \
	Vector3: Vector3One,           \
	Vector4: Vector4One            \
) ()

#define VectorAdd(v, x) _Generic((v), \
	Vector2: _Generic((x),            \
		Vector2: Vector2Add,          \
		default: Vector2AddValue      \
	),                                \
	Vector3: _Generic((x),            \
		Vector3: Vector3Add,          \
		default: Vector3AddValue      \
	),                                \
	Vector4: _Generic((x),            \
		Vector4: Vector4Add,          \
		default: Vector4AddValue      \
	)                                 \
) (v, x)

#define VectorSubtract(v, x) _Generic((v), \
	Vector2: _Generic((x),                 \
		Vector2: Vector2Subtract,          \
		default: Vector2SubtractValue      \
	),                                     \
	Vector3: _Generic((x),                 \
		Vector3: Vector3Subtract,          \
		default: Vector3SubtractValue      \
	),                                     \
	Vector4: _Generic((x),                 \
		Vector4: Vector4Subtract,          \
		default: Vector4SubtractValue      \
	)                                      \
) (v, x)

#define VectorLength(v) _Generic((v), \
	Vector2: Vector2Length,           \
	Vector3: Vector3Length,           \
	Vector4: Vector4Length            \
) (v)

#define VectorLengthSqr(v) _Generic((v), \
	Vector2: Vector2LengthSqr,           \
	Vector3: Vector3LengthSqr,           \
	Vector4: Vector4LengthSqr            \
) (v)

#define VectorDotProduct(v1, v2) _Generic((v1), \
	Vector2: _Generic((v2),                     \
		Vector2: Vector2DotProduct,             \
		default: rl_tgempty                     \
	),                                          \
	Vector3: _Generic((v2),                     \
		Vector3: Vector3DotProduct,             \
		default: rl_tgempty                     \
	),                                          \
	Vector4: _Generic((v2),                     \
		Vector4: Vector4DotProduct,             \
		default: rl_tgempty                     \
	)                                           \
) (v1, v2)

#define VectorDistance(v1, v2) _Generic((v1), \
	Vector2: _Generic((v2),                   \
		Vector2: Vector2Distance,             \
		default: rl_tgempty                   \
	),                                        \
	Vector3: _Generic((v2),                   \
		Vector3: Vector3Distance,             \
		default: rl_tgempty                   \
	),                                        \
	Vector4: _Generic((v2),                   \
		Vector4: Vector4Distance,             \
		default: rl_tgempty                   \
	)                                         \
) (v1, v2)

#define VectorDistanceSqr(v1, v2) _Generic((v1), \
	Vector2: _Generic((v2),                      \
	    Vector2: Vector2DistanceSqr,             \
		default: rl_tgempty                      \
	),                                           \
	Vector3: _Generic((v2),                      \
	    Vector3: Vector3DistanceSqr,             \
		default: rl_tgempty                      \
	),                                           \
	Vector4: _Generic((v2),                      \
	    Vector4: Vector4DistanceSqr,             \
		default: rl_tgempty                      \
	)                                            \
) (v1, v2)

#define VectorAngle(v1, v2) _Generic((v1), \
	Vector2: _Generic((v2),                \
	    Vector2: Vector2Angle,             \
		default: rl_tgempty                \
	),                                     \
	Vector3: _Generic((v2),                \
	    Vector3: Vector3Angle,             \
		default: rl_tgempty                \
	)                                      \
) (v1, v2)

#define VectorScale(v, scalar) _Generic((v), \
	Vector2: Vector2Scale,                   \
	Vector3: Vector3Scale,                   \
	Vector4: Vector4Scale                    \
) (v, scalar)

#define VectorMultiply(v1, v2) _Generic((v1), \
	Vector2: _Generic((v2),                   \
		Vector2: Vector2Multiply,             \
		default: rl_tgempty                   \
	),                                        \
	Vector3: _Generic((v2),                   \
		Vector3: Vector3Multiply,             \
		default: rl_tgempty                   \
	),                                        \
	Vector4: _Generic((v2),                   \
		Vector4: Vector4Multiply,             \
		default: rl_tgempty                   \
	)                                         \
) (v1, v2)

#define VectorNegate(v) _Generic((v), \
	Vector2: Vector2Negate,           \
	Vector3: Vector3Negate,           \
	Vector4: Vector4Negate            \
) (v)

#define VectorDivide(v1, v2) _Generic((v1), \
	Vector2: _Generic((v2),                 \
	    Vector2: Vector2Divide,             \
		default: rl_tgempty                 \
	),                                      \
	Vector3: _Generic((v2),                 \
	    Vector3: Vector3Divide,             \
		default: rl_tgempty                 \
	),                                      \
	Vector4: _Generic((v2),                 \
	    Vector4: Vector4Divide,             \
		default: rl_tgempty                 \
	)                                       \
) (v1, v2)

#define VectorNormalize(v) _Generic((v), \
	Vector2: Vector2Normalize,           \
	Vector3: Vector3Normalize,           \
	Vector4: Vector4Normalize            \
) (v)

#define VectorTransform(v, m) _Generic((v), \
	Vector2: Vector2Transform,              \
	Vector3: Vector3Transform               \
) (v, m)

#define VectorReflect(v, normal) _Generic((v), \
	Vector2: _Generic((normal),                \
		Vector2: Vector2Reflect,               \
		default: rl_tgempty                    \
	),                                         \
	Vector3: _Generic((normal),                \
		Vector3: Vector3Reflect,               \
		default: rl_tgempty                    \
	)                                          \
) (v, normal)

#define VectorMin(v1, v2) _Generic((v1), \
	Vector2: _Generic((v2),              \
		Vector2: Vector2Min,             \
		default: rl_tgempty              \
	),                                   \
	Vector3: _Generic((v2),              \
		Vector3: Vector3Min,             \
		default: rl_tgempty              \
	),                                   \
	Vector4: _Generic((v2),              \
		Vector4: Vector4Min,             \
		default: rl_tgempty              \
	)                                    \
) (v1, v2)

#define VectorMax(v1, v2) _Generic((v1), \
	Vector2: _Generic((v2),              \
		Vector2: Vector2Max,             \
		default: rl_tgempty              \
	),                                   \
	Vector3: _Generic((v2),              \
		Vector3: Vector3Max,             \
		default: rl_tgempty              \
	),                                   \
	Vector4: _Generic((v2),              \
		Vector4: Vector4Max,             \
		default: rl_tgempty              \
	)                                    \
) (v1, v2)

/*
 * Macro disabled due to too big a difference in rotation semantics.
#define VectorRotate(v, f) _Generic((f), \
	Quaternion: _Generic((v),				 \
		Vector3: Vector3RotateByQuaternion,          \
		default: rl_tgempty\
	), \
	default: _Generic((v), \
		Vector2: Vector2Rotate, \
		default: rl_tgempty \
	)\
) (v, f)
*/

#define VectorMoveTowards(v, target, d) _Generic((v), \
	Vector2:  _Generic((target),                      \
	    Vector2: Vector2MoveTowards,                  \
		default: rl_tgempty                           \
	),                                                \
	Vector3:  _Generic((target),                      \
	    Vector3: Vector3MoveTowards,                  \
		default: rl_tgempty                           \
	),                                                \
	Vector4:  _Generic((target),                      \
	    Vector4: Vector4MoveTowards,                  \
		default: rl_tgempty                           \
	)                                                 \
) (v, target, d)

#define VectorInvert(v) _Generic((v), \
	Vector2: Vector2Invert,           \
	Vector3: Vector3Invert,           \
	Vector4: Vector4Invert            \
) (v)

#define VectorRefract(v, n, ratio) _Generic((v), \
	Vector2:  _Generic((n),                      \
	    Vector2: Vector2Refract,                 \
		default: rl_tgempty                      \
	),                                           \
	Vector3:  _Generic((n),                      \
	    Vector3: Vector3Refract,                 \
		default: rl_tgempty                      \
	)                                            \
) (v, n, ratio)

