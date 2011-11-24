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

#include "stardictdictionaryinfotest.h"

#include <plugins/stardict/stardictdictionaryinfo.h>

#include <QtTest/QtTest>

using namespace MulaPluginStarDict;

StarDictDictionaryInfoTest::StarDictDictionaryInfoTest()
{

}

StarDictDictionaryInfoTest::~StarDictDictionaryInfoTest()
{
}

void StarDictDictionaryInfoTest::testIfoFilePath()
{
    StarDictDictionaryInfo starDictDictionaryInfo;
    QString ifoFilePath = "/ifo/file/path/foobar.ifo";
    starDictDictionaryInfo.setIfoFilePath(ifoFilePath);
    QCOMPARE(starDictDictionaryInfo.ifoFilePath(), ifoFilePath);
}

void StarDictDictionaryInfoTest::testWordCount()
{
    StarDictDictionaryInfo starDictDictionaryInfo;
    quint32 wordCount = 100;
    starDictDictionaryInfo.setWordCount(wordCount);
    QCOMPARE(starDictDictionaryInfo.wordCount(), wordCount);
}

void StarDictDictionaryInfoTest::testBookName()
{
    StarDictDictionaryInfo starDictDictionaryInfo;
    QString bookName = "BookName";
    starDictDictionaryInfo.setBookName(bookName);
    QCOMPARE(starDictDictionaryInfo.bookName(), bookName);
}

void StarDictDictionaryInfoTest::testAuthor()
{
    StarDictDictionaryInfo starDictDictionaryInfo;
    QString author = "Author";
    starDictDictionaryInfo.setAuthor(author);
    QCOMPARE(starDictDictionaryInfo.author(), author);
}

void StarDictDictionaryInfoTest::testEmail()
{
    StarDictDictionaryInfo starDictDictionaryInfo;
    QString email = "email@email.com";
    starDictDictionaryInfo.setEmail(email);
    QCOMPARE(starDictDictionaryInfo.email(), email);
}

void StarDictDictionaryInfoTest::testWebsite()
{
    StarDictDictionaryInfo starDictDictionaryInfo;
    QString website = "http://website.org";
    starDictDictionaryInfo.setWebsite(website);
    QCOMPARE(starDictDictionaryInfo.website(), website);
}

void StarDictDictionaryInfoTest::testDateTime()
{
    StarDictDictionaryInfo starDictDictionaryInfo;
    QString dateTime = "11/022005 15:52:23";
    starDictDictionaryInfo.setDateTime(dateTime);
    QCOMPARE(starDictDictionaryInfo.dateTime(), dateTime);
}

void StarDictDictionaryInfoTest::testDescription()
{
    StarDictDictionaryInfo starDictDictionaryInfo;
    QString description = "Description";
    starDictDictionaryInfo.setDescription(description);
    QCOMPARE(starDictDictionaryInfo.description(), description);
}

void StarDictDictionaryInfoTest::testIndexFileSize()
{
    StarDictDictionaryInfo starDictDictionaryInfo;
    quint32 indexFileSize = 1024;
    starDictDictionaryInfo.setIndexFileSize(indexFileSize);
    QCOMPARE(starDictDictionaryInfo.indexFileSize(), indexFileSize);
}

void StarDictDictionaryInfoTest::testIndexOffsetBits()
{
    StarDictDictionaryInfo starDictDictionaryInfo;
    quint32 indexOffsetBits = 100;
    starDictDictionaryInfo.setIndexOffsetBits(indexOffsetBits);
    QCOMPARE(starDictDictionaryInfo.indexOffsetBits(), indexOffsetBits);
}

void StarDictDictionaryInfoTest::testSameTypeSequence()
{
    StarDictDictionaryInfo starDictDictionaryInfo;
    QString sameTypeSequence = "sameTypeSequence";
    starDictDictionaryInfo.setSameTypeSequence(sameTypeSequence);
    QCOMPARE(starDictDictionaryInfo.sameTypeSequence(), sameTypeSequence);
}

QTEST_MAIN(StarDictDictionaryInfoTest)

#include "stardictdictionaryinfotest.moc"

