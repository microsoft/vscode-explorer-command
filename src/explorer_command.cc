// Copyright (C) Microsoft Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

class __declspec(uuid(DLL_UUID)) ExplorerCommandHandler final : public RuntimeClass<RuntimeClassFlags<ClassicCom | InhibitRoOriginateError>, IExplorerCommand> {
 public:
  // IExplorerCommand implementation:
  IFACEMETHODIMP GetTitle(IShellItemArray* items, PWSTR* name) {
    wchar_t value_w[MAX_PATH];
    DWORD value_size_w = sizeof(value_w);
#if defined(INSIDER)
    RETURN_IF_FAILED(RegGetValueW(HKEY_CLASSES_ROOT, L"*\\shell\\VSCodeInsiders", L"", RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
                                  NULL, reinterpret_cast<LPBYTE>(&value_w), &value_size_w));
#else
    RETURN_IF_FAILED(RegGetValueW(HKEY_CLASSES_ROOT, L"*\\shell\\VSCode", L"", RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
                                  NULL, reinterpret_cast<LPBYTE>(&value_w), &value_size_w));
#endif
    return SHStrDup(value_w, name);
  }

  IFACEMETHODIMP GetIcon(IShellItemArray* items, PWSTR* icon) {
    std::filesystem::path module_path{ wil::GetModuleFileNameW<std::wstring>(wil::GetModuleInstanceHandle()) };
    module_path.replace_filename(EXE_NAME);
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
      module_path.replace_filename(EXE_NAME);
      DWORD count;
      RETURN_IF_FAILED(items->GetCount(&count));
      for (DWORD i = 0; i < count; ++i) {
        ComPtr<IShellItem> item;
        auto result = items->GetItemAt(i, &item);
        if (SUCCEEDED(result)) {
          wil::unique_cotaskmem_string path;
          result = item->GetDisplayName(SIGDN_FILESYSPATH, &path);
          if (SUCCEEDED(result)) {
            auto command{ wil::str_printf<std::wstring>(LR"-("%s" %s)-", module_path.c_str(), std::wstring(path.get()).c_str()) };
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
