#include "game.h"
#include "draw.h"
#include "shaders.h"
#include "meshes.h"
#include "textures.h"
#include "sounds.h"
#include "resources.h"

struct BONUS    bonus[MAX_BONUSES];
struct WALL     walls[3];
struct BRICK    bricks[MAX_BRICKS];
struct BALL     ball;
struct ROCKET   rocket;
struct CAMERA   camera;
struct CAMERA   cameras[2];

int             current_camera;

int             bonuses;

int             screen_width = 0;
int             screen_height = 0;
int             glow_framebuffer_width;
int             glow_framebuffer_height;
int             fullscreen = 0;

int             cursor_x;
int             cursor_y;

GLuint          cube_mesh;
GLuint          sphere_mesh;
GLuint          quad_mesh;

GLuint          fullscreen_va;
GLuint          fullscreen_vb;

GLuint          ui_va;
GLuint          ui_vb;

GLuint          coloring_shader;
GLuint          postprocess_shader;
GLuint          hblur_shader;
GLuint          vblur_shader;
GLuint          ui_shader;

GLuint          color_map;
GLuint          depth_map;
GLuint          blur_map;
GLuint          glow_map;
GLuint          ui_map;

GLuint          textures[10];

GLuint          texture_sampler;
GLuint          ui_sampler;

GLuint          color_framebuffer;
GLuint          blur_framebuffer;
GLuint          glow_framebuffer;
GLuint          sample_framebuffer;
GLuint          ui_framebuffer;

GLuint          color0buffer;
GLuint          depth0buffer;
GLuint          depth1buffer;

GLint           max_supported_samples;

ALuint          sound_sources[MAX_SOUND_SOURCES];
ALuint          sounds[MAX_SOUND];

int             game_state = 0;
int             game_music = -1;
int             music_state = 0;
int             game_over = PLAYER_LIVES;
int             selected_menu = 1;
int             game_start = 0;
int             score = 0;
int             bricks_count = MAX_BRICKS;
int             score_multipler = 1;
int             sound_off = 0;
int             invert_mouse;

static void
check_framebuffer()
{
    const GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch (err)
    {
    case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
        break;
    case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
        break;
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
        break;
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
        break;
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
        break;
    case GL_FRAMEBUFFER_COMPLETE:
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        fprintf(stderr, "Setup FBO failed. Duplicate attachment.\n");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
         fprintf(stderr, "Setup FBO failed. Missing attachment.\n");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
         fprintf(stderr, "Setup FBO failed. Missing draw buffer.\n");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
         fprintf(stderr, "Setup FBO failed. Missing read buffer.\n");
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
         fprintf(stderr, "Setup FBO failed. Unsupported framebuffer format.\n");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
         fprintf(stderr, "Setup FBO failed. Attached images must have the same number of samples.\n");
        break;
    default:
         fprintf(stderr, "Setup FBO failed. Fatal error.\n");
    }
}

