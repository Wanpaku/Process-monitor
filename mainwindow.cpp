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

#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    load_settings();
    create_language_menu();
    qInfo() << tr("Start program Process monitor.");
    starting_processes_load();
    create_used_memory_chartview();
    create_used_cpu_chartview();
    create_hard_disk_info_chartview();
    set_connections();
    set_update_timer();
}

MainWindow::~MainWindow()
{
    settings->deleteLater();
    delete ui;
}

void MainWindow::starting_processes_load()
{
    // Getting data about active processes through the thread
    if (!abort_loading) {

        auto future = std::async(std::launch::async, [this]() {
            const QScopedPointer<Dataloader> d_loader(new Dataloader());
            auto data_list = d_loader->loading_linux_proc_processes_data();
            return data_list;
        });
        std::priority_queue<info_tuple, std::vector<info_tuple>> columns;
        try {
            columns = future.get();
            load_linux_processes_columns(columns);
        } catch (std::exception& excep) {
            timer_processes->stop();
            const QString warn_message {
                tr("Error happens during cpu data list loading: ")
                + excep.what()
            };
            qCritical() << warn_message;
        }

    } else {
        timer_processes->stop();
        const QString warn_message { tr("Error happens during loading data.\n "
                                        "For details check the log file.") };
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
           "<p><center>Simple program that shows current processes, memory "
           "consumption, cpu usage and used space of the current hard disk. "
           "All indicators "
           "updating "
           "once per second.</p></center>"
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
    qInfo() << tr("Watched About info");
    msgBox.exec();
}

void MainWindow::set_update_timer()
{
    // Set timer for updating output of active processes every second
    timer_processes = new QTimer(this);
    connect(timer_processes, &QTimer::timeout, this,
            &MainWindow::starting_processes_load);
    timer_processes->start(processes_timer_count);
}

void MainWindow::load_linux_processes_columns(
    std::priority_queue<info_tuple, std::vector<info_tuple>>& columns)
{
    if (columns.empty()) {
        abort_loading = true;
        qWarning() << "Processes data list is empty.";
    } else {
        ui->processes_table->clearContents();
        ui->processes_table->setRowCount(0);

        while (!columns.empty()) {
            auto [memory, name, pid, thrcount, stime, path, cpu_usage]
                = columns.top();
            columns.pop();
            const int new_row = ui->processes_table->rowCount();
            ui->processes_table->insertRow(new_row);

            set_cell(new_row, static_cast<int>(TableColumns::PID), pid);
            set_cell(new_row, static_cast<int>(TableColumns::Process_name),
                     name);
            set_cell(new_row, static_cast<int>(TableColumns::Memory_used),
                     QString::number(memory));
            set_cell(new_row, static_cast<int>(TableColumns::Thread_number),
                     thrcount);
            set_cell(new_row, static_cast<int>(TableColumns::Launch_time),
                     stime);
            set_cell(new_row, static_cast<int>(TableColumns::Path_to_file),
                     path);
            set_cell(new_row, static_cast<int>(TableColumns::CPU_percent),
                     cpu_usage);
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

void MainWindow::set_cell(const int new_row, const int column,
                          const QString& content)
{
    // Inserting a content to appropriate cells of the table
    ui->processes_table->setItem(new_row, column,
                                 (new QTableWidgetItem(content)));
}

void MainWindow::on_processes_table_itemSelectionChanged()
{
    // Remembering current selected row
    selected_row = ui->processes_table->currentRow();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    qInfo() << tr("Exit from the app");
    event->accept();
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

    linux_kill_process_args.append(pid);
    process->start(linux_kill_process_program, linux_kill_process_args);
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

void MainWindow::create_used_memory_chartview()
{
    // Creating widget for showing total used memory
    UsedMemoryWidget* mem_widget = new UsedMemoryWidget(this);
    auto* mem_layout = ui->memory_group_box->layout();
    if (mem_layout != nullptr) {
        auto* hor_layout = qobject_cast<QHBoxLayout*>(mem_layout);
        if (hor_layout != nullptr) {
            hor_layout->addWidget(mem_widget);
        }
    }
    mem_layout->addWidget(mem_widget);
    mem_widget->load_used_memory_data();
}

void MainWindow::create_used_cpu_chartview()
{
    UsedCPUWidget* cpu_widget = new UsedCPUWidget(this);
    auto* cpu_layout = ui->used_cpu_groupbox->layout();
    if (cpu_layout != nullptr) {
        auto* hor_layout = qobject_cast<QHBoxLayout*>(cpu_layout);
        if (hor_layout != nullptr) {
            hor_layout->addWidget(cpu_widget);
        }
    }
    cpu_widget->loading_cpu_data();
}

void MainWindow::create_hard_disk_info_chartview()
{
    HardDiskWidget* hard_disk_widget = new HardDiskWidget(this);
    auto* hard_disk_layout = ui->hard_disk_groupbox->layout();
    if (hard_disk_layout != nullptr) {
        auto* hor_layout = qobject_cast<QHBoxLayout*>(hard_disk_layout);
        if (hor_layout != nullptr) {
            hor_layout->addWidget(hard_disk_widget);
        }
    }
    hard_disk_widget->loading_hard_disks_info();
}
