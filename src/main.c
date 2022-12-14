#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"
#include "phong.glsl.h"
#include "lenna.png.h"
#include "dice.obj.h"

static struct {
    float rx, ry;
    sg_pass_action pass_action;
    sg_pipeline pip;
    sg_bindings bind;
} state;

typedef struct {
    float x, y, z;
    uint32_t color;
    int16_t u, v;
} vertex_t;

void init(void) {
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext()
    });

    sg_buffer dice = sg_make_buffer(&(sg_buffer_desc){
        .data = (sg_range){ &obj_dice_data, obj_dice_data_size * sizeof(float) },
        .label = "cube-vertices"
    });

    /* NOTE: SLOT_tex is provided by shader code generation */
    sg_image img = sg_make_image(&(sg_image_desc){
        .width = img_lenna_width,
        .height = img_lenna_height,
        .data.subimage[0][0] = SG_RANGE(img_lenna_data),
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .label = "cube-texture"
    });

    /* a shader */
    sg_shader shd = sg_make_shader(phong_shader_desc(sg_query_backend()));

    /* a pipeline state object */
    state.pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            .attrs = {
                [ATTR_vs_a_pos].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vs_a_normal].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vs_a_tex_coords].format = SG_VERTEXFORMAT_FLOAT2
            }
        },
        .shader = shd,
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true
        },
        .label = "cube-pipeline"
    });
    
    state.bind = (sg_bindings) {
        .vertex_buffers[0] = dice,
        .fs_images[SLOT_diffuse_texture] = img
    };

    /* default pass action */
    state.pass_action = (sg_pass_action) {
        .colors[0] = { .action=SG_ACTION_CLEAR, .value={0.1f, 0.1f, 0.1f, 1.0f} }
    };
}

void frame(void) {
    /* compute model-view-projection matrix */
    const float t = (float)(sapp_frame_duration() * 60.0);
    hmm_mat4 proj = HMM_Perspective(45.0f, sapp_widthf()/sapp_heightf(), 0.01f, 1000.0f);
    hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 4.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
    vs_params_t vs_params;
    state.rx += 0.1f; state.ry += 0.2f;
    hmm_mat4 rxm = HMM_Rotate(state.rx * t, HMM_Vec3(1.0f, 0.0f, 0.0f));
    hmm_mat4 rym = HMM_Rotate(state.ry * t, HMM_Vec3(0.0f, 1.0f, 0.0f));
    hmm_mat4 model = HMM_MultiplyMat4(rxm, rym);
    vs_params.model = model;
    vs_params.view = view;
    vs_params.projection = proj;
    
    /* render the frame */
    sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &SG_RANGE(vs_params));
    sg_draw(0, obj_dice_face_count, 1);
    sg_end_pass();
    sg_commit();
}

void cleanup(void) {
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .width = 800,
        .height = 600,
        .sample_count = 4,
        .gl_force_gles2 = true,
        .window_title = "Textured Cube (sokol-app)",
        .icon.sokol_default = true,
    };
}
