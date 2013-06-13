/*
 * KFritz
 *
 * Copyright (C) 2010-2012 Joachim Wilke <kfritz@joachim-wilke.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "KFritzDbusService.h"
#include "KFritzDbusServiceAdaptor.h"
#include <iostream>
#include "liblog++/Log.h"
#include "DialDialog.h"

KFritzDbusService::KFritzDbusService(KMainWindow *parent)
: QObject(parent)
{
    new KFritzDbusServiceAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/KFritz",this);
    dbus.registerService("org.kde.KFritz");
}

void KFritzDbusService::dialNumber(const QString &number) {
    INF("DBUS dialNumber:" + number.toStdString());
    DialDialog d(static_cast<KMainWindow*>(parent()), number.toStdString());

    d.exec();
    d.clearFocus();
    d.setFocus();
    d.raise();
    d.activateWindow();
}
