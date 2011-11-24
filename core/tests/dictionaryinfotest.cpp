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

#include "dictionaryinfotest.h"

#include <core/dictionaryinfo.h>

#include <QtTest/QtTest>

using namespace MulaCore;

DictionaryInfoTest::DictionaryInfoTest()
{

}

DictionaryInfoTest::~DictionaryInfoTest()
{
}

void DictionaryInfoTest::testPlugin()
{
    DictionaryInfo dictionaryInfo;
    QString plugin = "Plugin";
    dictionaryInfo.setPlugin(plugin);
    QCOMPARE(dictionaryInfo.plugin(), plugin);
}

void DictionaryInfoTest::testName()
{
    DictionaryInfo dictionaryInfo;
    QString name = "Name";
    dictionaryInfo.setName(name);
    QCOMPARE(dictionaryInfo.name(), name);
}

void DictionaryInfoTest::testAuthor()
{
    DictionaryInfo dictionaryInfo;
    QString author = "Author";
    dictionaryInfo.setAuthor(author);
    QCOMPARE(dictionaryInfo.author(), author);
}

void DictionaryInfoTest::testDescription()
{
    DictionaryInfo dictionaryInfo;
    QString description = "Description";
    dictionaryInfo.setDescription(description);
    QCOMPARE(dictionaryInfo.description(), description);
}

void DictionaryInfoTest::testWordCount()
{
    DictionaryInfo dictionaryInfo;
    long wordCount = 294967295L;
    dictionaryInfo.setWordCount(wordCount);
    QCOMPARE(dictionaryInfo.wordCount(), wordCount);
}

QTEST_MAIN(DictionaryInfoTest)

#include "dictionaryinfotest.moc"

