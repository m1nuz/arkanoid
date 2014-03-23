#include "meshes.h"

extern GLuint cube_mesh;
extern GLuint sphere_mesh;
extern GLuint quad_mesh;
extern GLuint fullscreen_va;

extern GLuint color_map;
extern GLuint depth_map;
extern GLuint blur_map;
extern GLuint glow_map;
extern GLuint ui_map;

void
draw_cube(GLuint shader, const float3 color, const float *transform)
{
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    int loc_mvp = glGetUniformLocation(shader, "mvp");
    int loc_c = glGetUniformLocation(shader, "color");

    glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, transform);

    glUniform3f(loc_c, color[0], color[1], color[2]);

    glBindVertexArray(cube_mesh);

    glDrawElements(GL_TRIANGLES, CUBE_INDICES_NUM, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);

    glDisable(GL_CULL_FACE);
}

void
draw_sphere(GLuint shader, const float3 color, const float *transform)
{
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    int loc_mvp = glGetUniformLocation(shader, "mvp");
    int loc_c = glGetUniformLocation(shader, "color");

    glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, transform);

    glUniform3f(loc_c, color[0], color[1], color[2]);

    glBindVertexArray(sphere_mesh);

    glDrawElements(GL_TRIANGLES, 16 * 16 * 6, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);

    glDisable(GL_CULL_FACE);
}

void
draw_quad(GLuint shader, const float3 color, const float *transform)
{
    int loc_mvp = glGetUniformLocation(shader, "mvp");
    int loc_c = glGetUniformLocation(shader, "color");

    glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, transform);

    glUniform3f(loc_c, color[0], color[1], color[2]);

    glBindVertexArray(quad_mesh);

    glDrawElements(GL_TRIANGLES, QUAD_INDICES_NUM, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);
}

void
draw_fullscreen_quad(GLuint shader, GLuint sampler)
{
    int loc_cmp = glGetUniformLocation(shader, "color_map");
    int loc_gmp = glGetUniformLocation(shader, "glow_map");
    int loc_ump = glGetUniformLocation(shader, "ui_map");

    glUniform1i(loc_cmp, 0);
    glUniform1i(loc_gmp, 1);
    glUniform1i(loc_ump, 2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color_map);
    glBindSampler(0, sampler);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, glow_map);
    glBindSampler(1, sampler);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, ui_map);
    glBindSampler(2, sampler);

    glBindVertexArray(fullscreen_va);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
