/*
 * File: z_en_cow.c
 * Overlay: ovl_En_Cow
 * Description: Lighting test objects
 */

#include "z_en_cow.h"

extern MtxF* Matrix_GetCurrent();
extern void Matrix_SetTranslateUniformScaleMtxF(MtxF* mf, f32 scale,
    f32 translateX, f32 translateY, f32 translateZ);

#define FLAGS 0x00000030
#define SCALE 0.01f

void EnCow_Init(Actor* thisx, PlayState* play);
void EnCow_Destroy(Actor* thisx, PlayState* play);
void EnCow_Update(Actor* thisx, PlayState* play2);
void EnCow_Draw(Actor* thisx, PlayState* play);

ActorInit En_Cow_InitVars = {
    ACTOR_EN_COW,
    ACTORCAT_BG,
    FLAGS,
    OBJECT_COW,
    sizeof(EnCow),
    (ActorFunc)EnCow_Init,
    (ActorFunc)EnCow_Destroy,
    (ActorFunc)EnCow_Update,
    (ActorFunc)EnCow_Draw,
};

#define XSTR(s) STR(s)
#define STR(s) #s
static const char* const LightTypeNames[LIGHT_TYPE_MAX] = {
    "None",
    "1Dir",
    "1Point",
    "2DirSame",
    "2DirDiff",
    "2PointSame",
    "2PointDiff",
    "PointDir",
    XSTR(G_MAX_LIGHTS) "DirDiff",
    XSTR(G_MAX_LIGHTS) "PointDiff",
};
static const u8 LightTypeCounts[LIGHT_TYPE_MAX] = {
    0, 1, 1, 2, 2, 2, 2, 2, G_MAX_LIGHTS, G_MAX_LIGHTS,
};
static const u8 LightTypeRandColors[LIGHT_TYPE_MAX] = {
    0, 0, 0, 0, 1, 0, 1, 1, 1, 1,
};
static const u8 LightTypeModes[LIGHT_TYPE_MAX] = {
    0, 0, 1, 0, 0, 1, 1, 2, 0, 1
};

#define NUM_COLORS 18
static Color_RGB8 const Colors[NUM_COLORS] = {
    {255, 255, 255},
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 255, 0},
    {0, 255, 255},
    {255, 0, 255},
    {255, 128, 0},
    {128, 0, 255},
    {255, 128, 128},
    {128, 255, 128},
    {128, 128, 255},
    {255, 255, 128},
    {128, 255, 255},
    {255, 128, 255},
    {192, 192, 192},
    {128, 128, 128},
    {64, 64, 64},
};

static const char* const ColorNames[NUM_COLORS] = {
    "White",
    "Red",
    "Green",
    "Blue",
    "Yellow",
    "Cyan",
    "Magenta",
    "Orange",
    "Purple",
    "Light Red",
    "Light Green",
    "Light Blue",
    "Light Yellow",
    "Light Cyan",
    "Light Magenta",
    "Light Gray",
    "Gray",
    "Dark Gray",
};

static void SetLightColor(EnCow* this, s32 slot){
    u8 idx = LightTypeRandColors[this->lightType] ? 
        (u8)(Rand_Next() % NUM_COLORS) : (u8)(this->lightColorIndex);
    this->lights[slot].col = Colors[idx];
}

static void InitLight(EnCow* this, s32 slot, bool isPoint){
    this->lights[slot].isPoint = isPoint;
    SetLightColor(this, slot);
    if(isPoint){
        this->lights[slot].l.p.pos = (Vec3f){
            this->actor.home.pos.x + Rand_CenteredFloat(80.0f),
            this->actor.home.pos.y + Rand_CenteredFloat(80.0f),
            this->actor.home.pos.z + Rand_CenteredFloat(80.0f)
        };
        this->lights[slot].l.p.vel = (Vec3f){
            Rand_CenteredFloat(1.0f),
            Rand_CenteredFloat(1.0f),
            Rand_CenteredFloat(1.0f)
        };
    }else{
        this->lights[slot].l.d.rot = (Vec3s){
            Rand_Next(),
            Rand_Next(),
            Rand_Next()
        };
        this->lights[slot].l.d.vel = (Vec3s){
            (s16)(Rand_Next() & 0x3FF) - 0x200,
            (s16)(Rand_Next() & 0x3FF) - 0x200,
            (s16)(Rand_Next() & 0x3FF) - 0x200
        };
    }
}

