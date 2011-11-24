/******************************************************************************
 * This file is part of the Mula project
 * Copyright (c) 2011 Laszlo Papp <lpapp@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "wordentrytest.h"

#include <plugins/stardict/wordentry.h>

#include <QtTest/QtTest>

using namespace MulaPluginStarDict;

WordEntryTest::WordEntryTest()
{

}

WordEntryTest::~WordEntryTest()
{
}

void WordEntryTest::testData()
{
    WordEntry wordEntry;
    QByteArray data = "Data";
    wordEntry.setData(data);
    QCOMPARE(wordEntry.data(), data);
}

void WordEntryTest::testDataOffset()
{
    WordEntry wordEntry;
    quint32 dataOffset = 100;
    wordEntry.setDataOffset(dataOffset);
    QCOMPARE(wordEntry.dataOffset(), dataOffset);
}

void WordEntryTest::testDataSize()
{
    WordEntry wordEntry;
    quint32 dataSize = 200;
    wordEntry.setDataSize(dataSize);
    QCOMPARE(wordEntry.dataSize(), dataSize);
}

QTEST_MAIN(WordEntryTest)

#include "wordentrytest.moc"

