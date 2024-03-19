#include <fxcg/display.h>
#include <fxcg/keyboard.h>
#include <fxcg/app.h>
#include <string.h>
#include "./fpmath.h"
#include "./gl.h"

#define VOID_COLOUR (vec3){FTOFIX(0.1f), FTOFIX(0.1f), FTOFIX(0.1f)}

#define AMBIENT FTOFIX(0.1f)

#define MAX_BOUNCE 5

#define FOV 80.0f

#define RG FTOFIX(31.0f)
#define B FTOFIX(63.0f)

#define NUMOFSPHERES 2
#define NUMOFPLANES 5
#define NUMOFLIGHTS 1

#define SCR_W FTOFIX(SCR_WF)
#define SCR_H FTOFIX(SCR_HF)
#define SCR_WF 384.0f
#define SCR_HF 216.0f

const unsigned short* keyboard_register = (unsigned short*)0xA44B0000;
unsigned short lastkey[8];
unsigned short holdkey[8];

vec3 pos = {0, 0, 0};

struct Material {
    vec3 colour;
    fixed32_t smoothness;
};

struct Sphere {
    vec3 center;
    fixed32_t radius;
    struct Material material;
};

struct Plane {
    vec3 max;
    vec3 min;
    vec3 normal;
    struct Material material;
};

struct Light {
    vec3 lightColour;
    fixed32_t light;
    struct Sphere sphere;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct HitInfo {
    vec3 point;
    vec3 normal;
    fixed32_t dst;
    int hit;
    struct Material material;
};

struct HitInfo RaySphere(struct Ray ray, struct Sphere sphere) {
    struct HitInfo hit;

    vec3 oc = vec3_minus(ray.origin, xyz(mat4_mul_vec4(translate(vec3_neg(pos)), vec4_from_vec3(sphere.center, FPT_ONE))));

    fixed32_t a = dot(ray.direction, ray.direction);
    fixed32_t b = fix_mul(FPT_TWO, dot(oc, ray.direction));
    fixed32_t c = dot(oc, oc) - fix_mul(sphere.radius, sphere.radius);

    fixed32_t discriminant = fix_mul(b, b) - fix_mul(fix_mul(131072, a), c);
    if (discriminant < 0) {
        hit.hit = 0;
        return hit;
    }

    fixed32_t t_hit = fix_div(-b - sqrt(discriminant), fix_mul(FPT_TWO, a));
    if (t_hit > 1) {
        hit.material = sphere.material;
        hit.point = vec3_add(ray.origin, vec3_mul_s(ray.direction, t_hit));
        hit.dst = t_hit;
        hit.normal = vec3_normalize(vec3_div_s(vec3_minus(hit.point, sphere.center), sphere.radius));
        hit.hit = 1;
        return hit;
    }
    else {
        hit.hit = 0;
        return hit;
    }
}

struct HitInfo RayPlane(struct Ray ray, struct Plane plane) {
    struct HitInfo hit;

    vec3 dirfrac;
    // ray.dir is unit direction vector of ray
    dirfrac.x = fix_div(FPT_ONE, ray.direction.x);
    dirfrac.y = fix_div(FPT_ONE, ray.direction.y);
    dirfrac.z = fix_div(FPT_ONE, ray.direction.z);
    // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
    // ray.origin is origin of ray
    fixed32_t t1 = fix_mul(plane.min.x - ray.origin.x, dirfrac.x);
    fixed32_t t2 = fix_mul(plane.max.x - ray.origin.x, dirfrac.x);
    fixed32_t t3 = fix_mul(plane.min.y - ray.origin.y, dirfrac.y);
    fixed32_t t4 = fix_mul(plane.max.y - ray.origin.y, dirfrac.y);
    fixed32_t t5 = fix_mul(plane.min.z - ray.origin.z, dirfrac.z);
    fixed32_t t6 = fix_mul(plane.max.z - ray.origin.z, dirfrac.z);

    fixed32_t tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
    fixed32_t tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

    // if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
    if (tmax < 0)
    {
        hit.hit = 0;
        return hit;
    }

