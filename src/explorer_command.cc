// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include <windows.h>

#include <filesystem>
#include <string>
#include <utility>

#include <shlwapi.h>
#include <shobjidl_core.h>
#include <userenv.h>
#include <wrl/module.h>
#include <wrl/implements.h>
#include <wrl/client.h>
#include "wil/stl.h"
#include "wil/filesystem.h"
#include "wil/win32_helpers.h"

using Microsoft::WRL::ClassicCom;
using Microsoft::WRL::ComPtr;
using Microsoft::WRL::InhibitRoOriginateError;
using Microsoft::WRL::Module;
using Microsoft::WRL::ModuleType;
using Microsoft::WRL::RuntimeClass;
using Microsoft::WRL::RuntimeClassFlags;

extern "C" BOOL WINAPI DllMain(HINSTANCE instance,
                               DWORD reason,
                               LPVOID reserved) {
  switch (reason) {
    case DLL_PROCESS_ATTACH:
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
  }

  return true;
}

namespace {
  // Extracted from
  // https://source.chromium.org/chromium/chromium/src/+/main:base/command_line.cc;l=109-159

  std::wstring QuoteForCommandLineArg(const std::wstring& arg) {
  // We follow the quoting rules of CommandLineToArgvW.
  // http://msdn.microsoft.com/en-us/library/17w5ykft.aspx
  std::wstring quotable_chars(L" \\\"");
  if (arg.find_first_of(quotable_chars) == std::wstring::npos) {
    // No quoting necessary.
    return arg;
  }

  std::wstring out;
  out.push_back('"');
  for (size_t i = 0; i < arg.size(); ++i) {
    if (arg[i] == '\\') {
      // Find the extent of this run of backslashes.
      size_t start = i, end = start + 1;
      for (; end < arg.size() && arg[end] == '\\'; ++end) {}
      size_t backslash_count = end - start;

      // Backslashes are escapes only if the run is followed by a double quote.
      // Since we also will end the string with a double quote, we escape for
      // either a double quote or the end of the string.
      if (end == arg.size() || arg[end] == '"') {
        // To quote, we need to output 2x as many backslashes.
        backslash_count *= 2;
      }
      for (size_t j = 0; j < backslash_count; ++j)
        out.push_back('\\');

      // Advance i to one before the end to balance i++ in loop.
      i = end - 1;
    } else if (arg[i] == '"') {
      out.push_back('\\');
      out.push_back('"');
    } else {
      out.push_back(arg[i]);
    }
  }
  out.push_back('"');

  return out;
}

}

class __declspec(uuid(DLL_UUID)) ExplorerCommandHandler final : public RuntimeClass<RuntimeClassFlags<ClassicCom | InhibitRoOriginateError>, IExplorerCommand> {
 public:
  // IExplorerCommand implementation:
  IFACEMETHODIMP GetTitle(IShellItemArray* items, PWSTR* name) {
    const size_t kMaxStringLength = 1024;
    wchar_t value_w[kMaxStringLength];
    wchar_t expanded_value_w[kMaxStringLength];
    DWORD value_size_w = sizeof(value_w);
#if defined(INSIDER)
    const wchar_t kTitleRegkey[] = L"Software\\Classes\\VSCodeInsidersContextMenu";
#else
    const wchar_t kTitleRegkey[] = L"Software\\Classes\\VSCodeContextMenu";
#endif
    HKEY subhkey = nullptr;
    LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, kTitleRegkey, 0, KEY_READ, &subhkey);
    if (result != ERROR_SUCCESS) {
      result = RegOpenKeyEx(HKEY_CURRENT_USER, kTitleRegkey, 0, KEY_READ, &subhkey);
    }

