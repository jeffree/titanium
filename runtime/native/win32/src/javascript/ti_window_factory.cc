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

#include "ti_window_factory.h"
#include "ti_runtime.h"

TiWindowFactory::TiWindowFactory(TiRuntime *ti)
{
	BindMethod("createWindow", &TiWindowFactory::createWindow);
	BindMethod("getWindow", &TiWindowFactory::getWindow);
	BindProperty("mainWindow", &mainWindow);
	BindProperty("currentWindow", &currentWindow);

	mainWindow.Set(TiChromeWindow::getMainWindow()->getTiUserWindow()->ToNPObject());
	currentWindow.Set(ti->getWindow()->getTiUserWindow()->ToNPObject());
}

void TiWindowFactory::createWindow(const CppArgumentList &args, CppVariant *result)
{
	TiUserWindow *window;

	if (args.size() > 0) {
		if (args[0].isString()) {
			std::string id = args[0].ToString();
			if (args.size() > 1) {
				window = new TiUserWindow(id.c_str(), args[1].ToBoolean());
			}
			else {
				window = new TiUserWindow(id.c_str());
			}
		} else if (args[0].isObject()) {
			std::string id = GetStringProperty(args[0], "id");
			bool usingChrome = GetBoolProperty(args[0], "usingChrome");

			window = new TiUserWindow(id.c_str(), usingChrome);
		}
	} else {
		window = new TiUserWindow();
	}

	result->Set(window->ToNPObject());
}

void TiWindowFactory::getWindow(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isString()) {
		TiChromeWindow *window = TiChromeWindow::getWindow(args[0].ToString().c_str());

		if (window != NULL) {
			result->Set(window->getTiUserWindow()->ToNPObject());
		} else {
			result->SetNull();
		}
	}
}