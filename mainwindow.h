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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dataloader.h"
#include "usedmemorywidget.h"
#include "usedcpuwidget.h"
#include "harddiskwidget.h"

#include <QActionGroup>
#include <QDebug>
#include <QDir>
#include <QEvent>
#include <QMainWindow>
#include <QMessageBox>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QSysInfo>
#include <QTimer>
#include <QTranslator>
#include <QCloseEvent>

QT_BEGIN_NAMESPACE

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

enum class TableColumns : std::uint8_t {
    PID,
    Process_name,
    Memory_used,
    CPU_percent,
    Thread_number,
    Launch_time,
    Path_to_file,

};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    MainWindow(const MainWindow& src) = delete;
    auto operator=(const MainWindow& rhs) -> MainWindow& = delete;
    MainWindow(const MainWindow&& src) = delete;
    auto operator=(const MainWindow&& rhs) -> MainWindow& = delete;

private slots:
    static void on_actionQuit_triggered();
    void at_language_changed(QAction* action);
    void on_actionAbout_triggered();
    void on_processes_table_itemSelectionChanged();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    Ui::MainWindow* ui { nullptr };
    QSettings* settings { nullptr };
    QActionGroup* languages_group { nullptr };
    QTranslator start_translator;
    QTimer* timer_processes { nullptr };

    const int processes_timer_count { 1000 };
    int selected_row { -1 };
    const QString linux_kill_process_program { "kill" };
    QStringList linux_kill_process_args { "-9" };

    bool abort_loading { false };

    void load_settings();
    void create_language_menu();
    void load_interface_language(const QString& interface_language);
    void switch_translator(QTranslator& translator, const QString& filename);
    void changeEvent(QEvent* event) override;
    void save_interface_language_config();
    void load_linux_processes_columns(
        std::priority_queue<info_tuple, std::vector<info_tuple>>& columns);
    void set_update_timer();
    void show_context_menu() const;
    void set_connections();
    void on_kill_selected_process_triggered();
    void kill_process_warning(const QString& pid, const QString& name);
    void kill_process_exec(const QString& pid, const QString& name);
    void create_used_memory_chartview();
    void kill_process_finished();
    void handle_kill_process_std_error();
    void handle_kill_process_qprocess_error(QProcess::ProcessError error);
    void starting_processes_load();
    void set_cell(const int new_row, const int column, const QString& content);
    void on_clear_selection_triggered();
    void formatting_processes_table();
    void create_used_cpu_chartview();
    void create_hard_disk_info_chartview();
};
#endif // MAINWINDOW_H
