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

#include "settingsdialog.h"

#include "stardict.h"

#include <QFileDialog>

using namespace MulaPluginStarDict;

SettingsDialog::SettingsDialog(StarDict *plugin, QWidget *parent)
    : QDialog(parent)
    , m_plugin(plugin)
{
    setupUi(this);

    reformatListsBox->setChecked(m_plugin->m_reformatLists);
    expandAbbreviationsBox->setChecked(m_plugin->m_expandAbbreviations);
    dictionaryDirList->addItems(m_plugin->m_dictDirs);

    connect(this, SIGNAL(accepted()), SLOT(apply()));
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::addDictionaryDirButton()
{
    QString directoryName = QFileDialog::getExistingDirectory(this, tr("Select dictionaries directory"));
    if (!directoryName.isEmpty())
    {
        dictionaryDirList->addItem(directoryName);
    }
}

void SettingsDialog::removeDictionaryDirButton()
{
    delete dictionaryDirList->takeItem(dictionaryDirList->currentRow());
}

void SettingsDialog::moveUpDictionaryDirButton()
{
    if (dictionaryDirList->currentRow() > 0)
    {
        dictionaryDirList->insertItem(dictionaryDirList->currentRow(),
                                 dictionaryDirList->takeItem(dictionaryDirList->currentRow()));
        dictionaryDirList->setCurrentRow(dictionaryDirList->currentRow() - 1);
    }
}

void SettingsDialog::moveDownDictDirButton()
{
    if (dictionaryDirList->currentRow() < dictionaryDirList->count() - 1)
    dictionaryDirList->insertItem(dictionaryDirList->currentRow(),
                             dictionaryDirList->takeItem(dictionaryDirList->currentRow() + 1));
}

void SettingsDialog::apply()
{
    m_plugin->m_reformatLists = reformatListsBox->isChecked();
    m_plugin->m_expandAbbreviations = expandAbbreviationsBox->isChecked();
    m_plugin->m_dictionaryDirectories.clear();
    for (int i = 0; i < dictionaryDirList->count(); ++i)
        m_plugin->m_dictionaryDirectories << dictionaryDirList->item(i)->text();
}

