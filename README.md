# DLL Unlinking via PEB->Ldr

A short demo of a well-known Windows technique: hiding a loaded DLL
from the PEB module lists. Educational only - nothing here is meant
to be used in malware.

## How it works

Windows keeps every loaded module in three linked lists inside
`PEB->Ldr`:

- `InLoadOrderModuleList`
- `InMemoryOrderModuleList`
- `InInitializationOrderModuleList`

Each module is an `LDR_DATA_TABLE_ENTRY`, linked through a different
`LIST_ENTRY` field in each list. To hide a module, you find its entry
in each list and splice it out:
```
FlinkPrev->Flink = Flink
Flink->Blink     = FlinkPrev
```

After that, the module no longer shows up when the PEB lists are
walked (e.g. by `EnumProcessModules` or `CreateToolhelp32Snapshot`).

The code reaches the PEB via `gs:[0x60]` on x64 and `fs:[0x30]` on
x86, then follows `PEB->Ldr` at `+0x18` / `+0xC`.

## Cross-platform

Works on both x64 and x86. All offsets and structure layouts are
selected at compile time through `#if defined(_WIN64)` macros in
`unlink.h` - no separate source files, no runtime checks.

## Build

Requires MSVC (Visual Studio). Open the folder in Visual Studio, or use
`cl /std:c++17 /EHsc main.cpp`. Pick x64 or x86 in the
Developer Command Prompt - the `_WIN64` macros in `unlink.h` select the
right offsets.

If you use a different compiler, replace `__readgsqword` /
`__readfsdword` with the equivalent intrinsic available in yours.

The PEB and LDR structures are walked through raw offsets on purpose —
no `winternl.h` types, no `PPEB` / `PLDR_DATA_TABLE_ENTRY`. This keeps
the technique explicit: every field is read by the offset it actually
lives at, not through a header that hides the layout.

## Disclaimer

This is a demonstration of a known technique for learning purposes.
It is not malware, and it is not a call to write any. Use it on your
own machines and in your own processes.

Sorry if your eyes bleed from this code.
