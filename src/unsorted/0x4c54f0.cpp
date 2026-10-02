// Decompiled by space-bunny-free, finished by deepseek-v4.1-flash; further tried by GPT-6.1-sol, edited by deepseek-v4.1, further tried by Space Bunny Free. Names are provisional.
//
// Space Bunny Free, issue 4579, LAST PASS: 70.2 -> 78.6 percent (558 of 583
// bytes), no MATCH. Six workers had been stuck at 70.2 for six issues; this pass
// broke it. Read this first, because every earlier note below says 70.2 and
// several of them call a change a dead end that is now worth 5 points.
//
// The four changes, in the order they were found. None of the four is the
// destructor's register rotation, which five workers chased:
//
// 1. THE INSERT CONDITION'S POLARITY WAS WRONG, and the way to get it right is to
//    put the ASSIGN in the then-branch and the INSERT in the else. The exe tests
//    `xor ecx,ecx; cmp eax,ebx; sete cl; neg cl; sbb ecx,ecx; inc ecx; test cl,cl;
//    jne 0x4c56a2` and 0x4c56a2 is the insert, so it inserts when the found key
//    EQUALS the key being added. Written as
//    `if (!(atEnd || (strcmp(e->key.ptr, key.ptr) == 0))) { assign } else { insert }`
//    that is what the file below says. Every earlier attempt kept the old
//    `if (e == last || !(strcmp(...) == 0)) { insert } else { assign }`, which is
//    the same bytes at the same score until the rest of this pass lands.
// 2. DAT_005119b8 IS `char[]`, NOT `char*`, and that is worth 5.7 points (72.9 to
//    78.6) once 1, 3 and 4 are in. Three earlier passes measured it in isolation
//    and got 68.7, so they concluded it was a dead end: it is not, it is only a
//    dead end while the insert branch is the wrong way round. The exe pushes
//    `push 0x5119b8`, an immediate address, and data/globals.csv agrees (char[]
//    in 74 of 80 files). THIS IS THE LESSON: a measured dead end is only a dead
//    end for the spelling that was measured.
// 3. The FUN_004c5c60 receiver must be read from the GLOBAL, not from a local:
//    `e = ((Class_004c5c60*)DAT_0051fdb8)->FUN_004c5c60(key.ptr);` sitting next to
//    an unrelated `s = DAT_0051fdb8;` (still needed for `last` and for the
//    insert; dropping it is 59.0). That is what turns the exe's
//    `mov ecx,[DAT] ... mov ebp,ecx` (load into the receiver, copy for later)
//    into ours: 72.9. Reading `last` from the global as well is 71.8, and moving
//    the `s = DAT_0051fdb8;` after the call is 71.4.
// 4. The new + the old map are built by TWO STATIC INLINE HELPERS, `MapIsLoaded`
//    and `LoadMap`, not by statements in the body. Writing them out in the body
//    scores 69.2, and dropping MapIsLoaded for a plain `if (DAT_0051fdb8)` scores
//    70.2, so both helpers are load-bearing: `MapIsLoaded` is what turns the
//    null test into a materialised bool (`xor ecx,ecx; test edi,edi; setne cl;
//    test cl,cl`), which is three instructions further from the original's plain
//    `test eax,eax; je` but lands in the right register state everywhere else.
// Plus one detail inside `Vec_004c54f0::Free2`: the zeroing order after the
// `operator delete` is `last, first, end`, and the exe stores [edi+4], [edi+8],
// [edi+0xc] in that order, so `last` first is right and the old spelling had it
// the other way round.
//
// 1, 3 and 4 came out of `uv run tools/permute.py 0x4c54f0 --minutes 20 --seed 11
// --jobs 4` (71.8 on its own); 2 fell out of re-checking a measured dead end once
// the rest had landed. The file below is that result after every temporary,
// self-assignment, goto, `do {} while (0)` and `while (1)` has been taken back
// out, with the control flow, the loops and the local names restored by hand.
// Each removal was checked on its own: self-assignment removal and the
// goto/do-while removal are free, simplifying `(0 != X) != 0` to `X` is worth 0.4,
// and the rest is neutral.
//
// The census below (143 of 188 instructions identical, 28 regions) is after all
// of that. What is still open, in the order it is worth trying:
//   * the destruction block, still the largest single difference, now bounded from
//     both sides by sixteen measured variants (see below),
//   * the insert test's missing `sete cl; neg cl; sbb ecx,ecx; inc ecx`: the source
//     still has to compute the key comparison twice, once as a canonical byte bool
//     and once as a 0/1 int (four attempts measured below),
//   * `mov dl` against `mov cl` for the flag (not the constructor's store order)
//     and `mov eax,ecx` against `mov edx,ecx` in the strcpy, both single registers.
//
// Re-measured on the corrected spelling, because point 2 is the lesson: the old
// numbers below were taken against a broken insert branch and every one of these
// is a fresh measurement (build/scratch/0x4c54f0/x*.cpp, y*.cpp, z*.cpp):
//   * the insert condition's materialised forms are all slightly WORSE here, so
//     leave it alone: a named `bool keysEqual` 77.9, `bool atEnd` instead of
//     `int atEnd` 76.8, and `int keysDiffer` plus `bool keysEqual = !keysDiffer`
//     (the shape the exe's `neg cl; sbb ecx,ecx; inc ecx` wants) 77.9 against
//     78.6 for the plain inline `strcmp(...) == 0`,
//   * the destructor's inline-free shapes are still dead ends on the new base,
//     but one of them is now close: the free written out in the destructor is
//     64.3, and a `~Vec_004c54f0` defined in the file and called explicitly is
//     77.9 (it emits `push 0; mov ecx,reg; call ??_G`, see below),
//   * MapIsLoaded must return `bool`: `int` is 70.9 and returning the pointer
//     itself is 70.9, because either form loses the `setne cl`.
//   * merging the FUN_004c93f0 call so both branches reach one call site (which is
//     what the exe does: `mov esi,eax; add esi,4` after the insert, then `mov ecx,esi`
//     into a shared call) is still worse: a `Class_004c93f0* slot` assigned in both
//     branches is 77.4 and an `Elem_004c5bc0* slot` with `&slot->value` after the
//     if is 76.5,
//   * and two neutral spellings, so take either: writing the whole condition out
//     with no `atEnd` local, and declaring `atEnd` inside the block instead of at
//     function scope, both stay at 78.6 with 558 bytes.
//   * the destruction block is the one thing that has NOT moved, and the helper
//     idea that worked twice elsewhere does not reach it: the free as a
//     `static inline void FreeVec(Vec_004c54f0*)` is 64.3, the whole destruction in
//     a `static inline void DestroyMap(Class_004c5840*)` is 64.3, the loop bounds
//     read through `w` instead of `v` is 64.3, and declaring `w` before `s` is
//     neutral at 78.6. It still wants the object in ebx and the vector in edi with
//     the free inlined, which no member-function or destructor spelling reaches,
//     for the reason measured further down (a member function containing
//     ::operator delete is never inlined, and one without it always folds).
//
//
// Space Bunny Free retry (issue 4401, about 40 scratch scores, all in
// build/scratch/0x4c54f0/): still 70.2 percent (553 bytes), no MATCH. The body
// IS CHANGED NOW, by the pass above; everything below that is history and says
// 70.2 because that is what was measured then. That pass only re-measured the
// alternatives and re-confirmed which of them are dead ends. Do NOT trust a
// score for `char[]` here: the first attempt at that patch replaced the word
// inside this comment block instead of the declaration (the first literal
// occurrence of `extern char* DAT_005119b8;` is on the line above, not the
// declaration), so the variant scored the same and looked free. Patched
// properly, `extern char DAT_005119b8[];` really is 68.7 percent, which
// confirms the older measurement, so the pointer stays for now.
//
// NEW MEASUREMENTS THIS PASS, so nobody repeats them:
// - The destruction block needs FOUR callee-saved registers (esi, ebp, edi,
//   ebx) for the original's 0x224 frame, because the object is copied from
//   eax into ebx and the vector is materialised as `lea edi, [eax+1]` and then
//   freed through [edi+4], [edi+8], [edi+0xc]. That is why the original
//   re-reads `section` from the stack instead of hoisting it.
// - Every spelling that inlines the free drops to three callee-saved
//   registers: the vector folds onto the object base ([edi+5], [edi+9],
//   [edi+0xd]), the frame becomes 0x220, `section` is hoisted into ebx, and
//   the score falls to 57.0. This fold resisted every shape tried this pass:
//   the free written out inline in the destructor, a real `std::vector`
//   member (57.5, and it emits ??_GElem_004c5bc0), a member destructor
//   ~Vec_004c54f0, a `static inline FreeEntries(Vec*)` helper, a two-parameter
//   `static inline FreeMap(Vec*, Class*)` helper, `(Vec*)((char*)s + 1)`
//   instead of `&s->v`, the loop bounds read through w instead of the object,
//   and the free kept as a member but reduced to two statements
//   (`this->first = this->last = this->end = 0;`) so /Ob2 does inline it.
//   The only way found to keep the vector a separate register is to leave
//   Free2 out of line, which is what this file does: right frame, right
//   0x224, but obj/vec across ebx/edi are the other way round and there is a
//   call where the original inlines.
// - `int idx[1]` and the inline free are the same threshold, not two
//   problems: with only three callee-saved registers the index is promoted to
//   ebx and the array trick stops holding. Fixing the free fixes the index.
// - The destroy loop must stay in the same body as the free. Moved into its
//   own `static inline` helper it emits `push 0; mov ecx, elem; call ??_G`
//   instead of the original's direct `call ??1Elem_004c5bc0`.
// - `delete DAT_0051fdb8;` 56.8. An explicit `->~Class_004c5840()` plus
//   `::operator delete` at the call site is 62.1, the best of the
//   inline-free family: that shape does put the vector in edi, but it has no
//   ebx for the object and re-reads the global for the second delete.
// - `char[]` for DAT_005119b8, patched properly: 68.7 (see above).
//
// THE INSERT CONDITION WAS INVERTED IN THIS FILE, AND IS NOW FIXED (see the
// pass at the top: the assign goes in the then-branch, the insert in the else).
// The rest of this note is why that reading was right. Read from the bytes at
// 0x4c568d..0x4c569b: `cmp eax, ebx` (ebx is the zero register), `sete cl`
// makes cl 1 when strcmp returned 0, `neg cl` makes it 0xff, and
// `test cl, cl; jne 0x4c56a2` therefore JUMPS TO THE INSERT when the found
// key is EQUAL to the key being added. The condition in this file is
// `!(strcmp(...) == 0)`, which inserts when they differ. The source
// condition is `e == s->last || (strcmp(e->key.ptr, key.ptr) == 0)`, and the
// int sitting beside the bool in ecx (`sbb ecx, ecx; inc ecx`) is that
// polarity, not the other one. Nothing written so far reproduces both the
// polarity and the layout at once.
// - The insert result is USED in the original: `mov esi, eax; add esi, 4`
//   right after the 0x4c59d0 call, and `mov ecx, esi` feeds a SINGLE shared
//   `FUN_004c93f0(value)` call at 0x4c56e1 that both branches reach (the
//   not-taken path does `lea ecx, [edi+4]; jmp 0x4c56e1`). This file
//   discards the result and duplicates the call. Merging the assignment
//   after the if with a separate element pointer scores 64.5 to 68.1, and
//   with a `Class_004c93f0*` local for the receiver 68.1, so the merge
//   alone is not enough.
// - Closest insert spelling found: a `static inline bool SameKey(a, b)
//   { return strcmp(a, b) == 0; }` helper in `e == last || SameKey(...)`
//   with the assignment after the if gives `xor ecx, ecx; cmp eax, ebx;
//   sete cl; test cl, cl; je` and scores 70.1 (build/scratch/0x4c54f0/g1.cpp),
//   still missing the original's `neg cl; sbb ecx,ecx; inc ecx`, and MSVC
//   hoists the strcmp above the `e == last` test. Whoever retries should
//   start from g1 and look for the int form of the comparison beside the
//   bool, which means the comparison result is used twice in the source.
// - Also still open and unaffected by any of this: the FUN_004c4420 /
//   FUN_004c48c0 argument order (the original loads `f.current` into ecx
//   first, then lea's the buffer into edx, then pushes), the `mov eax, ecx`
//   versus `mov edx, ecx` that holds the strlen length for the strcpy, the
//   `mov dl` versus `mov cl` for the constructor's char argument, and the
//   prologue's `mov eax, [esp+8]` before `sub esp, 0x224`.

