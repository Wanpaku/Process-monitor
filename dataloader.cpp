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

#include "dataloader.h"

Dataloader::Dataloader(const QString& proc_path_, QWidget* parent)
    : proc_path(proc_path_)
{
}

Dataloader::Dataloader(QWidget* parent)
{
}

auto Dataloader::loading_linux_proc_cpu_data() -> QStringList
{
    // Loading cpu info from /proc/stat directory
    try {
        QFile input_file(proc_path);
        if (!input_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const std::string& error_message { "Can't open the path: "
                                               + proc_path.toStdString() };
            throw std::runtime_error(error_message);
        }
        const QString cpu_info = input_file.readLine();
        data_list = cpu_info.split(' ', Qt::SkipEmptyParts);

    } catch (std::exception& excep) {
        const QString error_message = excep.what();
        qWarning() << error_message;
    }
    return data_list;
}

auto Dataloader::loading_linux_proc_mem_data() -> QStringList
{
    // Loading memory info from /proc/meminfo
    try {
        QFile input_file(proc_path);
        if (!input_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const std::string& error_message { "Can't open the path: "
                                               + proc_path.toStdString() };
            throw std::runtime_error(error_message);
        }

        QTextStream in_stream(&input_file);
        QString line = in_stream.readLine();
        while (!line.isNull()) {
            const QStringList line_split = line.split(' ', Qt::SkipEmptyParts);
            data_list.append(line_split[1]);
            line = in_stream.readLine();
        }
    } catch (std::exception& excep) {
        const QString error_message = excep.what();
        qWarning() << error_message;
    }
    return data_list;
}

auto Dataloader::loading_linux_proc_processes_data()
    -> std::priority_queue<info_tuple, std::vector<info_tuple>>
{
    // Loading info about active processes from different proc directories
    const QString proc_processes_path { "/proc/" };
    using iter_flag = QDirListing::IteratorFlag;
    const static QRegularExpression reg_pid("^\\d*$");
    QStringList pid_list;
    for (const auto& folder_path :
         QDirListing(proc_processes_path, iter_flag::DirsOnly)) {
        const QString folder_name = folder_path.baseName();
        const QRegularExpressionMatch match = reg_pid.match(folder_name);
        if (match.hasMatch()) {
            pid_list.append(folder_name);
        }
    }

    std::priority_queue<info_tuple, std::vector<info_tuple>> columns;
    for (const auto& pid : std::as_const(pid_list)) {
        const QString process_name_path
            = proc_processes_path + pid + "/" + "comm";
        const QString process_memory_path
            = proc_processes_path + pid + "/" + "statm";
        const QString process_thrcount_path
            = proc_processes_path + pid + "/" + "status";
        const QString process_exec_path
            = proc_processes_path + pid + "/" + "exe";

        const QString process_name = get_process_name(process_name_path);
        if (process_name.isEmpty()) {
            continue;
        }
        const QString process_memory = get_process_memory(process_memory_path);
        const QString process_thrcount
            = get_process_threads_count(process_thrcount_path);
        const QString process_path_to_exec
            = get_process_path_to_exec(process_exec_path);
        const QString process_start_time = get_process_start_time(pid);
        const QString process_cpu_usage = get_proc_cpu_usage(pid);

        columns.emplace(process_memory.toInt(), process_name, pid,
                        process_thrcount, process_start_time,
                        process_path_to_exec, process_cpu_usage);
    }

    return columns;
}

auto Dataloader::get_process_name(const QString& process_name_path) -> QString
{
    QString process_name;
    try {
        QFile input_file(process_name_path);
        if (!input_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const std::string& error_message {
                "Can't open the path: " + process_name_path.toStdString()
            };
            throw std::runtime_error(error_message);
        }
        QTextStream in_stream(&input_file);
        process_name = in_stream.readLine().trimmed();

    } catch (std::exception& excep) {
        const QString error_message = excep.what();
        qWarning() << error_message;
    }
    return process_name;
}

auto Dataloader::get_process_memory(const QString& process_mem_path) -> QString
{
    QString process_memory_mb;
    try {
        QFile input_file(process_mem_path);
        if (!input_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const std::string& error_message {
                "Can't open the path: " + process_mem_path.toStdString()
            };
            throw std::runtime_error(error_message);
        }
        QTextStream in_stream(&input_file);
        const QString process_memory = in_stream.readLine().trimmed();
        const QStringList process_memory_split
            = process_memory.split(" ", Qt::SkipEmptyParts);
        const int process_memory_kb = process_memory_split[1].toInt();
        // Resident size memory is in pages. Therefore it needs to be converted
        // in Mb.
        constexpr int kb_in_mb { 1024 };
        constexpr int bytes_in_page { 4096 };
        constexpr int bytes_in_kb { 1024 };
        process_memory_mb = QString::number(
            (process_memory_kb * bytes_in_page / bytes_in_kb) / kb_in_mb);

    } catch (std::exception& excep) {
        const QString error_message = excep.what();
        qWarning() << error_message;
    }
    return process_memory_mb;
}

