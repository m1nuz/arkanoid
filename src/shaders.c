#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "shaders.h"

static char     shaderinfo[4096];

static GLuint compile_shader(GLenum _shadertype, const char *_source, int _srclen)
{

    assert(_source);
    assert((_shadertype == GL_VERTEX_SHADER) || (_shadertype == GL_FRAGMENT_SHADER) || (_shadertype == GL_GEOMETRY_SHADER));

    GLuint sh = glCreateShader(_shadertype);

    char *fullsource = (char*)malloc(_srclen + 1);
    strncpy(fullsource, _source, _srclen);

    fullsource[_srclen] = 0;

    glShaderSource(sh, 1, (const char **)&fullsource, 0);

    free(fullsource);

    glCompileShader(sh);

    GLint status = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &status);

    if(!status)
    {
        GLsizei written = 0;
        glGetShaderInfoLog(sh, sizeof(shaderinfo), &written, shaderinfo);

        fprintf(stderr, "error: %s\n", shaderinfo);

        return -1;
    }

    return sh;
}
static void link_shader(GLuint _program)
{
    GLint linked = 0;

    glLinkProgram(_program);
    glGetProgramiv(_program, GL_LINK_STATUS, &linked);

    if(!linked)
    {
        GLsizei written = 0;
        glGetProgramInfoLog(_program, sizeof(shaderinfo), &written, shaderinfo);

        fprintf(stderr, "error: %s\n", shaderinfo);

        return;
    }
}

GLuint new_shader(const char *_vs_text, const char *_fs_text)
{
    GLuint pid = glCreateProgram();

    GLuint vid = compile_shader(GL_VERTEX_SHADER, _vs_text, strlen(_vs_text));
    GLuint fid = compile_shader(GL_FRAGMENT_SHADER, _fs_text, strlen(_fs_text));

    glAttachShader(pid, vid);
    glAttachShader(pid, fid);

    link_shader(pid);

    glDeleteShader(vid);
    glDeleteShader(fid);

    return pid;
}

const char color_vertex_shader[] =
{
    "#version 330 core\n"
    "layout(location = 0) in vec3 vertex;\n"
    "layout(location = 1) in vec2 texcoord;\n"
    "layout(location = 2) in vec3 normal;\n"
    "uniform mat4 mvp;\n"
    "out vec2 texcoord0;\n"
    "void main() {\n"
    "texcoord0 = texcoord;\n"
    "gl_Position = mvp * vec4(vertex, 1);\n"
    "}\n"
};

const char color_fragment_shader[] =
{
    "#version 330 core\n"
    "uniform vec3 color;\n"
    "in vec2 texcoord0;\n"
    "layout(location = 0, index = 0) out vec4 fragcolor;\n"
    "void main() {\n"
    "fragcolor = vec4(color, 1);\n"
    "}\n"
};

const char postprocess_vertex_shader[] =
{
    "#version 330 core\n"
    "layout(location = 0) in vec3 position;\n"
    "layout(location = 1) in vec2 texcoord;\n"
    "out vec2 texcoord0;\n"
    "void main() {\n"
    "gl_Position =  vec4(position, 1);\n"
    "texcoord0 = texcoord;\n"
    "}\n"
};

const char postprocess_fragment_shader[] =
{
    "#version 330 core\n"
    "uniform sampler2D color_map;\n"
    "uniform sampler2D glow_map;\n"
    "uniform sampler2D ui_map;\n"
    "in vec2 texcoord0;\n"
    "layout(location = 0, index = 0) out vec4 fragcolor;\n"
    "void main() {\n"
    "vec4 glow = texture(glow_map, texcoord0);\n"
    "vec4 color = texture(color_map, texcoord0);\n"
    "vec4 ui = texture(ui_map, texcoord0);\n"
    //"fragcolor = ui;\n"
    "vec4 c0 = clamp((color + glow) - (color * glow), 0.0, 1.0);\n"
    "if(length(ui.xyz) < 0.15) fragcolor = c0;\n"
    "else fragcolor = ui;\n"
    //"fragcolor = ui + c0;\n"
    "}\n"
};

const char hblur_fragment_shader[] =
{
    "#version 330 core\n"
    "uniform sampler2D tex0;\n"
    "uniform vec2 size;\n"
    "in vec2 texcoord0;\n"
    "layout(location = 0, index = 0) out vec4 finalcolor;\n"
    "uniform float offset[5] = float[](0.0, 1.0, 2.0, 3.0, 4.0);\n"
    "uniform float weight[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);\n"

    //"float gaussian(float x, float deviation){ \n"
    //"return (1.0 / sqrt(2.0 * 3.141592 * deviation)) * exp(-((x * x) / (2.0 * deviation)));\n"
    //"}\n"

    "void main() {\n"
    "vec2 texcoord = gl_FragCoord.xy;\n"
    //"vec2 size = vec2(1.0 / 800.0, 1.0 / 450.0);\n"
    "float scale = 2.0;\n"
    "finalcolor = texture(tex0, texcoord * size) * weight[0];\n"
    "for(int i = 1; i < 5; i++) {\n"
    "finalcolor += texture(tex0, (texcoord + vec2(offset[i] * scale, 0.0)) * size) * weight[i];\n"
    "finalcolor += texture(tex0, (texcoord - vec2(offset[i] * scale, 0.0)) * size) * weight[i];\n"
    "}\n"
    "}\n"
};

const char vblur_fragment_shader[] =
{
    "#version 330 core\n"
    "uniform sampler2D tex0;\n"
    "uniform vec2 size;\n"
    "in vec2 texcoord0;\n"
    "layout(location = 0, index = 0) out vec4 finalcolor;\n"
    "uniform float offset[5] = float[](0.0, 1.0, 2.0, 3.0, 4.0);\n"
    "uniform float weight[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);\n"

    //"float gaussian(float x, float deviation){ \n"
    //"return (1.0 / sqrt(2.0 * 3.141592 * deviation)) * exp(-((x * x) / (2.0 * deviation)));\n"
    //"}\n"

    "void main() {\n"
    "vec2 texcoord = gl_FragCoord.xy;\n"
    //"vec2 size = vec2(1.0 / 800.0, 1.0 / 450.0);\n"
    "float scale = 2.0;\n"
    "finalcolor = texture(tex0, texcoord * size) * weight[0];\n"
    "for(int i = 1; i < 5; i++) {\n"
    "finalcolor += texture(tex0, (texcoord + vec2(0.0, offset[i] * scale)) * size) * weight[i];\n"
    "finalcolor += texture(tex0, (texcoord - vec2(0.0, offset[i] * scale)) * size) * weight[i];\n"
    "}\n"
    "}\n"
};

const char ui_fragment_shader[] =
{
    "#version 330 core\n"
    "uniform sampler2D color_map;\n"
    "uniform vec4 color;\n"
    "in vec2 texcoord0;\n"
    "layout(location = 0, index = 0) out vec4 fragcolor;\n"
    "void main() {\n"
    "vec4 c = color * texture(color_map, texcoord0);\n"
    "if(c.a < 0.2) discard;\n"
    "fragcolor = c;\n"
    "}\n"
};
