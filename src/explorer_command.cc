// Copyright (C) Microsoft Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include <filesystem>
#include <memory>
#include <string>
#include <utility>

#include <shlwapi.h>
#include <shobjidl_core.h>
#include <wrl/module.h>
#include <wrl/implements.h>
#include <wrl/client.h>
#include <winrt/Windows.ApplicationModel.Resources.Core.h>

using Microsoft::WRL::ClassicCom;
using Microsoft::WRL::ComPtr;
using Microsoft::WRL::InhibitRoOriginateError;
using Microsoft::WRL::Module;
using Microsoft::WRL::ModuleType;
using Microsoft::WRL::RuntimeClass;
using Microsoft::WRL::RuntimeClassFlags;
using namespace ::winrt::Windows::ApplicationModel::Resources::Core;

#define RETURN_IF_FAILED(expr)                                          \
  do {                                                                  \
    HRESULT hresult = (expr);                                           \
    if (FAILED(hresult)) {                                              \
      return hresult;                                                   \
    }                                                                   \
  } while (0)

namespace {

struct LocalAllocDeleter {
  void operator()(void* ptr) const { ::LocalFree(ptr); }
};

template <typename T>
std::unique_ptr<T, LocalAllocDeleter> TakeLocalAlloc(T*& ptr) {
  return std::unique_ptr<T, LocalAllocDeleter>(std::exchange(ptr, nullptr));
}

}

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
    const auto resource =
#if defined(INSIDER)
      ResourceManager::Current().MainResourceMap()
        .GetValue(L"ExplorerCommand_OpenWithCodeInsiders", ResourceContext::GetForViewIndependentUse())
        .ValueAsString();
#else
      ResourceManager::Current().MainResourceMap()
        .GetValue(L"ExplorerCommand_OpenWithCode", ResourceContext::GetForViewIndependentUse())
        .ValueAsString();
#endif
    return SHStrDup(resource.data(), name);
  }

  IFACEMETHODIMP GetIcon(IShellItemArray* items, PWSTR* icon) {
    HMODULE handle = GetModuleHandleW(nullptr);
    wchar_t module[MAX_PATH];
    GetModuleFileNameW(handle, module, MAX_PATH);
    std::filesystem::path prog_name = std::wstring(module);
    prog_name.replace_filename(EXE_NAME);
    return SHStrDup(prog_name.c_str(), icon);
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
      DWORD count;
      RETURN_IF_FAILED(items->GetCount(&count));
      ComPtr<IShellItem> item;
      RETURN_IF_FAILED(items->GetItemAt(0, &item));
      LPWSTR path;
      RETURN_IF_FAILED(item->GetDisplayName(SIGDN_FILESYSPATH, &path));

      HMODULE handle = GetModuleHandleW(nullptr);
      wchar_t module[MAX_PATH];
      GetModuleFileNameW(handle, module, MAX_PATH);
      std::filesystem::path prog_name = std::wstring(module);
      prog_name.replace_filename(EXE_NAME);
      std::wstring command{prog_name.wstring() + L" "};
      command += std::wstring(TakeLocalAlloc(path).get());

      STARTUPINFOEX startup_info{0};
      startup_info.StartupInfo.cb = sizeof(STARTUPINFOEX);
      PROCESS_INFORMATION temp_process_info = {};
      RETURN_IF_FAILED(CreateProcessW(
          nullptr, command.data(),
          nullptr /* lpProcessAttributes */, nullptr /* lpThreadAttributes */,
          false /* bInheritHandles */, 
          EXTENDED_STARTUPINFO_PRESENT | CREATE_UNICODE_ENVIRONMENT,
          nullptr /* lpEnvironment */, path,
          &startup_info.StartupInfo, &temp_process_info));
      // Close thread and process handles of the new process.
      CloseHandle(temp_process_info.hProcess);
      CloseHandle(temp_process_info.hThread);
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
