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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dataloader.h"
#include "usedmemorywidget.h"
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

QT_BEGIN_NAMESPACE

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

enum class TableColumns : std::uint8_t {
    PID,
    Process_name,
    Memory_used,
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

private:
    Ui::MainWindow* ui { nullptr };
    QSettings* settings { nullptr };
    QActionGroup* languages_group { nullptr };
    QTranslator start_translator;
    UsedMemoryWidget* mem_widget { nullptr };
    QTimer* timer_processes { nullptr };

    const int processes_timer_count { 1000 };
    int selected_row { -1 };
    const QString linux_kill_process_program { "kill" };
    QStringList linux_kill_process_args { "-9" };
    const QString windows_kill_process_program {
        R"("C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe")"
    };
    QStringList windows_kill_process_args { "Stop-Process", "-Name" };
    const QString linux_process_program { "/bin/bash" };
    const QStringList linux_process_args {
        "-c",
        "ps axwwo \"%p#%c|\" -o rss -o thcount -o stime -o exe --sort "
        "-rss "
        "c "
        "--no-headers | cat"
    };

    const QString windows_shell_program {
        R"("C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe")"
    };
    const QStringList windows_process_args {
        "-Command",
        "Get-Process",
        "|",
        "Select-Object",
        "Id, @{Label=\"Separator\";Expression={\" | \"}},",
        "ProcessName, @{Label=\"Separator2\";Expression={\" | \"}},",
        "@{Name='Memory(MB)';Expression={[Math]::Round($_.WorkingSet64 / 1MB, "
        "2).ToString(\"F2\")}}, @{Label=\"Separator3\";Expression={\" | \"}},",
        "@{Name='ThreadCount';Expression={$_.Threads.Count}},",
        "@{Name='FormattedStartTime';Expression={$_.StartTime.ToString('yyyy-"
        "MM-dd_HH:mm:ss')}}, @{Label=\"Separator4\";Expression={\" | \"}},",
        "Path",
        "|",
        "Format-Table  -HideTableHeaders | "
        "Out-String -Width 500 "
    };

    QString os_name;
    bool abort_loading { false };
    Dataloader* d_loader { nullptr };

    void load_settings();
    void create_language_menu();
    void load_interface_language(const QString& interface_language);
    void switch_translator(QTranslator& translator, const QString& filename);
    void changeEvent(QEvent* event) override;
    void save_interface_language_config();
    void load_process_columns(const QStringList& processes_list);
    void set_update_timer();
    void show_context_menu() const;
    void set_connections();
    void on_kill_selected_process_triggered();
    void kill_process_warning(const QString& pid, const QString& name);
    void kill_process_exec(const QString& pid, const QString& name);
    void create_chartview();
    void kill_process_finished();
    void handle_kill_process_std_error();
    void handle_kill_process_qprocess_error(QProcess::ProcessError error);
    void starting_processes_load();
    void linux_fill_processes_columns(const QStringList& processes_list);
    void windows_fill_processes_columns(const QStringList& processes_list);
    auto get_reg_proc(const QRegularExpression& reg_proc,
                      const QString& list_item, const QString& title);
    void set_cell(const int new_row, const int column, const QString& content);
    void on_clear_selection_triggered();
    void formatting_processes_table();
};
#endif // MAINWINDOW_H