auto Dataloader::get_process_threads_count(
    const QString& process_thrcount_path) -> QString
{
    QString process_thrcount;
    try {
        QFile input_file(process_thrcount_path);
        if (!input_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const std::string& error_message {
                "Can't open the path: " + process_thrcount_path.toStdString()
            };
            throw std::runtime_error(error_message);
        }
        QTextStream in_stream(&input_file);
        QString status_file = in_stream.readLine().trimmed();
        while (!status_file.isNull()) {
            if (status_file.startsWith("Threads:")) {
                QStringList threads_raw
                    = status_file.split(':', Qt::SkipEmptyParts);
                process_thrcount = threads_raw[1].trimmed();
                break;
            }
            status_file = in_stream.readLine().trimmed();
        }

    } catch (std::exception& excep) {
        const QString error_message = excep.what();
        qWarning() << error_message;
    }
    return process_thrcount;
}

auto Dataloader::get_process_path_to_exec(const QString& process_exec_path)
    -> QString
{
    QString process_path_to_exec;
    try {
        const QFileInfo fileInfo(process_exec_path);

        if (fileInfo.isSymLink()) {
            process_path_to_exec = fileInfo.symLinkTarget();
        } else {
            qWarning() << process_exec_path << tr(" is not a symlink.");
        }

        if (process_path_to_exec.isEmpty()) {
            process_path_to_exec = "--";
        }

    } catch (std::exception& excep) {
        const QString error_message = excep.what();
        qWarning() << error_message;
    }
    return process_path_to_exec;
}

auto Dataloader::get_process_start_time(const QString& pid) -> QString
{

    const QStringList stat_file_split = get_proc_pid_stat_list(pid);

    const auto ticks = static_cast<double>(sysconf(_SC_CLK_TCK));
    const double process_time_ticks { stat_file_split[20].toDouble() };
    const double process_time_sec = process_time_ticks / ticks;

    const QStringList uptime_split = get_proc_uptime_list();

    const double uptime_double = uptime_split[0].toDouble();
    const double process_duration_sec = uptime_double - process_time_sec;
    std::time_t total_seconds { std::time(nullptr) };

    total_seconds -= static_cast<long>(process_duration_sec);
    std::stringstream s_stream;
    struct tm local_time {};
    const tm* time { localtime_r(&total_seconds, &local_time) };

    s_stream << std::put_time(time, "%d-%m-%Y_%H:%M:%S");
    const std::string time_str { s_stream.str() };

    return QString::fromStdString(time_str);
}

auto Dataloader::get_proc_pid_stat_list(const QString& pid) -> QStringList
{
    const QString proc_pid_stat_path { "/proc/" + pid + "/stat" };
    QFile input_file(proc_pid_stat_path);
    if (!input_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const std::string& error_message { "Can't open the path!" };
        throw std::runtime_error(error_message);
    }
    QTextStream in_stream(&input_file);
    QString stat_file = in_stream.readLine().trimmed();
    static const QRegularExpression cut_name_reg { QRegularExpression(
        R"(\(.*\)\s)") };
    stat_file.remove(cut_name_reg);

    QStringList stat_file_split { stat_file.split(' ', Qt::SkipEmptyParts) };

    return stat_file_split;
}

auto Dataloader::get_proc_uptime_list() -> QStringList
{
    const QString proc_uptime_path { "/proc/uptime" };
    QFile uptime_file(proc_uptime_path);
    if (!uptime_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const std::string& error_message { "Can't open the path!" };
        throw std::runtime_error(error_message);
    }

    QTextStream uptime_stream(&uptime_file);
    const QString uptime = uptime_stream.readLine().trimmed();
    const QStringList uptime_split = uptime.split(' ', Qt::SkipEmptyParts);

    return uptime_split;
}

auto Dataloader::get_proc_cpu_usage(const QString& pid) -> QString
{
    const QStringList proc_pid_stat_list = get_proc_pid_stat_list(pid);
    const QStringList proc_uptime_list = get_proc_uptime_list();

    const auto ticks = static_cast<double>(sysconf(_SC_CLK_TCK));

    const double total_process_time_sec = (proc_pid_stat_list[12].toDouble()
                                           + proc_pid_stat_list[13].toDouble()
                                           + proc_pid_stat_list[14].toDouble()
                                           + proc_pid_stat_list[15].toDouble())
        / ticks;
    const double proc_start_time = proc_pid_stat_list[20].toDouble();
    const double proc_start_time_sec = proc_start_time / ticks;
    const double total_uptime = proc_uptime_list[0].toDouble();
    const double total_machine_work_time = total_uptime - proc_start_time_sec;
    const int cpu_cores = get_cpu_cores_amount();

    auto proc_cpu_usage
        = 100 * (total_process_time_sec / total_machine_work_time) / cpu_cores;

    proc_cpu_usage = std::ceil(proc_cpu_usage * 100) / 100;

    return QString::number(proc_cpu_usage);
}

auto Dataloader::get_cpu_cores_amount() -> int
{
    const QString proc_cpuinfo_path { "/proc/cpuinfo" };
    QFile cpuinfo_file(proc_cpuinfo_path);
    if (!cpuinfo_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const std::string& error_message { "Can't open the path!" };
        throw std::runtime_error(error_message);
    }
    int cpu_cores_count { 0 };
    QTextStream cpuinfo_stream(&cpuinfo_file);
    QString cpuinfo_line = cpuinfo_stream.readLine().trimmed();

    while (!cpuinfo_line.isNull()) {
        if (cpuinfo_line.contains("processor")) {
            ++cpu_cores_count;
        }
        cpuinfo_line = cpuinfo_stream.readLine().trimmed();
    }

    return cpu_cores_count;
}