void EnCow_Init(Actor* thisx, PlayState* play) {
    EnCow* this = (EnCow*)thisx;
    Actor_SetScale(&this->actor, SCALE);
    this->menuIndex = 0;
    this->meshIndex = 0;
    this->meshRotating = 0;
    this->meshRot = (Vec3s){0, 0, 0};
    
    this->lightType = 1;
    this->lightsMoving = 1;
    this->lightColorIndex = 0;
    this->showLightVis = 1;
    this->k[0] = 5;
    this->k[1] = 20;
    this->k[2] = 140;
    this->amb.r = this->amb.g = this->amb.b = 0x20;
    
    this->numLights = 1;
    InitLight(this, 0, false);
}

void EnCow_Destroy(Actor* thisx, PlayState* play) {
    EnCow* this = (EnCow*)thisx;

}

static void PlaySound(s16 sound){
    Audio_PlaySfxGeneral(sound, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                         &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

static inline void CursorThroughMenu(s8* index, s8 max, bool* left, bool* right){
    if(*left) --*index;
    if(*right) ++*index;
    if(*index < 0){
        *index = 0;
        *left = false;
    }
    if(*index >= max){
        *index = max - 1;
        *right = false;
    }
}

static inline void CheckToggleAttribute(bool doToggle, u8* attr){
    if(!doToggle) return;
    *attr = !*attr;
    PlaySound(*attr ? NA_SE_SY_DECIDE : NA_SE_SY_CANCEL);
}

#define NUM_MESHES 21
static Gfx* const Meshes[NUM_MESHES] = {
    Suzanne32Verts,
    SuzanneNormals,
    SuzanneColors,
    SuzannePacked,
    SuzanneLtToA,
    SuzanneAComp,
    SuzanneACRev,
    SuzanneCel,
    SuzanneCelTex,
    SuzanneCelTexLerp,
    SuzanneCelBlend,
    SuzanneCelTexBlend,
    SuzanneCelLtCol,
    SuzanneCelAO,
    SuzanneCelThreshs,
    NULL,
    SuzanneLtToA,
    SuzanneCelBlend,
    SuzanneCulBlend,
    SuzanneCelThreshs,
    SuzanneCulThreshs,
};
static const char* const MeshNames[NUM_MESHES] = {
    "Suzanne32Verts",
    "SuzanneNormals",
    "SuzanneColors",
    "SuzannePacked",
    "SuzanneLtToA",
    "SuzanneAComp",
    "SuzanneACRev",
    "SuzanneCel",
    "SuzanneCelTex",
    "SuzanneCelTexLerp",
    "SuzanneCelBlend",
    "SuzanneCelTexBlend",
    "SuzanneCelLtCol",
    "SuzanneCelAO",
    "SuzanneCelThreshs",
    "====Nothing====",
    "SuzanneLtToA",
    "SuzanneCelBlend",
    "SuzanneCulBlend",
    "SuzanneCelThreshs",
    "SuzanneCulThreshs",
};
void EnCow_Update(Actor* thisx, PlayState* play) {
    EnCow* this = (EnCow*)thisx;
    if(this->meshRotating){
        this->meshRot.x += 100;
        this->meshRot.z -= 70;
    }
    
    if(this->lightsMoving){
        for(s32 i=0; i<this->numLights; ++i){
            if(this->lights[i].isPoint){
                CowPointLight* l = &this->lights[i].l.p;
                l->pos.x += l->vel.x;
                l->pos.y += l->vel.y;
                l->pos.z += l->vel.z;
                l->vel.x += Rand_CenteredFloat(0.1f);
                l->vel.y += Rand_CenteredFloat(0.1f);
                l->vel.z += Rand_CenteredFloat(0.1f);
                if(l->pos.x > this->actor.home.pos.x +  70.0f) l->vel.x -= 0.1f;
                if(l->pos.x < this->actor.home.pos.x + -70.0f) l->vel.x += 0.1f;
                if(l->pos.y > this->actor.home.pos.y +  40.0f) l->vel.y -= 0.1f;
                if(l->pos.y < this->actor.home.pos.y + -40.0f) l->vel.y += 0.1f;
                if(l->pos.z > this->actor.home.pos.z +  40.0f) l->vel.z -= 0.1f;
                if(l->pos.z < this->actor.home.pos.z + -40.0f) l->vel.z += 0.1f;
                if(l->vel.x > 2.0f) l->vel.x *= 0.9f;
                if(l->vel.y > 1.0f) l->vel.y *= 0.9f;
                if(l->vel.z > 2.0f) l->vel.z *= 0.9f;
            }else{
                CowDirLight* l = &this->lights[i].l.d;
                l->rot.x += l->vel.x;
                l->rot.y += l->vel.y;
                l->rot.z += l->vel.z;
            }
        }
    }
    
    Input* input = &play->state.input[0];
    bool left = CHECK_BTN_ALL(input->press.button, BTN_DLEFT);
    bool right = CHECK_BTN_ALL(input->press.button, BTN_DRIGHT);
    bool up = CHECK_BTN_ALL(input->press.button, BTN_DUP);
    bool down = CHECK_BTN_ALL(input->press.button, BTN_DDOWN);
    bool lPress = CHECK_BTN_ALL(input->press.button, BTN_L);
    bool lHold = CHECK_BTN_ALL(input->cur.button, BTN_L);
    
    if(up) --this->menuIndex;
    if(down) ++this->menuIndex;
    if(this->menuIndex < 0) this->menuIndex = COW_MENU_MAX - 1;
    if(this->menuIndex >= COW_MENU_MAX) this->menuIndex = 0;
    
    u8* kPtr = &this->k[0];
    switch(this->menuIndex){
    case COW_MENU_MESH:
        CursorThroughMenu(&this->meshIndex, NUM_MESHES, &left, &right);
        CheckToggleAttribute(lPress, &this->meshRotating);
        break;
    case COW_MENU_LIGHT_TYPE:
        CheckToggleAttribute(lPress, &this->lightsMoving);
        s8 lastLightType = this->lightType;
        CursorThroughMenu(&this->lightType, LIGHT_TYPE_MAX, &left, &right);
        if(this->lightType != lastLightType){
            u8 mode = LightTypeModes[this->lightType];
            u8 newNumLights = LightTypeCounts[this->lightType];
            u8 reinitColors = LightTypeRandColors[this->lightType]
                != LightTypeRandColors[lastLightType];
            for(s32 i=0; i<newNumLights; ++i){
                u8 isPoint = mode == 1 || (mode == 2 && i == 1);
                if(i >= this->numLights || this->lights[i].isPoint != isPoint){
                    InitLight(this, i, isPoint);
                }else if(reinitColors){
                    SetLightColor(this, i);
                }
            }
            this->numLights = newNumLights;
        }
        break;
    case COW_MENU_LIGHT_COLOR:
        CheckToggleAttribute(lPress, &this->showLightVis);
        s8 lastLightColorIndex = this->lightColorIndex;
        CursorThroughMenu(&this->lightColorIndex, NUM_COLORS, &left, &right);
        if(lastLightColorIndex != this->lightColorIndex && !LightTypeRandColors[this->lightType]){
            for(s32 i=0; i<this->numLights; ++i){
                SetLightColor(this, i);
            }
        }
        break;
    case COW_MENU_KQ:
        ++kPtr;
        __attribute__((fallthrough));
    case COW_MENU_KL:
        ++kPtr;
        __attribute__((fallthrough));
    case COW_MENU_KC:
        {
            s32 delta = lHold ? 0x10 : 1;
            delta = left ? -delta : right ? delta : 0;
            *kPtr = (u8)((s32)*kPtr + delta);
            left = right = false;
        }
        break;
    }
    
    if(up || down || left || right){
        PlaySound(NA_SE_SY_CURSOR);
    }
}

void EnCow_Draw(Actor* thisx, PlayState* play) {
    EnCow* this = (EnCow*)thisx;
    OPEN_DISPS(play->state.gfxCtx);
    
    // Menu
    GfxPrint printer;
    GfxPrint_Init(&printer);
    Gfx *opaStart = POLY_OPA_DISP;
    Gfx *gfx = Graph_GfxPlusOne(POLY_OPA_DISP);
    gSPDisplayList(OVERLAY_DISP++, gfx);
    GfxPrint_Open(&printer, gfx);
    GfxPrint_SetColor(&printer, 255, 255, 255, 255);
    GfxPrint_SetPos(&printer, 9, 26);
    switch(this->menuIndex){
    case COW_MENU_MESH:
        GfxPrint_Printf(&printer, "Mesh: %s", MeshNames[this->meshIndex]);
        break;
    case COW_MENU_LIGHT_TYPE:
        GfxPrint_Printf(&printer, "Light type: %s", LightTypeNames[this->lightType]);
        break;
    case COW_MENU_LIGHT_COLOR:
        GfxPrint_Printf(&printer, "Light color: %s", ColorNames[this->lightColorIndex]);
        break;
    case COW_MENU_KC: GfxPrint_Printf(&printer, "kc: %02X", this->k[0]); break;
    case COW_MENU_KL: GfxPrint_Printf(&printer, "kl: %02X", this->k[1]); break;
    case COW_MENU_KQ: GfxPrint_Printf(&printer, "kq: %02X", this->k[2]); break;
    }
    gfx = GfxPrint_Close(&printer);
    gSPEndDisplayList(gfx++);
    Graph_BranchDlist(opaStart, gfx);
    POLY_OPA_DISP = gfx;
    
    // Lighting
    Color_RGB8 lightColor = Colors[this->lightColorIndex];
    PosLight *lights = Graph_Alloc(play->state.gfxCtx, (this->numLights + 1) * sizeof(PosLight));
    s32 l;
    for(l=0; l < this->numLights; ++l){
        lights[l].l.col[0] = lights[l].l.colc[0] = this->lights[l].col.r;
        lights[l].l.col[1] = lights[l].l.colc[1] = this->lights[l].col.g;
        lights[l].l.col[2] = lights[l].l.colc[2] = this->lights[l].col.b;
        if(this->lights[l].isPoint){
            lights[l].p.kc = this->k[0];
            lights[l].p.kl = this->k[1];
            lights[l].p.kq = this->k[2];
            lights[l].p.pos[0] = (s16)this->lights[l].l.p.pos.x;
            lights[l].p.pos[1] = (s16)this->lights[l].l.p.pos.y;
            lights[l].p.pos[2] = (s16)this->lights[l].l.p.pos.z;
        }else{
            lights[l].l.type = 0;
            Matrix_SetTranslateRotateYXZ(0.0f, 0.0f, 0.0f, &this->lights[l].l.d.rot);
            static Vec3f unitvec = {1.0f, 0.0f, 0.0f};
            Vec3f d;
            Matrix_MultVec3f(&unitvec, &d);
            lights[l].l.dir[0] = (s8)(d.x * 127.0f);
            lights[l].l.dir[1] = (s8)(d.y * 127.0f);
            lights[l].l.dir[2] = (s8)(d.z * 127.0f);
        }
    }
    lights[l].l.col[0] = lights[l].l.colc[0] = this->amb.r;
    lights[l].l.col[1] = lights[l].l.colc[1] = this->amb.g;
    lights[l].l.col[2] = lights[l].l.colc[2] = this->amb.b;
    gSPSetLights(POLY_OPA_DISP++, this->numLights, *lights);
    
    // Mesh
    if(Meshes[this->meshIndex] != NULL){
        Matrix_SetTranslateRotateYXZ(this->actor.home.pos.x, this->actor.home.pos.y,
            this->actor.home.pos.z, &this->meshRot);
        Matrix_Scale(SCALE, SCALE, SCALE, MTXMODE_APPLY);
        gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(play->state.gfxCtx, __FILE__, __LINE__),
                  G_MTX_MODELVIEW | G_MTX_LOAD);
        gSPDisplayList(POLY_OPA_DISP++, Meshes[this->meshIndex]);
    }
    
    // Lighting visualizers
    for(l=0; l < this->numLights && this->showLightVis; ++l){
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->lights[l].col.r,
            this->lights[l].col.g, this->lights[l].col.b, 0xFF);
        if(this->lights[l].isPoint){
            Matrix_SetTranslateUniformScaleMtxF(Matrix_GetCurrent(), SCALE * 0.6f,
                this->lights[l].l.p.pos.x, this->lights[l].l.p.pos.y, this->lights[l].l.p.pos.z);
            gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(play->state.gfxCtx, __FILE__, __LINE__),
                      G_MTX_MODELVIEW | G_MTX_LOAD);
            gSPDisplayList(POLY_OPA_DISP++, PointLightSphere);
        }else{
            Matrix_SetTranslateRotateYXZ(this->actor.home.pos.x, this->actor.home.pos.y,
                this->actor.home.pos.z, &this->lights[l].l.d.rot);
            Matrix_Scale(SCALE, SCALE, SCALE, MTXMODE_APPLY);
            gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(play->state.gfxCtx, __FILE__, __LINE__),
                      G_MTX_MODELVIEW | G_MTX_LOAD);
            gSPDisplayList(POLY_OPA_DISP++, DirLightArrow);
        }
    }
    
    // Reset state
    Gfx_SetupDL_25Opa(play->state.gfxCtx);
    CLOSE_DISPS(play->state.gfxCtx);
}
