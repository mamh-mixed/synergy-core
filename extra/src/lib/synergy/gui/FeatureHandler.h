/*
 * Synergy -- mouse and keyboard sharing utility
 * Copyright (C) 2025 Synergy App Ltd
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

class QDialog;
class QMainWindow;

class FeatureHandler
{
public:
  static FeatureHandler &instance()
  {
    static FeatureHandler instance;
    return instance;
  }

  void handleMainWindow(QMainWindow *mainWindow);
  void handleAppStart();
  void handleSettings(QDialog *parent) const;

  // Switches the active settings file between system and user scope and
  // restarts the application. The community removed system/user scope from
  // upstream's UI; Synergy keeps the feature downstream as a license-tier
  // capability. Caller is responsible for license-tier gating.
  static void switchScope(QDialog *parent, bool toSystemScope);

private:
  void addTestMenu();

  QMainWindow *m_pMainWindow = nullptr;
};
