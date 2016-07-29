#
# This file is part of Arskom EasyCompress Image Compression Tool
# source code package.
#
# Copyright (C) 2016 Arskom Ltd. (http://arskom.com.tr)
# Please contact Burak ARSLAN <burak.arslan@arskom.com.tr>
#                Arda OZDEMIR <arda.ozdemir@arskom.com.tr>
# for more information.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
#

#-------------------------------------------------
#
# Project created by QtCreator 2016-06-20T09:06:17
#
#-------------------------------------------------

QT       += core gui widgets

lessThan(QT_MAJOR_VERSION, 5): error("requires Qt 5")

# Because of: QAbstractScrollArea::sizeAdjustPolicy
#     cf. http://doc.qt.io/qt-5/qabstractscrollarea.html#sizeAdjustPolicy-prop
lessThan(QT_MINOR_VERSION, 2): error("requires Qt 5.2")

TARGET = img_han
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    wheeledgraphicsview.cpp

HEADERS  += mainwindow.h \
    wheeledgraphicsview.h

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -std=c++11

RESOURCES += resource/resource.qrc
