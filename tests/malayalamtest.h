/*
 *  Copyright (C) 2009  Anne-Marie Mahfouf   <annma@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef MALAYALAMTEST_H
#define MALAYALAMTEST_H

#include <qtest_kde.h>
#include <QtCore/QObject>

class MalayalamTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void matchChar();
    void decomposeConsonant();

};


#endif /* MALAYALAMTEST_H */
