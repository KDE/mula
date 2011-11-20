/******************************************************************************
 * This file is part of the Mula project
 * Copyright (C) 2011 Laszlo Papp <lpapp@kde.org>
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

import QtQuick 1.1
import com.nokia.meego 1.0

PageStackWindow {
    id: rootWindow
    property int pageMargin: 16

    // ListPage is what we see when the app starts, it links to
    // the component specific pages
    initialPage: MainPage { }

    // These tools are shared by most sub-pages by assigning the
    // id to a page's tools property
    ToolBarLayout {
        id: commonTools
        visible: false
        ToolIcon {
            iconId: "toolbar-back";
            onClicked: { myMenu.close(); pageStack.pop(); }
        }
        ToolIcon {
            iconId: "toolbar-view-menu";
            onClicked: (myMenu.status == DialogStatus.Closed) ? myMenu.open() : myMenu.close()
        }
    }
}
