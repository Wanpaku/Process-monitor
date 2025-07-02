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

#ifndef HARDDISKWIDGET_H
#define HARDDISKWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QStorageInfo>

class HardDiskWidget : public QWidget {
    Q_OBJECT
public:
    explicit HardDiskWidget(QWidget* parent = nullptr);
    void loading_hard_disks_info();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    double total_size { 0 };
    double free_size { 0 };
    double avail_size { 0 };
    double used_size { 0 };
    QTimer* hdd_timer { nullptr };
    const int timer_count { 1000 };

    void load_settings();
    void set_timer();
    void repaint_widget();
    void set_connection() const;

signals:
    void data_loaded();
};

#endif // HARDDISKWIDGET_H