    DWORD type = REG_EXPAND_SZ;
    RegQueryValueEx(subhkey, L"Title", nullptr, &type,
                    reinterpret_cast<LPBYTE>(&value_w), &value_size_w);
    RegCloseKey(subhkey);
    value_size_w = ExpandEnvironmentStrings(value_w, expanded_value_w, kMaxStringLength);
    return (value_size_w && value_size_w < kMaxStringLength)
        ? SHStrDup(expanded_value_w, name)
        : SHStrDup(L"UnExpected Title", name);
  }

  IFACEMETHODIMP GetIcon(IShellItemArray* items, PWSTR* icon) {
    std::filesystem::path module_path{ wil::GetModuleFileNameW<std::wstring>(wil::GetModuleInstanceHandle()) };
    module_path = module_path.remove_filename().parent_path().parent_path();
    module_path /= EXE_NAME;
    return SHStrDupW(module_path.c_str(), icon);
  }

  IFACEMETHODIMP GetToolTip(IShellItemArray* items, PWSTR* infoTip) {
    *infoTip = nullptr;
    return E_NOTIMPL;
  }

  IFACEMETHODIMP GetCanonicalName(GUID* guidCommandName) {
    *guidCommandName = GUID_NULL;
    return S_OK;
  }

  IFACEMETHODIMP GetState(IShellItemArray* items, BOOL okToBeSlow, EXPCMDSTATE* cmdState) {
    *cmdState = ECS_ENABLED;
    return S_OK;
  }

  IFACEMETHODIMP GetFlags(EXPCMDFLAGS* flags) {
    *flags = ECF_DEFAULT;
    return S_OK;
  }

  IFACEMETHODIMP EnumSubCommands(IEnumExplorerCommand** enumCommands) {
    *enumCommands = nullptr;
    return E_NOTIMPL;
  }

  IFACEMETHODIMP Invoke(IShellItemArray* items, IBindCtx* bindCtx) {
    if (items) {
      std::filesystem::path module_path{ wil::GetModuleFileNameW<std::wstring>(wil::GetModuleInstanceHandle()) };
      module_path = module_path.remove_filename().parent_path().parent_path();
      module_path /= EXE_NAME;
      DWORD count;
      RETURN_IF_FAILED(items->GetCount(&count));
      for (DWORD i = 0; i < count; ++i) {
        ComPtr<IShellItem> item;
        auto result = items->GetItemAt(i, &item);
        if (SUCCEEDED(result)) {
          wil::unique_cotaskmem_string path;
          result = item->GetDisplayName(SIGDN_FILESYSPATH, &path);
          if (SUCCEEDED(result)) {
            auto command{ wil::str_printf<std::wstring>(LR"-("%s" %s)-", module_path.c_str(), QuoteForCommandLineArg(path.get()).c_str()) };
            wil::unique_process_information process_info;
            STARTUPINFOW startup_info = {sizeof(startup_info)};
            RETURN_IF_WIN32_BOOL_FALSE(CreateProcessW(
              nullptr,
              command.data(),
              nullptr /* lpProcessAttributes */,
              nullptr /* lpThreadAttributes */,
              false /* bInheritHandles */, 
              CREATE_NO_WINDOW,
              nullptr,
              nullptr,
              &startup_info,
              &process_info));
          }
        }
      }
    }
    return S_OK;
  }
};

CoCreatableClass(ExplorerCommandHandler)
CoCreatableClassWrlCreatorMapInclude(ExplorerCommandHandler)

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
  if (ppv == nullptr)
    return E_POINTER;
  *ppv = nullptr;
  return Module<ModuleType::InProc>::GetModule().GetClassObject(rclsid, riid, ppv);
}

STDAPI DllCanUnloadNow(void) {
  return Module<ModuleType::InProc>::GetModule().GetObjectCount() == 0 ? S_OK : S_FALSE;
}

STDAPI DllGetActivationFactory(HSTRING activatableClassId,
                               IActivationFactory** factory) {
  return Module<ModuleType::InProc>::GetModule().GetActivationFactory(activatableClassId, factory);
}
