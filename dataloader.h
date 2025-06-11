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

class Dataloader : public QObject {
    Q_OBJECT
public:
    Dataloader(const QString& program_, const QStringList& args_,
               QWidget* parent = nullptr);

private:
    QString program;
    QStringList args;
    QStringList data_output_list;

    void loading_data();
    void data_loading_finished(int exit_code,
                               QProcess::ExitStatus exit_status);
    void get_standard_output();
    void handle_load_std_error();
    void handle_load_qprocess_error(QProcess::ProcessError error);

signals:
    void sending_output(const QStringList& output_list);
};

#endif // DATALOADER_H
