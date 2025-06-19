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

#ifndef DATALOADER_H
#define DATALOADER_H

#include <QDebug>
#include <QMessageBox>
#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QStringList>
#include <QSysInfo>
#include <QTranslator>
#include <QFile>
#include <QTextStream>
#include <QDirListing>
#include <QFileInfo>
#include <queue>

using info_tuple
    = std::tuple<int, QString, QString, QString, QString, QString, QString>;

class Dataloader : public QObject {
    Q_OBJECT
public:
    explicit Dataloader(const QString& proc_path_, QWidget* parent = nullptr);
    explicit Dataloader(QWidget* parent = nullptr);
    auto loading_linux_proc_cpu_data() -> QStringList;
    auto loading_linux_proc_mem_data() -> QStringList;
    auto loading_linux_proc_processes_data()
        -> std::priority_queue<info_tuple, std::vector<info_tuple>>;

private:
    QStringList data_list;
    QString proc_path;

    auto get_process_name(const QString& process_name_path) -> QString;
    auto get_process_memory(const QString& process_mem_path) -> QString;
    auto get_process_threads_count(const QString& process_thrcount_path)
        -> QString;
    auto get_process_path_to_exec(const QString& process_exec_path) -> QString;
    auto get_process_start_time(const QString& pid) -> QString;
    auto get_proc_pid_stat_list(const QString& pid) -> QStringList;
    auto get_proc_uptime_list() -> QStringList;
    auto get_proc_cpu_usage(const QString& pid) -> QString;
    static auto get_cpu_cores_amount() -> int;
};

#endif // DATALOADER_H
