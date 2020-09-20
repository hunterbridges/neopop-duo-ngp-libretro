// ----------
// NeoPop Duo
// ----------

#ifndef DUO_SETTINGS_H
#define DUO_SETTINGS_H

#include "../mednafen/mednafen-types.h"
#include "../mednafen/git.h"

#define MEDNAFEN_CORE_NAME_MODULE "ngp"
#define MEDNAFEN_CORE_NAME "NeoPop Duo"
#define MEDNAFEN_CORE_VERSION "v0.1.00.0"
#define MEDNAFEN_CORE_EXTENSIONS "ngp|ngc|ngpc|npc"
#define MEDNAFEN_CORE_TIMING_FPS 60.25
#define MEDNAFEN_CORE_GEOMETRY_BASE_W 160 
#define MEDNAFEN_CORE_GEOMETRY_BASE_H 152
#define MEDNAFEN_CORE_GEOMETRY_MAX_W 160
#define MEDNAFEN_CORE_GEOMETRY_MAX_H 152
#define MEDNAFEN_CORE_GEOMETRY_ASPECT_RATIO (20.0 / 19.0)
#define FB_WIDTH 160
#define FB_HEIGHT 152

#define FB_MAX_HEIGHT FB_HEIGHT

#define MAX_PLAYERS 2
#define MAX_BUTTONS 7

extern const MDFNSetting_EnumList LanguageList[];
extern MDFNSetting NGPSettings[];
extern const InputDeviceInputInfoStruct IDII[];
extern InputDeviceInfoStruct InputDeviceInfo[];
extern const InputPortInfoStruct PortInfo[];
extern InputInfoStruct InputInfo;
extern const FileExtensionSpecStruct KnownExtensions[];

#endif