// Space Bunny Free pass (issue 4579, 2026-10-02): still 70.2% (553 of 583), the
// body below is unchanged, best kept. WHAT THE BYTE DIFF ACTUALLY SHOWS, from a
// per-instruction aligner (build/scratch/0x4c54f0/insdiff.py, which prints the
// difflib opcodes with addresses so "differs" can be split from "is merely
// further down"): 127 of the original's 188 instructions are byte-identical,
// 61 differ, in 35 separate regions, so the diffs SPREAD over the whole
// function rather than sitting in one block. About half of the regions are pure
// jump-target shifts (our function is 30 bytes shorter, so every branch past
// the first difference moves); those cost nothing to fix. The real structural
// differences are only five:
//   1. the destruction block: the original loads the global into eax before the
//      pushes and ends with object=ebx, vec=edi, Free2 inlined; ours loads it
//      into edi after `push edi` and ends with object=edi, vec=ebx, Free2 called
//      out of line (~20 instructions),
//   2. `mov dl, [esp+0x17]` (flag) in the inlined constructor where ours has
//      `mov cl`, one register,
//   3. `mov eax, ecx` / `mov ecx, eax` in the strcpy expansion where ours keeps
//      the length in edx, two registers,
//   4. the 0x4c2f60 call site: `mov ecx,[esp+0x238]; push ecx; lea ecx,[esp+0x20]`
//      against our `mov eax,[esp+0x238]; lea ecx,[esp+0x1c]; push eax`, and
//      `lea edx,[esp+0x134]` against our `lea ecx,[esp+0x134]` for the 0x4c4420
//      destination (same slot, different scratch register), plus `push 0x5119b8`
//      against our load of DAT_005119b8's contents,
//   5. the insert branch: the original has the int form of the key comparison
//      beside the bool (`xor ecx,ecx; cmp; sete cl; neg cl; sbb ecx,ecx; inc ecx;
//      test cl,cl`) and one shared FUN_004c93f0 call, ours has a bare
//      `test eax,eax` and two call sites.
// So 3, 4 and 5 are each one or two registers, and 2 is one register: they are
// all scratch-register ties, not different algorithms. STACK LAYOFF IS ALREADY
// RIGHT: key at frame+0, flag at +7, the index at +8, f at +0xc, value at +0x24,
// name at +0x124, and every [esp+k] in both sides resolves to the same slot, so
// no class member offset or local order is still wrong.
//
// Space Bunny Free, the same pass, sixteen more variants (build/scratch/0x4c54f0,
// every one checked, best kept is the file below). The destruction block is now
// bounded from both sides, which is why nobody has got past 70.2 in six tries:
//   * MSVC 5 will NOT inline a member function whose body calls ::operator delete.
//     Free2 stays out of line; an explicit `w->~Vec_004c54f0()` becomes
//     `push 0; mov ecx,reg; call ??_G` whether ~Vec is defined in the class body
//     (a1, d1, d4 = 69.6) or out of line (a2 = 56.8, d2 = 56.2, and d2 does
//     materialise `lea esi,[edi+1]`, the one thing the original does), and the
//     implicit member destruction of a non-trivial Vec calls ??_G as well (a2).
//   * It DOES inline a member function with no call in it (e2: `w->Zero()`), and
//     then it folds the vector onto the object: [edi+5], [edi+9], [edi+0xd],
//     three callee-saved registers, frame 0x220. The free written out in the
//     destructor folds the same way whatever the spelling: loop bounds through
//     the object (b1 = 57.0), through the vector (b2 = 57.0), no local vector at
//     all (b3 = 57.0), zeroing before the free (b4 = 55.2), and a call-free Vec
//     destructor run implicitly with the buffer freed explicitly (e1 = 55.9).
//   So the shapes that inline the free fold, and the shapes that keep four
//   callee-saved registers leave a call behind. There is no shape in between, and
//   the original has neither defect: it inlines the free AND keeps object=ebx
//   and vec=edi. Whatever produced it is not a member function of the vector
//   reachable from this file, which is worth knowing before another attempt.
// The smaller ties are also pinned down:
//   * The `mov dl` against `mov cl` for the flag is not the constructor's store
//     order: `v.count` first (h2) and `v.count` last (h3) both stay at 70.2,
//     `v.first = v.last = v.end = 0; v.count = count;` (h4) is 69.1.
//   * `Class_004c4420* cur = f.current;` before the 0x4c4420 call (g1) is 70.2
//     and byte-identical, so the receiver load cannot be scheduled first that way.
//   * `char[]` measured a third time, with `cur` as well (g2 = 68.7). SUPERSEDED:
//     on the corrected insert branch it is worth 5.7 points, see the top of this
//     file. The 68.7 was real, it was just measured against a broken spelling.
//     With it the
//     `push 0x5119b8` DOES match; all that is left in that block is the
//     destination pointer landing in ecx in ours and in edx then eax in the
//     original, because the original loads the receiver into ecx first
//     (`mov ecx,[esp+0x28]`) and we compute the destination into ecx first.
//     Same three instructions, no algorithm difference.
// The insert condition, four more attempts, and it is a genuine three-way bind:
// the original needs a canonical 0/0xff byte bool (`sete`+`neg`), a 0/1 int of
// the same comparison taken off the carry `neg` leaves (`sbb`+`inc`) that is
// then DEAD, and the strcmp short-circuited behind `e == last`. Measured:
//   i1 `e == last || (strcmp(...) == 0)`, the polarity the exe has (it jumps to
//      the insert when the keys are EQUAL), 70.2 and 553 bytes, emitting
//      `cmp eax,ebx; je insert` where the original emits `test cl,cl; jne insert`:
//      same score, but strictly more faithful, so the next attempt should start here;
//   j1 `bool same = ...; int differ = !same;` with differ unused, 69.6: it DOES
//      give `xor ecx,ecx; cmp eax,ebx; ... sete cl; test cl,cl; jne insert`, the
//      right polarity and a materialised bool, but MSVC hoists the whole strcmp
//      above the `e == last` test, so the short circuit is gone;
//   k1 `bool same = (e != last) && (strcmp(...) == 0)`, 64.0; k2 the same with
//      `int same`, 67.2;
//   j2/j3 a `static inline int Differ(a,b) { return strcmp(a,b) != 0; }` used as
//      `!Differ(...)` or `Differ(...)`, both 70.2 and both emit exactly the
//      current `cmp eax,ebx; je` with no bool and no int, so an inlined
//      int-returning helper does not materialise anything either.
// The permuter (seed 11, 20 minutes, --jobs 4) found nothing better.
// Retry #3361 by mimo-v2.6-pro: still 70.2% (553 bytes). Kept variant A (below).
//   * Read order: declaring `e = v.last;` before `p = v.first;` now emits
//     `mov ebp,[edi+9]; mov esi,[edi+5]`, matching the original's last-then-first
//     order (was first-then-last). Score unchanged at 70.2, but strictly closer.
//   * Register roles (correcting an older note): the current spelling `s = this;
//     w = &s->v; w->Free2(); delete(s);` yields obj in EDI and vec in EBX
//     (`lea ebx,[edi+1]`). The original has obj in EAX->EBX and vec in EDI
//     (`lea edi,[eax+1]; mov ebx,eax`). So obj/vec are swapped across ebx/edi.
//   * Every inline-free form drops to 57.0% (frame shrinks 0x224 -> 0x220 and
//     `push ebx; mov ebx,[esp+0x22c]` hoists section above _strcmpi): free written
//     inline in the destructor, inline destruction in FUN_004c54f0 (3 callee-saved),
//     inline with a materialised vec w (4 regs but w folds to obj+5), and a
//     this-based spelling (obj=edi,first=esi,last=ebp, frees ebx). Only the
//     out-of-line Free2 + s/w spelling holds 70.2%.
//   * Latent bug to fix before any MATCH: DAT_005119b8 is a char[] (matched files
//     declare `extern char DAT_005119b8[];` and the original does `push 0x5119b8`,
//     an immediate address). This file declares it `extern char* DAT_005119b8;`, so
//     the FUN_004c48c0 def arg compiles to `mov edx,[0x5119b8]; push edx` (pushes the
//     buffer contents, not its address). Switching to char[] fixes that one push to
//     match, but reshuffles the FUN_004c4420/48c0 block registers (lea edx vs lea
//     eax) and scores 68.7%. The declaration is right and the register tie is the
//     blocker, so it needs the upstream destructor register swap first.
//     RE-CONFIRMED: patched properly (the first literal occurrence of that
//     text is on this line, not the declaration, which is what made an earlier
//     attempt look free) `extern char DAT_005119b8[];` scores 68.7, so the
//     pointer stays until the destructor registers move.
// Retry #3141 by GPT-6.1-sol: five worker checks plus an unsigned-index trial found no improvement; best remains 70.2%. Reordering destructor pointer declarations, making the old global object explicit, and rewriting the indexed for loop as while all reproduced the same score. Remaining differences are documented below, especially vector destruction/codegen and register allocation.
// #2959 retry by GPT-6.1-sol: six checks reconfirmed 70.2%; strlen/memcpy and
// other variants did not improve the saved source. No MATCH.
// Retry #1769: the saved best remains 70.2% after seven worker checks; the final batch did not MATCH. Lower-scoring local-copy, bool and split-condition trials were reverted.
// deepseek-v4.1-flash (#2405): still 70.2%. The residual is the destruction block's
// register rotation: the original loads the global into eax, tests it, then copies
// eax->ebx and computes edi = eax+1; ours loads straight into edi and computes
// ebx = edi+1. The inlined ~Class_004c5840 spelling is required (every alternate
// destructor spelling drops to ~57). Merging the FUN_004c93f0(value) assignment after
// both the find and insert branches is semantically right but flips the branch layout
// (68.5); forcing the "keys differ" test into a real bool scores 68.7. headers.py and
// a 0..400 dummy-declaration sweep are both flat.
// Suspected original bug: the map owner is constructed with an uninitialised stack
// byte as its count/flag (`mov dl, byte [esp+0x17]` at 0x4c556a; that slot is never
// written in this function), so `new Class_004c5840(flag)` has a garbage argument.
// Loads a TDF section into the global map at 0x51fdb8: the section name is
// compared with the one already loaded, the map is thrown away and rebuilt,
// then every section of the file contributes one entry keyed by its own
// name. The map's owner (0x4c5840) is a packed 17-byte class whose vector
// member starts one byte in, so the insert helper (0x4c59d0) is handed
// this+1; the key the owner is born with is an uninitialised local byte
// (the exe reads [esp+0x17], never written here), kept as `flag`.
//
// Best so far as of the pass above: 71.8 percent (559 bytes), from 57. The two
// changes that first got it to 70.2 were both about
// forcing a value into the register or slot the original uses:
//
// 1. The destructor's vector free. The original materialises the vector's
//    `this` as `lea edi, [eax+1]` and then frees through `[edi+4]`, `[edi+8]`
//    and `[edi+0xc]`. Written as member accesses of `v`, MSVC 5 folds the +1
//    away and uses this+5/+9/+0xd. Keeping a local `Class_004c5840* s = this;`
//    and `Vec_004c54f0* w = &s->v;` alongside the object-base reads, and
//    calling `w->Free2()`, keeps a separate vec register (ebx = this+1) so the
//    free is not folded to this+5. 57 -> 70.2 percent.
//
// 2. The loop counter. It has to end up in the frame slot at [esp+0x18], the
//    way the original has it, rather than in ebx, and that also makes the frame
//    0x224 instead of 0x220. Declaring the index as a one-element array
//    (`int idx[1]`) is what does it.
//
// What still differs, all of it listed so the next attempt does not repeat it:
//  - the prologue order: the original does `mov eax, [esp+8]` and then loads
//    the global before `push edi`; this file has them the other way round.
//  - the inlined `~vector`. The original frees the buffer and zeroes three
//    fields through edi before the object delete; this file emits an
//    out-of-line Free2 call and then a single object delete.
//  - the strcpy sequence keeps its length in edx where the original uses eax.
//  - the argument loads before the 0x4c2f60 call, `mov eax, ecx` order.
//
// And the one thing that is a source of real doubt rather than codegen, see the
// note on the insert condition below. The original materialises the
// "keys differ" test as a bool in cl with the int form beside it
// (`xor ecx,ecx; cmp; sete cl; neg cl; sbb ecx,ecx; inc ecx; test cl,cl`),
// where this file has a plain `test eax,eax`.
//
// Tried by deepseek-v4.1-flash, both worse:
//  - `delete DAT_0051fdb8;` with an implicit destructor: 56.8 percent. The
//    delete makes the compiler keep the object in a preserved register from
//    the prologue (`push ebx; mov ebx,[esp+0x22c]` before _strcmpi).
//  - the same free written out inline inside ~Class_004c5840 instead of
//    through Free2: 57.0 percent, and the destructor stops being inlined.
// Keeping the explicit `->~Class_004c5840()` call plus the out-of-line
// Free2 helper is what holds the 70.2 percent.
//
// Retry by deepseek-v4.1-flash, confirmed all of the above and added three
// more dead ends, none better than 70.2:
//  - real `std::vector<Elem_004c5bc0>` member plus `delete DAT_0051fdb8`:
//    56.4 percent, and it emits a scalar deleting destructor (??
//    _GElem_004c5bc0@@QAEPAXI@Z) the original does not have.
//  - explicit `old->~Class_004c5840(); operator delete(old);` with the vector
//    free inlined in the destructor: 57.0 percent; the whole destruction gets
//    its own frame and `push ebx; mov ebx,[esp+0x22c]` hoists above _strcmpi.
//  - the destruction in a static inline helper taking the object pointer:
//    57.0 percent, same hoist.
//  - a named `Class_004c5840* old = DAT_0051fdb8;` temp at the call site:
//    70.2 percent, byte-identical output to the current file, no effect.
// The surviving problem is unchanged: the original loads the global into eax,
// tests it, then copies eax to ebx and uses edi = eax+1 for the vector; ours
// puts the object straight in edi (ebx = object+1). Separating the destruction
// from the inlined destructor is what triggers the ebx hoist, so the inline
// destructor is not the thing to change.
// Three more dead ends from deepseek-v4.1 (all below 70.2, best kept):
//  - `delete DAT_0051fdb8;` with an in-class inline destructor: 57.0 percent.
//    It forces `push ebx; mov ebx,[esp+0x22c]` (section) before _strcmpi and
//    shrinks the frame from 0x224 to 0x220.
//  - an explicit `w->~Vec_004c54f0()` call instead of `w->Free2()`: 69.6 percent.
//    MSVC turns it into a `push 0; call` deleting-destructor call, never the
//    inlined free the original has.
//  - the insert written as one expression, `&Class_004c54d0(key, Class_004c9180())`,
//    with FUN_004c93f0 hoisted after the if: 68.9 percent, 545 bytes.
// Retry by GPT-6.1-sol: the 70.2 percent version remains best. A fresh local
// copy of section did not change codegen; materializing strcmp equality before
// the last-entry test scored 69.6 percent; splitting that last-entry test into
// a separate insertion path scored 57.2 percent and emitted a scalar deleting
// destructor. Keep the original short-circuit condition.
#include <string.h>
#include <memory.h>
#include <stdio.h>

