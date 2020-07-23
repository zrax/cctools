# This file is part of CCTools.
#
# CCTools is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# CCTools is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with CCTools.  If not, see <http://www.gnu.org/licenses/>.

set(CommonWidgets_HEADERS
    About.h
    EditorTabWidget.h
)

set(CommonWidgets_SOURCES
    EditorTabWidget.cpp
)

add_library(CommonWidgets STATIC ${CommonWidgets_HEADERS} ${CommonWidgets_SOURCES})
target_link_libraries(CommonWidgets Qt5::Core Qt5::Gui Qt5::Widgets)
