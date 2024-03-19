#include "./fpmath.h"

fixed32_t fix_mul(fixed32_t A, fixed32_t B) {
    return (((fixed64_t)A * (fixed64_t)B) >> FPT_FBITS);
}

fixed32_t fix_div(fixed32_t A, fixed32_t B) {
    if (B == FPT_ZERO) {
        return (fixed64_t)FPT_MAX;
    }

    return (((fixed64_t)A << FPT_FBITS) / (fixed64_t)B);
}

fixed32_t sqrt(fixed32_t A)
{
    int invert = 0;
    int iter = FPT_FBITS;
    int l, i;

    if (A < 0)
        return (-1);
    if (A == 0 || A == FPT_ONE)
        return (A);
    if (A < FPT_ONE && A > 6) {
        invert = 1;
        A = fix_div(FPT_ONE, A);
    }
    if (A > FPT_ONE) {
        int s = A;

        iter = 0;
        while (s > 0) {
            s >>= 2;
            iter++;
      }
    }

    /* Newton's iterations */
    l = (A >> 1) + 1;
    for (i = 0; i < iter; i++)
        l = (l + fix_div(A, l)) >> 1;
    if (invert)
        return (fix_div(FPT_ONE, l));
    return (l);
}

fixed32_t sin(fixed32_t fp)
{
    int sign = 1;
    fixed32_t sqr, result;
    const fixed32_t SK[2] = {
      FTOFIX(7.61e-03),
      FTOFIX(1.6605e-01)
    };

    fp %= 2 * FPT_PI;
    if (fp < 0)
      fp = FPT_PI * 2 + fp;
    if ((fp > FPT_HALF_PI) && (fp <= FPT_PI)) 
      fp = FPT_PI - fp;
    else if ((fp > FPT_PI) && (fp <= (FPT_PI + FPT_HALF_PI))) {
      fp = fp - FPT_PI;
      sign = -1;
    } else if (fp > (FPT_PI + FPT_HALF_PI)) {
      fp = (FPT_PI << 1) - fp;
      sign = -1;
    }
    sqr = fix_mul(fp, fp);
    result = SK[0];
    result = fix_mul(result, sqr);
    result -= SK[1];
    result = fix_mul(result, sqr);
    result += FPT_ONE;
    result = fix_mul(result, fp);
    return sign * result;
}

fixed32_t cos(fixed32_t A)
{
    return (sin(FPT_HALF_PI - A));
}

fixed32_t tan(fixed32_t A)
{
    return fix_div(sin(A), cos(A));
}

fixed32_t max(fixed32_t a, fixed32_t b) {
    if (a > b) return a;
    else return b;
}

fixed32_t min(fixed32_t a, fixed32_t b) {
    if (a < b) return a;
    else return b;
}

fixed32_t floor(fixed32_t a) {
    return (a >> FPT_FBITS) << FPT_FBITS;
}

fixed32_t fract(fixed32_t a) {
    return a - floor(a);
}

fixed32_t mix(fixed32_t x, fixed32_t y, fixed32_t a) {
    return fix_mul(x, FPT_ONE - a) + fix_mul(y, a);
}

float sinf(float x) {
    double sign = 1;
    if (x < 0) {
        sign = -1.0;
        x = -x;
    }
    if (x > 360) x -= (x / 360) * 360;
    x *= F_PI / 180.0;
    double res = 0;
    double term = x;
    int k = 1;
    while (res + term != res) {
        res += term;
        k += 2;
        term *= -x * x / k / (k - 1);
    }

    return sign * res;
}

float cosf(float x) {
    return sinf((F_PI / 2) - x);
}

float tanf(float x) {
    return sinf(x) / cosf(x);
}

fixed32_t deg_to_rad(fixed32_t deg) { return fix_mul(deg, fix_div(FPT_PI, 46080)); }
fixed32_t rad_to_deg(fixed32_t rad) { return fix_mul(rad, fix_div(46080, FPT_PI)); }

fixed32_t sign(fixed32_t x) { return ITOFIX((x > 0) - (x < 0)); }

fixed32_t lerp(fixed32_t start, fixed32_t end, fixed32_t t) {
    return start + fix_mul(end - start, t);
}
 
vec2 vec2_neg(vec2 a) { return (vec2){-a.x, -a.y}; }

vec2 vec2_add(vec2 a, vec2 b) {
    return (vec2){a.x + b.x, a.y + b.y};
}

vec2 vec2_minus(vec2 a, vec2 b) {
    return (vec2){a.x - b.x, a.y - b.y};
}

vec2 vec2_mul(vec2 a, vec2 b) {
    return (vec2){fix_mul(a.x, b.x), fix_mul(a.y, b.y)};
}

vec2 vec2_mul_s(vec2 a, fixed32_t b) {
    return (vec2){fix_mul(a.x, b), fix_mul(a.y, b)};
}

vec2 vec2_div(vec2 a, vec2 b) {
    return (vec2){fix_div(a.x, b.x), fix_div(a.y, b.y)};
}