extern char DAT_0051fdc0[256];
extern char DAT_005119b8[];

void __cdecl FUN_004d83a0(int);
void __cdecl operator delete(void* p);

// A reference counted string handle. 0x4c91b0 builds one from a C string,
// 0x4c9180 makes an empty one, 0x4c93f0 assigns a C string and 0x4c9390 is
// the release.
class Class_004c9390 {
public:
    char* ptr;

    void FUN_004c9390();
};

class Class_004c93f0 {
public:
    char* ptr;

    Class_004c93f0* FUN_004c93f0(const char* text);
};

class Class_004c91a0 {
public:
    char* ptr;

    Class_004c91a0(const Class_004c91a0& other);
};

class Class_004c91b0 {
public:
    char* ptr;

    Class_004c91b0(const char* text);
    ~Class_004c91b0() { ((Class_004c9390*)this)->FUN_004c9390(); }
};

class Class_004c9180 {
public:
    char* ptr;

    Class_004c9180();
    ~Class_004c9180() { ((Class_004c9390*)this)->FUN_004c9390(); }
};

// One entry of the global map: a key and a value, both string handles.
struct Elem_004c5bc0 {
    Class_004c91a0 key;                  // +0x0
    Class_004c91a0 value;                // +0x4

    ~Elem_004c5bc0();
};

