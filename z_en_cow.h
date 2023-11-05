#ifndef Z_EN_COW_H
#define Z_EN_COW_H

#include "ultra64.h"
#include "global.h"
#include "assets/objects/object_cow/object_cow.h"

enum CowMenu {
    COW_MENU_MESH,
    COW_MENU_LIGHT_TYPE,
    COW_MENU_LIGHT_COLOR,
    COW_MENU_KC,
    COW_MENU_KL,
    COW_MENU_KQ,
    COW_MENU_MAX
};

enum LightType {
    LIGHT_TYPE_NONE,
    LIGHT_TYPE_1DIR,
    LIGHT_TYPE_1POINT,
    LIGHT_TYPE_2DIRSAME,
    LIGHT_TYPE_2DIRDIFF,
    LIGHT_TYPE_2POINTSAME,
    LIGHT_TYPE_2POINTDIFF,
    LIGHT_TYPE_POINTDIR,
    LIGHT_TYPE_MAXDIRDIFF,
    LIGHT_TYPE_MAXPOINTDIFF,
    LIGHT_TYPE_MAX
};

typedef struct {
    Vec3s rot;
    Vec3s vel;
} CowDirLight;

typedef struct {
    Vec3f pos;
    Vec3f vel;
} CowPointLight;

typedef union {
    CowDirLight d;
    CowPointLight p;
} CowBothLight;

typedef struct {
    CowBothLight l;
    Color_RGB8 col;
    u8 isPoint;
} CowLight;

typedef struct EnCow {
    Actor actor;
    s8 menuIndex;
    s8 meshIndex;
    u8 meshRotating;
    Vec3s meshRot;
    s8 lightType;
    u8 lightsMoving;
    s8 lightColorIndex;
    u8 showLightVis;
    u8 k[3]; // kc, kl, kq
    Color_RGB8 amb;
    u8 numLights;
    CowLight lights[G_MAX_LIGHTS];
} EnCow;

#endif