    // if tmin > tmax, ray doesn't intersect AABB
    if (tmin > tmax)
    {
        hit.hit = 0;
        return hit;
    }

    hit.dst = tmin;
    hit.material = plane.material;
    hit.point = vec3_add(ray.origin, vec3_mul_s(ray.direction, tmin));
    hit.normal = plane.normal;
    hit.hit = 1;
    return hit;
}

struct HitInfo TracePlanes(struct Ray ray, struct Plane planes[NUMOFPLANES]) {
    struct HitInfo rec_hit;
    struct HitInfo hit;
    hit.hit = 0;
    hit.dst = 327647232;
    hit.material.colour = (vec3){FPT_ONE, FPT_ONE, FPT_ONE};
    hit.material.smoothness = 0;

    for (int i = 0; i < NUMOFPLANES; i++) {
        rec_hit = RayPlane(ray, planes[i]);
        if (rec_hit.hit == 1 && rec_hit.dst < hit.dst) hit = rec_hit;
    }

    return hit;
}

struct HitInfo TraceSpheres(struct Ray ray, struct Sphere spheres[NUMOFSPHERES]) {
    struct HitInfo rec_hit;
    struct HitInfo hit;
    hit.hit = 0;
    hit.dst = 327647232;
    hit.material.colour = (vec3){FPT_ONE, FPT_ONE, FPT_ONE};
    hit.material.smoothness = 0;

    for (int i = 0; i < NUMOFSPHERES; i++) {
        rec_hit = RaySphere(ray, spheres[i]);
        if (rec_hit.hit == 1 && rec_hit.dst < hit.dst) hit = rec_hit;
    }

    return hit;
}

struct HitInfo TraceLSpheres(struct Ray ray, struct Light lights[NUMOFLIGHTS]) {
    struct HitInfo rec_hit;
    struct HitInfo hit;
    hit.hit = 0;
    hit.dst = 327647232;
    hit.material.colour = (vec3){FPT_ONE, FPT_ONE, FPT_ONE};
    hit.material.smoothness = 0;

    for (int i = 0; i < NUMOFLIGHTS; i++) {
        rec_hit = RaySphere(ray, lights[i].sphere);
        if (rec_hit.hit == 1 && rec_hit.dst < hit.dst) hit = rec_hit;
    }

