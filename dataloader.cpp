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

Dataloader::Dataloader(const QString& program_, const QStringList& args_,
                       QWidget* parent)
    : program(program_)
    , args(args_)
{
    loading_data();
}

void Dataloader::loading_data()
{
    // Loading active processes to QStringList through QProcess and shell
    // commands
    QProcess* process = new QProcess(this);
    connect(process, &QProcess::readyReadStandardError, this,
            &Dataloader::handle_load_std_error);
    connect(process, &QProcess::readyReadStandardOutput, this,
            &Dataloader::get_standard_output);
    connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &Dataloader::data_loading_finished);
    connect(process, &QProcess::errorOccurred, this,
            &Dataloader::handle_load_qprocess_error);

    process->start(program, args);
}

void Dataloader::handle_load_std_error()
{
    // Handle standard errors that appear in a shell output
    auto* process = qobject_cast<QProcess*>(sender());
    if (process != nullptr) {
        const QString error_message = process->readAllStandardError();
        const QString error_output
            = tr("Downloading processes failed. Error: ") + error_message;
        qCritical() << error_output;
        process->deleteLater();
    }
}

void Dataloader::get_standard_output()
{
    // Download standard shell output to QStringList container
    auto* process = qobject_cast<QProcess*>(sender());
    if (process != nullptr) {
        const QString output(process->readAllStandardOutput());
        QStringList lines = output.split("\n", Qt::SkipEmptyParts);

        for (const QString& line : std::as_const(lines)) {
            if (!line.simplified().isEmpty()) {
                data_output_list.append(line.simplified());
            }
        }
    }
}

void Dataloader::data_loading_finished(int exit_code,
                                       QProcess::ExitStatus exit_status)
{
    // Emitting signal with data list after QProcess finished and deleting it
    auto* process = qobject_cast<QProcess*>(sender());
    if (process != nullptr) {
        if (exit_code != 0 && exit_status != QProcess::NormalExit) {
            qCritical() << "Process finished with exit code: " << exit_code
                        << " and exit status: " << exit_status;
        } else {
            emit sending_output(data_output_list);
        }

        process->deleteLater();
    }
}

void Dataloader::handle_load_qprocess_error(QProcess::ProcessError error)
{
    // Handle QProcess errors
    auto* process = qobject_cast<QProcess*>(sender());
    if (process != nullptr) {
        const QString warn_message {
            tr("QProcess error occured during loading "
               "active processse.\n Error number:  ")
            + QString::number(error)
        };
        qCritical() << warn_message;
        process->deleteLater();
    }
}
