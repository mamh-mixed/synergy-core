/*
 * Synergy -- mouse and keyboard sharing utility
 * Copyright (C) 2024 Synergy App Ltd
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

#include "ExtraSettings.h"

#include "common/Settings.h"

#include <QDebug>

namespace synergy::gui {

const auto kSerialKeySettingKey = QStringLiteral("synergy/serialKey");
const auto kActivatedSettingKey = QStringLiteral("synergy/activated");
const auto kGraceStartSettingKey = QStringLiteral("synergy/graceStartEpochSecs");

void ExtraSettings::load()
{
  m_serialKey = Settings::value(kSerialKeySettingKey).toString();
  m_activated = Settings::value(kActivatedSettingKey).toBool();
  m_graceStartEpochSecs = Settings::value(kGraceStartSettingKey).toLongLong();
}

void ExtraSettings::sync()
{
  if (!Settings::isWritable()) {
    qCritical() << "unable to save synergy settings, file not writable:" << Settings::settingsFile();
    return;
  }

  Settings::setValue(kSerialKeySettingKey, m_serialKey);
  Settings::setValue(kActivatedSettingKey, m_activated);
  Settings::setValue(kGraceStartSettingKey, m_graceStartEpochSecs);
  Settings::save();
}

QString ExtraSettings::fileName() const
{
  return Settings::settingsFile();
}

bool ExtraSettings::isWritable() const
{
  return Settings::isWritable();
}

} // namespace synergy::gui
