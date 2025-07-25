/*
 * Deskflow -- mouse and keyboard sharing utility
 * Copyright (C) 2012 Symless Ltd.
 * Copyright (C) 2008 Volker Lanz (vl@fidra.de)
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

#include "SettingsDialog.h"

#ifdef DESKFLOW_GUI_HOOK_HEADER
#include DESKFLOW_GUI_HOOK_HEADER
#endif

#include "gui/core/CoreProcess.h"
#include "gui/messages.h"
#include "gui/paths.h"
#include "gui/tls/TlsCertificate.h"
#include "gui/tls/TlsUtility.h"
#include "gui/validators/ScreenNameValidator.h"
#include "gui/validators/ValidationError.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QtCore>
#include <QtGui>

using namespace deskflow::gui;
using namespace deskflow::gui::proxy;

SettingsDialog::SettingsDialog(
    QWidget *parent, IAppConfig &appConfig, const IServerConfig &serverConfig, const CoreProcess &coreProcess
)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
      Ui::SettingsDialogBase(),
      m_appConfig(appConfig),
      m_serverConfig(serverConfig),
      m_coreProcess(coreProcess),
      m_tlsUtility(appConfig)
{
  setupUi(this);

  // force the first tab, since qt creator sets the active tab as the last one
  // the developer was looking at, and it's easy to accidentally save that.
  m_pTabWidget->setCurrentIndex(0);

  loadFromConfig();
  updateControls();

  m_pScreenNameError = new validators::ValidationError(this);
  m_pLineEditScreenName->setValidator(
      new validators::ScreenNameValidator(m_pLineEditScreenName, m_pScreenNameError, &serverConfig.screens())
  );

  connect(m_pCheckBoxEnableTls, &QCheckBox::toggled, this, &SettingsDialog::updateTlsControls);

#ifdef DESKFLOW_GUI_HOOK_SETTINGS
  DESKFLOW_GUI_HOOK_SETTINGS
#endif
}

//
// Auto-connect slots
//

void SettingsDialog::on_m_pCheckBoxLogToFile_stateChanged(int i)
{
  bool checked = i == 2;

  m_pLabelLogPath->setEnabled(checked);
  m_pLineEditLogFilename->setEnabled(checked);
  m_pButtonBrowseLog->setEnabled(checked);
}

void SettingsDialog::on_m_pButtonBrowseLog_clicked()
{
  QString fileName = QFileDialog::getSaveFileName(
      this, tr("Save log file to..."), m_pLineEditLogFilename->text(), "Logs (*.log *.txt)"
  );

  if (!fileName.isEmpty()) {
    m_pLineEditLogFilename->setText(fileName);
  }
}

void SettingsDialog::on_m_pCheckBoxEnableTls_clicked(bool)
{
  updateTlsControls();
}

void SettingsDialog::on_m_pPushButtonTlsCertPath_clicked()
{
  QString fileName = QFileDialog::getSaveFileName(
      this, tr("Select a TLS certificate to use..."), m_pLineEditTlsCertPath->text(), "Cert (*.pem)", nullptr,
      QFileDialog::DontConfirmOverwrite
  );

  if (!fileName.isEmpty()) {
    m_pLineEditTlsCertPath->setText(fileName);

    if (QFile(fileName).exists()) {
      updateKeyLengthOnFile(fileName);
    } else {
      qDebug("no tls certificate file at: %s", qUtf8Printable(fileName));
    }
  }
}

void SettingsDialog::on_m_pPushButtonTlsRegenCert_clicked()
{
  if (m_tlsUtility.generateCertificate()) {
    QMessageBox::information(this, tr("TLS Certificate Regenerated"), tr("TLS certificate regenerated successfully."));
  }
}

void SettingsDialog::on_m_pCheckBoxServiceEnabled_toggled(bool)
{
  updateControls();
}

//
// End of auto-connect slots
//

void SettingsDialog::showEvent(QShowEvent *event)
{
  QDialog::showEvent(event);
  emit shown();
}

void SettingsDialog::accept()
{
  if (!m_pLineEditScreenName->hasAcceptableInput()) {
    QMessageBox::warning(this, tr("Invalid screen name"), m_pScreenNameError->message());
    return;
  }

  m_appConfig.setScreenName(m_pLineEditScreenName->text());
  m_appConfig.setPort(m_pSpinBoxPort->value());
  m_appConfig.setNetworkInterface(m_pLineEditInterface->text());
  m_appConfig.setLogLevel(m_pComboLogLevel->currentIndex());
  m_appConfig.setLogToFile(m_pCheckBoxLogToFile->isChecked());
  m_appConfig.setLogFilename(m_pLineEditLogFilename->text());
  m_appConfig.setElevateMode(static_cast<ElevateMode>(m_pComboElevate->currentIndex()));
  m_appConfig.setAutoHide(m_pCheckBoxAutoHide->isChecked());
  m_appConfig.setPreventSleep(m_pCheckBoxPreventSleep->isChecked());
  m_appConfig.setTlsCertPath(m_pLineEditTlsCertPath->text());
  m_appConfig.setTlsKeyLength(m_pComboBoxTlsKeyLength->currentText().toInt());
  m_appConfig.setTlsEnabled(m_pCheckBoxEnableTls->isChecked());
  m_appConfig.setLanguageSync(m_pCheckBoxLanguageSync->isChecked());
  m_appConfig.setInvertScrollDirection(m_pCheckBoxScrollDirection->isChecked());
  m_appConfig.setEnableService(m_pCheckBoxServiceEnabled->isChecked());
  m_appConfig.setCloseToTray(m_pCheckBoxCloseToTray->isChecked());
  m_appConfig.setInvertConnection(m_pCheckBoxInvertConnection->isChecked());
  m_appConfig.setEnableDragAndDrop(m_pCheckBoxDragAndDrop->isChecked());
  m_appConfig.setEnableLibei(m_pCheckBoxUseLibei->isChecked());

  QDialog::accept();
}

void SettingsDialog::loadFromConfig()
{
  m_pLineEditScreenName->setText(m_appConfig.screenName());
  m_pSpinBoxPort->setValue(m_appConfig.port());
  m_pLineEditInterface->setText(m_appConfig.networkInterface());
  m_pComboLogLevel->setCurrentIndex(m_appConfig.logLevel());
  m_pCheckBoxLogToFile->setChecked(m_appConfig.logToFile());
  m_pLineEditLogFilename->setText(m_appConfig.logFilename());
  m_pCheckBoxAutoHide->setChecked(m_appConfig.autoHide());
  m_pCheckBoxPreventSleep->setChecked(m_appConfig.preventSleep());
  m_pCheckBoxLanguageSync->setChecked(m_appConfig.languageSync());
  m_pCheckBoxScrollDirection->setChecked(m_appConfig.invertScrollDirection());
  m_pCheckBoxServiceEnabled->setChecked(m_appConfig.enableService());
  m_pCheckBoxCloseToTray->setChecked(m_appConfig.closeToTray());
  m_pComboElevate->setCurrentIndex(static_cast<int>(m_appConfig.elevateMode()));
  m_pCheckBoxDragAndDrop->setChecked(m_appConfig.enableDragAndDrop());
  m_pCheckBoxUseLibei->setChecked(m_appConfig.enableLibei());

  // Toggle handler is not meant for programmatic changes.
  m_pRadioSystemScope->blockSignals(true);
  if (m_appConfig.isSystemScope()) {
    m_pRadioSystemScope->setChecked(true);
  } else {
    m_pRadioUserScope->setChecked(true);
  }
  m_pRadioSystemScope->blockSignals(false);

  m_pCheckBoxInvertConnection->setChecked(m_appConfig.invertConnection());

  updateTlsCheckbox();
  updateTlsControls();
}

void SettingsDialog::updateTlsCheckbox()
{
  bool tlsAvailable = m_appConfig.isWritable();

  auto &locked = m_appConfig.settings().getLockedSettings();
  if (locked.contains("cryptoEnabled")) {
    qDebug("locking tls setting");
    tlsAvailable = false;
  }

  m_pCheckBoxEnableTls->setEnabled(tlsAvailable);
  m_pCheckBoxEnableTls->setChecked(m_tlsUtility.isEnabled());
}

void SettingsDialog::updateTlsControls()
{
  QString tlsCertPath = paths::tlsFilePath(m_appConfig.tlsCertPath(), m_appConfig.isSystemScope());
  if (QFile(tlsCertPath).exists()) {
    updateKeyLengthOnFile(tlsCertPath);
  } else {
    const auto keyLengthText = QString::number(m_appConfig.tlsKeyLength());
    m_pComboBoxTlsKeyLength->setCurrentIndex(m_pComboBoxTlsKeyLength->findText(keyLengthText));
  }

  const auto writable = m_appConfig.isWritable();
  const auto clientMode = m_appConfig.clientGroupChecked();
  const auto tlsChecked = m_pCheckBoxEnableTls->isChecked();

  m_pLineEditTlsCertPath->setText(m_appConfig.tlsCertPath());
  m_pLineEditTlsCertPath->setPlaceholderText(tlsCertPath);

  auto enabled = writable && tlsChecked && !clientMode;
  m_pLabelTlsKeyLength->setEnabled(enabled);
  m_pComboBoxTlsKeyLength->setEnabled(enabled);
  m_pLabelTlsCert->setEnabled(enabled);
  m_pLineEditTlsCertPath->setEnabled(enabled);
  m_pPushButtonTlsCertPath->setEnabled(enabled);
  m_pPushButtonTlsRegenCert->setEnabled(enabled);

  auto &locked = m_appConfig.settings().getLockedSettings();
  if (locked.contains("tlsCertPath")) {
    qDebug("locking tls cert path setting");
    m_pLineEditTlsCertPath->setEnabled(false);
    m_pPushButtonTlsCertPath->setEnabled(false);
    m_pPushButtonTlsRegenCert->setEnabled(false);
  }
  if (locked.contains("tlsKeyLength")) {
    qDebug("locking tls key length setting");
    m_pComboBoxTlsKeyLength->setEnabled(false);
  }
}

bool SettingsDialog::isClientMode() const
{
  return m_coreProcess.mode() == deskflow::gui::CoreProcess::Mode::Client;
}

void SettingsDialog::updateKeyLengthOnFile(const QString &path)
{
  TlsCertificate ssl;
  if (!QFile(path).exists()) {
    qFatal("tls certificate file not found: %s", qUtf8Printable(path));
  }

  auto length = ssl.getCertKeyLength(path);
  auto index = m_pComboBoxTlsKeyLength->findText(QString::number(length));
  m_pComboBoxTlsKeyLength->setCurrentIndex(index);
  m_appConfig.setTlsKeyLength(length);
}

void SettingsDialog::updateControls()
{
  using enum deskflow::gui::ISettings::Scope;

#if defined(Q_OS_WIN)
  const auto serviceAvailable = true;
#else
  // service not supported on unix yet, so always disable.
  const auto serviceAvailable = false;
  m_pGroupService->setTitle("Service (Windows only)");
#endif

  const auto writable = m_appConfig.isWritable();
  const auto serviceChecked = m_pCheckBoxServiceEnabled->isChecked();
  const auto logToFile = m_pCheckBoxLogToFile->isChecked();

  const auto allUsersReadOnly = QStringLiteral("All users (read-only)");
  const auto currentUserReadOnly = QStringLiteral("Current user (read-only)");

  const auto &systemSettings = m_appConfig.settings().getSystemSettings();
  const auto &userSettings = m_appConfig.settings().getUserSettings();

  const auto saveButton = m_pButtonBox->button(QDialogButtonBox::Save);
  if (!saveButton) {
    qFatal("save button not found");
  }
  saveButton->setEnabled(writable);

  if (!systemSettings.isWritable()) {
    m_pRadioSystemScope->setText(allUsersReadOnly);
    m_pRadioSystemScope->setEnabled(false);
    m_pRadioUserScope->setEnabled(false);
  }

  if (!userSettings.isWritable()) {
    m_pRadioUserScope->setText(currentUserReadOnly);
  }

  m_pRadioSystemScope->setToolTip(m_appConfig.settings().getSystemSettings().fileName());
  m_pRadioUserScope->setToolTip(m_appConfig.settings().getUserSettings().fileName());

  m_pLineEditScreenName->setEnabled(writable);
  m_pSpinBoxPort->setEnabled(writable);
  m_pLineEditInterface->setEnabled(writable);
  m_pComboLogLevel->setEnabled(writable);
  m_pCheckBoxLogToFile->setEnabled(writable);
  m_pCheckBoxAutoHide->setEnabled(writable);
  m_pCheckBoxPreventSleep->setEnabled(writable);
  m_pCheckBoxCloseToTray->setEnabled(writable);

  // TLS area
  m_pLineEditTlsCertPath->setEnabled(writable);
  m_pComboBoxTlsKeyLength->setEnabled(writable);
  m_pPushButtonTlsCertPath->setEnabled(writable);
  m_pPushButtonTlsRegenCert->setEnabled(writable);

  m_pCheckBoxServiceEnabled->setEnabled(writable && serviceAvailable);
  m_pLabelElevate->setEnabled(writable && serviceChecked && serviceAvailable);
  m_pComboElevate->setEnabled(writable && serviceChecked && serviceAvailable);

  m_pCheckBoxLanguageSync->setEnabled(writable && isClientMode());
  m_pCheckBoxScrollDirection->setEnabled(writable && isClientMode());

  m_pLabelLogPath->setEnabled(writable && logToFile);
  m_pLineEditLogFilename->setEnabled(writable && logToFile);
  m_pButtonBrowseLog->setEnabled(writable && logToFile);

  // Experimental tab
  m_pCheckBoxInvertConnection->setEnabled(writable);
  m_pCheckBoxDragAndDrop->setEnabled(writable);
  m_pCheckBoxUseLibei->setEnabled(writable);

#ifdef Q_OS_LINUX
  m_pCheckBoxDragAndDrop->setEnabled(false);
#endif

#ifndef Q_OS_LINUX
  m_pCheckBoxUseLibei->setEnabled(false);
#endif

  updateTlsControls();
}
