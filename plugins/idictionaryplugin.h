/******************************************************************************
 * This file is part of the MULA project
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

#ifndef MULA_PLUGIN_IDICTPLUGIN_H
#define MULA_PLUGIN_IDICTPLUGIN_H

#include <QtPlugin>
#include <QCore/QStringList>
#include <QCore/QDir>
#include <QCore/QCoreApplication>
#include <QCore/QVariant>

namespace MULAPlugin
{

    /**
     * This class represents information about dictionary.
     */
    class DictionaryData
    {
        public:
            /**
             * Construct an empty DictionaryData object.
             */
            DictionaryData()
                : m_wordsCount(-1L)
            {
            }

            /**
             * Construct a DictionaryData object from the desired data.
             * @param plugin A plugin name
             * @param name A dictionary name
             * @param author A dictionary author
             * @param desription A dictionary description
             * @param wordsCount A count of words that available in dictionary
             */
            DictionaryData(const QString &plugin, const QString &name, const QString &author = QString(), 
                            const QString &description = QString(), long wordsCount = -1L);

            const QString &plugin() const;

            const QString &name() const;

            const QString &author() const;

            const QString &description() const;

            long wordsCount() const;

            void setPlugin(const QString &plugin);

            void setName(const QString &name);

            void setAuthor(const QString &author);

            void setDescription(const QString &description);

            void setWordsCount(long wordsCount);

        private:
            QString m_plugin;
            QString m_name;
            QString m_author;
            QString m_description;
            long m_wordsCount;
    };

    /**
     * This class represent a translation.
     */
    class Translation
    {
        public:

            /**
             * Construct an empty translation.
             */
            Translation()
            {
            }

            /**
             * Construct a translation from data.
             * @param title A translation title
             * @param dictionaryName A full dictionary name
             * @param translation A translation
             */
            Translation(const QString &title, const QString &dictionaryName,
                        const QString &translation);

            /**
             * Return the translation title.
             */
            const QString &title() const;

            /**
             * Return the dictionary name.
             */
            const QString &dictName() const;

            /**
             * Return the translation.
             */
            const QString &translation() const;

            /**
             * Set a translation title.
             */
            void setTitle(const QString &title);

            /**
             * Set a dictionary name.
             */
            void setDictName(const QString &dictName);

            /**
             * Set a translation.
             */
            void setTranslation(const QString &translation);

        private:
            QString m_title;
            QString m_dictName;
            QString m_translation;
    };

    /**
     * This is a base, interface class for all the dictionary plugins classes.
     */
    class IDictionaryPlugin
    {
        public:

            /**
             * This enum describes the features of a dictionary plugin.
             */
            enum Feature
            {
                /**
                 * No features.
                 */
                None          = 0x00,

                /**
                 * Dictionary plugin can search for similar words using
                 * fuzzy algoritms.
                 */
                SearchSimilar = 0x01,

                /**
                 * Dictionary plugin has a settings dialog.
                 */
                SettingsDialog = 0x02,
            };

            Q_DECLARE_FLAGS(Features, Feature)

            /**
             * Destructor.
             */
            virtual ~IDictonaryPlugin();

            /**
             * Return the plugin name.
             */
            virtual QString name() const = 0;

            /**
             * Return the plugin version.
             */
            virtual QString version() const = 0;

            /**
             * Return the plugin description.
             */
            virtual QString description() const = 0;

            /**
             * Return the plugin authors.
             */
            virtual QStringList authors() const = 0;

            /**
             * Return a features supported by dictionary plugin.
             */
            virtual Features features() const;

            /**
             * Return a list of available dictionaries.
             */
            virtual QStringList availableDictionaries() const = 0;

            /**
             * Return a list of loaded dictionaries.
             */
            virtual QStringList loadedDictionaries() const = 0;

            /**
             * Set a list of loaded dictionaries.
             */
            virtual void setLoadedDictionaries(const QStringList &loadedDictionaries) = 0;

            /**
             * Return true if translation exists in dictionary,
             * otherwise returns false.
             */
            virtual bool isTranslatable(const QString &dictionary, const QString &word) = 0;

            /**
             * Return translation for word from dictionary. If word not found
             * returns empty string.
             */
            virtual Translation translate(const QString &dictionary, const QString &word) = 0;

            /**
             * Return a list of similar to "word" words from all loaded dictionaries.
             * Works only if SearchSimilar feature is enabled.
             */
            virtual QStringList findSimilarWords(const QString &dictionary, const QString &word);

            /**
             * Return a required resource. Scheme of URLs:
             *   plugin://plugin_name/...
             */
            virtual QVariant resource(int type, const QUrl &name);

            /**
             * Return information about the dictionary. The dictionary may be not loaded
             * but available.
             */
            virtual DictionaryInfo dictInfo(const QString &dictionary) = 0;

            /**
             * Run a settings dialog and return QDialog::DialogCode.
             */
            virtual int execSettingsDialog(QWidget *parent = 0);

        protected:
            /**
             * Return a directory that contains the data of the plugin.
             */
            QString workPath() const;
    };

    Q_DECLARE_OPERATORS_FOR_FLAGS(IDictPlugin::Features)

}

Q_DECLARE_INTERFACE(MULAPlugin::IDictionaryPlugin, "org.mula.IDictionaryPlugin/1.0")

#endif // MULA_PLUGIN_IDICTIONARYPLUGIN_H