static int
init_graphics()
{
    glow_framebuffer_width = screen_width / 2;
    glow_framebuffer_height = screen_height / 2;

    glGetIntegerv(GL_MAX_SAMPLES, &max_supported_samples);

    cube_mesh = new_cube();
    sphere_mesh = new_sphere(16, 16);
    quad_mesh = new_quad();

    glGenVertexArrays(1, &fullscreen_va);
    glBindVertexArray(fullscreen_va);

    glGenBuffers(1, &fullscreen_vb);

    glBindBuffer(GL_ARRAY_BUFFER, fullscreen_vb);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(v3t2_t), fullscreen_quad_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(v3t2_t), (void*)offsetof(v3t2_t, x));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(v3t2_t), (void*)offsetof(v3t2_t, u));
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &ui_va);
    glBindVertexArray(ui_va);

    glGenBuffers(1, &ui_vb);

    glBindBuffer(GL_ARRAY_BUFFER, ui_vb);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(v3t2_t), NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(v3t2_t), (void*)offsetof(v3t2_t, x));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(v3t2_t), (void*)offsetof(v3t2_t, u));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    coloring_shader = new_shader(color_vertex_shader, color_fragment_shader);
    postprocess_shader = new_shader(postprocess_vertex_shader, postprocess_fragment_shader);
    hblur_shader = new_shader(postprocess_vertex_shader, hblur_fragment_shader);
    vblur_shader = new_shader(postprocess_vertex_shader, vblur_fragment_shader);
    ui_shader = new_shader(postprocess_vertex_shader, ui_fragment_shader);

    color_map = new_texture2D(GL_RGB8, screen_width, screen_height, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    depth_map = new_texture2D(GL_DEPTH_COMPONENT16, screen_width, screen_height, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);
    ui_map = new_texture2D(GL_RGB8, screen_width, screen_height, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    blur_map = new_texture2D(GL_RGB8, glow_framebuffer_width, glow_framebuffer_height, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glow_map = new_texture2D(GL_RGB8, glow_framebuffer_width, glow_framebuffer_height, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    texture_sampler = new_sampler2D(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ui_sampler = new_sampler2D(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &color_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, color_framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_map, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);

    check_framebuffer();

    glGenFramebuffers(1, &ui_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, ui_framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ui_map, 0);

    check_framebuffer();

    glGenFramebuffers(1, &blur_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, blur_framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blur_map, 0);

    check_framebuffer();

    glGenFramebuffers(1, &glow_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, glow_framebuffer);

    glGenRenderbuffers(1, &depth1buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth1buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screen_width, screen_height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth1buffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glow_map, 0);

    check_framebuffer();

    glGenFramebuffers(1, &sample_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, sample_framebuffer);

    glGenRenderbuffers(1, &color0buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, color0buffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, max_supported_samples, GL_RGBA8, screen_width, screen_width);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color0buffer);

    glGenRenderbuffers(1, &depth0buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth0buffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, max_supported_samples, GL_DEPTH_COMPONENT16, screen_width, screen_width);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth0buffer);

    check_framebuffer();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /*textures[0] = get_texture("gameover.tga");
    textures[1] = get_texture("pressenter.tga");
    textures[2] = get_texture("quit.tga");
    textures[3] = get_texture("return.tga");
    textures[4] = get_texture("pressspace.tga");
    textures[5] = get_texture("score.tga");
    textures[6] = get_texture("numbers.tga");
    textures[7] = get_texture("copyright.tga");
    textures[8] = get_texture("version.tga");*/

    textures[0] = new_texture2D(gameover_internal_format, gameover_width, gameover_height, gameover_format, GL_UNSIGNED_BYTE, (void*)gameover_data);
    textures[1] = new_texture2D(pressenter_internal_format, pressenter_width, pressenter_height, pressenter_format, GL_UNSIGNED_BYTE, (void*)pressenter_data);
    textures[2] = new_texture2D(quit_internal_format, quit_width, quit_height, quit_format, GL_UNSIGNED_BYTE, (void*)quit_data);
    textures[3] = new_texture2D(return_internal_format, return_width, return_height, return_format, GL_UNSIGNED_BYTE, (void*)return_data);
    textures[4] = new_texture2D(pressspace_internal_format, pressspace_width, pressspace_height, pressspace_format, GL_UNSIGNED_BYTE, (void*)pressspace_data);
    textures[5] = new_texture2D(score_internal_format, score_width, score_height, score_format, GL_UNSIGNED_BYTE, (void*)score_data);
    textures[6] = new_texture2D(numbers_internal_format, numbers_width, numbers_height, numbers_format, GL_UNSIGNED_BYTE, (void*)numbers_data);
    textures[7] = new_texture2D(copyright_internal_format, copyright_width, copyright_height, copyright_format, GL_UNSIGNED_BYTE, (void*)copyright_data);
    textures[8] = new_texture2D(version_internal_format, version_width, version_height, version_format, GL_UNSIGNED_BYTE, (void*)version_data);

    return 0;
}

static int
clean_graphics()
{
    glDeleteVertexArrays(1, &fullscreen_va);
    glDeleteVertexArrays(1, &ui_va);

    glDeleteBuffers(1, &fullscreen_vb);
    glDeleteBuffers(1, &ui_vb);

    glDeleteVertexArrays(1, &cube_mesh);
    glDeleteVertexArrays(1, &sphere_mesh);
    glDeleteVertexArrays(1, &quad_mesh);

    glDeleteProgram(coloring_shader);
    glDeleteProgram(postprocess_shader);
    glDeleteProgram(hblur_shader);
    glDeleteProgram(vblur_shader);
    glDeleteProgram(ui_shader);

    glDeleteTextures(1, &color_map);
    glDeleteTextures(1, &depth_map);
    glDeleteTextures(1, &blur_map);
    glDeleteTextures(1, &glow_map);

    glDeleteSamplers(1, &texture_sampler);

    glDeleteFramebuffers(1, &color_framebuffer);
    glDeleteFramebuffers(1, &glow_framebuffer);
    glDeleteFramebuffers(1, &blur_framebuffer);
    glDeleteFramebuffers(1, &sample_framebuffer);

    return 0;
}

static int
init_audio()
{
    alGenSources(MAX_SOUND_SOURCES, sound_sources);

    /*sounds[HIT_SOUND] = get_sound("hit.wav");
    sounds[LASER_SOUND] = get_sound("laser.wav");
    sounds[EXPLOSION_SOUND] = get_sound("explosion.wav");
    sounds[COIN_SOUND] = get_sound("coin.wav");
    sounds[THEME0_SOUND] = get_sound("houseloop.wav");
    sounds[BLIP_SOUND] = get_sound("blip.wav");*/

    sounds[HIT_SOUND] = new_sound(hit_format, hit_frequency, hit_size, (void*)hit_data);
    sounds[LASER_SOUND] = new_sound(laser_format, laser_frequency, laser_size, (void*)laser_data);
    sounds[EXPLOSION_SOUND] = new_sound(explosion_format, explosion_frequency, explosion_size, (void*)explosion_data);
    sounds[COIN_SOUND] = new_sound(coin_format, coin_frequency, coin_size, (void*)coin_data);
    sounds[BLIP_SOUND] = new_sound(blip_format, blip_frequency, blip_size, (void*)blip_data);
    sounds[THEME0_SOUND] = new_sound(houseloop_format, houseloop_frequency, houseloop_size, (void*)houseloop_data);

    const ALfloat ori[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};

    alListener3f(AL_POSITION, 0, 0, 0);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    alListenerfv(AL_ORIENTATION, ori);

    return 0;
}

static int
clean_audio()
{
    alDeleteSources(MAX_SOUND_SOURCES, sound_sources);

    for(int i = 0; i < MAX_SOUND; i++)
        alDeleteBuffers(1, &sounds[i]);

    return 0;
}

int
play_sound(enum SOUNDS sound, int looped)
{
    if(sound_off == 1)
        return 0;

    for(int i = 0; i < MAX_SOUND_SOURCES; i++)
    {
        ALint status = 0;

        alGetSourcei(sound_sources[i], AL_SOURCE_STATE, &status);

        if((status == AL_INITIAL) || (status == AL_STOPPED) || (status == 0))
        {
            alSource3f(sound_sources[i], AL_POSITION, 0.0f, 0.0f, 0.0f);
            alSource3f(sound_sources[i], AL_VELOCITY, 0.0f, 0.0f, 0.0f);
            alSourcef(sound_sources[i], AL_PITCH, 1.0f);
            alSourcef(sound_sources[i], AL_GAIN, 1.0f);

            if(looped != 0)
                alSourcei(sound_sources[i], AL_LOOPING, AL_TRUE);

            alSourcei(sound_sources[i], AL_BUFFER, sounds[sound]);
            alSourcePlay(sound_sources[i]);

            return i;
        }
    }

    return 0;
}

int
pause_sound(int source)
{
    alSourcePause(sound_sources[source]);
    return source;
}

int
unpause_sound(int source)
{
    alSourcePlay(sound_sources[source]);
    return source;
}

static void
init_bricks()
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;

    float start_z = -20;
    float start_x = WALL_MIN_X + BRICK_SIZE_X + 0.1;

    const int max_rows = BRICKS_ROWS;
    int rows = 0;

    for(int i = 0; i < MAX_BRICKS; i++)
    {
        if(i % max_rows == 0)
            rows++;

        bricks[i].type = i % 3;
        bricks[i].dead = 0;
        bricks[i].hit = 0;

        switch(bricks[i].type)
        {
        case 0:
            bricks[i].force = 1;
            break;
        case 1:
            bricks[i].force = 2;
            break;
        case 2:
            bricks[i].force = 3;
            break;
        }

        bricks[i].x = start_x + (i % max_rows) * 1.6;
        bricks[i].y = 0;
        bricks[i].z = start_z + rows * 0.8;

        bricks[i].bbox.min[0] = bricks[i].x - BRICK_SIZE_X;
        bricks[i].bbox.min[1] = bricks[i].y - BRICK_SIZE_Y;
        bricks[i].bbox.min[2] = bricks[i].z - BRICK_SIZE_Z;

        bricks[i].bbox.max[0] = bricks[i].x + BRICK_SIZE_X;
        bricks[i].bbox.max[1] = bricks[i].y + BRICK_SIZE_Y;
        bricks[i].bbox.max[2] = bricks[i].z + BRICK_SIZE_Z;

        memcpy(&bricks[i].transform, identitymatrix, sizeof(identitymatrix));
        scale(bricks[i].transform, BRICK_SIZE_X, BRICK_SIZE_Y, BRICK_SIZE_Z);
        translate(bricks[i].transform, bricks[i].x, bricks[i].y, bricks[i].z);
    }
}

static int
restart_game()
{
    score = 0;
    score_multipler = 1;
    bricks_count = MAX_BRICKS;

    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;

    cameras[0].x = 0;
    cameras[0].y = 3;
    cameras[0].z = 10;
    cameras[0].a = -30;
    cameras[0].b = 0;
    cameras[0].c = 0;

    cameras[1].x = 0;
    cameras[1].y = 10;
    cameras[1].z = 30;
    cameras[1].a = -90;
    cameras[1].b = 0;
    cameras[1].c = 0;

    memcpy(&camera, &cameras[current_camera], sizeof(camera));

    rocket.x = (float)screen_width * -0.5;
    rocket.y = 0;
    rocket.z = -2;
    rocket.size = ROCKET_SIZE_X;
    rocket.size_bonus = -1;

    ball.x = 0;
    ball.y = 0;
    ball.z = rocket.z - (ROCKET_SIZE_Z + BALL_SIZE) * 0.5 - 0.2;
    ball.speed = 1.0;
    ball.size = BALL_SIZE;

    const float wall_scales[][3] =
    {
        {0.1, 0.5, (WALL_MAX_Z - WALL_MIN_Z) * 0.5},
        {(WALL_MAX_X - WALL_MIN_X) * 0.53, 0.5, 0.1},
        {0.1, 0.5, (WALL_MAX_Z - WALL_MIN_Z) * 0.5}
    };

    init_bricks();

    walls[0].x = WALL_MIN_X - 0.3;
    walls[0].y = 0;
    walls[0].z = WALL_MIN_Z * 0.5;

    walls[1].x = 0;
    walls[1].y = 0;
    walls[1].z = WALL_MIN_Z;

    walls[2].x = WALL_MAX_X + 0.3;
    walls[2].y = 0;
    walls[2].z = WALL_MIN_Z * 0.5;

    for(int i = 0; i < countof(walls); i++)
    {
        memcpy(&walls[i].transform, identitymatrix, sizeof(identitymatrix));
        scale(walls[i].transform, wall_scales[i][0], wall_scales[i][1], wall_scales[i][2]);
        translate(walls[i].transform, walls[i].x, walls[i].y, walls[i].z);
    }

    return 0;
}

static int
startup_game()
{
    if(init_graphics() != 0)
        return -1;

    if(init_audio() !=0)
        return -1;

    restart_game();

    return 0;
}

static int
shutdown_game()
{
    clean_graphics();
    clean_audio();

    return 0;
}

static void
idle()
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;

    static float angle = 0;
    angle += 0.05f;

    // collision
    for(int i = 0; i < MAX_BRICKS; i++)
    {
        if(!bricks[i].dead)
            if(overlaps(&ball.bbox, &bricks[i].bbox) == 1)
            {
                if(ball.speed < 1.0)
                {
                    ball.speed += 0.1;
                }

                ball.w = -ball.w;
                ball.v = -(bricks[i].x - ball.x) * ball.w;

                bricks[i].hit++;
                bricks[i].type--;

                if(bricks[i].hit >= bricks[i].force)
                {
                    bricks[i].dead = 1;

                    bricks_count--;

                    score += 10 * score_multipler;

                    play_sound(EXPLOSION_SOUND, 0);

                    int h = rand() % 2;

                    if(h == 0)
                        continue;

                    if(bonuses > MAX_BONUSES)
                        continue;

                    int j = bonuses;

                    bonus[j].x = bricks[i].x;
                    bonus[j].y = bricks[i].y;
                    bonus[j].z = bricks[i].z;

                    bonus[j].type = rand() % MAX_BONUS_TYPE;
                    bonus[j].alive = 1;

                    if(bonuses < MAX_BONUSES)
                        bonuses++;
                }
                else
                {
                    play_sound(HIT_SOUND, 0);
                }
            }
    }

    if(overlaps(&rocket.bbox, &ball.bbox) == 1)
    {
        if(ball.moving == 1)
            play_sound(HIT_SOUND, 0);

        if(rocket.can_catch  == 0)
        {
            float d = rocket.x - ball.x;

            ball.w = -ball.w;
            ball.v = d * ball.w + ball.w * 0.3;
            ball.z = rocket.z - (ROCKET_SIZE_Z + BALL_SIZE) - 0.2;
        }
        else
        {
            ball.w = 0;
            ball.v = 0;
            ball.moving = 0;
            ball.z = rocket.z - (ROCKET_SIZE_Z + BALL_SIZE) - 0.2;
        }
    }

    for(int i = 0; i < bonuses; i++)
    {
        if(bonus[i].alive != 1)
            continue;

        if(overlaps(&bonus[i].bbox, &rocket.bbox) == 1)
        {
            switch(bonus[i].type)
            {
            case 0: // expand
                if(rocket.size_bonus == -1)
                {
                    rocket.size = ROCKET_SIZE_X * 2;
                    rocket.size_bonus = 0;
                }
                else if(rocket.size_bonus == 1)
                {
                    rocket.size = ROCKET_SIZE_X;
                    rocket.size_bonus = -1;
                }
                else
                {
                    rocket.size_bonus = 0;
                }
                break;
            case 1: // divide
                if(rocket.size_bonus == -1)
                {
                    rocket.size = ROCKET_SIZE_X * 0.5;
                    rocket.size_bonus = 1;
                }
                else if(rocket.size_bonus == 0)
                {
                    rocket.size = ROCKET_SIZE_X;
                    rocket.size_bonus = -1;
                }
                else
                {
                    rocket.size_bonus = 1;
                }
                break;
            case 2: // slow
                ball.speed = 0.85;
                break;
            case 3: // catch
                rocket.can_catch = 1;
                break;
            case 4:
                score_multipler += 1;
                break;
            }

            // remove bonus
            if(bonuses > 1)
            {
                memcpy(&bonus[i], &bonus[bonuses - 1], sizeof(bonus[i]));
                memset(&bonus[bonuses - 1], 0, sizeof(bonus[bonuses - 1]));
                bonuses--;
            }
            else if((bonuses == 1) && (bonuses != 0))
            {
                memset(bonus, 0, sizeof(struct BONUS) * MAX_BONUSES);
                bonuses--;
            }

            score += 20 * score_multipler;

            play_sound(COIN_SOUND, 0);
        }

        if(bonus[i].z > WALL_MAX_Z)
        {
            // remove bonus
            if(bonuses > 1)
            {
                memcpy(&bonus[i], &bonus[bonuses - 1], sizeof(bonus[i]));
                bonuses--;
            }
            else if((bonuses == 1) && (bonuses != 0))
                bonuses--;
        }
    }

    if(ball.z < WALL_MIN_Z)
    {
        ball.w = -ball.w;
        ball.v = ball.v;
    }

    if(ball.z < WALL_MIN_Z * 2)
    {
        ball.speed = 1.0;
        ball.moving = 0;
        ball.z = -2.6;
        ball.v = 0;
        ball.w = 0;
    }

    if(ball.z > WALL_MAX_Z)
    {
        ball.speed = 1.0;
        ball.moving = 0;
        ball.z = -2.6;
        ball.v = 0;
        ball.w = 0;

        game_over--;
    }

    if(ball.x > WALL_MAX_X)
    {
        ball.v = -ball.v;
        ball.x = WALL_MAX_X - ball.size * 0.5f;
    }

    if(ball.x < WALL_MIN_X)
    {
        ball.v = -ball.v;
        ball.x = WALL_MIN_X + ball.size * 0.5f;
    }

    // update

    if(bricks_count <= 0)
    {
        init_bricks();

        bricks_count = MAX_BRICKS;

        ball.w = 0;
        ball.v = 0;
        ball.moving = 0;
        ball.z = rocket.z - (ROCKET_SIZE_Z + BALL_SIZE) - 0.2;
    }

    if(game_over > 0)
    {
        if(invert_mouse == 1)
            rocket.x =  6 * (1.0 - (float)cursor_x / (screen_width * 0.5));
        else
            rocket.x =  6 * ((float)cursor_x / (screen_width * 0.5) - 1);

        if(ball.moving == 0)
        {
            ball.x = rocket.x;
        }

        ball.x += ball.v * ball.speed;
        ball.z += ball.w * ball.speed;
    }

    memcpy(rocket.transform, identitymatrix, sizeof(rocket.transform));
    scale(rocket.transform, rocket.size, 0.2, 0.1);
    rotate(rocket.transform, 0, X_AXIS);
    rotate(rocket.transform, 0, Y_AXIS);
    rotate(rocket.transform, 0, Z_AXIS);
    translate(rocket.transform, rocket.x, rocket.y, rocket.z);

    memcpy(ball.transform, identitymatrix, sizeof(ball.transform));
    scale(ball.transform, ball.size, ball.size, ball.size);
    rotate(ball.transform, 0, X_AXIS);
    rotate(ball.transform, 0, Y_AXIS);
    rotate(ball.transform, 0, Z_AXIS);
    translate(ball.transform, ball.x, ball.y, ball.z);

    for(int i = 0; i < bonuses; i++)
    {
        bonus[i].z += BONUS_SPEED;

        bonus[i].bbox.min[0] = bonus[i].x - 0.3;
        bonus[i].bbox.min[1] = bonus[i].y - 0.3;
        bonus[i].bbox.min[2] = bonus[i].z - 0.3;

        bonus[i].bbox.max[0] = bonus[i].x + 0.3;
        bonus[i].bbox.max[1] = bonus[i].y + 0.3;
        bonus[i].bbox.max[2] = bonus[i].z + 0.3;

        memcpy(bonus[i].transform, identitymatrix, sizeof(bonus[i].transform));
        scale(bonus[i].transform, 0.2, 0.2, 0.2);

        rotate(bonus[i].transform, angle * 200.0, X_AXIS);
        rotate(bonus[i].transform, 0, Y_AXIS);
        rotate(bonus[i].transform, angle * 200.0, Z_AXIS);

        translate(bonus[i].transform, bonus[i].x, bonus[i].y, bonus[i].z);
    }

    rocket.bbox.min[0] = rocket.x - rocket.size;
    rocket.bbox.min[1] = rocket.y - ROCKET_SIZE_Y;
    rocket.bbox.min[2] = rocket.z - ROCKET_SIZE_Z;

    rocket.bbox.max[0] = rocket.x + rocket.size;
    rocket.bbox.max[1] = rocket.y + ROCKET_SIZE_Y;
    rocket.bbox.max[2] = rocket.z + ROCKET_SIZE_Z;

    ball.bbox.min[0] = ball.x - ball.size;
    ball.bbox.min[1] = ball.y - ball.size;
    ball.bbox.min[2] = ball.z - ball.size;

    ball.bbox.max[0] = ball.x + ball.size;
    ball.bbox.max[1] = ball.y + ball.size;
    ball.bbox.max[2] = ball.z + ball.size;
}

