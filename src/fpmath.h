#ifndef FPMATH_H
#define FPMATH_H

#define FPT_WBITS  17
#define FPT_BITS  32

#define FPT_FBITS  (FPT_BITS - FPT_WBITS)
#define FPT_FMASK  (((fpt)1 << FPT_FBITS) - 1)

typedef int fixed32_t;
typedef long long int fixed64_t;
typedef unsigned int ufixed32_t;
typedef unsigned long long int ufixed64_t;

#define FTOFIX(R) ((fixed32_t)((R) * FPT_ONE + ((R) >= 0 ? 0.5 : -0.5)))
#define ITOFIX(I) ((fixed64_t)(I) << FPT_FBITS)
#define FIXTOI(F) ((F) >> FPT_FBITS)

#define FPT_ONE       ((fixed32_t)((fixed32_t)1 << FPT_FBITS))
#define FPT_ZERO      ((fixed32_t)0)
#define FPT_MINUS_ONE (-FPT_ONE)
#define FPT_ONE_HALF  (FPT_ONE >> 1)
#define FPT_TWO       (FPT_ONE + FPT_ONE)
#define FPT_MAX       ((fixed32_t)((ufixed32_t)~0 >> 1))
#define FPT_MIN       (~FPT_MAX)
#define FPT_ABS_MAX   FPT_MAX
#define FPT_ABS_MIN   ((fixed32_t)1)
#define FPT_PI        FTOFIX(3.14159265358979323846)
#define FPT_TWO_PI    FTOFIX(2 * 3.14159265358979323846)
#define FPT_HALF_PI   FTOFIX(3.14159265358979323846 / 2)
#define FPT_ONE_OVER_PI FTOFIX(1.0f / 3.14159265358979323846f)
#define FPT_E         FTOFIX(2.7182818284590452354)

#define F_PI 3.14159265358979323846f

#define _fpt_add_overflow_handler return FPT_MAX;
#define _fpt_add_underflow_handler return FPT_MIN;

#define _fpt_sub_overflow_handler return FPT_MAX;
#define _fpt_sub_underflow_handler return FPT_MIN;

#define _fpt_mul_overflow_handler return FPT_MAX;
#define _fpt_mul_underflow_handler return FPT_MIN;

#define _fpt_div_overflow_handler return FPT_MAX;
#define _fpt_div_underflow_handler return FPT_MIN;

#define FIXTOF(T) ((float) ((T)*((float)(1)/(float)(1 << FPT_FBITS))))

#define fpt_abs(A) ((A) < 0 ? -(A) : (A))

fixed32_t fix_mul(fixed32_t A, fixed32_t B);
fixed32_t fix_div(fixed32_t A, fixed32_t B);

fixed32_t sqrt(fixed32_t A);

fixed32_t sin(fixed32_t fp);
fixed32_t cos(fixed32_t A);
fixed32_t tan(fixed32_t A);

fixed32_t max(fixed32_t a, fixed32_t b);
fixed32_t min(fixed32_t a, fixed32_t b);

fixed32_t floor(fixed32_t a);
fixed32_t fract(fixed32_t a);

fixed32_t mix(fixed32_t x, fixed32_t y, fixed32_t a);

float sinf(float x);
float cosf(float x);
float tanf(float x);

fixed32_t deg_to_rad(fixed32_t deg);
fixed32_t rad_to_deg(fixed32_t rad);

fixed32_t sign(fixed32_t x);

fixed32_t lerp(fixed32_t start, fixed32_t end, fixed32_t t);

typedef struct vec2 {
    fixed32_t x;
    fixed32_t y;
} vec2;
 
vec2 vec2_neg(vec2 a);
vec2 vec2_add(vec2 a, vec2 b);
vec2 vec2_minus(vec2 a, vec2 b);
vec2 vec2_mul(vec2 a, vec2 b);
vec2 vec2_mul_s(vec2 a, fixed32_t b);
vec2 vec2_div(vec2 a, vec2 b);
vec2 vec2_div_s(vec2 a, fixed32_t b);

vec2 vec2_normalize(vec2 a);
fixed32_t vec2_dot(vec2 a, vec2 b);
fixed32_t vec2_length(vec2 a);

typedef struct vec3 {
    fixed32_t x;
    fixed32_t y;
    fixed32_t z;
} vec3;

vec3 vec3_from_s(fixed32_t a);
vec3 vec3_neg(vec3 a);
vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_minus(vec3 a, vec3 b);
vec3 vec3_mul(vec3 a, vec3 b);
vec3 vec3_mul_s(vec3 a, fixed32_t b);
vec3 vec3_div(vec3 a, vec3 b);
vec3 vec3_div_s(vec3 a, fixed32_t b);
vec3 vec3_normalize(vec3 a);
fixed32_t dot(vec3 a, vec3 b);
vec3 vec3_reflect(vec3 i, vec3 n);
fixed32_t vec3_length(vec3 a);
vec3 vec3_lerp(vec3 start, vec3 end, fixed32_t t);

typedef struct vec4 {
    fixed32_t x;
    fixed32_t y;
    fixed32_t z;
    fixed32_t w;
} vec4;

vec3 xyz(vec4 a);

typedef struct mat4 {
    vec4 m1;
    vec4 m2;
    vec4 m3;
    vec4 m4;
} mat4;

mat4 mat4_0();
mat4 translate(vec3 delta);
mat4 rotation(fixed32_t pitchRad);
mat4 identity();
vec4 vec4_from_vec3(vec3 a, fixed32_t w);
vec4 vec4_add(vec4 a, vec4 b);
vec4 vec4_mul(vec4 a, vec4 b);
vec4 vec4_mul_s(vec4 a, fixed32_t b);
fixed32_t vec4_dot(vec4 a, vec4 b);
vec4 mat4_mul_vec4(mat4 a, vec4 b);

#endif