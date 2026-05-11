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

#include "TestSettings.h"

#include "common/Constants.h"
#include "common/Settings.h"

#include <QDebug>
#include <QFile>
#include <QSettings>

namespace synergy::gui {

namespace {

QString envOrFile(const char *envName, const QString &fileValue, bool fileEnabled)
{
  const auto env = qEnvironmentVariable(envName);
  if (!env.isEmpty()) {
    return env;
  }
  return fileEnabled ? fileValue : QString();
}

} // namespace

TestSettings &TestSettings::instance()
{
  static TestSettings inst;
  return inst;
}

TestSettings::TestSettings()
    : m_fileName(QStringLiteral("%1/%2.test.conf").arg(Settings::UserDir, kAppName))
{
  load();
}

void TestSettings::load()
{
  m_enabled = false;
  m_licensing = false;
  m_serialKey.clear();
  m_apiUrlActivate.clear();
  m_apiUrlCheck.clear();
  m_startTimeEpochSecs = 0;
  m_verbose = false;
  m_skipRemoteCheck = false;
  m_allowExpiredLicenses = false;

  if (!QFile::exists(m_fileName)) {
    qDebug().noquote() << "test settings file not found, test mode off:" << m_fileName;
    return;
  }

  QSettings ini(m_fileName, QSettings::IniFormat);
  m_enabled = ini.value(QStringLiteral("test/enabled"), false).toBool();
  if (!m_enabled) {
    qDebug().noquote() << "test settings file present but test/enabled=false:" << m_fileName;
    return;
  }
  m_licensing = ini.value(QStringLiteral("test/licensing"), false).toBool();
  m_serialKey = ini.value(QStringLiteral("test/serialKey")).toString();
  m_apiUrlActivate = ini.value(QStringLiteral("test/apiUrlActivate")).toString();
  m_apiUrlCheck = ini.value(QStringLiteral("test/apiUrlCheck")).toString();
  m_startTimeEpochSecs = ini.value(QStringLiteral("test/startTime"), 0).toLongLong();

  m_verbose = ini.value(QStringLiteral("features/verbose"), false).toBool();
  m_skipRemoteCheck = ini.value(QStringLiteral("features/skipRemoteCheck"), false).toBool();
  m_allowExpiredLicenses = ini.value(QStringLiteral("features/allowExpiredLicenses"), false).toBool();

  qInfo().noquote() << "test mode enabled, loaded:" << m_fileName;
}

void TestSettings::reload()
{
  load();
}

QString TestSettings::serialKey() const
{
  return envOrFile("SYNERGY_TEST_SERIAL_KEY", m_serialKey, m_enabled);
}

QString TestSettings::apiUrlActivate() const
{
  return envOrFile("SYNERGY_TEST_API_URL_ACTIVATE", m_apiUrlActivate, m_enabled);
}

QString TestSettings::apiUrlCheck() const
{
  return envOrFile("SYNERGY_TEST_API_URL_CHECK", m_apiUrlCheck, m_enabled);
}

qint64 TestSettings::startTimeEpochSecs() const
{
  const auto env = qEnvironmentVariable("SYNERGY_TEST_START_TIME");
  if (!env.isEmpty()) {
    return env.toLongLong();
  }
  return m_enabled ? m_startTimeEpochSecs : 0;
}

} // namespace synergy::gui