class Class_004c54d0 : public Elem_004c5bc0 {
public:
    Class_004c54d0(const Class_004c91a0& a, const Class_004c91a0& b);
};

// The map itself, keyed by the string the handle points at.
#pragma pack(push, 1)
class Class_004c5c60 {
public:
    char unknown_0[5];
    Elem_004c5bc0* first;               // +0x5
    Elem_004c5bc0* last;                // +0x9

    Elem_004c5bc0* FUN_004c5c60(const char* key);
};

// The same vector seen from the insert helper, which is handed the object
// plus one, so its pointers sit four bytes lower.
class Vec_004c54f0 {
public:
    char count;                          // +0x0
    char pad[3];
    Elem_004c5bc0* first;                // +0x4
    Elem_004c5bc0* last;                 // +0x8
    Elem_004c5bc0* end;                  // +0xc

    Elem_004c5bc0* FUN_004c59d0(Elem_004c5bc0* pos, Elem_004c5bc0* val);

    void Free2()
    {
        ::operator delete(this->first);
        this->last = 0;
        this->first = 0;
        this->end = 0;
    }
};

class Class_004c5840 {
public:
    char unknown_0;                      // +0x0
    Vec_004c54f0 v;                       // +0x1 (_First at +0x5)

    Class_004c5840(char count);
    ~Class_004c5840();
};
#pragma pack(pop)

