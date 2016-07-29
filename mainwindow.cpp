/******************************************* 100 cols. ********************************************/

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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "wheeledgraphicsview.h"

#include <QtConcurrent/QtConcurrentRun>

#include <math.h>

#include <mutex>
#include <string>
#include <iostream>

#include <QMovie>
#include <QEvent>
#include <QImage>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include <QMatrix>
#include <QBuffer>
#include <QSlider>
#include <QScrollBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QImageReader>
#include <QImageWriter>
#include <QGraphicsView>
#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QStandardPaths>


static QString LBL_NEW_SIZE_TEXT = QLatin1String(
    "<html>"
        "<head/>"
        "<body>"
            "<p>"
                "<span style=\"font-weight:600;\">%1</span> KB, "
                "<span style=\"font-weight:600; color:%2;\">%3%4</span> %5"
            "</p>"
        "</body>"
    "</html>");


MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , m_current_scale(0)
        , m_fast(false)
        , m_orig_size(0)
        , m_processing(false)
        , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->lyt_transform->setAlignment(ui->sld_zoom, Qt::AlignHCenter);

    ui->graphicsView->setSlider(ui->sld_zoom);

    connect(ui->btn_open, SIGNAL(clicked(bool)), ui->action_open, SIGNAL(triggered(bool)));
    connect(ui->btn_save, SIGNAL(clicked(bool)), ui->action_save_as, SIGNAL(triggered(bool)));

    m_scene = new QGraphicsScene(this);

    ui->sld_quality->setDisabled(true);
    ui->sld_scale->setDisabled(true);
    ui->sld_zoom->setDisabled(true);
    ui->btn_save->setDisabled(true);
    ui->btn_rotate_left->setDisabled(true);
    ui->btn_rotate_right->setDisabled(true);

    this->setWindowTitle("Arskom EasyCompress");

    ui->lbl_new_size->clear();
    ui->lbl_new_dimensions->clear();

    ui->lbl_orig_size->clear();
    ui->lbl_orig_dimensions->clear();

    m_scene = new QGraphicsScene(this);
    m_loading_animation = new QMovie(":/images/loading.gif");
}

MainWindow::~MainWindow() {
    delete ui;
    delete m_scene;
    delete m_loading_animation;
}

void MainWindow::showEvent(QShowEvent *e) {
    QMainWindow::showEvent(e);

    const auto &desktop_rect = QApplication::desktop()->screen()->rect();

    // TODO: Store last window state and position
    {
        this->resize(desktop_rect.width() * .75, desktop_rect.height() * .75);
        this->move(desktop_rect.center() - this->rect().center());\

        auto toolbox_minw = ui->toolBox->minimumWidth();
        ui->splitter->setSizes(QList<int>() << toolbox_minw << width() - toolbox_minw);
    }

    auto readable_formats = QImageReader::supportedImageFormats();
    auto writeable_formats = QImageWriter::supportedImageFormats();
    if (! writeable_formats.contains("webp")) {
        auto readable_formats_str = QLatin1String(readable_formats.join(", "));
        auto writeable_formats_str = QLatin1String(writeable_formats.join(", "));

        QMessageBox::critical(this, tr("WebP Not Supported"),
                tr("Please install WebP Support for Qt before running this program.\n\n"
                   "Readable Formats: (%1)\n"
                   "Writeable Formats: (%2)\n").arg(readable_formats_str, writeable_formats_str));

        // must call quit() after exec() in main
        QTimer::singleShot(0, qApp, SLOT(quit()));
    }
}

void MainWindow::on_action_open_triggered() {
    const auto &desktop_abs = QStandardPaths::standardLocations(
                QStandardPaths::DesktopLocation);

    m_image_path = QFileDialog::getOpenFileName(
            this, tr("Open File"), desktop_abs.first(),
            tr("JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp);;WEBP (*.webp)"));

    if (m_image_path.isEmpty()) {
        qDebug() << "Empty string returned";
        return;
    }

    const auto &file_info = QFileInfo(m_image_path);

    if (! file_info.isReadable()) {
        qDebug() << "File not readable";
        QMessageBox::critical(this, tr("Critical Error"), tr("File is not readable"));
        return;
    }

    m_orig_image.load(m_image_path);

    m_orig_size = file_info.size();

    m_new_w = m_orig_image.width();
    m_new_h = m_orig_image.height();

    m_current_scale = 100;
    m_new_size = m_orig_size;

    m_pixmap = QPixmap::fromImage(m_orig_image);
    show_pixmap();

    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    ui->statusBar->showMessage(tr("%1 was opened successfully (%2KB).")
                                 .arg(file_info.baseName()).arg(m_orig_size / 1024.0, 0, 'f', 1));

    ui->sld_quality->setDisabled(false);
    ui->sld_scale->setDisabled(false);
    ui->sld_zoom->setDisabled(false);
    ui->btn_save->setDisabled(false);
    ui->btn_rotate_left->setDisabled(false);
    ui->btn_rotate_right->setDisabled(false);

    reprocess_image();
}

