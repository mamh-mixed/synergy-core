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
#include "synergy/gui/SettingsMigration.h"
#include "synergy/gui/SettingsScope.h"
#include "synergy/gui/TestSettings.h"
#include "synergy/gui/constants.h"

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QDialog>
#include <QFileInfo>
#include <QLabel>
#include <QPalette>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QRadioButton>
#include <QSettings>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

using namespace synergy::gui;

void FeatureHandler::handleMainWindow(QMainWindow *mainWindow)
{
  m_pMainWindow = mainWindow;
}

void FeatureHandler::handleAppStart()
{
  // wl-clipboard's focus-stealing behavior on common compositors makes it
  // unfit for shipping; force the setting off on every launch regardless
  // of how it got set.
  Settings::setValue(Settings::Core::UseWlClipboard, false);

  if (TestSettings::instance().isEnabled()) {
    addTestMenu();
  }
}

void FeatureHandler::addTestMenu()
{
  if (m_pMainWindow == nullptr) {
    qWarning("cannot add test menu, main window not set");
    return;
  }

  auto testMenu = new QMenu(QObject::tr("Test"), m_pMainWindow);
  m_pMainWindow->menuBar()->addMenu(testMenu);

  auto fatalAction = new QAction(QObject::tr("Trigger fatal error"), m_pMainWindow);
  QObject::connect(fatalAction, &QAction::triggered, [] { qFatal("test fatal error"); });
  testMenu->addAction(fatalAction);

  auto criticalAction = new QAction(QObject::tr("Trigger critical error"), m_pMainWindow);
  QObject::connect(criticalAction, &QAction::triggered, [] { qCritical("test critical error"); });
  testMenu->addAction(criticalAction);
}

void FeatureHandler::handleSettings(QDialog *parent) const
{
  if (parent == nullptr) {
    return;
  }
  if (auto *wlClipboard = parent->findChild<QWidget *>(QStringLiteral("widgetWlClipboard"))) {
    wlClipboard->hide();
  }

  const auto &licenseHandler = LicenseHandler::instance();
  if (licenseHandler.isEnabled() && !licenseHandler.license().isSettingsScopeAvailable()) {
    return;
  }
  addScopeTab(parent);
}

namespace {

QString pathLabel(const QString &path)
{
  if (QFileInfo::exists(path)) {
    return QStringLiteral("<a href=\"file://%1\">%1</a>").arg(path);
  }
  return QStringLiteral("<code>%1</code> %2")
      .arg(path, QObject::tr("(not yet created)"));
}

} // namespace

void FeatureHandler::addScopeTab(QDialog *parent) const
{
  auto *tabWidget = parent->findChild<QTabWidget *>();
  if (tabWidget == nullptr) {
    qWarning("scope tab: no QTabWidget in settings dialog, skipping");
    return;
  }

  auto *scopeTab = new QWidget(tabWidget);
  auto *layout = new QVBoxLayout(scopeTab);

  auto *intro = new QLabel(
      QObject::tr("Choose where %1 stores its settings. Changes take effect when %1 restarts.")
          .arg(QApplication::applicationName()),
      scopeTab
  );
  intro->setWordWrap(true);
  layout->addWidget(intro);
  layout->addSpacing(8);

  const auto mutedColor = scopeTab->palette().color(QPalette::Disabled, QPalette::WindowText).name();
  const auto helpStyle = QStringLiteral("color: %1;").arg(mutedColor);
  const auto pathStyle = QStringLiteral("color: %1; font-size: 10pt;").arg(mutedColor);

  const auto buildOption = [&](const QString &title, const QString &help, const QString &path) {
    auto *radio = new QRadioButton(title, scopeTab);
    auto *helpLabel = new QLabel(help, scopeTab);
    helpLabel->setWordWrap(true);
    helpLabel->setIndent(22);
    helpLabel->setStyleSheet(helpStyle);
    auto *pathWidget = new QLabel(pathLabel(path), scopeTab);
    pathWidget->setIndent(22);
    pathWidget->setOpenExternalLinks(true);
    pathWidget->setTextInteractionFlags(Qt::TextBrowserInteraction);
    pathWidget->setWordWrap(true);
    pathWidget->setStyleSheet(pathStyle);

    layout->addWidget(radio);
    layout->addWidget(helpLabel);
    layout->addWidget(pathWidget);
    layout->addSpacing(14);
    return radio;
  };

  auto *userRadio = buildOption(
      QObject::tr("Current user"),
      QObject::tr("Settings apply only to your user account on this computer."),
      Settings::UserSettingFile
  );
  auto *systemRadio = buildOption(
      QObject::tr("All users"),
      QObject::tr("Settings apply to every user on this computer. Requires administrator privileges."),
      Settings::SystemSettingFile
  );

  const bool isSystem = (Settings::settingsFile() == Settings::SystemSettingFile);
  userRadio->setChecked(!isSystem);
  systemRadio->setChecked(isSystem);

  layout->addStretch();

  QObject::connect(systemRadio, &QRadioButton::toggled, parent, [parent, userRadio, systemRadio](bool toSystem) {
    if (SettingsScope::switchTo(parent, toSystem)) {
      return;
    }
    QSignalBlocker blockUser(userRadio);
    QSignalBlocker blockSystem(systemRadio);
    userRadio->setChecked(!toSystem);
    systemRadio->setChecked(toSystem ? false : true);
  });

  tabWidget->addTab(scopeTab, QObject::tr("Scope"));
}
