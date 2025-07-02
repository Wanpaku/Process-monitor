/*Copyright (C) 2025  Teg Miles
 This file is part of Process monitor.

Movar is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

Movar is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Process monitor. If not, see <https://www.gnu.org/licenses/>.*/

#include "harddiskwidget.h"

HardDiskWidget::HardDiskWidget(QWidget* parent)
    : QWidget { parent }
{
    load_settings();
    set_timer();
    set_connection();
}

void HardDiskWidget::loading_hard_disks_info()
{
    const QScopedPointer<QStorageInfo> storage(
        new QStorageInfo(QStorageInfo::root()));
    constexpr double gigabytes_den { 1024 * 1024 * 1024 };
    storage->refresh();
    total_size = static_cast<double>(storage->bytesTotal()) / gigabytes_den;
    free_size = static_cast<double>(storage->bytesFree()) / gigabytes_den;
    avail_size
        = static_cast<double>(storage->bytesAvailable()) / gigabytes_den;
    used_size = total_size - free_size;
    // qDebug() << used_size;
    emit data_loaded();
}

void HardDiskWidget::load_settings()
{
    // Load default widget settings
    QPalette pal = palette();
    const QBrush background_brush(Qt::darkMagenta, Qt::CrossPattern);
    pal.setBrush(QPalette::Window, background_brush);
    setAutoFillBackground(true);
    setPalette(pal);
}

void HardDiskWidget::set_timer()
{
    hdd_timer = new QTimer(this);
    connect(hdd_timer, &QTimer::timeout, this,
            &HardDiskWidget::loading_hard_disks_info);
    hdd_timer->start(timer_count);
}

void HardDiskWidget::repaint_widget()
{
    repaint();
}

void HardDiskWidget::set_connection() const
{
    connect(this, &HardDiskWidget::data_loaded, this,
            &HardDiskWidget::repaint_widget);
}

void HardDiskWidget::paintEvent(QPaintEvent* event)
{
    const QScopedPointer<QPainter> painter(new QPainter(this));
    painter->setRenderHint(QPainter::Antialiasing);
    const QColor light_bluish(133, 193, 233);
    QPen pen(light_bluish);
    pen.setWidth(2);
    painter->setPen(pen);
    const QColor dark_bluish(27, 79, 114);
    const QBrush brush(dark_bluish);
    painter->setBrush(brush);
    const int width = this->width();
    const int height = this->height();

    QFont default_font = this->font();
    default_font.setBold(true);
    painter->setFont(default_font);

    const QString total_size_text
        = tr("Total disk size: ") + QString::number(total_size) + tr(" Gb");

    const QString cur_used_size_text(tr("Used: ") + QString::number(used_size)
                                     + tr(" Gb"));

    const QRect used_mem_rect(
        0, 0, static_cast<int>(width * (used_size / total_size)), height);
    painter->drawRect(used_mem_rect);
    painter->drawText(used_mem_rect, Qt::AlignCenter | Qt::TextWordWrap,
                      cur_used_size_text);

    const QColor yellos(255, 243, 51);
    QPen pen_total_mem(yellos);
    pen_total_mem.setWidth(2);
    painter->setPen(pen_total_mem);
    const int text_pos_x = width - (width / 4);
    const int text_pos_y = static_cast<int>(height - (height / 2.5));
    painter->drawText(text_pos_x, text_pos_y, total_size_text);
}