    return hit;
}

vec3 TraceLight(struct Ray ray, struct Sphere spheres[NUMOFSPHERES], struct Light lights[NUMOFLIGHTS], vec3 normal) {
    vec3 colour = (vec3){0, 0, 0};

    struct HitInfo hit;

    for (int i = 0; i < NUMOFLIGHTS; i++) {
        ray.direction = vec3_normalize(vec3_minus(lights[i].sphere.center, ray.origin));
        hit = TraceSpheres(ray, spheres);

        if (hit.hit == 0) {
            fixed32_t dist = vec3_length(vec3_minus(lights[i].sphere.center, ray.origin));

            fixed32_t invSqr = fix_div(lights[i].light, fix_mul(dist, dist));

            fixed32_t cosineTerm = dot(ray.direction, normal);
            if (cosineTerm < 0) cosineTerm = 0;

            fixed32_t atten = fix_mul(invSqr, fix_mul(FPT_ONE_OVER_PI, cosineTerm));
            if (atten > FPT_ONE) atten = FPT_ONE;

            colour = vec3_mul_s(lights[i].lightColour, atten); 
        }
    }

    return colour;
}

vec3 Trace(struct Ray ray, struct Sphere spheres[NUMOFSPHERES], struct Plane planes[NUMOFPLANES], struct Light lights[NUMOFLIGHTS], unsigned int* randstate) {
    vec3 light = (vec3){FPT_ONE, FPT_ONE, FPT_ONE};
    vec3 colour = VOID_COLOUR;

    fixed32_t refDim = 39322;

    struct HitInfo sHit = TraceSpheres(ray, spheres);
    struct HitInfo lsHit = TraceLSpheres(ray, lights);
    struct HitInfo pHit = TracePlanes(ray, planes);

    for (int i = 0; i < MAX_BOUNCE; i++) {
        if (sHit.hit == 1 && sHit.dst < pHit.dst && sHit.dst < lsHit.dst) {
            if (sHit.material.smoothness == 0) {
                ray.origin = sHit.point;
                colour = sHit.material.colour;
                light = vec3_mul(light, TraceLight(ray, spheres, lights, sHit.normal));
                if (refDim > FPT_ONE) refDim = FPT_ONE;
                break;
            }
            else {
                vec3 refcolour = sHit.material.colour;
                ray.origin = sHit.point;
                ray.direction = vec3_reflect(ray.direction, sHit.normal);
                sHit = TraceSpheres(ray, spheres);
                pHit = TracePlanes(ray, planes);
            }
        }
        else if (lsHit.hit == 1 && lsHit.dst < pHit.dst) {
            colour = sHit.material.colour;
            light = (vec3){FPT_ONE, FPT_ONE, FPT_ONE};
            if (refDim > FPT_ONE) refDim = FPT_ONE;
            break;
        }
        else if (pHit.hit == 1) {
            if (pHit.material.smoothness == 0) {
                ray.origin = pHit.point;
                colour = pHit.material.colour;
                light = vec3_mul(light, TraceLight(ray, spheres, lights, pHit.normal));
                if (refDim > FPT_ONE) refDim = FPT_ONE;
                break;
            }
            else {
                vec3 refcolour = pHit.material.colour;
                ray.origin = vec3_add(pHit.point, (vec3){0, 0, FTOFIX(0.1f)});
                ray.direction = vec3_reflect(ray.direction, pHit.normal);
                sHit = TraceSpheres(ray, spheres);
                pHit = TracePlanes(ray, planes);
            }
        }

        refDim -= 6554;
    }

    light = vec3_add(light, (vec3){AMBIENT, AMBIENT, AMBIENT});
    if (light.x > FPT_ONE) light.x = FPT_ONE;
    if (light.y > FPT_ONE) light.y = FPT_ONE;
    if (light.z > FPT_ONE) light.z = FPT_ONE;

    return vec3_mul(vec3_mul_s(colour, refDim), light);
}

// KEYBOARD INPUT
void keyupdate(void) {
   memcpy(holdkey, lastkey, sizeof(unsigned short)*8);
   memcpy(lastkey, keyboard_register, sizeof(unsigned short)*8);
}
int keydownlast(int basic_keycode) {
   int row, col, word, bit; 
   row = basic_keycode%10; 
   col = basic_keycode/10-1; 
   word = row>>1; 
   bit = col + 8*(row&1); 
   return (0 != (lastkey[word] & 1<<bit)); 
}
int keydownhold(int basic_keycode) {
   int row, col, word, bit; 
   row = basic_keycode%10; 
   col = basic_keycode/10-1; 
   word = row>>1; 
   bit = col + 8*(row&1); 
   return (0 != (holdkey[word] & 1<<bit)); 
}

int rendered = 0;

int main(void) {
    Bdisp_AllClr_VRAM();
    Bdisp_EnableColor(1);

    Bdisp_PutDisp_DD();

    float imageAspectRatio = SCR_WF / SCR_HF;

    struct Ray ray;
    ray.origin = (vec3){0, 0, 0};

    struct Sphere sphere[100];
	sphere[0].center = (vec3){FTOFIX(-2.5f), FTOFIX(3.0f), FTOFIX(-7.0f)};
    sphere[0].radius = FTOFIX(2.0f);
    sphere[0].material.colour = (vec3){FPT_ONE, FPT_ONE, FPT_ONE};
    sphere[0].material.smoothness = 1;

    sphere[1].center = (vec3){FTOFIX(2.5f), FTOFIX(3.0f), FTOFIX(-8.5f)};
    sphere[1].radius = FTOFIX(2.0f);
    sphere[1].material.colour = (vec3){FTOFIX(0.8), FTOFIX(0.4), FTOFIX(0.4)};
    sphere[1].material.smoothness = 0;

    struct Plane plane[100];
    //left wall red
    plane[0].max = (vec3){FTOFIX(-5.0f), FTOFIX(5.0f), 0};
    plane[0].min = (vec3){FTOFIX(-5.1f), FTOFIX(-5.0f), FTOFIX(-11.0f)};
    plane[0].normal = (vec3){FPT_ONE, 0, 0};
    plane[0].material.colour = (vec3){FPT_ONE, 0, 0};
    plane[0].material.smoothness = 0;

    //floor white
    plane[1].max = (vec3){FTOFIX(-5.0f), FTOFIX(5.0f), 0};
    plane[1].min = (vec3){FTOFIX(5.0f), FTOFIX(5.1f), FTOFIX(-11.0f)};
    plane[1].normal = (vec3){0, -FPT_ONE, 0};
    plane[1].material.colour = (vec3){FPT_ONE, FPT_ONE, FPT_ONE};
    plane[1].material.smoothness = 0;

    //right wall green
    plane[2].max = (vec3){FTOFIX(5.0f), FTOFIX(5.0f), 0};
    plane[2].min = (vec3){FTOFIX(5.1f), FTOFIX(-5.0f), FTOFIX(-11.0f)};
    plane[2].normal = (vec3){-FPT_ONE, 0, 0};
    plane[2].material.colour = (vec3){0, FPT_ONE, 0};
    plane[2].material.smoothness = 0;

    //roof white
    plane[3].max = (vec3){FTOFIX(-5.0f), FTOFIX(-5.0f), 0};
    plane[3].min = (vec3){FTOFIX(5.0f), FTOFIX(-5.0f), FTOFIX(-11.0f)};
    plane[3].normal = (vec3){0, FPT_ONE, 0};
    plane[3].material.colour = (vec3){FPT_ONE, FPT_ONE, FPT_ONE};
    plane[3].material.smoothness = 0;

    //back wall mirror
    plane[4].max = (vec3){FTOFIX(-5.0f), FTOFIX(5.0f), FTOFIX(-11.0f)};
    plane[4].min = (vec3){FTOFIX(5.0f), FTOFIX(-5.0f), FTOFIX(-11.0f)};
    plane[4].normal = (vec3){0, 0, FPT_ONE};
    plane[4].material.colour = (vec3){FTOFIX(0.9f), FTOFIX(0.9f), FTOFIX(0.9f)};
    plane[4].material.smoothness = 0;

    struct Light light[100];
    light[0].sphere.center = (vec3){0, FTOFIX(-4.8f), FTOFIX(-7.0f)};
    light[0].sphere.radius = FTOFIX(0.5f);
    light[0].lightColour = (vec3){FPT_ONE, FPT_ONE, FPT_ONE};
    light[0].sphere.material.colour = (vec3){FPT_ONE, FPT_ONE, FPT_ONE};
    light[0].light = FTOFIX(100.0f);

    vec3 lastError = (vec3){0, 0, 0};

    while (1) {
        if (keydownlast(48) && !keydownhold(48)) {
            return 0; 
        }

        if (rendered == 0) {
            for (int h = 0; h < SCR_HF; h++) {
                for (int w = 0; w < SCR_WF; w++) {
                    unsigned int randstate = (w + 1) * (h + 1);

                    fixed32_t x = FTOFIX((2 * ((w + 0.5) / SCR_WF) - 1) * tanf((FOV / 2) * (F_PI / 180)) * imageAspectRatio);
                    fixed32_t y = FTOFIX((1 - 2 * (h + 0.5) / SCR_HF) * tanf((FOV / 2) * (F_PI / 180)));

                    ray.direction = vec3_normalize((vec3){x, -y, -FPT_ONE});

                    vec3 value = (vec3){0, 0, 0};

                    value = vec3_add(value, Trace(ray, sphere, plane, light, &randstate));

                    value = vec3_add(value, lastError);
                    lastError = (vec3){value.x - fix_div(floor(fix_mul(value.x, RG)), RG), value.y - fix_div(floor(fix_mul(value.y, B)), B), value.z - fix_div(floor(fix_mul(value.z, RG)), RG)};

                    setPixel(w, h, colourFromDec(value));
                }
            }
        }
        rendered = 1;

        Bdisp_PutDisp_DD();

        keyupdate();
    }
 
    return 0;
}