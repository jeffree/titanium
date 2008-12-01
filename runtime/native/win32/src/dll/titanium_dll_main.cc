/*
* Copyright 2006-2008 Appcelerator, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "resource.h"
#include <windows.h>

#include <tchar.h>
#include <atlbase.h>
#include <commdlg.h>
#include <objbase.h>
#include <shlwapi.h>
#include <wininet.h>
#include <commctrl.h>

#undef max

#include "base/scoped_ptr.h"
#include "webkit/glue/webpreferences.h"
#include "webkit/glue/weburlrequest.h"
#include "webkit/glue/webframe.h"
#include "webkit/glue/webview.h"
#include "webkit/glue/webview_delegate.h"
#include "webkit/glue/webwidget_delegate.h"
#include "webkit/glue/plugins/webplugin_delegate_impl.h"
#include "webkit/glue/webkit_glue.h"
#include "base/gfx/point.h"
#include "base/file_util.h"
#include "base/basictypes.h"
#include "base/resource_util.h"
#include "base/ref_counted.h"
#include "base/path_service.h"
#include "base/at_exit.h"
#include "base/process_util.h"
#include "base/message_loop.h"
#include "base/icu_util.h"
#include "base/win_util.h"
#include "net/base/net_module.h"
#include "sandbox/src/sandbox_factory.h"
#include "sandbox/src/dep.h"
#include "webview_host.h"
#include "webwidget_host.h"
#include "simple_resource_loader_bridge.h"
#include "test_shell_request_context.h"

#include "titanium_dll_main.h"
#include "ti_chrome_window.h"
#include "ti_web_view_delegate.h"
#include "ti_url.h"
#include "ti_utils.h"
#include "ti_dev_mode.h"

using namespace std;

#ifdef _MANAGED
#pragma managed(push, off)
#endif

HMODULE hModule;

BOOL APIENTRY DllMain( HMODULE hModule_,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	hModule = hModule_;

	switch (ul_reason_for_call) {

		case DLL_PROCESS_ATTACH:
			TiChromeWindow::initWindowClass(hModule);
			break;

		case DLL_PROCESS_DETACH:
			TiChromeWindow::removeWindowClass(hModule);
			break;

	}

    return TRUE;
}

std::string TiGetDataResource(HMODULE module, int resource_id) {
	void *data_ptr;
	size_t data_size;
	return base::GetDataResourceFromModule(hModule, resource_id,
		&data_ptr, &data_size) ?
		std::string(static_cast<char*>(data_ptr), data_size) : std::string();

}

std::string TiNetResourceProvider(int key) {
	return TiGetDataResource(hModule, key);
}

void chromiumInit()
{

	base::EnableTerminationOnHeapCorruption();
	
	INITCOMMONCONTROLSEX InitCtrlEx;

	InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCtrlEx.dwICC  = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&InitCtrlEx);
	HRESULT res = OleInitialize(NULL);

	win_util::WinVersion win_version = win_util::GetWinVersion();
	if (win_version == win_util::WINVERSION_XP ||
		win_version == win_util::WINVERSION_SERVER_2003) {
		// On Vista, this is unnecessary since it is controlled through the
		// /NXCOMPAT linker flag.
		// Enforces strong DEP support.
		sandbox::SetCurrentProcessDEP(sandbox::DEP_ENABLED);
	}
	
	wstring resourcesPath;
	PathService::Get(base::DIR_EXE, &resourcesPath);
	file_util::AppendToPath(&resourcesPath, L"Resources");
	
	wstring cache_path;
	PathService::Get(base::DIR_EXE, &cache_path);
    file_util::AppendToPath(&cache_path, L"cache");

	SimpleResourceLoaderBridge::Init(new TestShellRequestContext(cache_path, net::HttpCache::NORMAL));
	icu_util::Initialize();
	net::NetModule::SetResourceProvider(TiNetResourceProvider);
}

TiChromeWindow *mainWindow = NULL;
int titaniumInit (wstring appXmlPath)
{
	TiAppConfig *appConfig = new TiAppConfig(appXmlPath);

	if (appConfig->getError() != NULL) {
		systemError(UTF8ToWide(std::string(appConfig->getError())).c_str());
		return -1;
	}

	TiURL::init();	
	TiChromeWindow::setTiAppConfig(appConfig);
	mainWindow = new TiChromeWindow(hModule, appConfig->getMainWindow());

	return 0;
}

void runTitanium ()
{

	mainWindow->open();
	
	MessageLoop::current()->Run();
}


TITANIUM_DLL_API int __cdecl runTitaniumAppDev(wstring &appXmlPath, wstring &projectPath, wstring &runtimePath, map<wstring,wstring> &pluginPaths)
{
	base::AtExitManager exit_manager;
	MessageLoopForUI message_loop;

	chromiumInit();
	int result = titaniumInit(appXmlPath);

	if (result != 0) {
		return result;
	}

	TiDevMode::isDevMode = true;
	TiDevMode::instance()->projectPath = projectPath;
	TiDevMode::instance()->runtimePath = runtimePath;
	TiDevMode::instance()->pluginPaths = pluginPaths;

	runTitanium();

	return 0;
}


TITANIUM_DLL_API int __cdecl runTitaniumApp(wstring &appXmlPath) {
	base::AtExitManager exit_manager;
	MessageLoopForUI message_loop;

	chromiumInit();
	int result = titaniumInit(appXmlPath);

	if (result != 0) {
		return result;
	}

	runTitanium();

	return 0;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif