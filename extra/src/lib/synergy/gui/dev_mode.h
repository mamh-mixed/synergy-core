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

#include "common/VersionInfo.h"
#include "synergy/build_config.h"

#include <QString>
#include <QStringLiteral>

namespace synergy::gui {

inline QString titleWithDevSuffix(const QString &productName)
{
  if constexpr (synergy::kIsDevBuild) {
    return QStringLiteral("%1 - Developer build - v%2").arg(productName, kDisplayVersion);
  } else {
    return productName;
  }
}

} // namespace synergy::gui
