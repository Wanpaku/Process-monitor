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
#include "logger.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDir>
#include <QDebug>
#include <memory>

auto main(int argc, char* argv[]) -> int
{
    Logger::init();
    qInfo() << QObject::tr("Start program.");
    std::unique_ptr<QApplication> app { std::make_unique<QApplication>(argc,
                                                                       argv) };
    QApplication::setWindowIcon(
        QIcon(":/icons/icons/content-management-system.png"));
    std::unique_ptr<MainWindow> main_window { std::make_unique<MainWindow>() };
    main_window->show();
    return app->exec();
}
