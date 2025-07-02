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

#ifndef USEDMEMORYWIDGET_H
#define USEDMEMORYWIDGET_H

#include "dataloader.h"
#include <QPainter>
#include <QTimer>
#include <QWidget>
#include <future>

class UsedMemoryWidget : public QWidget {
    Q_OBJECT
public:
    explicit UsedMemoryWidget(QWidget* parent = nullptr);
    void load_used_memory_data();
    void set_default_widget_settings();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QTimer* used_memory_timer { nullptr };
    const int used_memory_time_update { 1000 };
    int current_used_memory_amount { 0 };
    double total_memory_amount { 0. };
    double used_memory_percent_double { 0 };
    const QString proc_mem_path { "/proc/meminfo" };

    void set_connections() const;
    void set_timer();

signals:
    void memory_data_loaded();

private slots:
    void repaint_widget();
};

#endif // USEDMEMORYWIDGET_H
