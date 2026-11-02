#include <Windows.h>                                                                                                                          // leaked by theholytorch
#include <TlHelp32.h>                                                                                                                         // leaked by theholytorch
#include <winternl.h>                                                                                                                         // leaked by theholytorch
#include <array>                                                                                                                              // leaked by theholytorch
#include <cstdint>                                                                                                                            // leaked by theholytorch
#include <cstdio>                                                                                                                             // leaked by theholytorch
#include <fstream>                                                                                                                            // leaked by theholytorch
#include <memory>                                                                                                                             // leaked by theholytorch
#include <optional>                                                                                                                           // leaked by theholytorch
#include <string_view>                                                                                                                        // leaked by theholytorch
#include <vector>                                                                                                                             // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
#define DBG(fmt, ...) printf("[dbg] " fmt "\n", ##__VA_ARGS__)                                                                                // leaked by theholytorch
#define ERR(fmt, ...) printf("[err] " fmt " (gle=%lu)\n", ##__VA_ARGS__, GetLastError())                                                      // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
struct HandleDeleter {                                                                                                                        // leaked by theholytorch
    void operator()(HANDLE h) const noexcept {                                                                                                // leaked by theholytorch
        if (h && h != INVALID_HANDLE_VALUE)                                                                                                   // leaked by theholytorch
            CloseHandle(h);                                                                                                                   // leaked by theholytorch
    }                                                                                                                                         // leaked by theholytorch
};                                                                                                                                            // leaked by theholytorch
using UniqueHandle = std::unique_ptr<std::remove_pointer_t<HANDLE>, HandleDeleter>;                                                           // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
[[nodiscard]] std::optional<DWORD> find_process(std::wstring_view name) {                                                                     // leaked by theholytorch
    UniqueHandle snap{ CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) };                                                                     // leaked by theholytorch
    if (!snap || snap.get() == INVALID_HANDLE_VALUE) return std::nullopt;                                                                     // leaked by theholytorch
    PROCESSENTRY32W e{}; e.dwSize = sizeof(e);                                                                                                // leaked by theholytorch
    if (!Process32FirstW(snap.get(), &e)) return std::nullopt;                                                                                // leaked by theholytorch
    do {                                                                                                                                      // leaked by theholytorch
        if (name == e.szExeFile) return e.th32ProcessID;                                                                                      // leaked by theholytorch
    } while (Process32NextW(snap.get(), &e));                                                                                                 // leaked by theholytorch
    return std::nullopt;                                                                                                                      // leaked by theholytorch
}                                                                                                                                             // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
[[nodiscard]] std::optional<uintptr_t> remote_module_base(DWORD pid, std::wstring_view name) {                                                // leaked by theholytorch
    UniqueHandle snap{ CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid) };                                              // leaked by theholytorch
    if (!snap || snap.get() == INVALID_HANDLE_VALUE) return std::nullopt;                                                                     // leaked by theholytorch
    MODULEENTRY32W e{}; e.dwSize = sizeof(e);                                                                                                 // leaked by theholytorch
    if (!Module32FirstW(snap.get(), &e)) return std::nullopt;                                                                                 // leaked by theholytorch
    do {                                                                                                                                      // leaked by theholytorch
        if (name == e.szModule) return reinterpret_cast<uintptr_t>(e.modBaseAddr);                                                            // leaked by theholytorch
    } while (Module32NextW(snap.get(), &e));                                                                                                  // leaked by theholytorch
    return std::nullopt;                                                                                                                      // leaked by theholytorch
}                                                                                                                                             // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
[[nodiscard]] std::optional<std::vector<uint8_t>> read_file(std::string_view path) {                                                          // leaked by theholytorch
    std::ifstream f(path.data(), std::ios::binary | std::ios::ate);                                                                           // leaked by theholytorch
    if (!f) return std::nullopt;                                                                                                              // leaked by theholytorch
    auto sz = f.tellg();                                                                                                                      // leaked by theholytorch
    f.seekg(0);                                                                                                                               // leaked by theholytorch
    std::vector<uint8_t> buf(static_cast<size_t>(sz));                                                                                        // leaked by theholytorch
    f.read(reinterpret_cast<char*>(buf.data()), buf.size());                                                                                  // leaked by theholytorch
    return buf;                                                                                                                               // leaked by theholytorch
}                                                                                                                                             // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
[[nodiscard]] std::optional<std::string> exe_dir_path(std::string_view filename) {                                                            // leaked by theholytorch
    char buf[MAX_PATH]{};                                                                                                                     // leaked by theholytorch
    if (!GetModuleFileNameA(nullptr, buf, MAX_PATH))                                                                                          // leaked by theholytorch
        return std::nullopt;                                                                                                                  // leaked by theholytorch
    std::string path(buf);                                                                                                                    // leaked by theholytorch
    auto sep = path.find_last_of("\\/");                                                                                                      // leaked by theholytorch
    if (sep == std::string::npos)                                                                                                             // leaked by theholytorch
        return std::nullopt;                                                                                                                  // leaked by theholytorch
    path.resize(sep + 1);                                                                                                                     // leaked by theholytorch
    path += filename;                                                                                                                         // leaked by theholytorch
    return path;                                                                                                                              // leaked by theholytorch
}                                                                                                                                             // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
[[nodiscard]] std::optional<DWORD> find_parked_thread(DWORD pid) {                                                                            // leaked by theholytorch
    uintptr_t ntdll_lo = reinterpret_cast<uintptr_t>(GetModuleHandleA("ntdll.dll"));                                                          // leaked by theholytorch
    if (!ntdll_lo) { ERR("GetModuleHandleA(ntdll) failed"); return std::nullopt; }                                                            // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(ntdll_lo);                                                                                // leaked by theholytorch
    auto* nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(ntdll_lo + dos->e_lfanew);                                                               // leaked by theholytorch
    uintptr_t ntdll_hi = ntdll_lo + nt->OptionalHeader.SizeOfImage;                                                                           // leaked by theholytorch
    DBG("ntdll range: %llx - %llx", (unsigned long long)ntdll_lo, (unsigned long long)ntdll_hi);                                              // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    UniqueHandle snap{ CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0) };                                                                      // leaked by theholytorch
    if (!snap || snap.get() == INVALID_HANDLE_VALUE) { ERR("TH32CS_SNAPTHREAD failed"); return std::nullopt; }                                // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    THREADENTRY32 e{}; e.dwSize = sizeof(e);                                                                                                  // leaked by theholytorch
    if (!Thread32First(snap.get(), &e)) return std::nullopt;                                                                                  // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    int total = 0, opened = 0, suspended = 0;                                                                                                 // leaked by theholytorch
    do {                                                                                                                                      // leaked by theholytorch
        if (e.th32OwnerProcessID != pid) continue;                                                                                            // leaked by theholytorch
        ++total;                                                                                                                              // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
        UniqueHandle thr{                                                                                                                     // leaked by theholytorch
            OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, e.th32ThreadID)                                // leaked by theholytorch
        };                                                                                                                                    // leaked by theholytorch
        if (!thr) { DBG("  tid %lu OpenThread failed gle=%lu", e.th32ThreadID, GetLastError()); continue; }                                   // leaked by theholytorch
        ++opened;                                                                                                                             // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
        if (SuspendThread(thr.get()) == static_cast<DWORD>(-1)) {                                                                             // leaked by theholytorch
            DBG("  tid %lu SuspendThread failed gle=%lu", e.th32ThreadID, GetLastError()); continue;                                          // leaked by theholytorch
        }                                                                                                                                     // leaked by theholytorch
        ++suspended;                                                                                                                          // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
        CONTEXT ctx{}; ctx.ContextFlags = CONTEXT_CONTROL;                                                                                    // leaked by theholytorch
        bool parked = false;                                                                                                                  // leaked by theholytorch
        if (GetThreadContext(thr.get(), &ctx)) {                                                                                              // leaked by theholytorch
            parked = ctx.Rip >= ntdll_lo && ctx.Rip < ntdll_hi;                                                                               // leaked by theholytorch
            DBG("  tid %lu rip=%llx parked=%d", e.th32ThreadID, (unsigned long long)ctx.Rip, (int)parked);                                    // leaked by theholytorch
        }                                                                                                                                     // leaked by theholytorch
        else {                                                                                                                                // leaked by theholytorch
            DBG("  tid %lu GetThreadContext failed gle=%lu", e.th32ThreadID, GetLastError());                                                 // leaked by theholytorch
        }                                                                                                                                     // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
        ResumeThread(thr.get());                                                                                                              // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
        if (parked) { DBG("selected parked tid %lu", e.th32ThreadID); return e.th32ThreadID; }                                                // leaked by theholytorch
    } while (Thread32Next(snap.get(), &e));                                                                                                   // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    DBG("find_parked_thread: total=%d opened=%d suspended=%d — none parked in ntdll", total, opened, suspended);                              // leaked by theholytorch
    return std::nullopt;                                                                                                                      // leaked by theholytorch
}                                                                                                                                             // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
struct MapResult {                                                                                                                            // leaked by theholytorch
    uintptr_t image_base;                                                                                                                     // leaked by theholytorch
    uintptr_t entry_point;                                                                                                                    // leaked by theholytorch
};                                                                                                                                            // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
[[nodiscard]] std::optional<MapResult> manual_map(HANDLE proc, const std::vector<uint8_t>& raw) {                                             // leaked by theholytorch
    auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(raw.data());                                                                        // leaked by theholytorch
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return std::nullopt;                                                                             // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(raw.data() + dos->e_lfanew);                                                       // leaked by theholytorch
    if (nt->Signature != IMAGE_NT_SIGNATURE) return std::nullopt;                                                                             // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    const auto& opt = nt->OptionalHeader;                                                                                                     // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    auto* alloc = static_cast<uint8_t*>(                                                                                                      // leaked by theholytorch
        VirtualAllocEx(proc, reinterpret_cast<LPVOID>(opt.ImageBase),                                                                         // leaked by theholytorch
            opt.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));                                                                      // leaked by theholytorch
    if (!alloc)                                                                                                                               // leaked by theholytorch
        alloc = static_cast<uint8_t*>(                                                                                                        // leaked by theholytorch
            VirtualAllocEx(proc, nullptr, opt.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));                                        // leaked by theholytorch
    if (!alloc) return std::nullopt;                                                                                                          // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    uintptr_t base = reinterpret_cast<uintptr_t>(alloc);                                                                                      // leaked by theholytorch
    int64_t   delta = static_cast<int64_t>(base) - static_cast<int64_t>(opt.ImageBase);                                                       // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    WriteProcessMemory(proc, alloc, raw.data(), opt.SizeOfHeaders, nullptr);                                                                  // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    const auto* secs = IMAGE_FIRST_SECTION(nt);                                                                                               // leaked by theholytorch
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i) {                                                                              // leaked by theholytorch
        const auto& s = secs[i];                                                                                                              // leaked by theholytorch
        if (s.SizeOfRawData)                                                                                                                  // leaked by theholytorch
            WriteProcessMemory(proc, alloc + s.VirtualAddress,                                                                                // leaked by theholytorch
                raw.data() + s.PointerToRawData, s.SizeOfRawData, nullptr);                                                                   // leaked by theholytorch
    }                                                                                                                                         // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    const auto& reloc_dd = opt.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];                                                                // leaked by theholytorch
    if (reloc_dd.VirtualAddress && delta != 0) {                                                                                              // leaked by theholytorch
        auto* blk = reinterpret_cast<const IMAGE_BASE_RELOCATION*>(                                                                           // leaked by theholytorch
            raw.data() + reloc_dd.VirtualAddress);                                                                                            // leaked by theholytorch
        const auto* end = reinterpret_cast<const IMAGE_BASE_RELOCATION*>(                                                                     // leaked by theholytorch
            reinterpret_cast<const uint8_t*>(blk) + reloc_dd.Size);                                                                           // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
        while (blk < end && blk->SizeOfBlock) {                                                                                               // leaked by theholytorch
            DWORD  count = (blk->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);                                                 // leaked by theholytorch
            auto* entries = reinterpret_cast<const WORD*>(blk + 1);                                                                           // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
            for (DWORD j = 0; j < count; ++j) {                                                                                               // leaked by theholytorch
                if ((entries[j] >> 12) != IMAGE_REL_BASED_DIR64) continue;                                                                    // leaked by theholytorch
                uintptr_t va = base + blk->VirtualAddress + (entries[j] & 0xFFF);                                                             // leaked by theholytorch
                uintptr_t val{};                                                                                                              // leaked by theholytorch
                ReadProcessMemory(proc, reinterpret_cast<LPCVOID>(va), &val, sizeof(val), nullptr);                                           // leaked by theholytorch
                val += static_cast<uintptr_t>(delta);                                                                                         // leaked by theholytorch
                WriteProcessMemory(proc, reinterpret_cast<LPVOID>(va), &val, sizeof(val), nullptr);                                           // leaked by theholytorch
            }                                                                                                                                 // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
            blk = reinterpret_cast<const IMAGE_BASE_RELOCATION*>(                                                                             // leaked by theholytorch
                reinterpret_cast<const uint8_t*>(blk) + blk->SizeOfBlock);                                                                    // leaked by theholytorch
        }                                                                                                                                     // leaked by theholytorch
    }                                                                                                                                         // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    const auto& imp_dd = opt.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];                                                                     // leaked by theholytorch
    if (imp_dd.VirtualAddress && imp_dd.VirtualAddress < raw.size()) {                                                                        // leaked by theholytorch
        auto* desc = reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR*>(                                                                        // leaked by theholytorch
            raw.data() + imp_dd.VirtualAddress);                                                                                              // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
        const auto* raw_end = raw.data() + raw.size();                                                                                        // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
        while (reinterpret_cast<const uint8_t*>(desc) + sizeof(*desc) <= raw_end && desc->Name) {                                             // leaked by theholytorch
            if (desc->Name >= raw.size()) { ++desc; continue; }                                                                               // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
            const char* dll_name = reinterpret_cast<const char*>(raw.data() + desc->Name);                                                    // leaked by theholytorch
            HMODULE     mod = LoadLibraryA(dll_name);                                                                                         // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
            const DWORD int_rva = desc->OriginalFirstThunk ? desc->OriginalFirstThunk : desc->FirstThunk;                                     // leaked by theholytorch
            if (!int_rva || int_rva >= raw.size()) { ++desc; continue; }                                                                      // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
            auto* thunk = reinterpret_cast<const IMAGE_THUNK_DATA64*>(raw.data() + int_rva);                                                  // leaked by theholytorch
            uintptr_t iat_va = base + desc->FirstThunk;                                                                                       // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
            for (SIZE_T k = 0; ; ++k) {                                                                                                       // leaked by theholytorch
                const auto* entry = &thunk[k];                                                                                                // leaked by theholytorch
                if (reinterpret_cast<const uint8_t*>(entry) + sizeof(*entry) > raw_end) break;                                                // leaked by theholytorch
                if (!entry->u1.AddressOfData) break;                                                                                          // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
                uintptr_t fn{};                                                                                                               // leaked by theholytorch
                if (entry->u1.Ordinal & IMAGE_ORDINAL_FLAG64) {                                                                               // leaked by theholytorch
                    fn = reinterpret_cast<uintptr_t>(GetProcAddress(mod,                                                                      // leaked by theholytorch
                        reinterpret_cast<LPCSTR>(IMAGE_ORDINAL64(entry->u1.Ordinal))));                                                       // leaked by theholytorch
                }                                                                                                                             // leaked by theholytorch
                else {                                                                                                                        // leaked by theholytorch
                    DWORD ibn_rva = static_cast<DWORD>(entry->u1.AddressOfData);                                                              // leaked by theholytorch
                    if (ibn_rva < raw.size()) {                                                                                               // leaked by theholytorch
                        auto* ibn = reinterpret_cast<const IMAGE_IMPORT_BY_NAME*>(raw.data() + ibn_rva);                                      // leaked by theholytorch
                        fn = reinterpret_cast<uintptr_t>(GetProcAddress(mod, ibn->Name));                                                     // leaked by theholytorch
                    }                                                                                                                         // leaked by theholytorch
                }                                                                                                                             // leaked by theholytorch
                WriteProcessMemory(proc,                                                                                                      // leaked by theholytorch
                    reinterpret_cast<LPVOID>(iat_va + k * sizeof(uintptr_t)),                                                                 // leaked by theholytorch
                    &fn, sizeof(fn), nullptr);                                                                                                // leaked by theholytorch
            }                                                                                                                                 // leaked by theholytorch
            ++desc;                                                                                                                           // leaked by theholytorch
        }                                                                                                                                     // leaked by theholytorch
    }                                                                                                                                         // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i) {                                                                              // leaked by theholytorch
        const auto& s = secs[i];                                                                                                              // leaked by theholytorch
        if (!s.SizeOfRawData && !s.Misc.VirtualSize) continue;                                                                                // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
        DWORD prot{};                                                                                                                         // leaked by theholytorch
        bool  exec = !!(s.Characteristics & IMAGE_SCN_MEM_EXECUTE);                                                                           // leaked by theholytorch
        bool  read = !!(s.Characteristics & IMAGE_SCN_MEM_READ);                                                                              // leaked by theholytorch
        bool  writ = !!(s.Characteristics & IMAGE_SCN_MEM_WRITE);                                                                             // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
        if (exec && writ) prot = PAGE_EXECUTE_READWRITE;                                                                                      // leaked by theholytorch
        else if (exec && read) prot = PAGE_EXECUTE_READ;                                                                                      // leaked by theholytorch
        else if (exec)         prot = PAGE_EXECUTE;                                                                                           // leaked by theholytorch
        else if (writ)         prot = PAGE_READWRITE;                                                                                         // leaked by theholytorch
        else if (read)         prot = PAGE_READONLY;                                                                                          // leaked by theholytorch
        else                   prot = PAGE_NOACCESS;                                                                                          // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
        DWORD old{};                                                                                                                          // leaked by theholytorch
        SIZE_T sz = s.Misc.VirtualSize ? s.Misc.VirtualSize : s.SizeOfRawData;                                                                // leaked by theholytorch
        VirtualProtectEx(proc, alloc + s.VirtualAddress, sz, prot, &old);                                                                     // leaked by theholytorch
    }                                                                                                                                         // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    return MapResult{ base, base + opt.AddressOfEntryPoint };                                                                                 // leaked by theholytorch
}                                                                                                                                             // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
[[nodiscard]] std::optional<uintptr_t> stomp_and_hijack(                                                                                      // leaked by theholytorch
    HANDLE            proc,                                                                                                                   // leaked by theholytorch
    DWORD             pid,                                                                                                                    // leaked by theholytorch
    DWORD             tid,                                                                                                                    // leaked by theholytorch
    std::wstring_view dll_name,                                                                                                               // leaked by theholytorch
    uintptr_t         image_base,                                                                                                             // leaked by theholytorch
    uintptr_t         dllmain_va                                                                                                              // leaked by theholytorch
) {                                                                                                                                           // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    auto dll_base = remote_module_base(pid, dll_name);                                                                                        // leaked by theholytorch
    if (!dll_base) { DBG("stomp: module not found"); return std::nullopt; }                                                                   // leaked by theholytorch
    DBG("stomp: %.*ls base=%llx", (int)dll_name.size(), dll_name.data(), (unsigned long long) * dll_base);                                    // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    IMAGE_DOS_HEADER dos{};                                                                                                                   // leaked by theholytorch
    ReadProcessMemory(proc, reinterpret_cast<LPCVOID>(*dll_base), &dos, sizeof(dos), nullptr);                                                // leaked by theholytorch
    if (dos.e_magic != IMAGE_DOS_SIGNATURE) { DBG("stomp: bad DOS magic"); return std::nullopt; }                                             // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    IMAGE_NT_HEADERS64 nt{};                                                                                                                  // leaked by theholytorch
    ReadProcessMemory(proc, reinterpret_cast<LPCVOID>(*dll_base + dos.e_lfanew), &nt, sizeof(nt), nullptr);                                   // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    uintptr_t shbase = *dll_base + dos.e_lfanew                                                                                               // leaked by theholytorch
        + offsetof(IMAGE_NT_HEADERS64, OptionalHeader)                                                                                        // leaked by theholytorch
        + nt.FileHeader.SizeOfOptionalHeader;                                                                                                 // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    constexpr DWORD  kPageSz = 0x1000;                                                                                                        // leaked by theholytorch
    constexpr SIZE_T kMinNeeded = 200; // shellcode + rsp_slot                                                                                // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    uintptr_t tail_va = 0;                                                                                                                    // leaked by theholytorch
    DWORD     tail_size = 0;                                                                                                                  // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    for (WORD i = 0; i < nt.FileHeader.NumberOfSections; ++i) {                                                                               // leaked by theholytorch
        IMAGE_SECTION_HEADER sec{};                                                                                                           // leaked by theholytorch
        ReadProcessMemory(proc,                                                                                                               // leaked by theholytorch
            reinterpret_cast<LPCVOID>(shbase + i * sizeof(IMAGE_SECTION_HEADER)),                                                             // leaked by theholytorch
            &sec, sizeof(sec), nullptr);                                                                                                      // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
        if (!(sec.Characteristics & IMAGE_SCN_CNT_CODE)) continue;                                                                            // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
        DWORD aligned = (sec.Misc.VirtualSize + kPageSz - 1) & ~(kPageSz - 1);                                                                // leaked by theholytorch
        DWORD tail = aligned - sec.Misc.VirtualSize;                                                                                          // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
        char sname[9]{}; memcpy(sname, sec.Name, 8);                                                                                          // leaked by theholytorch
        DBG("stomp: section '%s' vsz=%lu page_tail=%lu", sname, sec.Misc.VirtualSize, tail);                                                  // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
        if (tail >= kMinNeeded) {                                                                                                             // leaked by theholytorch
            tail_va = *dll_base + sec.VirtualAddress + sec.Misc.VirtualSize;                                                                  // leaked by theholytorch
            tail_size = tail;                                                                                                                 // leaked by theholytorch
            DBG("stomp: selected tail at %llx (%lu bytes)", (unsigned long long)tail_va, tail_size);                                          // leaked by theholytorch
            break;                                                                                                                            // leaked by theholytorch
        }                                                                                                                                     // leaked by theholytorch
    }                                                                                                                                         // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    if (!tail_va) { DBG("stomp: no code section with sufficient page tail"); return std::nullopt; }                                           // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    UniqueHandle thr{                                                                                                                         // leaked by theholytorch
        OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, tid)                                               // leaked by theholytorch
    };                                                                                                                                        // leaked by theholytorch
    if (!thr) { ERR("stomp_hijack: OpenThread tid=%lu failed", tid); return std::nullopt; }                                                   // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    if (SuspendThread(thr.get()) == static_cast<DWORD>(-1)) {                                                                                 // leaked by theholytorch
        ERR("stomp_hijack: SuspendThread failed"); return std::nullopt;                                                                       // leaked by theholytorch
    }                                                                                                                                         // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    CONTEXT ctx{}; ctx.ContextFlags = CONTEXT_FULL;                                                                                           // leaked by theholytorch
    if (!GetThreadContext(thr.get(), &ctx)) {                                                                                                 // leaked by theholytorch
        ERR("stomp_hijack: GetThreadContext failed");                                                                                         // leaked by theholytorch
        ResumeThread(thr.get());                                                                                                              // leaked by theholytorch
        return std::nullopt;                                                                                                                  // leaked by theholytorch
    }                                                                                                                                         // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    uintptr_t orig_rip = ctx.Rip;                                                                                                             // leaked by theholytorch
    DBG("stomp_hijack: tid=%lu orig_rip=%llx", tid, (unsigned long long)orig_rip);                                                            // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    uintptr_t rsp_slot = tail_va + tail_size - 8;                                                                                             // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    std::vector<uint8_t> sc(tail_size, 0);                                                                                                    // leaked by theholytorch
    SIZE_T n = 0;                                                                                                                             // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    auto e8 = [&](int v) { sc[n++] = static_cast<uint8_t>(v); };                                                                              // leaked by theholytorch
    auto e32 = [&](uint32_t v) { for (int b = 0; b < 4; ++b) sc[n++] = static_cast<uint8_t>(v >> (b * 8)); };                                 // leaked by theholytorch
    auto e64 = [&](uint64_t v) { for (int b = 0; b < 8; ++b) sc[n++] = static_cast<uint8_t>(v >> (b * 8)); };                                 // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    e8(0x9C);                                                           // pushfq                                                             // leaked by theholytorch
    e8(0x50); e8(0x51); e8(0x52); e8(0x53);                            // push rax, rcx, rdx, rbx                                             // leaked by theholytorch
    e8(0x55); e8(0x56); e8(0x57);                                       // push rbp, rsi, rdi                                                 // leaked by theholytorch
    for (int r = 0; r < 8; ++r) { e8(0x41); e8(0x50 + r); }            // push r8..r15                                                        // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    e8(0x48); e8(0xB8); e64(rsp_slot);                                  // mov rax, rsp_slot                                                  // leaked by theholytorch
    e8(0x48); e8(0x89); e8(0x20);                                       // mov [rax], rsp                                                     // leaked by theholytorch
    e8(0x48); e8(0x83); e8(0xE4); e8(0xF0);                            // and rsp, -16                                                        // leaked by theholytorch
    e8(0x48); e8(0x83); e8(0xEC); e8(0x20);                            // sub rsp, 0x20  (shadow; 0x20 keeps RSP 0-mod-16 before call)        // leaked by theholytorch
    e8(0x48); e8(0xB9); e64(image_base);                                // mov rcx, image_base                                                // leaked by theholytorch
    e8(0x48); e8(0xBA); e64(1ULL);                                      // mov rdx, DLL_PROCESS_ATTACH                                        // leaked by theholytorch
    e8(0x45); e8(0x33); e8(0xC0);                                       // xor r8d, r8d                                                       // leaked by theholytorch
    e8(0x48); e8(0xB8); e64(dllmain_va);                                // mov rax, dllmain_va                                                // leaked by theholytorch
    e8(0xFF); e8(0xD0);                                                  // call rax                                                          // leaked by theholytorch
    e8(0x48); e8(0x83); e8(0xC4); e8(0x20);                            // add rsp, 0x20                                                       // leaked by theholytorch
    e8(0x48); e8(0xB8); e64(rsp_slot);                                  // mov rax, rsp_slot                                                  // leaked by theholytorch
    e8(0x48); e8(0x8B); e8(0x20);                                       // mov rsp, [rax]                                                     // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    for (int r = 7; r >= 0; --r) { e8(0x41); e8(0x58 + r); }           // pop r15..r8                                                         // leaked by theholytorch
    e8(0x5F); e8(0x5E); e8(0x5D); e8(0x5B);                            // pop rdi, rsi, rbp, rbx                                              // leaked by theholytorch
    e8(0x5A); e8(0x59); e8(0x58);                                       // pop rdx, rcx, rax                                                  // leaked by theholytorch
    e8(0x9D);                                                            // popfq                                                             // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    e8(0x68); e32(static_cast<uint32_t>(orig_rip));                     // push lo32(orig_rip)                                                // leaked by theholytorch
    e8(0xC7); e8(0x44); e8(0x24); e8(0x04);                            // mov dword [rsp+4], hi32                                             // leaked by theholytorch
    e32(static_cast<uint32_t>(orig_rip >> 32));                                                                                               // leaked by theholytorch
    e8(0xC3);                                                            // ret                                                               // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    DBG("stomp_hijack: shellcode %zu bytes, rsp_slot=%llx", n, (unsigned long long)rsp_slot);                                                 // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    DWORD old{};                                                                                                                              // leaked by theholytorch
    if (!VirtualProtectEx(proc, reinterpret_cast<LPVOID>(tail_va), tail_size, PAGE_EXECUTE_READWRITE, &old)) {                                // leaked by theholytorch
        ERR("stomp_hijack: VirtualProtectEx RWX failed");                                                                                     // leaked by theholytorch
        ResumeThread(thr.get());                                                                                                              // leaked by theholytorch
        return std::nullopt;                                                                                                                  // leaked by theholytorch
    }                                                                                                                                         // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    bool wpm_ok = !!WriteProcessMemory(proc, reinterpret_cast<LPVOID>(tail_va), sc.data(), tail_size, nullptr);                               // leaked by theholytorch
    VirtualProtectEx(proc, reinterpret_cast<LPVOID>(tail_va), tail_size, PAGE_EXECUTE_READ, &old);                                            // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    if (!wpm_ok) {                                                                                                                            // leaked by theholytorch
        ERR("stomp_hijack: WriteProcessMemory failed");                                                                                       // leaked by theholytorch
        ResumeThread(thr.get());                                                                                                              // leaked by theholytorch
        return std::nullopt;                                                                                                                  // leaked by theholytorch
    }                                                                                                                                         // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    FlushInstructionCache(proc, reinterpret_cast<LPVOID>(tail_va), tail_size);                                                                // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    ctx.Rip = static_cast<DWORD64>(tail_va);                                                                                                  // leaked by theholytorch
    if (!SetThreadContext(thr.get(), &ctx)) {                                                                                                 // leaked by theholytorch
        ERR("stomp_hijack: SetThreadContext failed");                                                                                         // leaked by theholytorch
        ResumeThread(thr.get());                                                                                                              // leaked by theholytorch
        return std::nullopt;                                                                                                                  // leaked by theholytorch
    }                                                                                                                                         // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    ResumeThread(thr.get());                                                                                                                  // leaked by theholytorch
    DBG("stomp_hijack: thread redirected to %llx inside %.*ls — all code PE-backed",                                                          // leaked by theholytorch
        (unsigned long long)tail_va, (int)dll_name.size(), dll_name.data());                                                                  // leaked by theholytorch
    return tail_va;                                                                                                                           // leaked by theholytorch
}                                                                                                                                             // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
int main() {                                                                                                                                  // leaked by theholytorch
    constexpr std::wstring_view injector_target = L"RobloxPlayerBeta.exe";                                                                    // leaked by theholytorch
    constexpr std::string_view  module_name = "DLL1.dll";                                                                                     // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    constexpr std::array<std::wstring_view, 2> modules_available{                                                                             // leaked by theholytorch
        L"advapi32.dll",                                                                                                                      // leaked by theholytorch
        L"combase.dll",                                                                                                                       // leaked by theholytorch
    };                                                                                                                                        // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    auto pid = find_process(injector_target);                                                                                                 // leaked by theholytorch
    if (!pid) { ERR("find_process failed — is the target running?"); return 1; }                                                              // leaked by theholytorch
    DBG("target pid=%lu", *pid);                                                                                                              // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    UniqueHandle proc{ OpenProcess(PROCESS_ALL_ACCESS, FALSE, *pid) };                                                                        // leaked by theholytorch
    if (!proc) { ERR("OpenProcess PROCESS_ALL_ACCESS failed — run as admin?"); return 1; }                                                    // leaked by theholytorch
    DBG("process handle ok");                                                                                                                 // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    auto dll_path = exe_dir_path(module_name);                                                                                                // leaked by theholytorch
    if (!dll_path) { ERR("exe_dir_path failed"); return 1; }                                                                                  // leaked by theholytorch
    DBG("dll path: %s", dll_path->c_str());                                                                                                   // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    auto dll_bytes = read_file(*dll_path);                                                                                                    // leaked by theholytorch
    if (!dll_bytes) { ERR("read_file failed — is payload.dll beside the exe?"); return 1; }                                                   // leaked by theholytorch
    DBG("dll size: %zu bytes", dll_bytes->size());                                                                                            // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    auto mapped = manual_map(proc.get(), *dll_bytes);                                                                                         // leaked by theholytorch
    if (!mapped) { DBG("manual_map failed"); return 1; }                                                                                      // leaked by theholytorch
    DBG("mapped: image_base=%llx entry=%llx",                                                                                                 // leaked by theholytorch
        (unsigned long long)mapped->image_base, (unsigned long long)mapped->entry_point);                                                     // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    auto tid = find_parked_thread(*pid);                                                                                                      // leaked by theholytorch
    if (!tid) { DBG("find_parked_thread failed — no suitable thread found"); return 1; }                                                      // leaked by theholytorch
    DBG("parked thread tid=%lu", *tid);                                                                                                       // leaked by theholytorch
                                                                                                                                              // leaked by theholytorch
    std::optional<uintptr_t> stomped;                                                                                                         // leaked by theholytorch
    for (auto& candidate : modules_available) {                                                                                               // leaked by theholytorch
        DBG("trying stomp+hijack via %.*ls", (int)candidate.size(), candidate.data());                                                        // leaked by theholytorch
        stomped = stomp_and_hijack(proc.get(), *pid, *tid, candidate,                                                                         // leaked by theholytorch
            mapped->image_base, mapped->entry_point);                                                                                         // leaked by theholytorch
        if (stomped) break;                                                                                                                   // leaked by theholytorch
    }                                                                                                                                         // leaked by theholytorch
    if (!stomped) { DBG("stomp_and_hijack failed on all candidates"); return 1; }                                                             // leaked by theholytorch
    DBG( "done — payload dispatched from PE-backed page tail %llx", (unsigned long long) * stomped );                                         // leaked by theholytorch
    return 0;                                                                                                                                 // leaked by theholytorch
}                                                                                                                                             // leaked by theholytorch
// leaked by theholytorch
// leaked by theholytorch
// leaked by theholytorch
// leaked by theholytorch