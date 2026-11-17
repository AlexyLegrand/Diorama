#include "world/blocktype.h"

const BlockInfo BLOCK_INFO[BLOCK_COUNT] = {
    [BLOCK_EMPTY]  = { .solid = false, .walkable_on = false, .color = BLANK },
    [BLOCK_GROUND] = { .solid = true,  .walkable_on = true,  .color = GREEN },
    [BLOCK_CRATE]  = { .solid = true,  .walkable_on = true,  .color = BROWN },
    [BLOCK_STAIRS] = { .solid = true,  .walkable_on = false,  .color = BEIGE },
    [BLOCK_LADDER] = { .solid = false, .walkable_on = false, .color = ORANGE },
    [BLOCK_STAR]     = { .solid = false, .walkable_on = false, .color = GOLD },
    [BLOCK_CRANK]    = { .solid = true,  .walkable_on = false, .color = MAROON },
    [BLOCK_PLATFORM] = { .solid = true,  .walkable_on = true,  .color = SKYBLUE },
    [BLOCK_PROP]     = { .solid = true,  .walkable_on = false, .color = BLANK },
};
