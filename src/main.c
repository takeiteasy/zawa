#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"
#include "dice.glsl.h"
#include "dice.obj.h"

#define MAX_DICE 3

typedef struct {
    hmm_vec3 position, color, velocity;
} die;

static struct {
    float rx, ry;
    sg_pass_action pass_action;
    sg_pipeline pip;
    sg_bindings bind;
    die dice[MAX_DICE];
    int diceCount;
} state;

void init(void) {
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext()
    });
    
    state.diceCount = MAX_DICE;
    for (int i = 0; i < MAX_DICE; i++) {
        state.dice[i] = (die) {
            .position = HMM_Vec3((i-1) * 3.f, 0.f, 0.f),
            .color = HMM_Vec3(i == 0 ? 1.f : 0.f, i == 1 ? 1.f : 0.f, i == 2 ? 1.f : 0.f),
            .velocity = HMM_Vec3(0.f, 0.f, 0.f)
        };
    }

    state.bind = (sg_bindings) {
        .vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
            .data = (sg_range){ &obj_dice_data, obj_dice_data_size * sizeof(float) },
            .label = "cube-vertices"
        }),
        .vertex_buffers[1] = sg_make_buffer(&(sg_buffer_desc) {
            .size = MAX_DICE * sizeof(hmm_vec3) * 3,
            .usage = SG_USAGE_STREAM,
            .label = "instance-data"
        }),
    };

    /* a shader */
    sg_shader shd = sg_make_shader(dice_shader_desc(sg_query_backend()));

    /* a pipeline state object */
    state.pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            .buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE,
            .attrs = {
                [ATTR_vs_pos] = { .format=SG_VERTEXFORMAT_FLOAT3, .buffer_index=0 },
                [ATTR_vs_norm] = { .format=SG_VERTEXFORMAT_FLOAT3, .buffer_index=0 },
                [ATTR_vs_texcoord] = { .format=SG_VERTEXFORMAT_FLOAT2, .buffer_index=0 },
                [ATTR_vs_inst_pos] = { .format=SG_VERTEXFORMAT_FLOAT3, .buffer_index=1 },
                [ATTR_vs_inst_col] = { .format=SG_VERTEXFORMAT_FLOAT3, .buffer_index=1 },
                [ATTR_vs_inst_vel] = { .format=SG_VERTEXFORMAT_FLOAT3, .buffer_index=1 }
            }
        },
        .shader = shd,
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true
        },
        .label = "cube-pipeline"
    });

    /* default pass action */
    state.pass_action = (sg_pass_action) {
        .colors[0] = { .action=SG_ACTION_CLEAR, .value={0.1f, 0.1f, 0.1f, 1.0f} }
    };
}

void frame(void) {
    sg_update_buffer(state.bind.vertex_buffers[1], &(sg_range){
        .ptr = state.dice,
        .size = (size_t)state.diceCount * sizeof(hmm_vec3) * 3
    });
    
    /* compute model-view-projection matrix */
    const float t = (float)(sapp_frame_duration() * 60.0);
    hmm_mat4 proj = HMM_Perspective(45.0f, sapp_widthf()/sapp_heightf(), 0.01f, 1000.0f);
    hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 10.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
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
    sg_draw(0, obj_dice_face_count, state.diceCount);
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
