// ----------
// NeoPop Duo
// ----------

#include "duo_helper.h"
#include "duo_settings.h"

void MDFNGI_reset(MDFNGI *gameinfo)
{
   gameinfo->Settings = NGPSettings;
   gameinfo->MasterClock = MDFN_MASTERCLOCK_FIXED(6144000);
   gameinfo->fps = 0;
   gameinfo->multires = false; // Multires possible?
  
   gameinfo->lcm_width = 160;
   gameinfo->lcm_height = 152;
   gameinfo->dummy_separator = NULL;
  
   gameinfo->nominal_width = 160;
   gameinfo->nominal_height = 152;
  
   gameinfo->fb_width = 160;
   gameinfo->fb_height = 152;
  
   gameinfo->soundchan = 2;
}

