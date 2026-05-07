# Synergy -- mouse and keyboard sharing utility
# Copyright (C) 2024 Synergy App Ltd
#
# This package is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# found in the file LICENSE that should have accompanied this file.
#
# This package is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

add_subdirectory(license)

# gui subdirectory disabled until hook modernization rewires it.
# The synergy GUI code was written against an older deskflow gui API
# (gui/string_utils.h, gui/styles.h, gui/diagnostic.h, gui/config/AppConfig.h,
# gui/config/Settings.h — all refactored away upstream). Re-enable when hook
# wiring lands and the gui integration is ported.
# add_subdirectory(gui)
