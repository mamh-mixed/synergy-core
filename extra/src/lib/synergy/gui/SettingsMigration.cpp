/*
 * Synergy -- mouse and keyboard sharing utility
 * Copyright (C) 2026 Synergy App Ltd
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

#include "SettingsMigration.h"

#include "common/Constants.h"
#include "common/Settings.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QStringLiteral>

#include <optional>
#include <utility>

namespace synergy::gui::migration {

namespace {

const auto kSchemaKey = QStringLiteral("migration/schemaVersion");
const auto kNotifiedKey = QStringLiteral("migration/notifiedFor");
const auto kLegacySystemScopeKey = QStringLiteral("systemScope");

QString extraFile()
{
  return QStringLiteral("%1/%2.extra.conf").arg(Settings::UserDir, kAppName);
}

int storedSchemaVersion()
{
  QSettings ini(extraFile(), QSettings::IniFormat);
  return ini.value(kSchemaKey, 0).toInt();
}

void writeSchemaVersion(int version)
{
  QSettings ini(extraFile(), QSettings::IniFormat);
  ini.setValue(kSchemaKey, version);
  ini.sync();
}

int notifiedSchemaVersion()
{
  QSettings ini(extraFile(), QSettings::IniFormat);
  return ini.value(kNotifiedKey, 0).toInt();
}

void writeNotifiedVersion(int version)
{
  QSettings ini(extraFile(), QSettings::IniFormat);
  ini.setValue(kNotifiedKey, version);
  ini.sync();
}

// Maps a legacy key + value to the new (key, value) pair, or returns
// nullopt to drop. Special cases (boolean → enum) handled inline.
std::optional<std::pair<QString, QVariant>> mapKey(const QString &oldKey, const QVariant &value)
{
  if (oldKey == "screenName") {
    return std::make_pair(Settings::Core::ComputerName, value);
  }
  if (oldKey == "port") {
    return std::make_pair(Settings::Core::Port, value);
  }
  if (oldKey == "interface") {
    return std::make_pair(Settings::Core::Interface, value);
  }
  if (oldKey == "logLevel2") {
    return std::make_pair(Settings::Log::Level, value);
  }
  if (oldKey == "logToFile") {
    return std::make_pair(Settings::Log::ToFile, value);
  }
  if (oldKey == "logFilename") {
    return std::make_pair(Settings::Log::File, value);
  }
  if (oldKey == "elevateModeEnum") {
    return std::make_pair(Settings::Daemon::Elevate, value);
  }
  if (oldKey == "cryptoEnabled") {
    return std::make_pair(Settings::Security::TlsEnabled, value);
  }
  if (oldKey == "autoHide") {
    return std::make_pair(Settings::Gui::Autohide, value);
  }
  if (oldKey == "lastVersion") {
    return std::make_pair(Settings::Core::LastVersion, value);
  }
  if (oldKey == "groupServerChecked") {
    if (value.toBool()) {
      return std::make_pair(Settings::Core::CoreMode, QVariant(Settings::Server));
    }
    return std::nullopt;
  }
  if (oldKey == "groupClientChecked") {
    if (value.toBool()) {
      return std::make_pair(Settings::Core::CoreMode, QVariant(Settings::Client));
    }
    return std::nullopt;
  }
  if (oldKey == "useExternalConfig") {
    return std::make_pair(Settings::Server::ExternalConfig, value);
  }
  if (oldKey == "configFile") {
    return std::make_pair(Settings::Server::ExternalConfigFile, value);
  }
  if (oldKey == "serverHostname") {
    return std::make_pair(Settings::Client::RemoteHost, value);
  }
  if (oldKey == "tlsCertPath") {
    return std::make_pair(Settings::Security::Certificate, value);
  }
  if (oldKey == "tlsKeyLength") {
    return std::make_pair(Settings::Security::KeySize, value);
  }
  if (oldKey == "preventSleep") {
    return std::make_pair(Settings::Core::PreventSleep, value);
  }
  if (oldKey == "languageSync") {
    return std::make_pair(Settings::Client::LanguageSync, value);
  }
  if (oldKey == "invertScrollDirection") {
    return std::make_pair(Settings::Client::InvertYScroll, value);
  }
  if (oldKey == "enableService") {
    const auto mode = value.toBool() ? Settings::Service : Settings::Desktop;
    return std::make_pair(Settings::Core::ProcessMode, QVariant(mode));
  }
  if (oldKey == "closeToTray") {
    return std::make_pair(Settings::Gui::CloseToTray, value);
  }
  if (oldKey == "showCloseReminder") {
    return std::make_pair(Settings::Gui::CloseReminder, value);
  }
  if (oldKey == "enableUpdateCheck") {
    return std::make_pair(Settings::Gui::AutoUpdateCheck, value);
  }
  return std::nullopt;
}

// Sentinel keys that indicate a QSettings instance holds legacy-format
// data (Synergy 1.x, pre-Settings::* API). Detection by sentinel rather
// than file existence is necessary because on Linux the legacy NativeFormat
// path coincides with beta's IniFormat path.
bool looksLikeLegacy(const QSettings &legacy)
{
  return legacy.contains(QStringLiteral("startedBefore")) || legacy.contains(QStringLiteral("groupServerChecked")) ||
         legacy.contains(QStringLiteral("groupClientChecked")) || legacy.contains(kLegacySystemScopeKey);
}

// Dumps every key/value from `legacy` into a sibling INI file. Cross-platform
// backup — works whether legacy lives in a file (Linux), a plist (macOS),
// or the registry (Windows), because we're dumping the in-memory contents,
// not file-copying.
QString backupLegacy(const QSettings &legacy, const QString &destPath)
{
  if (QFile::exists(destPath)) {
    QFile::remove(destPath);
  }
  QSettings backup(destPath, QSettings::IniFormat);
  for (const auto &key : legacy.allKeys()) {
    backup.setValue(key, legacy.value(key));
  }
  backup.sync();
  return destPath;
}

// Returns true iff at least one key was migrated from `legacy` to the
// QSettings instance at `newPath`.
int migrateOneScope(QSettings &legacy, const QString &newPath)
{
  QSettings newSettings(newPath, QSettings::IniFormat);
  int migrated = 0;
  int dropped = 0;
  for (const auto &oldKey : legacy.allKeys()) {
    if (auto mapped = mapKey(oldKey, legacy.value(oldKey)); mapped.has_value()) {
      newSettings.setValue(mapped->first, mapped->second);
      migrated++;
    } else {
      dropped++;
    }
  }
  newSettings.sync();
  qInfo("settings migration: scope %s — migrated %d keys, dropped %d obsolete", qPrintable(newPath), migrated, dropped);
  return migrated;
}

bool s_migrationRanThisLaunch = false;
QString s_lastBackupPath;

// True iff `legacy`'s storage backend points at the same on-disk file as
// `newPath`. On Linux this is the case (NativeFormat → INI at the same
// QStandardPaths location upstream's Settings uses); on macOS/Windows
// they're separate (plist / registry vs. our explicit IniFormat path).
// When same, we MUST NOT clear() the legacy storage — that would wipe
// the new keys we just wrote.
bool sharesPathWith(const QSettings &legacy, const QString &newPath)
{
  const auto legacyCanonical = QFileInfo(legacy.fileName()).canonicalFilePath();
  const auto newCanonical = QFileInfo(newPath).canonicalFilePath();
  if (legacyCanonical.isEmpty() || newCanonical.isEmpty()) {
    // canonicalFilePath returns empty if the file doesn't exist; fall back
    // to absolute path comparison.
    return QFileInfo(legacy.fileName()).absoluteFilePath() == QFileInfo(newPath).absoluteFilePath();
  }
  return legacyCanonical == newCanonical;
}

void maybeClearLegacy(QSettings &legacy, const QString &newPath, const char *scopeLabel)
{
  if (sharesPathWith(legacy, newPath)) {
    qDebug(
        "settings migration: %s legacy shares storage with new file, skipping clear (cleanSettings will handle)",
        scopeLabel
    );
    return;
  }
  if (!legacy.isWritable()) {
    qWarning("settings migration: %s legacy not writable, leaving legacy keys in place", scopeLabel);
    return;
  }
  legacy.clear();
  legacy.sync();
  qInfo("settings migration: cleared %s legacy storage at %s", scopeLabel, qPrintable(legacy.fileName()));
}

bool runLegacyMigration()
{
  bool any = false;

  // User scope.
  QSettings legacyUser(QSettings::NativeFormat, QSettings::UserScope, kAppName, kAppName);
  const bool legacyHadSystemScope = legacyUser.value(kLegacySystemScopeKey, false).toBool();
  if (looksLikeLegacy(legacyUser)) {
    s_lastBackupPath = backupLegacy(legacyUser, Settings::UserSettingFile + QStringLiteral(".legacy.bak"));
    qInfo().noquote() << "settings migration: user-scope legacy backed up to" << s_lastBackupPath;
    if (migrateOneScope(legacyUser, Settings::UserSettingFile) > 0) {
      any = true;
    }
    maybeClearLegacy(legacyUser, Settings::UserSettingFile, "user-scope");
  }

  // System scope.
  QSettings legacySystem(QSettings::NativeFormat, QSettings::SystemScope, kAppName, kAppName);
  if (looksLikeLegacy(legacySystem)) {
    const auto backupPath = Settings::SystemSettingFile + QStringLiteral(".legacy.bak");
    s_lastBackupPath = backupLegacy(legacySystem, backupPath);
    qInfo().noquote() << "settings migration: system-scope legacy backed up to" << s_lastBackupPath;
    if (migrateOneScope(legacySystem, Settings::SystemSettingFile) > 0) {
      any = true;
    }
    maybeClearLegacy(legacySystem, Settings::SystemSettingFile, "system-scope");
  }

  // If legacy user-scope had systemScope=true, the user's preference is to
  // run on system scope. Persist for the post-Settings hook to pick up.
  if (legacyHadSystemScope) {
    QSettings extra(extraFile(), QSettings::IniFormat);
    extra.setValue(QStringLiteral("migration/preferSystemScope"), true);
    extra.sync();
  }

  return any;
}

} // namespace

bool migrateIfNeeded()
{
  if (storedSchemaVersion() >= kCurrentSchemaVersion) {
    return false;
  }

  s_migrationRanThisLaunch = runLegacyMigration();
  writeSchemaVersion(kCurrentSchemaVersion);
  return s_migrationRanThisLaunch;
}

bool preferSystemScope()
{
  QSettings extra(extraFile(), QSettings::IniFormat);
  return extra.value(QStringLiteral("migration/preferSystemScope"), false).toBool();
}

void showNoticeIfPending(QWidget *parent)
{
  if (notifiedSchemaVersion() >= kCurrentSchemaVersion) {
    return;
  }
  if (!s_migrationRanThisLaunch) {
    writeNotifiedVersion(kCurrentSchemaVersion);
    return;
  }

  QMessageBox::information(
      parent, QObject::tr("Settings updated"),
      QObject::tr("<p>We've migrated your settings to a new format used by this version of Synergy.</p>"
                  "<p>Your previous settings have been backed up to:</p>"
                  "<p><code>%1</code></p>"
                  "<p>If anything looks different, please contact us.</p>")
          .arg(s_lastBackupPath)
  );
  writeNotifiedVersion(kCurrentSchemaVersion);
}

} // namespace synergy::gui::migration
