// ----------
// NeoPop Duo
// ----------

#include "duo_settings.h"

const MDFNSetting_EnumList LanguageList[] =
{
 { "japanese", 0, "Japanese" },
 { "0", 0 },

 { "english", 1, "English" },
 { "1", 1 },

 { NULL, 0 },
};

const MDFNSetting_EnumList VideoMixList[] =
{
 { "p1", 0, "Show P1 Only" },
 { "0", 0 },

 { "p2", 1, "Show P2 Only" },
 { "1", 1 },

 { "both", 2, "Show Both Players" },
 { "2", 2 },

 { NULL, 0 },
};

const MDFNSetting_EnumList AudioMixList[] =
{
 { "p1", 0, "Play P1 Audio Only" },
 { "0", 0 },

 { "p2", 1, "Play P2 Audio Only" },
 { "1", 1 },

 { "both", 2, "Play Audio from Both Players" },
 { "2", 2 },

 { "match", 3, "Match Video Mix" },
 { "3", 3 },

 { NULL, 0 },
};

const MDFNSetting_EnumList VideoLayoutList[] =
{
 { "h", 0, "Horizontal" },
 { "0", 0 },

 { "v", 1, "Vertical" },
 { "1", 1 },

 { "sh", 2, "Horizontal (Switched)" },
 { "2", 2 },

 { "sv", 2, "Vertical (Switched)" },
 { "3", 3 },

 { NULL, 0 },
};

MDFNSetting NGPSettings[] =
{
 { "ngp.language", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, "Language games should display text in.", NULL, MDFNST_ENUM, "english", NULL, NULL, NULL, NULL, LanguageList },
 { "ngp.videomix", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, "Choose which screens to display.", NULL, MDFNST_ENUM, "both", NULL, NULL, NULL, NULL, VideoMixList },
 { "ngp.videolayout", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, "Layout when displaying both screens.", NULL, MDFNST_ENUM, "h", NULL, NULL, NULL, NULL, VideoLayoutList },
 { "ngp.audiomix", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, "Choose which screens to play audio for.", NULL, MDFNST_ENUM, "match", NULL, NULL, NULL, NULL, AudioMixList },
 { NULL }
};

const InputDeviceInputInfoStruct IDII[] =
{
 { "up", "UP ↑", 0, IDIT_BUTTON, "down" },
 { "down", "DOWN ↓", 1, IDIT_BUTTON, "up" },
 { "left", "LEFT ←", 2, IDIT_BUTTON, "right" },
 { "right", "RIGHT →", 3, IDIT_BUTTON, "left" },
 { "a", "A", 5, IDIT_BUTTON_CAN_RAPID,  NULL },
 { "b", "B", 6, IDIT_BUTTON_CAN_RAPID, NULL },
 { "option", "OPTION", 4, IDIT_BUTTON, NULL },
};

InputDeviceInfoStruct InputDeviceInfo[] =
{
 {
  "gamepad",
  "Gamepad",
  NULL,
  NULL,
  sizeof(IDII) / sizeof(InputDeviceInputInfoStruct),
  IDII,
 }
};

const InputPortInfoStruct PortInfo[] =
{
 { "builtin", "Built-In", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo, "gamepad" }
};

InputInfoStruct InputInfo =
{
 sizeof(PortInfo) / sizeof(InputPortInfoStruct),
 PortInfo
};

const FileExtensionSpecStruct KnownExtensions[] =
{
 { ".ngp", "Neo Geo Pocket ROM Image" },
 { ".ngc", "Neo Geo Pocket Color ROM Image" },
 { NULL, NULL }
};

