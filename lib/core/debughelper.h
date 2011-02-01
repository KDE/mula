/******************************************************************************
 * This file is part of the MULA Project
 * Copyright (c) 2011 Laszlo Papp <djszapi@archlinux.us>
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

#ifndef MULA_CORE_DEBUGHELPER_H
#define MULA_CORE_DEBUGHELPER_H

#include "mula_core_export.h"

#include <QtCore/QString>

#define DEBUG_FUNC_NAME MulaCore::DbgHelper dbgHelper(Q_FUNC_INFO);
#define DEBUG_BLOCK MulaCore::DbgHelper dbgHelper;
#define DEBUG_TEXT(X) dbgHelper.addText(X);
#define DEBUG_TEXT2(X, Y) dbgHelper.addText(QString(X).arg(Y));

#define NO_COLOR 1

namespace MulaCore
{
    class MULA_CORE_EXPORT DbgHelper
    {
        public:
            DbgHelper();
            DbgHelper( const QString& t );
            ~DbgHelper();

            void addText( const QString& t );
        private:
            QString txt;
            static int indent;
            static int colorIndex;
            int myColor;
            bool noFunctionName;
    };
}

#endif // MULA_CORE_DEBUGHELPER_H
