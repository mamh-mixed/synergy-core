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

#include "FeatureHandler.h"

#include "common/Settings.h"
#include "license/LicenseHandler.h"
#include "synergy/gui/constants.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDialog>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>

using namespace synergy::gui;

void FeatureHandler::handleMainWindow(QMainWindow *mainWindow)
{
  Q_UNUSED(mainWindow);
}

void FeatureHandler::handleSettings(QDialog *parent) const
{
  Q_UNUSED(parent);

  // Synergy-specific settings UI (scope radios, etc.) lives in extra/. To be
  // wired when the synergy widget-injection layer lands; until then, the
  // scope-switching helper below is callable directly when needed.
}

void FeatureHandler::switchScope(QDialog *parent, bool toSystemScope)
{
  const auto &licenseHandler = LicenseHandler::instance();
  if (licenseHandler.isEnabled() && !licenseHandler.license().isSettingsScopeAvailable() && toSystemScope) {
    qWarning("settings scope not available for this license tier");
    return;
  }

  const auto userScopeText = QStringLiteral("current user");
  const auto systemScopeText = QStringLiteral("all users");
  const auto from = toSystemScope ? userScopeText : systemScopeText;
  const auto to = toSystemScope ? systemScopeText : userScopeText;
  const auto result = QMessageBox::information(
      parent, "Switch settings profile",
      QString("Switching settings from %1 to %2 requires %3 to restart.\n\n"
              "Would you like to restart the application now?")
          .arg(from, to, QApplication::applicationName()),
      QMessageBox::Yes | QMessageBox::Cancel
  );

  if (result != QMessageBox::Yes) {
    return;
  }

  const auto destFile = toSystemScope ? Settings::SystemSettingFile : Settings::UserSettingFile;
  const auto sourceFile = toSystemScope ? Settings::UserSettingFile : Settings::SystemSettingFile;

  QSettings dest(destFile, QSettings::IniFormat);
  if (dest.allKeys().isEmpty()) {
    qDebug() << "destination settings empty, copying from:" << sourceFile;
    QSettings source(sourceFile, QSettings::IniFormat);
    for (const auto &key : source.allKeys()) {
      dest.setValue(key, source.value(key));
    }
    dest.sync();
  }

  Settings::setSettingsFile(destFile);

  // Hot-switching the active settings file mid-run touches enough state
  // (signal/slot wiring, cached UI defaults) that a clean restart is the
  // safest path. Inlined because upstream's `deskflow::gui::diagnostic::restart()`
  // is defined in Diagnostic.cpp but not declared in the public header.
  const auto program = QCoreApplication::applicationFilePath();
  auto arguments = QCoreApplication::arguments();
  if (const auto resetIndex = arguments.indexOf(QStringLiteral("--reset")); resetIndex != -1) {
    arguments.remove(resetIndex);
  }
  qInfo("launching new process: %s", qPrintable(program));
  QProcess::startDetached(program, arguments);
  qDebug("exiting current process");
  QCoreApplication::exit();
}
