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

#ifndef WHEELEDGRAPHICSVIEW_H
#define WHEELEDGRAPHICSVIEW_H

#include <QGraphicsView>

QT_FORWARD_DECLARE_CLASS(QSlider)

class WheeledGraphicsView : public QGraphicsView {
    Q_OBJECT
public:
    explicit WheeledGraphicsView(QWidget *parent = 0);

    QSlider *slider() const;
    void setSlider(QSlider *slider);

protected:
    virtual void wheelEvent(QWheelEvent* event) override;

private:
    QSlider *m_slider;
};

#endif // WHEELEDGRAPHICSVIEW_H