void MainWindow::reprocess_image() {
    reprocess_image(ui->sld_scale->value(), ui->sld_quality->value());
}

void MainWindow::on_action_save_as_triggered() {
    QString default_path;
    {
        const auto &desktop_abs = QStandardPaths::standardLocations(
                    QStandardPaths::DesktopLocation);

        if (desktop_abs.count() == 0) {
            default_path = QDir().absolutePath();
        }
        else {
            default_path = desktop_abs.first();
        }
    }

    QString target_path = m_image_path;
    QString ext = m_image_path.split(".").last().toLower();

    if (QImageReader::supportedImageFormats().contains(ext.toUtf8())) {
        target_path = target_path.mid(0, target_path.length() - ext.length() - 1); // -1 is for "."
    }
    target_path = QStringLiteral("%1.webp").arg(target_path);

    QString image_path = QFileDialog::getSaveFileName(
            this, tr("Save File"),
            QDir(default_path).absoluteFilePath(target_path),
            tr("WEBP (*.webp)"));

    if (image_path.isEmpty()) {
        return;
    }

    QFile ostr(image_path);
    if (ostr.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        ostr.write(m_out_data);
    }
    else {
        QMessageBox::critical(this, tr("File could not be saved"),
                              tr("%1 could not be opened for writing").arg(image_path));
    }
}

void MainWindow::on_action_exit_triggered(){
    close();
}

void MainWindow::show_pixmap_fast() {
    auto orig_w = m_orig_image.width();
    auto orig_h = m_orig_image.height();

    // for showing to the user
    m_pixmap = m_pixmap.scaled(orig_w, orig_h, Qt::KeepAspectRatio, Qt::FastTransformation);

    m_scene->clear();
    m_scene->addPixmap(m_pixmap);
    m_scene->setSceneRect(m_pixmap.rect());
}

void MainWindow::show_pixmap() {
    std::lock_guard<std::mutex> guard(m_mutex);

    auto orig_w = m_orig_image.width();
    auto orig_h = m_orig_image.height();

    // for showing to the user
    m_pixmap = m_pixmap.scaled(orig_w, orig_h, Qt::KeepAspectRatio, Qt::FastTransformation);

    m_scene->clear();
    m_scene->addPixmap(m_pixmap);
    m_scene->setSceneRect(m_pixmap.rect());

    /* set original image information */
    ui->lbl_orig_size->setTextFormat(Qt::RichText);
    ui->lbl_orig_size->setText(
            QStringLiteral("<b>%1</b> KB")
                .arg(m_orig_size / 1024.0, 0, 'f', 1));


    ui->lbl_orig_dimensions->setTextFormat(Qt::RichText);
    ui->lbl_orig_dimensions->setText(
            QStringLiteral("%1x%2 (<b>%3</b> megapixels)")
                .arg(m_orig_image.width())
                .arg(m_orig_image.height())
                .arg(orig_w * orig_h / 1e6, 0, 'f', 1));

    /* set slider information */
    ui->lbl_scale->setText(QStringLiteral("%1").arg(m_current_scale));
    ui->lbl_quality->setText(QString::number(ui->sld_quality->value()));

    /* set slider information */
    ui->lbl_new_dimensions->setTextFormat(Qt::RichText);
    ui->lbl_new_dimensions->setText(
            QStringLiteral("%1x%2 (<b>%3</b> megapixels)")
                .arg(m_new_w)
                .arg(m_new_h)
                .arg(m_new_w * m_new_h / 1e6, 0, 'f', 1));

    /* set compressed image information */
    int comp_ratio = 1000 * m_new_size / m_orig_size;
    int weight_loss = 1000 - comp_ratio;

    ui->lbl_new_size->setTextFormat(Qt::RichText);
    QString lbl_new_size_text = LBL_NEW_SIZE_TEXT
            .arg(m_new_size / 1024.0, 0, 'f', 1);

    if (comp_ratio == 1000) {
        ui->lbl_new_size->setText(lbl_new_size_text.arg("black")
                            .arg("No").arg("").arg(tr("compression")));
    }
    else if(comp_ratio > 1000) {
        ui->lbl_new_size->setText(lbl_new_size_text.arg("red")
                            .arg(weight_loss / 10.0, 0, 'f', 1).arg("%").arg(tr("increase")));
    }
    else if(comp_ratio < 1000) {
        ui->lbl_new_size->setText(lbl_new_size_text.arg("green")
                            .arg(weight_loss / 10.0, 0, 'f', 1).arg("%").arg(tr("compression")));
    }

    m_processing = false;
    ui->lbl_busy->clear();
}

