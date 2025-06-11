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

#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    os_name = QSysInfo::prettyProductName();
    load_settings();
    create_language_menu();
    starting_processes_load();
    create_chartview();
    set_connections();
    set_update_timer();
}

MainWindow::~MainWindow()
{
    qInfo() << tr("Exit from the app");
    settings->deleteLater();
    d_loader->deleteLater();
    delete ui;
}

void MainWindow::starting_processes_load()
{
    // Getting data about active processes through the thread
    if (!abort_loading) {
        QString program { "" };
        QStringList args;
        if (os_name.contains("Linux")) {
            program = linux_process_program;
            args = linux_process_args;
        } else if (os_name.contains("Windows")) {
            program = windows_shell_program;
            args = windows_process_args;
        }

        d_loader = new Dataloader(program, args);
        connect(d_loader, &Dataloader::sending_output, this,
                &MainWindow::load_process_columns);
    } else {
        timer_processes->stop();
        const QString warn_message { tr(
            "Error with loading data.\n For details check the log file.") };
        qCritical() << warn_message;
        QMessageBox::warning(nullptr, tr("Warning!"), warn_message,
                             QMessageBox::Ok);
    }
}

void MainWindow::on_actionQuit_triggered()
{
    // Quit from the app
    QApplication::quit();
}

void MainWindow::load_settings()
{
    // Loading settings to variable settings
    if (settings == nullptr) {
        settings = new QSettings("Process_monitor");
    }
    settings->setFallbacksEnabled(false);
}

void MainWindow::create_language_menu()
{
    // Dynamic creation of a language interface menu
    languages_group = new QActionGroup(ui->menu_Interface_language);
    languages_group->setExclusive(true);

    connect(languages_group, SIGNAL(triggered(QAction*)), this,
            SLOT(at_language_changed(QAction*)));

    const QString& default_language { "English" };
    const QString current_language
        = settings->value("interface_language", default_language).toString();
    QString default_locale;

    if (current_language == default_language) {
        default_locale = "en_US";
    } else {
        default_locale = current_language;
    }

    const QString language_path = QApplication::applicationDirPath();

    const QDir dir(language_path);
    QStringList filenames = dir.entryList(QStringList("Process_monitor_*.qm"));
    for (const auto& filename : std::as_const(filenames)) {
        QString locale;
        locale = filename;
        locale.truncate(locale.lastIndexOf('.'));
        locale.remove(0, locale.lastIndexOf("r") + 2);
        const QString language
            = QLocale::languageToString(QLocale(locale).language());
        QAction* action = new QAction(language, ui->menubar);
        action->setCheckable(true);
        action->setData(locale);
        ui->menu_Interface_language->addAction(action);
        languages_group->addAction(action);
        if (default_locale == locale) {
            action->setChecked(true);
        }
    }

    load_interface_language(current_language);
}

void MainWindow::at_language_changed(QAction* action)
{
    // Function for a slot that reacting on changes of interface language
    if (action != nullptr) {
        load_interface_language(action->data().toString());
        qInfo() << tr("Interface language changed to: ")
                << action->data().toString();
    }
}

void MainWindow::load_interface_language(const QString& interface_language)
{
    // Downloading new interface language
    const QString& default_language { "English" };
    QString current_language
        = settings->value("interface_language", default_language).toString();

    current_language = interface_language;
    const QLocale locale = QLocale(current_language);
    QLocale::setDefault(locale);
    switch_translator(
        start_translator,
        QString("Process_monitor_%1.qm").arg(interface_language));

    save_interface_language_config();
}

