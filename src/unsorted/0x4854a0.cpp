// Decompiled by DeepSeek V4.1 Flash. Names are provisional.
// NOT a match. The unit-pool setup and the pool/free-list linking are in place;
// what still differs is the inlined std::sort over the ten player pointers
// (0x48562b..0x48587e). See the note at the bottom.

#include <algorithm>
#include <string.h>

class Class_00435100 {
public:
    int FUN_00435100();
};

struct Player_004854a0 {
    char unknown_0[4];
    unsigned int key;                   // +0x4
    char unknown_8[0x146 - 8];
    unsigned char field_146;            // +0x146
    char unknown_147[0x14b - 0x147];
};

#pragma pack(push, 1)
struct Game_004854a0 {
    char unknown_0[0x1b63];
    Player_004854a0 players[10];        // +0x1b63, stride 0x14b
    char unknown_2871[0x1434f - 0x2871];
    unsigned short field_1434f;         // +0x1434f
    unsigned short poolCount;           // +0x14351
    char unknown_14353[0x14357 - 0x14353];
    unsigned char* pool;                // +0x14357
    unsigned char* field_1435b;         // +0x1435b
    void* hotUnits;                     // +0x1435f
    void* hotRadar;                     // +0x14363
    char unknown_14367[0x1436f - 0x14367];
    unsigned short field_1436f;         // +0x1436f
    char unknown_14371[0x14373 - 0x14371];
    unsigned int field_14373;           // +0x14373
    char unknown_14377[0x1439b - 0x14377];
    unsigned int field_1439b;           // +0x1439b
    char unknown_1439f[0x37ee6 - 0x1439f];
    unsigned short unitsPerPlayer;      // +0x37ee6
    char unknown_37ee8[0x391e9 - 0x37ee8];
    Class_00435100* mode;               // +0x391e9
};
#pragma pack(pop)

extern Game_004854a0* g_game;

void* FUN_004d83b0(const char* name, unsigned int size);
int __stdcall FUN_00485940(Player_004854a0* a, Player_004854a0* b);

// FUNCTION: 0x4854a0
void __stdcall FUN_004854a0(void)
{
    g_game->field_1436f = 0;
    g_game->field_14373 &= 0xfffffffd;
    g_game->field_1434f = g_game->unitsPerPlayer;
    g_game->poolCount = (unsigned short)(g_game->unitsPerPlayer * 10 + 1);

    g_game->pool = (unsigned char*)FUN_004d83b0("UNIT MEMORY", g_game->poolCount * 0x118);
    memset(g_game->pool, 0, g_game->poolCount * 0x118);

    g_game->hotUnits = FUN_004d83b0("HOT UNITS", g_game->unitsPerPlayer * 0x14);
    g_game->hotRadar = FUN_004d83b0("HOT RADAR UNITS", g_game->unitsPerPlayer * 100);
    g_game->field_1435b = g_game->pool + g_game->poolCount * 0x118 - 0x118;

    unsigned short n;
    for (n = 0; n < g_game->poolCount; n++) {
        *(unsigned short*)(g_game->pool + n * 0x118 + 0xa8) = n;
        *(unsigned int*)(g_game->pool + n * 0x118 + 0x92) = g_game->field_1439b;
    }

    Player_004854a0* v[10];
    int k;
    for (k = 0; k < 10; k++)
        v[k] = &g_game->players[k];

    std::sort(v, v + 10, FUN_00485940);

    g_game->pool[0xff] = 0xff;
    *(unsigned int*)(g_game->pool + 0x96) = 0;
    int i;
    for (i = 0; i < 10; i++) {
        Player_004854a0* item = v[i];
        int c = g_game->unitsPerPlayer * i + 1;
        unsigned char* slot = g_game->pool + c * 0x118;
        *(unsigned char**)((char*)item + 0x67) = slot;
        *(unsigned char**)((char*)item + 0x6b) = slot + g_game->unitsPerPlayer * 0x118 - 0x118;
        *(unsigned short*)((char*)item + 0x6f) = *(unsigned short*)(slot + 0xa8);
        *(unsigned short*)((char*)item + 0x71) =
            *(unsigned short*)(*(unsigned char**)((char*)item + 0x6b) + 0xa8);
        for (unsigned char* q = slot; q <= *(unsigned char**)((char*)item + 0x6b); q += 0x118) {
            *(void**)(q + 0x96) = item;
            q[0xff] = *(unsigned char*)((char*)item + 0x146);
            *(unsigned int*)(q + 0xac) = 0xffffffffu;
        }
    }
}

// Still to fix: the std::sort call above has to inline exactly as the original
// does (0x48562b fill, 0x485657 insertion sort of ten pointers using the
// inlined FUN_00485940 compare and an out-of-line FUN_00488920 unguarded
// insert, 0x4856c7 the dead __introsort_loop body, 0x48578e the tail merge).
// Everything before 0x48562b and the final linking loop from 0x48587e are a
// first cut and not yet scored.
