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

#pragma once

#include <QString>

namespace synergy::gui {

// QA/test overrides for Synergy. Lives at `${UserDir}/${kAppName}.test.conf`,
// sibling to the main Synergy.conf settings file. Sourcing values from a file
// (rather than env vars) sidesteps the pain of getting env vars into GUI
// launches on macOS / Windows / VS Code F5.
//
// All accessors fall back to the corresponding SYNERGY_TEST_* env var when
// the env var is set, so existing CI/automation flows are unchanged.
class TestSettings
{
public:
  static TestSettings &instance();

  // True iff the test config file exists and `[test]/enabled=true`. The
  // value is cached at construction; call reload() to re-read.
  bool isEnabled() const
  {
    return m_enabled;
  }

  // Test overrides. Each accessor returns the env var when set, otherwise
  // the file value when isEnabled(), otherwise empty / 0.
  QString serialKey() const;
  QString apiUrlActivate() const;
  QString apiUrlCheck() const;
  qint64 startTimeEpochSecs() const;

  // Feature toggles read from the [features] section. File-only — no env
  // var fallback for these (no precedent).
  bool verbose() const
  {
    return m_verbose;
  }
  bool skipRemoteCheck() const
  {
    return m_skipRemoteCheck;
  }
  bool allowExpiredLicenses() const
  {
    return m_allowExpiredLicenses;
  }

  // Path to the test config file (whether or not it exists).
  QString fileName() const
  {
    return m_fileName;
  }

  // Re-read the file from disk (e.g., from a Test menu "Reload" action).
  void reload();

private:
  TestSettings();
  TestSettings(const TestSettings &) = delete;
  TestSettings &operator=(const TestSettings &) = delete;

  void load();

  QString m_fileName;
  bool m_enabled = false;
  QString m_serialKey;
  QString m_apiUrlActivate;
  QString m_apiUrlCheck;
  qint64 m_startTimeEpochSecs = 0;
  bool m_verbose = false;
  bool m_skipRemoteCheck = false;
  bool m_allowExpiredLicenses = false;
};

} // namespace synergy::gui