void MainWindow::reprocess_image_fast(int scale, int) {
    m_new_w = (m_orig_image.width() * scale) / 100;
    m_new_h = (m_orig_image.height() * scale)/100;

    m_pixmap = QPixmap::fromImage(
                m_orig_image.scaled(m_new_w, m_new_h, Qt::KeepAspectRatio, Qt::FastTransformation));

    show_pixmap_fast();
}

void MainWindow::reprocess_image(int scale, int quality) {
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        if (m_processing) {
            return;
        }
    }

    if (m_fast) {
        reprocess_image_fast(scale, quality);
    }
    else {
        reprocess_image_smooth(scale, quality);
    }
}

void MainWindow::reprocess_image_smooth(int scale, int quality) {
    ui->lbl_new_size->setText(tr("Compressing..."));
    ui->lbl_busy->setMovie(m_loading_animation);
    ui->lbl_busy->show();
    m_loading_animation->start();

    QtConcurrent::run(this, &MainWindow::reprocess_image_impl, scale, quality);
}

void MainWindow::reprocess_image_impl(int scale, int quality) {
    if (! rescale_image(scale)) {
        return;
    }

    if (! requality_image(quality)) {
        return;
    }

    QMetaObject::invokeMethod(this, "show_pixmap");
}

bool MainWindow::rescale_image(int scale) {
    double w = m_orig_image.width();
    double h = m_orig_image.height();

    m_new_w = static_cast<int>(std::round((w * scale) / 100));
    m_new_h = static_cast<int>(std::round((h * scale) / 100));

    m_current_scale = scale;

    m_pixmap = QPixmap::fromImage(m_orig_image.scaled(m_new_w, m_new_h,
                                                    Qt::KeepAspectRatio, Qt::SmoothTransformation));

    return true;
}

bool MainWindow::requality_image(int quality) {
    m_out_data.clear();

    QBuffer buffer(&m_out_data);
    buffer.open(QIODevice::WriteOnly);
    m_pixmap.save(&buffer, "WEBP", quality);

    m_new_size = buffer.size();
    qDebug() << "aaa" << quality;
    qDebug() << "image size(b) = " << buffer.size();

    QImage image;
    image.loadFromData(m_out_data);
    m_pixmap = QPixmap::fromImage(image);
    if (m_pixmap.isNull()) {
        QMessageBox::critical(this, tr("Critical Error"), tr("Image recompression error!"));
        return false;
    }

    return true;
}

void MainWindow::on_sld_quality_valueChanged(int value) {
    reprocess_image(ui->sld_scale->value(), value);
}

void MainWindow::on_sld_quality_sliderPressed() {
    m_fast = true;
}

void MainWindow::on_sld_quality_sliderReleased() {
    m_fast = false;
    reprocess_image();
}

void MainWindow::on_sld_scale_valueChanged(int scale) {
    reprocess_image(scale, ui->sld_quality->value());
}

void MainWindow::on_sld_scale_sliderPressed() {
    m_fast = true;
}

void MainWindow::on_sld_scale_sliderReleased() {
    m_fast = false;
    reprocess_image();
}

void MainWindow::on_btn_zoomin_clicked() {
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    int val = ui->sld_zoom->value();
    val = val + 3;
    ui->sld_zoom->setValue(val);
}

void MainWindow::on_btn_zoomout_clicked() {
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    int val = ui->sld_zoom->value();
    val = val - 3;
    ui->sld_zoom->setValue(val);
}

void MainWindow::on_btn_rotate_right_clicked(){
    ui->graphicsView->rotate(90);
}

void MainWindow::on_btn_rotate_left_clicked(){
    ui->graphicsView->rotate(-90);
}

void MainWindow::on_sld_zoom_valueChanged(int value){
    m_sld_zoom_value = value;

    m_zoom_factor = pow(10, ((value - 100) / 100.0));
    qDebug() << "zoom factor = " << m_zoom_factor << "// zoom slider value = " << value;

    QMatrix matrix;
    matrix.scale(m_zoom_factor, m_zoom_factor);

    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setMatrix(matrix);
}