void MainWindow::switch_translator(QTranslator& translator,
                                   const QString& filename)
{
    // Deleting old QTranslator and downloading new QTranslator
    qApp->removeTranslator(&translator);

    const QString path = QApplication::applicationDirPath() + '/';

    if (start_translator.load(path + filename)) {
        qApp->installTranslator(&start_translator);
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    // Updating of the interface language on the fly
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::save_interface_language_config()
{
    // Saving current interface language
    const QString& current_interface_language
        = languages_group->checkedAction()->data().toString();
    settings->setValue("interface_language", current_interface_language);
}

void MainWindow::on_actionAbout_triggered()
{
    // Show content of About message
    QMessageBox msgBox;
    msgBox.setWindowTitle("Process monitor");
    msgBox.setText(
        tr("<b><p><center>Process monitor</b></p></center>"
           "<p><center>Simple program that shows current processes and memory "
           "consumption. Active processes updating once per second,  used "
           "memory once per minute.</p></center>"
           "<p><center>The app icon created by "
           "<a href='https://www.flaticon.com/free-icon/"
           "content-management-system_2630878?term=system+monitor&page=1&"
           "position=1&origin=search&related_id=2630878'>Eucalyp - "
           "Flaticon</a></p></center>"
           "<p><center>License: <a "
           "href='https://www.gnu.org/licenses/gpl-3.0.html'>GNU Public "
           "License v3</a></p></center>"
           "<p><center>Copyright 2025 ©Teg Miles "
           "(movarocks2@gmail.com)</p></center>"));
    msgBox.exec();
    qInfo() << tr("Watched About info");
}

void MainWindow::set_update_timer()
{
    // Set timer for updating output of active processes every second
    timer_processes = new QTimer(this);
    connect(timer_processes, &QTimer::timeout, this,
            &MainWindow::starting_processes_load);
    timer_processes->start(processes_timer_count);
}

void MainWindow::load_process_columns(const QStringList& processes_list)
{
    // Loading active processes to proper columns of the Qt table widget
    if (processes_list.isEmpty()) {
        abort_loading = true;
        qWarning() << "Processes data list is empty.";
    } else {
        ui->processes_table->clearContents();
        ui->processes_table->setRowCount(0);
        if (os_name.contains("Linux")) {
            linux_fill_processes_columns(processes_list);
        } else if (os_name.contains("Windows")) {
            windows_fill_processes_columns(processes_list);
        }

        formatting_processes_table();
    }
}

void MainWindow::formatting_processes_table()
{
    // Applying formatting to processes table
    ui->processes_table->resizeColumnToContents(
        static_cast<int>(TableColumns::Process_name));

    ui->processes_table->horizontalHeader()->setResizeContentsPrecision(-1);

    for (int col = 0; col < ui->processes_table->columnCount(); ++col) {
        ui->processes_table->resizeColumnToContents(col);
    }
    ui->processes_table->verticalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);

    if (selected_row != -1) {
        ui->processes_table->selectRow(selected_row);
    }
}

auto MainWindow::get_reg_proc(const QRegularExpression& reg_proc,
                              const QString& list_item, const QString& title)
{
    // Catch processes categories by regex
    const QRegularExpressionMatch match_pid = reg_proc.match(list_item);
    QString result;
    if (match_pid.hasMatch()) {
        result = match_pid.captured(0);
    } else {
        result = "--";
    }

    return result;
}

void MainWindow::set_cell(const int new_row, const int column,
                          const QString& content)
{
    // Inserting a content to appropriate cells of the table
    ui->processes_table->setItem(new_row, column,
                                 (new QTableWidgetItem(content)));
}

void MainWindow::linux_fill_processes_columns(
    const QStringList& processes_list)
{
    // Get linux content for processes table and inserting it
    const QRegularExpression pid_reg(R"(^(?<pid>[0-9]+))");
    const QRegularExpression name_reg(R"((?<=#)(?<name>.+)(?=\s*\|))");
    const QRegularExpression mem_reg(R"((?<memory>(?<=\|)\d+(?=\s)))");
    const QRegularExpression thrc_reg(R"((?<thrcount>(?<=\d\s)\d+(?=\s)+))");
    const QRegularExpression time_reg(
        R"((?<time>(?<=\d\s)((\d+:\d+)|(\W*\d+)|(\w*\d+))(?=(\s\/)|(\s-))))");
    const QRegularExpression path_reg(R"((?<=\s)(?<path>\/.+)$)");

    for (const auto& list_row : std::as_const(processes_list)) {
        const QString pid = get_reg_proc(pid_reg, list_row, "pid");
        const QString name = get_reg_proc(name_reg, list_row, "name");
        const QString memory = get_reg_proc(mem_reg, list_row, "memory");
        const QString thrcount = get_reg_proc(thrc_reg, list_row, "thrcount");
        const QString time = get_reg_proc(time_reg, list_row, "time");
        const QString path = get_reg_proc(path_reg, list_row, "path");

        const int new_row = ui->processes_table->rowCount();
        ui->processes_table->insertRow(new_row);

        set_cell(new_row, static_cast<int>(TableColumns::PID), pid);
        set_cell(new_row, static_cast<int>(TableColumns::Process_name), name);

        constexpr double kilobytes_in_megabyte { 1024. };
        const double used_memory_mb
            = memory.toDouble() / kilobytes_in_megabyte;
        const QString used_memory_mb_str
            = QString::number(used_memory_mb, 'f', 1);

        set_cell(new_row, static_cast<int>(TableColumns::Memory_used),
                 used_memory_mb_str);
        set_cell(new_row, static_cast<int>(TableColumns::Thread_number),
                 thrcount);
        set_cell(new_row, static_cast<int>(TableColumns::Launch_time), time);
        set_cell(new_row, static_cast<int>(TableColumns::Path_to_file), path);
    }
}

void MainWindow::windows_fill_processes_columns(
    const QStringList& processes_list)
{
    // Get windows content for processes table and inserting it
    const QRegularExpression pid_reg(R"(^(?<pid>[0-9]+))");
    const QRegularExpression name_reg(
        R"((?<=\s)(?<name>([A-z_\-\.]+\w+))(?=\s))");
    const QRegularExpression mem_reg(R"((?<=(\s))(?<memory>\d+,\d\d))");
    const QRegularExpression thrc_reg(
        R"((?<=\|\s)(?<thrcount>\d+)(?=(\s\|)|(\s\d)))");
    const QRegularExpression time_reg(
        R"((?<time>(\d\d\d\d-\d\d-\d\d_\d\d:\d\d:\d\d)))");
    const QRegularExpression path_reg(R"((?<=\|\s)(?<path>([\w+]:\\.*))$)");

    for (const auto& list_row : processes_list) {
        const QString pid = get_reg_proc(pid_reg, list_row, "pid");
        const QString name = get_reg_proc(name_reg, list_row, "name");
        const QString memory = get_reg_proc(mem_reg, list_row, "memory");
        const QString thrcount = get_reg_proc(thrc_reg, list_row, "thrcount");
        const QString time = get_reg_proc(time_reg, list_row, "time");
        const QString path = get_reg_proc(path_reg, list_row, "path");

        const int new_row = ui->processes_table->rowCount();
        ui->processes_table->insertRow(new_row);

        set_cell(new_row, static_cast<int>(TableColumns::PID), pid);
        set_cell(new_row, static_cast<int>(TableColumns::Process_name), name);

        set_cell(new_row, static_cast<int>(TableColumns::Memory_used), memory);
        set_cell(new_row, static_cast<int>(TableColumns::Thread_number),
                 thrcount);
        set_cell(new_row, static_cast<int>(TableColumns::Launch_time), time);
        set_cell(new_row, static_cast<int>(TableColumns::Path_to_file), path);
    }
}

void MainWindow::on_processes_table_itemSelectionChanged()
{
    // Remembering current selected row
    selected_row = ui->processes_table->currentRow();
}

void MainWindow::set_connections()
{
    // Set global connections
    connect(ui->processes_table, &QWidget::customContextMenuRequested, this,
            &MainWindow::show_context_menu);
}

void MainWindow::show_context_menu() const
{
    // Creating and showing context menu for the table widget
    QMenu* menu = new QMenu();
    QAction* kill_selected_process
        = menu->addAction(tr("Kill selected process"));
    QAction* clear_selection = menu->addAction(tr("Clear selection"));

    connect(kill_selected_process, &QAction::triggered, this,
            &MainWindow::on_kill_selected_process_triggered);
    connect(clear_selection, &QAction::triggered, this,
            &MainWindow::on_clear_selection_triggered);

    menu->exec(QCursor::pos());
    menu->deleteLater();
}

void MainWindow::on_kill_selected_process_triggered()
{
    // Reaction on clicking menu button "kill selected process"
    timer_processes->stop();
    const QTableWidgetItem* selected_row = ui->processes_table->currentItem();
    if (selected_row != nullptr) {
        const int row = selected_row->row();
        const QTableWidgetItem* pid_widget = ui->processes_table->item(row, 0);
        const QTableWidgetItem* name_widget
            = ui->processes_table->item(row, 1);
        const QString pid_str = pid_widget->text();
        const QString name_str = name_widget->text();
        kill_process_warning(pid_str, name_str);
    }
    timer_processes->start(processes_timer_count);
}

void MainWindow::on_clear_selection_triggered()
{
    // Removing row selection
    timer_processes->stop();
    ui->processes_table->clearSelection();
    selected_row = -1;
    timer_processes->start(processes_timer_count);
}

void MainWindow::kill_process_warning(const QString& pid, const QString& name)
{
    // Warning before deleting selected process
    QMessageBox message;
    message.setIcon(QMessageBox::Question);
    message.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    const QString& warning { tr("Are you sure you want to delete process ")
                             + name };
    message.setText(warning);
    const int button_clicked = message.exec();
    if (button_clicked == QMessageBox::Ok) {
        kill_process_exec(pid, name);

    } else {
        qInfo() << tr("Cancel to kill process: ") << name;
    }
}

void MainWindow::kill_process_exec(const QString& pid, const QString& name)
{
    // Deleting selected process
    QProcess* process = new QProcess(this);
    connect(process, &QProcess::readyReadStandardError, this,
            &MainWindow::handle_kill_process_std_error);
    connect(process, &QProcess::finished, this,
            &MainWindow::kill_process_finished);
    connect(process, &QProcess::errorOccurred, this,
            &MainWindow::handle_kill_process_qprocess_error);
    if (os_name.contains("Linux")) {

        linux_kill_process_args.append(pid);
        process->start(linux_kill_process_program, linux_kill_process_args);

    } else if (os_name.contains("Windows")) {

        windows_kill_process_args.append(name);
        process->start(windows_kill_process_program,
                       windows_kill_process_args);
    }
}

void MainWindow::handle_kill_process_std_error()
{
    // Handle error that appears during killing process
    auto* process = qobject_cast<QProcess*>(sender());
    if (process != nullptr) {
        const QString error_message = process->readAllStandardError();
        const QString error_output
            = tr("Killing process failed. Error: ") + error_message;
        qCritical() << error_output;
        process->deleteLater();
        QMessageBox::warning(nullptr, tr("Warning!"), error_output,
                             QMessageBox::Ok);
    }
}

void MainWindow::kill_process_finished()
{
    // Cleaning after killing process
    auto* process = qobject_cast<QProcess*>(sender());
    if (process != nullptr) {
        process->deleteLater();
        ui->processes_table->removeRow(selected_row);
        selected_row = -1;

        const QString success_message = tr("The process killed successfully.");
        qInfo() << success_message;
    }
}

void MainWindow::MainWindow::handle_kill_process_qprocess_error(
    QProcess::ProcessError error)
{
    // Handling QProcess errors that appear during killing process
    const QString warn_message {
        tr("QProcess error occured during killing process.\n "
           "\n Error number:  ")
        + QString::number(error)
    };
    qCritical() << warn_message;
    QMessageBox::warning(nullptr, tr("Warning!"), warn_message,
                         QMessageBox::Ok);
}

void MainWindow::create_chartview()
{
    // Creating widget for showing total used memory
    mem_widget = new UsedMemoryWidget(os_name, this);
    ui->memory_vert_layout->addWidget(mem_widget);
    ui->memory_vert_layout->setStretch(1, 1);
}
