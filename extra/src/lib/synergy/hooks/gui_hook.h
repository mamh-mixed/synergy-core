/*
 * Synergy -- mouse and keyboard sharing utility
 * Copyright (C) 2024 - 2025 Synergy App Ltd
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

#pragma once

#include "common/Settings.h"
#include "synergy/gui/FeatureHandler.h"
#include "synergy/gui/SettingsMigration.h"
#include "synergy/gui/license/LicenseHandler.h"

#include <QDialog>
#include <QMainWindow>

namespace deskflow::gui {
class CoreProcess;
}

namespace synergy::hooks {

// Runs before any Settings::value() call, so that legacy-format keys can be
// migrated to the new format before upstream's cleanSettings() wipes them.
inline void onPreInit()
{
  synergy::gui::migration::migrateIfNeeded();

  // If the legacy user scope was running on system scope, switch the new
  // Settings to system scope so the user's preference is honored across
  // the upgrade. setSettingsFile() instantiates Settings; that's expected.
  if (synergy::gui::migration::preferSystemScope()) {
    Settings::setSettingsFile(Settings::SystemSettingFile);
  }
}

inline void onMainWindow(QMainWindow *mainWindow, deskflow::gui::CoreProcess *coreProcess)
{
  LicenseHandler::instance().handleMainWindow(mainWindow, coreProcess);
  FeatureHandler::instance().handleMainWindow(mainWindow);
  synergy::gui::migration::showNoticeIfPending(mainWindow);
}

inline bool onAppStart()
{
  FeatureHandler::instance().handleAppStart();
  return LicenseHandler::instance().handleAppStart();
}

inline void onSettings(QDialog *parent)
{
  LicenseHandler::instance().handleSettings(parent);
  FeatureHandler::instance().handleSettings(parent);
}

inline void onVersionCheck(QString &versionUrl)
{
  return LicenseHandler::instance().handleVersionCheck(versionUrl);
}

inline bool onCoreStart()
{
  return LicenseHandler::instance().handleCoreStart();
}

inline void onTestStart()
{
  LicenseHandler::instance().disable();
}

} // namespace synergy::hooks
