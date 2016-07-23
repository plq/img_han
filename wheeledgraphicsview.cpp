/*
 * This file is part of Arskom EasyCompress Image Compression Tool
 * source code package.
 *
 * Copyright (C) 2016 Arskom Ltd. (http://arskom.com.tr)
 * Please contact Burak ARSLAN <burak.arslan@arskom.com.tr>
 *                Arda OZDEMIR <arda.ozdemir@arskom.com.tr>
 * for more information.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include "wheeledgraphicsview.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QSlider>
#include <QScrollBar>
#include <QWheelEvent>
#include <QGraphicsView>


WheeledGraphicsView::WheeledGraphicsView(QWidget *parent)
        : QGraphicsView(parent)
        , m_slider(nullptr) {

}

// TODO: Reimplement this using signals and slots
void WheeledGraphicsView::wheelEvent(QWheelEvent *event){
    if(event->orientation() == Qt::Horizontal) {
        event->ignore();
    }
    else {
        if(event->delta() < 0){
            int val = m_slider->value();
            val = val - 3;
            m_slider->setValue(val);
        }
        else if(event->delta() > 0){
            int val = m_slider->value();
            val = val + 3;
            m_slider->setValue(val);
        }
    }
}

QSlider *WheeledGraphicsView::slider() const {
    return m_slider;
}

void WheeledGraphicsView::setSlider(QSlider *slider) {
    m_slider = slider;
}
