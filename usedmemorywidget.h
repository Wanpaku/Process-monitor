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

#ifndef USEDMEMORYWIDGET_H
#define USEDMEMORYWIDGET_H

#include "dataloader.h"
#include <QPainter>
#include <QTimer>
#include <QWidget>

class UsedMemoryWidget : public QWidget {
    Q_OBJECT
public:
    explicit UsedMemoryWidget(const QString& os_name_,
                              QWidget* parent = nullptr);
    ~UsedMemoryWidget() override;
    UsedMemoryWidget(const UsedMemoryWidget& src) = delete;
    auto operator=(const UsedMemoryWidget& rhs) -> UsedMemoryWidget& = delete;
    UsedMemoryWidget(const UsedMemoryWidget&& src) = delete;
    auto operator=(const UsedMemoryWidget&& rhs) -> UsedMemoryWidget& = delete;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Dataloader* u_dataloader { nullptr };
    QTimer* used_memory_timer { nullptr };
    const int used_memory_time_update { 60000 };
    int current_used_memory_amount { 0 };
    double total_memory_amount { 0. };
    double used_memory_percent_double { 0 };
    QString os_name;
    bool abort_loading { false };
    const QString linux_memory_program { "free" };
    const QStringList linux_memory_args { "--mega" };
    const QString windows_shell_program {
        R"("C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe")"
    };
    const QStringList windows_memory_args {
        "Get-CIMInstance", "Win32_OperatingSystem",   "|",
        "Select-Object",   "TotalVisibleMemorySize,", "FreePhysicalMemory"
    };
    Dataloader* d_loader { nullptr };

    void set_connections();
    void set_timer();
    void load_used_memory_data(const QStringList& used_memory_list);
    void starting_used_memory_load();
    void set_default_widget_settings();

signals:
    void memory_data_loaded();

private slots:
    void repaint_widget();
};

#endif // USEDMEMORYWIDGET_H
