// Decompiled by DeepSeek V4.1 Flash. Names are provisional.
// PARTIAL. Frame (sub esp,0x100 + 4 saved regs), __stdcall (id, file),
// recursion and the load flow are identified, but the body is not yet
// instruction-exact. Missing: the 0x10f/0x110 bitfield merge sequence and the
// 3-iteration piece-table copy at +0xc, and the exact FUN_00485f50 argument
// order. See the notes at the bottom.

// 0x487080: load one saved unit.  Signature proven from call sites
// (0x486fd0 passes a 0xb8-byte save record's id and the open file) and from the
// prologue: `test bx,bx` / `and ecx,0xffff` -> first arg is unsigned short, and
// `ret 8` -> __stdcall.
//
// g_game at 0x511de8; units array pointer at g_game+0x14357, stride 0x118.
// A record id of 0 points at no unit and returns 0.  Bit 0x10000000 of
// unit+0x110 marks the unit as already alive, also returning 0.

extern "C" int sprintf(char* buf, const char* fmt, ...);

struct Vec3_00487080 {
    int x, y, z;
};

#pragma pack(push, 1)
// The 0xb8-byte record 0x486fd0 reads; the same layout 0x487080 consumes.
// Name is at +0x0 (looked up with FUN_00488b10), id at +0x21.
struct SaveRec_00487080 {
    char name[0x21];                    // +0x0
    unsigned short id;                  // +0x21
    char unknown_23[0xb8 - 0x23];
};
#pragma pack(pop)

class Class_004b4560 {
public:
    int FUN_004b4560(char* name);
};
class Class_004b4800 {
public:
    int FUN_004b4800(char* name, int def);
};
class Class_004b4b50 {
public:
    int FUN_004b4b50(int a);
};
class Class_004b4c10 {
public:
    void FUN_004b4c10(int pos);
};
class Class_004b4c80 {
public:
    int FUN_004b4c80(void* buf, int len);
};
class Class_004b4ba0 {
public:
    int FUN_004b4ba0(char* name);
};

struct Unit_00487080;

extern void* g_game;

unsigned short __stdcall FUN_00488b10(const char* name);
Unit_00487080* __stdcall FUN_00485f50(unsigned char player, unsigned short typeId,
                                      Vec3_00487080 pos, int param_5, int mode,
                                      unsigned short id);
void __stdcall FUN_0048aac0(Unit_00487080* unit, Unit_00487080* builder, char piece, char p4);
void __stdcall FUN_00480250(Unit_00487080* unit, int id);
Unit_00487080* __stdcall FUN_0043a420(void* self, Unit_00487080* unit, void* file, char* name);
void __stdcall FUN_004388b0(int o);
void __stdcall FUN_0047db20(Unit_00487080* unit);
void __stdcall FUN_00401110(void* info, Unit_00487080* unit, void* file);
void __stdcall FUN_0043de30(void* vt, Unit_00487080* unit, void* file);
void __stdcall FUN_004b2040(void* o, void* file);
// FUNCTION: 0x487080
Unit_00487080* __stdcall FUN_00487080(unsigned short id, Class_004b4560* file)
{
    Unit_00487080* unit;
    if (id == 0) {
        unit = 0;
    } else {
        unit = (Unit_00487080*)(*(char**)((char*)g_game + 0x14357) + id * 0x118);
    }
    if (unit == 0 || (*(unsigned int*)((char*)unit + 0x110) & 0x10000000))
        return 0;
    return unit;
}

// Analysis notes for the next attempt (from disassembly, not yet coded):
//  - Record read at esp+0x18 (0xb8 bytes). Name at +0x0 -> FUN_00488b10 -> typeId.
//  - FUN_00485f50 call args pushed high->low: id(+0x21), (flags>>4)&3, 1,
//    {+0x2b,+0x2f,+0x33}, typeId, +0x20. Real signature is
//    (player, typeId, pos(3), param_5, mode, id).
//  - Record -> unit stores: +0x2f->+0x6e, +0x37->+0x64, +0x3b->+0x68,
//    +0x3d->+0x108, +0x3f->+0xb8, +0x89->recursive child, +0x8b->+0xf0,
//    +0x8d->+0xf9, +0x8e->+0xf4, +0x8f->+0x58, +0x93->+0x76, +0x97->+0x7a,
//    +0x9b->+0x7e, +0x9f->+0xac, +0xab->+0x104, +0xaf->+0xf5, +0xb0->+0xf6,
//    +0xb1->+0xf7, +0xb2->+0xba, +0xb4->+0x10f/+0x110 bitfield merge,
//    +0xb7->+0xf8, +0xb8->+0xfa, +0xb9->+0x10e.
//  - Unit header: +0x9a pointer (FUN_004b2040), list head +0x5c / tail +0x60,
//    +0xbc object (FUN_00401110), +0xc 3x 0x1c-byte piece copies.
