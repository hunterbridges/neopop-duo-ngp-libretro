// ----------
// NeoPop Duo
// ----------

#ifndef DUO_HELPER_H
#define DUO_HELPER_H

#include "../mednafen/mednafen-types.h"
#include "../mednafen/git.h"

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

void MDFNGI_reset(MDFNGI *gameinfo);

#endif
