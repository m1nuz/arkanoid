#include <string.h>
#include <math.h>

#include "utils.h"

void
multiply4x4(float *m1, float *m2)
{
    float temp[16];

    int x,y;

    for (x=0; x < 4; x++)
    {
        for(y=0; y < 4; y++)
        {
            temp[y + (x*4)] = (m1[x*4] * m2[y]) +
                              (m1[(x*4)+1] * m2[y+4]) +
                              (m1[(x*4)+2] * m2[y+8]) +
                              (m1[(x*4)+3] * m2[y+12]);
        }
    }

    memcpy(m1, temp, sizeof(float) << 4);
}

void
perspective(float *matrix, float fov, float aspect, float nearz, float farz)
{
    float range;

    range = tan(fov * 0.00872664625) * nearz; /* 0.00872664625 = PI/360 */
    memset(matrix, 0, sizeof(float) * 16);
    matrix[0] = (2 * nearz) / ((range * aspect) - (-range * aspect));
    matrix[5] = (2 * nearz) / (2 * range);
    matrix[10] = -(farz + nearz) / (farz - nearz);
    matrix[11] = -1;
    matrix[14] = -(2 * farz * nearz) / (farz - nearz);
}

void
translate(float *matrix, float x, float y, float z)
{
    float newmatrix[16] = IDENTITY_MATRIX4;

    newmatrix[12] = x;
    newmatrix[13] = y;
    newmatrix[14] = z;

    multiply4x4(matrix, newmatrix);
}

void
rotate(float *matrix, float angle, AXIS axis)
{
    const float d2r = 0.0174532925199; /* PI / 180 */
    const int cos1[3] = { 5, 0, 0 };
    const int cos2[3] = { 10, 10, 5 };
    const int sin1[3] = { 6, 2, 1 };
    const int sin2[3] = { 9, 8, 4 };
    float newmatrix[16] = IDENTITY_MATRIX4;

    newmatrix[cos1[axis]] = cos(d2r * angle);
    newmatrix[sin1[axis]] = -sin(d2r * angle);
    newmatrix[sin2[axis]] = -newmatrix[sin1[axis]];
    newmatrix[cos2[axis]] = newmatrix[cos1[axis]];

    multiply4x4(matrix, newmatrix);
}

void
scale(float *matrix, float x, float y, float z)
{
    float newmatrix[16] =
    {
        x, 0.f, 0.f, 0.f,
        0.f, y, 0.f, 0.f,
        0.f, 0.f, z, 0.f,
        0.f, 0.f, 0.f, 1.f
    };

    multiply4x4(matrix, newmatrix);
}

void
transpose(float *a)
{
    /*
    0,   1,  2,  3,
    4,   5,  6,  7,
    8,   9, 10, 11,
    12, 13, 14, 15; */

    float b[16];

    memcpy(b, a, sizeof(b));

    a[1] = b[4];
    a[2] = b[8];
    a[3] = b[12];

    a[6] = b[9];
    a[7] = b[13];
    a[11] = b[14];
}

int
overlaps(struct AABB *a, struct AABB *b)
{
    if((a->max[0] < b->min[0]) || (a->min[0] > b->max[0]))
        return 0;

    if((a->max[1] < b->min[1]) || (a->min[1] > b->max[1]))
        return 0;

    if((a->max[2] < b->min[2]) || (a->min[2] > b->max[2]))
        return 0;

    return 1;
}
