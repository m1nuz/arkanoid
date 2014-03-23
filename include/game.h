#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <native/native.h>
#include <gl3/gl3platform.h>
#include <al/al.h>

#include "utils.h"

#define GAME_NAME           "Arkanoid"

#define BACKGROUND_COLOR    0.0, 0.0, 0.0, 0.0

#define BALL_COLOR          1.0, 0, 0.0
#define BALL_SIZE           0.33
#define BALL_SPEED_Z        0.3

#define WALL_COLOR          0.7, 0.7, 0.7
#define WALL_MAX_X          6
#define WALL_MIN_X          -6
#define WALL_MIN_Z          -20
#define WALL_MAX_Z          5

#define ROCKET_COLOR        1.0, 0.5, 0
#define ROCKET_SIZE_X       1.0
#define ROCKET_SIZE_Y       0.2
#define ROCKET_SIZE_Z       0.1

#define MAX_BRICKS_TYPE     3
#define MAX_BRICKS          80
#define BRICK_SIZE_X        0.5
#define BRICK_SIZE_Y        0.3
#define BRICK_SIZE_Z        0.1
#define BRICK_COLOR0        0.0, 1.0, 0.0
#define BRICK_COLOR1        1.0, 1.0, 0.0
#define BRCIK_COLOR2        1.0, 0.0, 0.5
#define BRCIK_COLOR3        0.0, 1.0, 0.0
#define BRICKS_ROWS         8

#define BONUS_SPEED         0.125
#define MAX_BONUSES         10
#define MAX_BONUS_TYPE      5

#define MAX_SOUND_SOURCES   10

#define MAX_PLAYER_LIVES    3
#define PLAYER_LIVES        3

enum SOUNDS
{
    HIT_SOUND,
    LASER_SOUND,
    EXPLOSION_SOUND,
    COIN_SOUND,
    BLIP_SOUND,
    THEME0_SOUND,
    MAX_SOUND
};

struct BONUS
{
    float           x, y, z;       // position

    int             type;

    int             alive;

    struct AABB     bbox;

    float           transform[16];
};

struct WALL
{
    float           x, y, z;

    float           transform[16];
};

struct BRICK
{
    float           x, y, z;        // position

    int             type;
    int             force;
    int             hit;
    int             dead;

    struct AABB     bbox;

    float           transform[16];
};

struct BALL
{
    float           x, y, z;        // position
    float           v, w;           // velocity

    float           speed;          // speed multipler

    float           size;

    int             moving;

    struct AABB     bbox;

    float           transform[16];
};

struct ROCKET
{
    float           x, y, z;        // position
    float           size;

    int             size_bonus;     // size bonus type

    int             can_catch;

    struct AABB     bbox;

    float           transform[16];
};

struct CAMERA
{
    float           x, y, z; // position
    float           a, b, c; // rotation
};