void
draw_bricks(float *projection, float *view)
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;
    GLfloat mvp[16];

    for(int i = 0; i < MAX_BRICKS; i++)
    {
        if(bricks[i].dead)
            continue;

        const float3 color[MAX_BRICKS_TYPE] =
        {
            {BRICK_COLOR0},
            {BRICK_COLOR1},
            {BRCIK_COLOR2}
        };

        memcpy(mvp, identitymatrix, sizeof(mvp));
        multiply4x4(mvp, bricks[i].transform);
        multiply4x4(mvp, view);
        multiply4x4(mvp, projection);

        draw_cube(coloring_shader, color[bricks[i].type], mvp);
    }
}

void
draw_rocket(float *projection, float *view)
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;
    GLfloat mvp[16];

    const float3 color = {ROCKET_COLOR};

    memcpy(mvp, identitymatrix, sizeof(mvp));
    multiply4x4(mvp, rocket.transform);
    multiply4x4(mvp, view);
    multiply4x4(mvp, projection);

    draw_cube(coloring_shader, color, mvp);
}

void
draw_ball(float *projection, float *view)
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;
    GLfloat mvp[16];

    const float color[] = {BALL_COLOR};

    memcpy(mvp, identitymatrix, sizeof(mvp));
    multiply4x4(mvp, ball.transform);
    multiply4x4(mvp, view);
    multiply4x4(mvp, projection);

    draw_sphere(coloring_shader, color, mvp);
}

