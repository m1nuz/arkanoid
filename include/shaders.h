#pragma once

#include <gl3/gl3platform.h>

extern const char color_vertex_shader[];
extern const char color_fragment_shader[];
extern const char postprocess_vertex_shader[];
extern const char postprocess_fragment_shader[];
extern const char hblur_fragment_shader[];
extern const char vblur_fragment_shader[];
extern const char ui_fragment_shader[];

GLuint new_shader(const char *_vs_text, const char *_fs_text);
