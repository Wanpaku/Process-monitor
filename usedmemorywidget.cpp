/*Copyright (C) 2024  Teg Miles
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
along with Movar. If not, see <https://www.gnu.org/licenses/>.*/

#include "usedmemorywidget.h"

UsedMemoryWidget::UsedMemoryWidget(QWidget* parent)
    : QWidget { parent }
{
    set_default_widget_settings();
    set_connections();
    set_timer();
}

void UsedMemoryWidget::load_used_memory_data()
{
    std::future<QStringList> future = std::async(std::launch::async, [this]() {
        const QScopedPointer<Dataloader> d_loader(
            new Dataloader(proc_mem_path));
        auto data_list = d_loader->loading_linux_proc_mem_data();
        return data_list;
    });
    QStringList used_memory_list;
    try {
        used_memory_list = future.get();
    } catch (std::exception& excep) {
        used_memory_timer->stop();
        const QString warn_message {
            tr("Error happens during memory data list loading: ")
            + excep.what()
        };
        qCritical() << warn_message;
    }

    constexpr int min_mem_file_categories { 2 };
    if (used_memory_list.size() < min_mem_file_categories) {
        used_memory_timer->stop();
        const QString warn_message { tr(
            "Error! Not enough data in the memory list") };
        qCritical() << warn_message;
        QMessageBox::warning(nullptr, tr("Warning!"), warn_message,
                             QMessageBox::Ok);
    } else {
        total_memory_amount = used_memory_list[0].toInt();
        const double free_memory_amount = used_memory_list[2].toDouble();
        constexpr int kb_in_one_mb { 1024 };
        current_used_memory_amount = total_memory_amount - free_memory_amount;
        used_memory_percent_double
            = current_used_memory_amount / total_memory_amount;

        current_used_memory_amount /= kb_in_one_mb;
        total_memory_amount /= kb_in_one_mb;

        emit memory_data_loaded();
    }
}

void UsedMemoryWidget::set_default_widget_settings()
{
    // Load default widget settings

    QPalette pal = palette();
    const QBrush background_brush(Qt::darkMagenta, Qt::CrossPattern);
    pal.setBrush(QPalette::Window, background_brush);
    setAutoFillBackground(true);
    setPalette(pal);
}

void UsedMemoryWidget::set_connections() const
{
    // Setting signal and slot connections
    connect(this, &UsedMemoryWidget::memory_data_loaded, this,
            &UsedMemoryWidget::repaint_widget);
}

void UsedMemoryWidget::repaint_widget()
{
    // Repainting widget
    repaint();
}

void UsedMemoryWidget::set_timer()
{
    // Setting timer for used memory data loading
    used_memory_timer = new QTimer(this);
    connect(used_memory_timer, &QTimer::timeout, this,
            &UsedMemoryWidget::load_used_memory_data);
    used_memory_timer->start(used_memory_time_update);
}

void UsedMemoryWidget::paintEvent(QPaintEvent* event)
{
    // Creating qpainter parameters and draw the used memory bar
    const QScopedPointer<QPainter> painter(new QPainter(this));
    QPen pen(Qt::black);
    pen.setWidth(2);
    painter->setPen(pen);
    const QBrush brush(Qt::green);
    painter->setBrush(brush);
    const int width = this->width();
    const int height = this->height();
    QFont default_font = this->font();
    default_font.setBold(true);
    painter->setFont(default_font);

    const QString total_mem_text = tr("Total memory: ")
        + QString::number(total_memory_amount) + tr(" Mb");

    const QString cur_used_mem_text(QString::number(current_used_memory_amount)
                                    + tr(" Mb"));

    const QRect used_mem_rect(
        0, 0, static_cast<int>(width * used_memory_percent_double), height);
    painter->drawRect(used_mem_rect);
    painter->drawText(used_mem_rect, Qt::AlignCenter | Qt::TextWordWrap,
                      cur_used_mem_text);

    QPen pen_total_mem(Qt::cyan);
    pen_total_mem.setWidth(2);
    painter->setPen(pen_total_mem);
    painter->drawText(width - width / 4, height - height / 2.5,
                      total_mem_text);
}
