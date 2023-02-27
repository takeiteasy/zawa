#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "linalgb.h"
#include "dice.glsl.h"
#include "dice.obj.h"
#include "floor.glsl.h"
#include "plane.obj.h"
#include "ode/ode.h"

#define MAX_DICE 3

typedef struct {
    vec4 xxxx;
    vec4 yyyy;
    vec4 zzzz;
    vec4 color;
} DieVertex;

typedef struct {
    DieVertex vertex;
    dGeomID geom;
    dBodyID body;
} Die;

typedef struct {
    sg_pipeline pip;
    sg_bindings bind;
    Die dice[MAX_DICE];
    int count;
} Dice;

typedef struct {
    sg_pipeline pip;
    sg_bindings bind;
    mat4 model;
    dGeomID geom;
    dBodyID body;
} Floor;

#define MAX_CONTACTS 16

static struct {
    sg_pass_action pass_action;
    Dice dice;
    Floor floor;
    mat4 view, proj;
    dWorldID world;
    dSpaceID space;
    dJointGroupID contactGroup;
    dContact contacts[MAX_CONTACTS];
} state;

static void collision(void* data, dGeomID _a, dGeomID _b) {
    dBodyID a = dGeomGetBody(_a);
    dBodyID b = dGeomGetBody(_b);
    
    if (a && b && dAreConnectedExcluding(a, b, dJointTypeContact))
        return;
    
    for (int i = 0; i < dCollide(_a, _b, MAX_CONTACTS, &state.contacts[0].geom, sizeof(dContact)); i++) {
        state.contacts[i].surface.mode = dContactBounce;
        state.contacts[i].surface.mu = dInfinity;
        state.contacts[i].surface.mu2 = 0;
        state.contacts[i].surface.bounce = 0.5;
        
        dJointID c = dJointCreateContact(state.world, state.contactGroup, &state.contacts[i]);
        dJointAttach(c, a, b);
    }
}

void init(void) {
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext()
    });
    
    state.proj = Perspective(45.0f, sapp_widthf()/sapp_heightf(), .01f, 1000.f);
    state.view = LookAt(Vec3(0.f, 2.f, 2.2f),
                        Vec3(0.f, .8f, 0.f),
                        Vec3(0.f, 1.f, 0.f));
    
    state.dice.count = 0;
    for (int i = 0; i < MAX_DICE; i++)
        state.dice.dice[i].vertex.color = Vec4(i == 0 ? 1.f : 0.f, i == 1 ? 1.f : 0.f, i == 2 ? 1.f : 0.f, 1.f);
        
    state.dice.bind = (sg_bindings) {
        .vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
            .data = (sg_range){ &obj_dice_data, obj_dice_data_size * sizeof(float) },
            .label = "dice-vertices"
        }),
        .vertex_buffers[1] = sg_make_buffer(&(sg_buffer_desc) {
            .size = MAX_DICE * sizeof(DieVertex),
            .usage = SG_USAGE_STREAM,
            .label = "dice-instance-data"
        }),
    };
    state.dice.pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            .buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE,
            .attrs = {
                [ATTR_vsDice_pos] = { .format=SG_VERTEXFORMAT_FLOAT3, .buffer_index=0 },
                [ATTR_vsDice_norm] = { .format=SG_VERTEXFORMAT_FLOAT3, .buffer_index=0 },
                [ATTR_vsDice_texcoord] = { .format=SG_VERTEXFORMAT_FLOAT2, .buffer_index=0 },
                [ATTR_vsDice_inst_mat_xxxx] = { .format=SG_VERTEXFORMAT_FLOAT4, .buffer_index=1 },
                [ATTR_vsDice_inst_mat_yyyy] = { .format=SG_VERTEXFORMAT_FLOAT4, .buffer_index=1 },
                [ATTR_vsDice_inst_mat_zzzz] = { .format=SG_VERTEXFORMAT_FLOAT4, .buffer_index=1 },
                [ATTR_vsDice_inst_col] = { .format=SG_VERTEXFORMAT_FLOAT4, .buffer_index=1 },
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
    
    dInitODE();
    state.world = dWorldCreate();
    state.space = dHashSpaceCreate(0);
    state.contactGroup = dJointGroupCreate(0);
    
    dWorldSetGravity(state.world, 0.0, -9.81, 0.0);
    dWorldSetLinearDamping(state.world, 0.0001);
    dWorldSetAngularDamping(state.world, 0.005);
    dWorldSetMaxAngularSpeed(state.world, 200);
    dWorldSetContactMaxCorrectingVel(state.world, 0.2);
    dWorldSetContactSurfaceLayer(state.world, 0.001);
    dWorldSetCFM(state.world, 1E-15);
    dWorldSetERP(state.world, 10);
    
    state.floor.geom = dCreatePlane(state.space, 0.0, 1.0, 0.0, 0.0);
    state.floor.body = NULL;
}

void frame(void) {
    const float t = (float)(sapp_frame_duration() * 60.0);
    
    dSpaceCollide(state.space, 0, collision);
    dWorldQuickStep(state.world, 1.f / 60.f);
    dJointGroupEmpty(state.contactGroup);
    
    if (state.dice.count) {
        DieVertex buffer[state.dice.count];
        for (int i = 0, off = 0; i < state.dice.count; i++, off += 6) {
            Die *die = &state.dice.dice[i];
            const dReal *p = dBodyGetPosition(die->body);
            const dReal *r = dBodyGetRotation(die->body);
            DieVertex *v = &state.dice.dice[i].vertex;
            v->xxxx = Vec4(r[0], r[1], r[2],  p[0]);
            v->yyyy = Vec4(r[4], r[5], r[6],  p[1]);
            v->zzzz = Vec4(r[8], r[9], r[10], p[2]);
           memcpy(&buffer[i], v, sizeof(DieVertex));
        }
        sg_update_buffer(state.dice.bind.vertex_buffers[1], &(sg_range){
            .ptr = buffer,
            .size = state.dice.count * sizeof(DieVertex)
        });
    }
    
    vs_dice_params_t vs_dice_params = {
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
    sg_draw(0, obj_dice_face_count, state.dice.count);
    
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
                    if (state.dice.count < MAX_DICE) {
                        Die *die = &state.dice.dice[state.dice.count++];
                        die->body = dBodyCreate(state.world);
                        die->geom = dCreateBox(state.space, .2, .2, .2);
                        dGeomSetBody(die->geom, die->body);
                        
                        dMass mass;
                        dMassSetBox(&mass, 1, 1, 1, 1);
                        dBodySetMass(die->body, &mass);
                        dBodySetPosition(die->body, 0.f, 5.f, 0.f);
                    }
                    break;
                default:
                    break;
            }
            break;
        case SAPP_EVENTTYPE_RESIZED:
            state.proj = Perspective(45.f, sapp_widthf()/sapp_heightf(), .01f, 1000.f);
            break;
        default:
            break;
    }
}

void cleanup(void) {
    dCloseODE();
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
