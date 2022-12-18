#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_glue.h"

#include "linalgb.h"

#include "dice.glsl.h"
#include "dice.obj.h"
#include "floor.glsl.h"
#include "plane.obj.h"

#define MAX_DICE 3

typedef struct {
    vec3 position, color;
} Die;

typedef struct {
    sg_pipeline pip;
    sg_bindings bind;
    Die dice[MAX_DICE];
    int dice_count;
    mat4 model;
} DiceState;

typedef struct {
    sg_pipeline pip;
    sg_bindings bind;
    mat4 model;
} FloorState;

static struct {
    sg_pass_action pass_action;
    DiceState dice;
    FloorState floor;
    mat4 view, proj;
} state;

void init(void) {
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext()
    });
    
    state.proj = Perspective(45.0f, sapp_widthf()/sapp_heightf(), 0.01f, 1000.0f);
    state.view = LookAt(Vec3(0.0f, 2.f, 2.2f),
                        Vec3(0.0f, 0.8f, 0.0f),
                        Vec3(0.0f, 1.0f, 0.0f));
    
    state.dice.model = Mat4Identity();
    state.dice.dice_count = 0;
    for (int i = 0; i < MAX_DICE; i++) {
        state.dice.dice[i].position = Vec3(0, 0, 0);
        state.dice.dice[i].color = Vec3(i == 0 ? 1.f : 0.f, i == 1 ? 1.f : 0.f, i == 2 ? 1.f : 0.f);
    }
        
    state.dice.bind = (sg_bindings) {
        .vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
            .data = (sg_range){ &obj_dice_data, obj_dice_data_size * sizeof(float) },
            .label = "dice-vertices"
        }),
        .vertex_buffers[1] = sg_make_buffer(&(sg_buffer_desc) {
            .size = MAX_DICE * 2 * sizeof(vec3),
            .usage = SG_USAGE_STREAM,
            .label = "dice-instance-data"
        }),
    };
    state.dice.pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            .buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE,
            .attrs = {
                [ATTR_vsDice_pos]      = { .format=SG_VERTEXFORMAT_FLOAT3, .buffer_index=0 },
                [ATTR_vsDice_norm]     = { .format=SG_VERTEXFORMAT_FLOAT3, .buffer_index=0 },
                [ATTR_vsDice_texcoord] = { .format=SG_VERTEXFORMAT_FLOAT2, .buffer_index=0 },
                [ATTR_vsDice_inst_pos] = { .format=SG_VERTEXFORMAT_FLOAT3, .buffer_index=1 },
                [ATTR_vsDice_inst_col] = { .format=SG_VERTEXFORMAT_FLOAT3, .buffer_index=1 }
            }
        },
        .shader = sg_make_shader(dice_shader_desc(sg_query_backend())),
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true
        },
        .label = "dice-pipeline"
    });
    
    state.floor.model = Mat4Identity();
    state.floor.bind = (sg_bindings) {
        .vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
            .data = (sg_range){ &obj_plane_data, obj_plane_data_size * sizeof(float) },
            .label = "plane-vertices"
        })
    };
    state.floor.pip = sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .attrs = {
                [ATTR_vsFloor_pos].format      = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vsFloor_norm].format     = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vsFloor_texcoord].format = SG_VERTEXFORMAT_FLOAT2,
            }
        },
        .shader = sg_make_shader(floor_shader_desc(sg_query_backend())),
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true
        },
        .label = "plane-pipeline"
    });
    
    state.pass_action = (sg_pass_action) {
        .colors[0] = { .action=SG_ACTION_CLEAR, .value={0.f, 0.f, 0.f, 1.f} }
    };
}

void frame(void) {
//    const float t = (float)(sapp_frame_duration() * 60.0);
    
    if (state.dice.dice_count) {
        float data[state.dice.dice_count * 3 * 2];
        for (int i = 0, off = 0; i < state.dice.dice_count; i++, off += 6) {
            memcpy(data + off, &state.dice.dice[i].position, sizeof(vec3));
            memcpy(data + off + 3, &state.dice.dice[i].color, sizeof(vec3));
        }
        sg_update_buffer(state.dice.bind.vertex_buffers[1], &(sg_range){
            .ptr = data,
            .size = (size_t)state.dice.dice_count * sizeof(vec3) * 2
        });
    }
    
    vs_dice_params_t vs_dice_params = {
        .model = state.dice.model,
        .view = state.view,
        .projection = state.proj
    };
    
    fs_dice_light_t fs_dice_light = {
        .LightPosition    = Vec4(0.f, 7.f, -2.f, 0.f),
        .LightDirection   = Vec4(0.f, -1.f, 0.f, 0.f),
        .LightCutOff      = cosf(20.f * .01745329251994329576f),
        .LightOuterCutOff = cosf(25.f * .01745329251994329576f),
        .LightAmbient     = Vec4(.5f, .5f, .5f, 0.f),
        .LightDiffuse     = Vec4(1.f, 1.f, 1.f, 0.f),
        .LightSpecular    = Vec4(1.f, 1.f, 1.f, 0.f),
        .LightConstant    = 1.f,
        .LightLinear      = .09f,
        .LightQuadratic   = .032f
    };
    fs_floor_light_t fs_floor_light;
    memcpy(&fs_floor_light, &fs_dice_light, sizeof(fs_floor_light));
    
    sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(state.dice.pip);
    sg_apply_bindings(&state.dice.bind);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_dice_params, &SG_RANGE(vs_dice_params));
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_dice_light, &SG_RANGE(fs_dice_light));
    sg_draw(0, obj_dice_face_count, state.dice.dice_count);
    
    vs_floor_params_t vs_floor_params = {
        .model = state.floor.model,
        .view = state.view,
        .projection = state.proj,
        .color = Vec4(0.f, 0.f, 0.f, 1.f)
    };
    
    sg_apply_pipeline(state.floor.pip);
    sg_apply_bindings(&state.floor.bind);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_floor_params, &SG_RANGE(vs_floor_params));
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_floor_light, &SG_RANGE(fs_floor_light));
    sg_draw(0, obj_plane_face_count, 1);
    
    sg_end_pass();
    sg_commit();
}

void input(const sapp_event *e) {
    switch (e->type) {
        case SAPP_EVENTTYPE_KEY_DOWN:
            switch (e->key_code) {
                case SAPP_KEYCODE_ESCAPE:
                    sapp_quit();
                    break;
                case SAPP_KEYCODE_SPACE:
                    if (state.dice.dice_count < MAX_DICE)
                        state.dice.dice_count += 1;
                    break;
                default:
                    break;
            }
            break;
        case SAPP_EVENTTYPE_RESIZED:
            state.proj = Perspective(45.0f, sapp_widthf()/sapp_heightf(), 0.01f, 1000.0f);
            break;
        default:
            break;
    }
}

void cleanup(void) {
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .event_cb = input,
        .cleanup_cb = cleanup,
        .width = 800,
        .height = 600,
        .sample_count = 4,
        .gl_force_gles2 = true,
        .window_title = "ざわ。。。ざわ。。。",
        .icon.sokol_default = true,
    };
}
