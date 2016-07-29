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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsPixmapItem>
#include <QMutex>

#include <string>
#include <mutex>

QT_FORWARD_DECLARE_CLASS(QGraphicsScene)

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void showEvent(QShowEvent *e) override;

private slots:
    void on_sld_quality_valueChanged(int value);
    void on_sld_quality_sliderPressed();
    void on_sld_quality_sliderReleased();

    void on_sld_scale_valueChanged(int value);
    void on_sld_scale_sliderPressed();
    void on_sld_scale_sliderReleased();

    void on_btn_zoomin_clicked();
    void on_btn_zoomout_clicked();
    void on_sld_zoom_valueChanged(int value);

    void on_btn_rotate_right_clicked();
    void on_btn_rotate_left_clicked();

    void show_pixmap();  // runs in gui thread
    void reprocess_image_smooth(int scale, int);  // runs in gui thread
    void reprocess_image_impl(int scale, int quality);  // runs in concurrent thread

    void reprocess_image_fast(int scale, int);  // runs in gui thread
    void show_pixmap_fast();  // runs in gui thread

    void on_action_open_triggered();
    void on_action_save_as_triggered();
    void on_action_exit_triggered();

private:
    void reprocess_image();
    void reprocess_image(int scale, int quality);
    bool rescale_image(int);
    bool requality_image(int);

    void change_size();

    QByteArray m_out_data;
    QString m_image_path;
    std::mutex m_mutex;
    QPixmap m_pixmap;

    qint16 m_current_scale;
    qint64 m_new_size;

    bool m_fast;

    int m_new_w;
    int m_new_h;
    int m_sld_zoom_value;
    double m_zoom_factor;

    QGraphicsScene *m_scene;
    QMovie *m_loading_animation;
    QImage m_orig_image;
    qint64 m_orig_size;
    bool m_processing;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
