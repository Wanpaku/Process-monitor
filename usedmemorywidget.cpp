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

UsedMemoryWidget::UsedMemoryWidget(const QString& os_name_, QWidget* parent)
    : os_name(os_name_)
    , QWidget { parent }
{
    set_default_widget_settings();
    starting_used_memory_load();
    set_timer();
}

UsedMemoryWidget::~UsedMemoryWidget()
{
    d_loader->deleteLater();
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

void UsedMemoryWidget::starting_used_memory_load()
{
    // Downloading memory data through the thread
    if (!abort_loading) {
        QString program { "" };
        QStringList args;
        if (os_name.contains("Linux")) {
            program = linux_memory_program;
            args = linux_memory_args;
        } else if (os_name.contains("Windows")) {
            program = windows_shell_program;
            args = windows_memory_args;
        }

        d_loader = new Dataloader(program, args);
        set_connections();

    } else {
        used_memory_timer->stop();
        const QString warn_message { tr(
            "Error with loading data. \n For details check the log file.") };
        qCritical() << warn_message;
        QMessageBox::warning(nullptr, tr("Warning!"), warn_message,
                             QMessageBox::Ok);
    }
}

void UsedMemoryWidget::set_connections()
{
    // Setting signal and slot connections
    connect(this, &UsedMemoryWidget::memory_data_loaded, this,
            &UsedMemoryWidget::repaint_widget);
    connect(d_loader, &Dataloader::sending_output, this,
            &UsedMemoryWidget::load_used_memory_data);
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
            &UsedMemoryWidget::starting_used_memory_load);
    used_memory_timer->start(used_memory_time_update);
}

void UsedMemoryWidget::load_used_memory_data(
    const QStringList& used_memory_list)
{
    // Processing used memory data and emitting signal when the job is done
    if (used_memory_list.isEmpty()) {
        abort_loading = true;
        qWarning() << "Memory data list is empty.";
    } else {
        if (os_name.contains("Linux")) {

            const QStringList total_memory = used_memory_list[1].split(' ');
            total_memory_amount = total_memory[1].toInt();
            current_used_memory_amount = total_memory[2].toInt();
            used_memory_percent_double
                = current_used_memory_amount / total_memory_amount;

        } else if (os_name.contains("Windows")) {

            const int last_row_in_list
                = static_cast<int>(used_memory_list.size() - 1);
            const QStringList total_memory
                = used_memory_list[last_row_in_list].split(' ');
            total_memory_amount = total_memory[0].toInt();
            current_used_memory_amount = static_cast<int>(
                total_memory_amount - total_memory[1].toDouble());

            used_memory_percent_double
                = current_used_memory_amount / total_memory_amount;

            constexpr int kb_in_one_mb { 1024 };
            current_used_memory_amount /= kb_in_one_mb;
        }
        emit memory_data_loaded();
    }
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

    const QString text(QString::number(current_used_memory_amount)
                       + tr(" Mb"));

    const QRect used_mem_rect(
        0, 0, static_cast<int>(width * used_memory_percent_double), height);
    painter->drawRect(used_mem_rect);
    painter->drawText(used_mem_rect, Qt::AlignCenter | Qt::TextWordWrap, text);
}