extern Class_004c5840* DAT_0051fdb8;

Class_004c5840::Class_004c5840(char count)
{
    v.first = 0;
    v.count = count;
    v.last = 0;
    v.end = 0;
}

Class_004c5840::~Class_004c5840()
{
    Class_004c5840* s = this;
    Vec_004c54f0* w = &s->v;
    Elem_004c5bc0* e = v.last;
    Elem_004c5bc0* p = v.first;
    while (p != e) {
        p->~Elem_004c5bc0();
        p++;
    }
    w->Free2();
    ::operator delete(s);
}

// A TDF section: its name and the entries under it.
class Class_004c4420 {
public:
    const char* name;                    // +0x0

    void FUN_004c4420(char* dest, size_t count);
};

class Class_004c48c0 {
public:
    char unknown_0[0x19];

    int FUN_004c48c0(char* dst, const char* key, size_t size, const char* def);
};

// The open TDF file.
class Class_004c2ea0 {
public:
    int root;                            // +0x0
    Class_004c4420* current;             // +0x4
    int file;                            // +0x8

    Class_004c2ea0();
    ~Class_004c2ea0();
};

class Class_004c2f60 {
public:
    char unknown_0[4];
    int field_4;

    int FUN_004c2f60(char* filename);
};

class Class_004c3490 {
public:
    char unknown_0[4];
    int field_4;

