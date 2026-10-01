// Decompiled by space-bunny-free, verified by GPT-6.1-sol, edited by deepseek-v4.1,
// finished by deepseek-v4.1-flash and mimo-v2.6-pro. Names are provisional.
// Reads the installed DirectX version: first through dsetup.dll's
// DirectXSetupGetVersion, then, if that fails, through
// HKLM\Software\Microsoft\DirectX (the "InstalledVersion" DWORD on NT, the
// "Version" string on Win9x), and compares the result with the wanted version.
//
// mimo-v2.6-pro retry: 56.2%. The lever that finally moved the allocator was
// a five-field aggregate `struct V { unsigned minlo, majhi, majlo, minhi;
// DWORD status; } v = {0};` with the halves accessed through macros. That
// gives the original's entry zeroing shape: one `xor esi,esi` plus `mov
// reg,esi` copies, because MSVC expands a partial aggregate init `{0}` with
// copies instead of one xor per field. Plain scalar `= 0` inits, chains
// (`a = b = c = d = 0;`, all orders), and init-from-a-variable all fold to one
// `xor reg,reg` per field and never copy. Separating `ntver` from `status`
// (two locals sharing one slot in the original: the proc result dies at
// 0x4b50f5 and the registry DWORD is born at 0x4b5163) also mattered: with
// `&status` passed to RegQueryValueExA the whole struct stays in memory.
//
// What still differs: the aggregate init emits two fresh xors (minlo and
// majhi) where the original has one xor and a third `mov` copy, and lib still
// lands in ebx instead of the [esp+0x20] spill, so the halves shift one
// register (majhi ebp, majlo esi, minhi edi here vs majhi ebx, majlo ebp,
// minhi edi, minlo esi in the original) and minlo spills to [esp+0x20]. The
// extraction block and the strtok/atoi parse follow from that mapping. The
// failure exits still spell `memset(version, 0, 16); return 0;` twice; the
// original shares one tail at 0x4b52ba (xor edx,edx interleaved with the
// epilogue pops), and the shared-tail `goto fail` form costs score elsewhere
// (53.3%). Probes under build/scratch/0x4b5070/probe*.cpp show `{0}` on a
// 4 or 5 element aggregate always emits exactly two fresh xors then copies,
// so the original's single xor may need a one-explicit-value aggregate whose
// first element is the zero source.
//
// Kept as in the original: the "installed version is older" arm at 0x4b5233
// compares the major half against argument 2 (the minor half) instead of
// argument 1, and the Win9x query passes a 30-byte size for a buffer the
// frame only has room for from 0x28 to 0x45.
#include <windows.h>
#include <string.h>
#include <stdlib.h>

typedef int (__stdcall *FN_DIRECTXSETUPGETVERSION)(DWORD* major, DWORD* minor);


// FUNCTION: 0x4b5070
int __stdcall FUN_004b5070(int want0, int want1, int want2, int want3, int want4)
{
#define minlo v.f0
#define majhi v.f1
#define majlo v.f2
#define minhi v.f3
#define status v.f4
    struct V { unsigned f0, f1, f2, f3; DWORD f4; } v = {0};
    int isNT;
    HMODULE lib;

    lib = LoadLibraryA("dsetup.dll");
    isNT = 0;
    if (lib) {
        FARPROC proc = GetProcAddress(lib, "DirectXSetupGetVersion");
        if (proc) {
            DWORD dwMaj = 0;
            DWORD dwMin = 0;

            status = ((FN_DIRECTXSETUPGETVERSION)proc)(&dwMaj, &dwMin);
            if (status) {
                majhi = dwMaj >> 16;
                majlo = dwMaj & 0xffff;
                minhi = dwMin >> 16;
                minlo = dwMin & 0xffff;
            }
        }
        FreeLibrary(lib);
    }
    if (!status) {
        HKEY hKey = 0;
        OSVERSIONINFOA osvi;
        DWORD type;
        DWORD size;
        DWORD ntver;
        char version[30];

        isNT = 0;
        osvi.dwOSVersionInfoSize = sizeof(osvi);
        if (GetVersionExA(&osvi)) {
            isNT = osvi.dwPlatformId == 2;
        }
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\DirectX", 0, KEY_READ, &hKey) == 0) {
            LONG err;

            ntver = 0;
            if (isNT) {
                size = 4;
                err = RegQueryValueExA(hKey, "InstalledVersion", 0, &type, (LPBYTE)&ntver, &size);
            } else {
                size = 30;
                err = RegQueryValueExA(hKey, "Version", 0, &type, (LPBYTE)version, &size);
            }
            RegCloseKey(hKey);
            if (err) {
                memset(version, 0, 16);
                return 0;
            }
            if (isNT) {
                majlo = ntver & 0xff;
            } else {
                majhi = atoi(strtok(version, "."));
                majlo = atoi(strtok(0, "."));
                minhi = atoi(strtok(0, "."));
                minlo = atoi(strtok(0, "."));
            }
        } else {
            memset(version, 0, 16);
            return 0;
        }
    }
    if (isNT) {
        return majlo >= (unsigned int)want4;
    }
    if (majhi == (unsigned int)want0) {
        if (majlo == (unsigned int)want1) {
            if (minhi == (unsigned int)want2) {
                return minlo >= (unsigned int)want3;
            }
            return minhi >= (unsigned int)want2;
        }
        return majlo >= (unsigned int)want1;
    }
    return majhi >= (unsigned int)want1;
}
