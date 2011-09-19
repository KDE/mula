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

#ifndef MULA_PLUGIN_STARDICT_FILE
#define MULA_PLUGIN_STARDICT_FILE

#include <QtCore/QtAlgorithms>
#include <QtCore/QStringList>
#include <QtCore/QDir>

template <typename Function>
void __for_each_file(const QString& dirName, const QString& suffix,
                     const QStringList& orderList, const QStringList& disableList,
                     Function f)
{
    QDir dir(dirName);

    // Going through the subfolders
    foreach (QString entryName, dir.entryList(QDir::Dirs & QDir::NoDotAndDotDot))
    {
        QString absolutePath = dir.absoluteFilePath(entryName);
        __for_each_file(absolutePath, suffix, orderList, disableList, f);
    }
   
    foreach (QString entryName, dir.entryList(QDir::Files & QDir::Drives & QDir::NoDotAndDotDot))
    {
        QString absolutePath = dir.absoluteFilePath(entryName);
        if (absolutePath.endsWith(suffix)
                && qFind(orderList.begin(), orderList.end(), absolutePath) == orderList.end())
        {
            bool enable = qFind(disableList.begin(), disableList.end(), absolutePath) == disableList.end();
            f(absolutePath, enable);
        }
    }
}

template <typename Function>
void for_each_file(const QStringList& dirList, const QString& suffix,
                   const QStringList& orderList, const QStringList& disableList,
                   Function f)
{
    foreach (const QString& string, orderList)
    {
        bool disable = qFind(disableList.begin(), disableList.end(), string) != disableList.end();
        f(string, disable);
    }

    foreach (const QString& dirName, dirList)
        __for_each_file(dirName, suffix, orderList, disableList, f);
}

#endif // MULA_PLUGIN_STARDICT_FILE