    Class_004c4420* FUN_004c3490(int index);
};

class Class_004c3e10 {
public:
    char unknown_0[4];
    int field_4;

    void FUN_004c3e10();
};

class Class_004c3240 {
public:
    char unknown_0[4];
    int field_4;
    int field_8;

    void FUN_004c3240();
};

// Building the new map and remembering the section name. Written as its own
// inline function because writing it out in the caller changes the register
// allocation and costs 2.2 percent (71.4 -> 69.2).
static inline void LoadMap(char flag, char* section)
{
    DAT_0051fdb8 = new Class_004c5840(flag);
    FUN_004d83a0((int)DAT_0051fdb8);
    strcpy(DAT_0051fdc0, section);
}

// Also load-bearing as a separate function, for the same reason as LoadMap.
static inline bool MapIsLoaded()
{
    return DAT_0051fdb8 != 0;
}

// FUNCTION: 0x4c54f0
void __stdcall FUN_004c54f0(char* filename, char* section)
{
    int atEnd;
    char flag;
    Class_004c5840* s;

    if (_strcmpi(section, DAT_0051fdc0) == 0)
        return;
    if (MapIsLoaded())
        DAT_0051fdb8->~Class_004c5840();
    LoadMap(flag, section);
    {
        Class_004c2ea0 f;
        char value[256];
        char name[256];
        if (((Class_004c2f60*)&f)->FUN_004c2f60(filename)) {
            int idx[1];
            idx[0] = 0;
            while (((Class_004c3490*)&f)->FUN_004c3490(idx[0])) {
                f.current->FUN_004c4420(name, 0xff);
                ((Class_004c48c0*)f.current)->FUN_004c48c0(value, DAT_0051fdc0, 0xff, DAT_005119b8);
                if (strlen(value) != 0) {
                    Class_004c91b0 key(name);
                    Elem_004c5bc0* e;
                    s = DAT_0051fdb8;
                    e = ((Class_004c5c60*)DAT_0051fdb8)->FUN_004c5c60(key.ptr);
                    atEnd = (Elem_004c5bc0*)e == ((Class_004c5c60*)s)->last;
                    if (!(atEnd || (strcmp(e->key.ptr, key.ptr) == 0))) {
                        ((Class_004c93f0*)(&e->value))->FUN_004c93f0(value);
                    } else {
                        Class_004c9180 empty;
                        Class_004c54d0 tmp((Class_004c91a0&)key, (Class_004c91a0&)empty);
                        e = ((Vec_004c54f0*)(1 + (char*)s))->FUN_004c59d0((Elem_004c5bc0*)e, (Elem_004c5bc0*)&tmp);
                    }
                }
                ((Class_004c3e10*)&f)->FUN_004c3e10();
                idx[0]++;
            }
            ((Class_004c3240*)&f)->FUN_004c3240();
        }
    }
}