vec2 vec2_div_s(vec2 a, fixed32_t b) {
    return (vec2){fix_div(a.x, b), fix_div(a.y, b)};
}

vec2 vec2_normalize(vec2 a) {
    fixed32_t l = sqrt(fix_mul(a.x, a.x) + fix_mul(a.y, a.y));
    return (vec2){fix_div(a.x, l), fix_div(a.y, l)};
}

fixed32_t vec2_dot(vec2 a, vec2 b) {
    return fix_mul(a.x, b.x) + fix_mul(a.y, b.y);
}

fixed32_t vec2_length(vec2 a) {
    return sqrt(fix_mul(a.x, a.x) + fix_mul(a.y, a.y));
}

vec3 vec3_from_s(fixed32_t a) {
    return (vec3){a, a, a};
}

vec3 vec3_neg(vec3 a) { return (vec3){-a.x, -a.y, -a.z}; }

vec3 vec3_add(vec3 a, vec3 b) {
    return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

vec3 vec3_minus(vec3 a, vec3 b) {
    return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

vec3 vec3_mul(vec3 a, vec3 b) {
    return (vec3){fix_mul(a.x, b.x), fix_mul(a.y, b.y), fix_mul(a.z, b.z)};
}

vec3 vec3_mul_s(vec3 a, fixed32_t b) {
    return (vec3){fix_mul(a.x, b), fix_mul(a.y, b), fix_mul(a.z, b)};
}

vec3 vec3_div(vec3 a, vec3 b) {
    return (vec3){fix_div(a.x, b.x), fix_div(a.y, b.y), fix_div(a.z, b.z)};
}

vec3 vec3_div_s(vec3 a, fixed32_t b) {
    return (vec3){fix_div(a.x, b), fix_div(a.y, b), fix_div(a.z, b)};
}

vec3 vec3_normalize(vec3 a) {
    fixed32_t l = sqrt(fix_mul(a.x, a.x) + fix_mul(a.y, a.y) + fix_mul(a.z, a.z));
    return (vec3){fix_div(a.x, l), fix_div(a.y, l), fix_div(a.z, l)};
}

fixed32_t dot(vec3 a, vec3 b) {
    return fix_mul(a.x, b.x) + fix_mul(a.y, b.y) + fix_mul(a.z, b.z);
}

vec3 vec3_reflect(vec3 i, vec3 n) {
    return vec3_minus(i, vec3_mul_s(n, fix_mul(FPT_TWO, dot(n, i))));
}

fixed32_t vec3_length(vec3 a) {
    return sqrt(fix_mul(a.x, a.x) + fix_mul(a.y, a.y) + fix_mul(a.z, a.z));
}

vec3 vec3_lerp(vec3 start, vec3 end, fixed32_t t) {
    return (vec3){lerp(start.x, end.x, t), lerp(start.y, end.y, t), lerp(start.z, end.z, t)};
}

vec3 xyz(vec4 a) { return (vec3){a.x, a.y, a.z}; }

mat4 mat4_0() {
    mat4 res = {{0, 0, 0, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0}};

    return res;
}

mat4 translate(vec3 delta)
{
    mat4 m = mat4_0();
    m.m1.x = FPT_ONE;
    m.m2.y = FPT_ONE;
    m.m3.z = FPT_ONE;
    m.m4 = (vec4){delta.x, delta.y, delta.z, FPT_ONE};
    return m;
}

mat4 rotation(fixed32_t pitchRad)
{
    mat4 m = mat4_0();
    m.m1.x = cos(pitchRad);
    m.m1.z = sin(pitchRad);
    m.m2.y = FPT_ONE;
    m.m3.x = -sin(pitchRad);
    m.m3.z = cos(pitchRad);
    m.m4.w = FPT_ONE;
    return m;
}

mat4 identity()
{
    mat4 m = mat4_0();
    m.m1.x = FPT_ONE;
    m.m2.y = FPT_ONE;
    m.m3.z = FPT_ONE;
    m.m4.w = FPT_ONE;
    return m;
}

vec4 vec4_from_vec3(vec3 a, fixed32_t w) {
    return (vec4){a.x, a.y, a.z, w};
}

vec4 vec4_add(vec4 a, vec4 b) {
    return (vec4){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

vec4 vec4_mul(vec4 a, vec4 b) {
    return (vec4){fix_mul(a.x, b.x), fix_mul(a.y, b.y), fix_mul(a.z, b.z), fix_mul(a.w, b.w)};
}

vec4 vec4_mul_s(vec4 a, fixed32_t b) {
    return (vec4) { fix_mul(a.x, b), fix_mul(a.y, b), fix_mul(a.z, b), fix_mul(a.z, b)};
}

fixed32_t vec4_dot(vec4 a, vec4 b) {
    return fix_mul(a.x, b.x) + fix_mul(a.y, b.y) + fix_mul(a.z, b.z) + fix_mul(a.w, b.w);
}

vec4 mat4_mul_vec4(mat4 a, vec4 b) {
    return (vec4) { vec4_dot(a.m1, b), vec4_dot(a.m2, b), vec4_dot(a.m3, b), vec4_dot(a.m4, b)};
}