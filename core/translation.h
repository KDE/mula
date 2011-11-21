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

#ifndef MULA_CORE_TRANSLATION_H
#define MULA_CORE_TRANSLATION_H

#include "mula_core_export.h"

class QString;

namespace MulaCore
{
    /**
     * This class represent a translation.
     */
    class MULA_CORE_EXPORT Translation
    {
        public:

            /**
             * Construct an empty translation.
             */
            Translation();

            /**
             * Construct a translation from data
             *
             * @param title A translation title
             * @param dictionaryName A full dictionary name
             * @param translation A translation
             */
            Translation(const QString &title, const QString &dictionaryName,
                        const QString &translation);

            virtual ~Translation();

            /**
             * Return the translation title
             *
             * @return The translation title
             * @see setTitle
             */
            QString title() const;

            /**
             * Return the dictionary name
             *
             * @return The dictionary name
             * @see setDictionaryName
             */
            QString dictionaryName() const;

            /**
             * Return the translation string
             *
             * @return The translation string
             * @see setTranslation
             */
            QString translation() const;

            /**
             * Set the translation title
             *
             * @param title The translation title
             * @see title
             */
            void setTitle(const QString &title);

            /**
             * Set the dictionary name
             *
             * @param dictionaryName The dictionary name
             * @see dictionaryName
             */
            void setDictionaryName(const QString &dictionaryName);

            /**
             * Set the translation string
             *
             * @param translation The translation string
             * @see translation
             */
            void setTranslation(const QString &translation);

        private:
            class Private;
            Private *const d;
    };
}

#endif // MULA_CORE_TRANSLATION_H

