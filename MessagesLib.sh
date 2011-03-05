#!/bin/sh

# Helper script to extract translatable strings out of libfritz++ and into KDE scripty mechanism.
# This has to be called manually on library changes.

echo "/*\n\
 * KFritz\n\
 *\n\
 * Copyright (C) 2011 Joachim Wilke <kfritz@joachim-wilke.de>\n\
 *\n\
 * This program is free software; you can redistribute it and/or\n\
 * modify it under the terms of the GNU General Public License\n\
 * as published by the Free Software Foundation; either version 2\n\
 * of the License, or (at your option) any later version.\n\
 *\n\
 * This program is distributed in the hope that it will be useful,\n\
 * but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
 * GNU General Public License for more details.\n\
 *\n\
 * You should have received a copy of the GNU General Public License\n\
 * along with this program; if not, write to the Free Software\n\
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n\
 *\n\
 */\n\
 Automatically created by MessagesLib.sh\n" > LibFritzI18N.cpp

grep -ir I18N_NOOP libfritz++/*.cpp | sed -e 's/.*I18N_NOOP(\([^)]*\).*/i18n(\1)/' | sort | uniq >> LibFritzI18N.cpp
