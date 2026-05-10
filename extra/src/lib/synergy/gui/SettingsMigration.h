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

class QWidget;

namespace synergy::gui::migration {

// Bumped each time a new migration is added. Stored as
// migration/schemaVersion in Synergy.extra.conf so we don't re-run.
constexpr int kCurrentSchemaVersion = 1;

// Reads legacy-format keys (Synergy 1.x — QSettings::NativeFormat,
// user + system scope) and writes their new-format equivalents BEFORE
// upstream Settings is initialized, since Settings::cleanSettings()
// strips any key not in its allow-list. Backs up the original key set
// to <newPath>.legacy.bak before any modification. Idempotent — no-op
// if migration/schemaVersion is already current.
//
// Returns true iff a migration ran this launch (caller can show a notice).
bool migrateIfNeeded();

// True iff the legacy user scope had `systemScope=true`. Caller can use
// this after Settings::instance() is up to call
// Settings::setSettingsFile(SystemSettingFile) and honor the user's
// scope preference across the upgrade. Persisted in Synergy.extra.conf
// as `migration/preferSystemScope` so subsequent launches keep honoring it.
bool preferSystemScope();

// Modal one-time notice shown after MainWindow is open. No-op if not
// pending. Marks migration/notifiedFor=schemaVersion when dismissed.
void showNoticeIfPending(QWidget *parent);

} // namespace synergy::gui::migration