void
draw_walls(float *projection, float *view)
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;
    GLfloat mvp[16];

    for(int i = 0; i < countof(walls); i++)
    {
        const float color[] = {WALL_COLOR};

        memcpy(mvp, identitymatrix, sizeof(mvp));
        multiply4x4(mvp, walls[i].transform);
        multiply4x4(mvp, view);
        multiply4x4(mvp, projection);

        draw_cube(coloring_shader, color, mvp);
    }
}

void
draw_bonuses(float *projection, float *view)
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;
    GLfloat mvp[16];

    for(int i = 0; i < bonuses; i++)
    {
        const float color[][4] =
        {
            {0.0, 0.0, 1.0}, // expand
            {0.0, 0.7, 1.0}, // devide
            {1.0, 0.5, 0.5}, // slow
            {0.0, 1.0, 0.0}, // catch
            {1.0, 0.0, 1.0}
        };

        if(!bonus[i].alive)
            continue;

        memcpy(mvp, identitymatrix, sizeof(mvp));
        multiply4x4(mvp, bonus[i].transform);
        multiply4x4(mvp, view);
        multiply4x4(mvp, projection);

        draw_cube(coloring_shader, color[bonus[i].type], mvp);
    }
}

void
draw_blur(GLuint shader, GLuint tex)
{
    int loc_tex0 = glGetUniformLocation(shader, "tex0");
    int loc_sz = glGetUniformLocation(shader, "size");

    float2 size;
    size[0] = 1.0 / (float)glow_framebuffer_width;
    size[1] = 1.0 / (float)glow_framebuffer_height;

    glUniform1i(loc_tex0, 0);
    glUniform2f(loc_sz, size[0], size[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glBindSampler(0, texture_sampler);

    glBindVertexArray(fullscreen_va);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void
draw_ui(GLuint shader, GLuint texture, GLuint sampler, float3 color, float2 position, float2 size, int slot, int nslot)
{
    float aspect = (float)screen_width / screen_height;

    v3t2_t vertices[6];

    vertices[0].x = position[0] - size[0],  vertices[0].y = position[1] + size[1] * aspect, vertices[0].z = 0;
    vertices[1].x = position[0] + size[0],  vertices[1].y = position[1] + size[1] * aspect, vertices[1].z = 0;
    vertices[2].x = position[0] - size[0],  vertices[2].y = position[1] - size[1] * aspect, vertices[2].z = 0;
    vertices[3].x = position[0] + size[0],  vertices[3].y = position[1] + size[1] * aspect, vertices[3].z = 0;
    vertices[4].x = position[0] + size[0],  vertices[4].y = position[1] - size[1] * aspect, vertices[4].z = 0;
    vertices[5].x = position[0] - size[0],  vertices[5].y = position[1] - size[1] * aspect, vertices[5].z = 0;

    vertices[0].u = (float)slot * (1.f / nslot),        vertices[0].v = 0;
    vertices[1].u = (float)(slot + 1) * (1.f / nslot),  vertices[1].v = 0;
    vertices[2].u = (float)slot * (1.f / nslot),        vertices[2].v = 1;
    vertices[3].u = (float)(slot + 1) * (1.f / nslot),  vertices[3].v = 0;
    vertices[4].u = (float)(slot + 1) * (1.f / nslot),  vertices[4].v = 1;
    vertices[5].u = (float)slot * (1.f / nslot),        vertices[5].v = 1;


    glBindBuffer(GL_ARRAY_BUFFER, ui_vb);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    int loc_tex = glGetUniformLocation(shader, "color_map");
    int loc_c = glGetUniformLocation(shader, "color");

    glUniform1i(loc_tex, 0);
    glUniform4f(loc_c, color[0], color[1], color[2], 1.f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindSampler(0, sampler);

    glBindVertexArray(ui_va);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void
print_number(int number, GLuint shader, GLuint texture, GLuint sampler, float3 color, float2 position, float2 size)
{
    int digits_count = 0;
    int digits[20] = {};

    while(number > 0)
    {
        digits[digits_count++] = number % 10;

        number /= 10;
    }

    for(int i = 0; i < digits_count; i++)
    {
        float2 p = {position[0] + size[0] * i * 2.f, position[1]};

        draw_ui(shader, texture, sampler, color, p, size, digits[digits_count - 1 - i], 10);
    }
}

static void
display_game()
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;

    GLfloat projection[16];
    GLfloat view[16];

    perspective(projection, 45.0, (float)screen_width / screen_height, 0.1, 1000.0);

    memcpy(view, identitymatrix, sizeof(view));
    rotate(view, camera.a, X_AXIS);
    rotate(view, camera.b, Y_AXIS);
    rotate(view, camera.c, Z_AXIS);
    translate(view, -camera.x, -camera.y, -camera.z);

    glBindFramebuffer(GL_FRAMEBUFFER, sample_framebuffer);
    glViewport(0, 0, screen_width, screen_height);
    glClearColor(BACKGROUND_COLOR);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    glUseProgram(coloring_shader);

    draw_bricks(projection, view);
    draw_rocket(projection, view);
    draw_ball(projection, view);
    draw_walls(projection, view);
    draw_bonuses(projection, view);

    glBindFramebuffer(GL_FRAMEBUFFER, ui_framebuffer);
    glViewport(0, 0, screen_width, screen_height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(ui_shader);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    if(game_over == 0)
    {
        {
            float3 cc = {1, 1, 1};
            float2 pp = {0, 0};
            float2 ss = {0.3, 0.3};

            draw_ui(ui_shader, textures[0], ui_sampler, cc, pp, ss, 0, 1);
        }

        {
            float3 cc = {0.25, 1, 0};
            float2 pp = {0, -0.7};
            float2 ss = {0.20, 0.04};

            draw_ui(ui_shader, textures[4], ui_sampler, cc, pp, ss, 0, 1);
        }
    }

    {
        float3 cc = {1, 1, 0};
        float2 pp = {-0.85, 0.8};
        float2 ss = {0.10, 0.04};

        draw_ui(ui_shader, textures[5], ui_sampler, cc, pp, ss, 0, 1);
    }

    {
        float3 cc = {1, 1, 0};
        float2 pp = {-0.7, 0.808};
        float2 ss = {0.02, 0.034};

        print_number(score, ui_shader, textures[6], ui_sampler, cc, pp, ss);
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, glow_framebuffer);
    glViewport(0, 0, glow_framebuffer_width, glow_framebuffer_height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(coloring_shader);

    draw_bricks(projection, view);
    draw_rocket(projection, view);
    draw_ball(projection, view);
    draw_bonuses(projection, view);

    glBindFramebuffer(GL_FRAMEBUFFER, blur_framebuffer);
    glViewport(0, 0, glow_framebuffer_width, glow_framebuffer_height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(vblur_shader);

    draw_blur(vblur_shader, glow_map);

    glBindFramebuffer(GL_FRAMEBUFFER, glow_framebuffer);
    glViewport(0, 0, glow_framebuffer_width, glow_framebuffer_height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(hblur_shader);

    draw_blur(hblur_shader, blur_map);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, sample_framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, color_framebuffer);
    glBlitFramebuffer(0, 0, screen_width, screen_height, 0, 0, screen_width, screen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screen_width, screen_height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(postprocess_shader);

    draw_fullscreen_quad(postprocess_shader, texture_sampler);

    glUseProgram(0);
}

static void
display_menu()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screen_width, screen_height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(ui_shader);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    float3 sc = {1, 0, 0};

    if(game_start == 0)
    {
        float3 cc = {1, 1, 1};
        float2 pp = {0, 0.5};
        float2 ss = {0.3, 0.1};

        draw_ui(ui_shader, textures[1], ui_sampler, cc, pp, ss, 0, 1);
    }

    {
        float3 cc = {0.5, 0.5, 0.5};
        float2 pp = {0, -0.8};
        float2 ss = {0.09, 0.035};

        draw_ui(ui_shader, textures[7], ui_sampler, cc, pp, ss, 0, 1);
    }

    {
        float3 cc = {0.5, 0.5, 0.5};
        float2 pp = {0.9, -0.9};
        float2 ss = {0.08, 0.012};

        draw_ui(ui_shader, textures[8], ui_sampler, cc, pp, ss, 0, 1);
    }

    if(game_start == 1)
    {
        {
            float3 cc = {1, 1, 1};
            float2 pp = {0, -0.4};
            float2 ss = {0.08, 0.04};

            if(selected_menu == 0)
                memcpy(cc, sc, sizeof(cc));

            draw_ui(ui_shader, textures[2], ui_sampler, cc, pp, ss, 0, 1);
        }

        {
            float3 cc = {1, 1, 1};
            float2 pp = {0, -0.2};
            float2 ss = {0.14, 0.04};

            if(selected_menu == 1)
                memcpy(cc, sc, sizeof(cc));

            draw_ui(ui_shader, textures[3], ui_sampler, cc, pp, ss, 0, 1);
        }
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(0);
}

void
control_player(const struct window_event *event)
{
    if(event->type == MOUSE_MOVE)
    {
        cursor_x = event->motion.x;
        cursor_y = event->motion.y;
    }

    if(event->type == KEYBOARD_KEY_DOWN)
    {
        if(event->key_down.keys_state[KEY_ESCAPE])
        {
            game_state = 0;
            pause_sound(game_music);
        }

        if(event->key_down.keys_state[KEY_1])
        {
            memcpy(&camera, &cameras[0], sizeof(camera));

            current_camera = 0;
        }

        if(event->key_down.keys_state[KEY_2])
        {
            memcpy(&camera, &cameras[1], sizeof(camera));

            current_camera = 1;
        }

        if(event->key_down.keys_state[KEY_SPACE])
        {
            if((ball.moving == 0) && (game_over > 0))
            {
                ball.w = -BALL_SPEED_Z;
                ball.moving = 1;
                rocket.can_catch = 0;
            }

            if(game_over == 0)
            {
                game_over = PLAYER_LIVES;
                restart_game();
                return;
            }
        }
    }

    if(event->type == MOUSE_BUTTON_DOWN)
    {
        if(event->button_down.buttons & MBUTTON0)
        {
            if((ball.moving == 0) && (game_over > 0))
            {
                ball.w = -BALL_SPEED_Z;
                ball.moving = 1;
                rocket.can_catch = 0;
            }
        }
    }
}

void
control_menu(const struct window_event *event)
{
    if(event->type == KEYBOARD_KEY_DOWN)
    {
        if(event->key_down.keys_state[KEY_ENTER])
        {

            if(selected_menu == 0)
            {
                shutdown_game();
                exit(0);
            }
            else if(selected_menu == 1)
            {
                game_state = 1;
                game_start = 1;
            }

            if(game_music == -1)
            {
                game_music = play_sound(THEME0_SOUND, 1);
            }
            else
                unpause_sound(game_music);
        }

        if(game_start == 1)
        {
            if(event->key_down.keys_state[KEY_DOWN])
            {
                if(selected_menu > 0)
                {
                    selected_menu--;

                    play_sound(BLIP_SOUND, 0);
                }
            }

            if(event->key_down.keys_state[KEY_UP])
            {
                if(selected_menu < 1)
                {
                    selected_menu++;

                    play_sound(BLIP_SOUND, 0);
                }
            }            
        }
    }
}

int
game_loop(const struct window_event *event)
{
    switch(game_state)
    {
    case 0:
        control_menu(event);
        break;
    case 1:
        control_player(event);
        break;
    }

    return 1;
}

int
main(int argc, char *argv[])
{
    srand(time(0));

    for(int i = 0; i < argc; i++)
    {
        if(strcmp(argv[i], "-nosound") == 0)
            sound_off = 1;

        if(strcmp(argv[i], "-invertmouse") == 0)
            invert_mouse = 1;
    }

    atexit(opengl_cleanup);
    atexit(openal_cleanup);

    if(window_create(screen_width, screen_height, WF_FULLSCREEN | WF_HIDEN_CURSOR) == -1)
        return -1;

    window_get_size(&screen_width, &screen_height);

    if(opengl_init() == -1)
        return -1;

    if(openal_init() == -1)
        return -1;

    //const char *version = glGetString(GL_VERSION);
    //const char *renderer = glGetString(GL_RENDERER);
    //const char *vendor = glGetString(GL_VENDOR);
    //const char *extensions = glGetString(GL_EXTENSIONS);

    if(startup_game() != 0)
    {
        fprintf(stderr, "%s\n", "Can\'t start game");
        return 0;
    }    

    struct window_event event;

    int64_t next_time = time_get();

    const int TICKS_PER_SECOND = 30;
    const int SKIP_TICKS = time_frequency() / TICKS_PER_SECOND;
    const int MAX_FRAMESKIP = 5;

    int loops;
    //float interpolation = 0;

    while(window_poll(&event))
    {
        loops = 0;

        game_loop(&event);

        while(time_get() > next_time && loops < MAX_FRAMESKIP)
        {
            switch(game_state)
            {
            case 0:
                break;
            case 1:
                idle();
                break;
            }

            next_time += SKIP_TICKS;
            loops++;
        }

        //interpolation = (float)(time_get() + SKIP_TICKS - next_time) / (float)(SKIP_TICKS);

        switch(game_state)
        {
        case 0:
            display_menu();
            break;
        case 1:
            display_game();
            break;
        }

        window_process();
    }

    shutdown_game();

    return 0;
}
