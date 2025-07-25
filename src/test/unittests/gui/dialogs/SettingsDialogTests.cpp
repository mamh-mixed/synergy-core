/*
 * Deskflow -- mouse and keyboard sharing utility
 * Copyright (C) 2024 Symless Ltd.
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 *
 * This package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// `TestQtFullApp` freezes on windows ci, so exclude this test for now.
#ifndef WIN32

#include "gui/core/CoreProcess.h"
#include "gui/dialogs/SettingsDialog.h"
#include "shared/gui/TestQtFullApp.h"
#include "shared/gui/mocks/AppConfigMock.h"
#include "shared/gui/mocks/ServerConfigMock.h"
#include "shared/gui/mocks/SettingsMock.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace deskflow::gui;

TEST(SettingsDialogTests, ctor_getsScreenName)
{
  TestQtFullApp app;
  NiceMock<AppConfigMock> appConfig;
  NiceMock<ServerConfigMock> serverConfig;
  auto cpDeps = std::make_shared<CoreProcess::Deps>();
  CoreProcess coreProcess(appConfig, serverConfig, cpDeps);

  NiceMock<SettingsMock> mockSettings;
  ON_CALL(appConfig, settings()).WillByDefault(ReturnRef(mockSettings));

  EXPECT_CALL(appConfig, screenName()).Times(1);

  SettingsDialog settingsDialog(nullptr, appConfig, serverConfig, coreProcess);
}

#endif
