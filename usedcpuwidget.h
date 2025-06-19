#ifndef USEDCPUWIDGET_H
#define USEDCPUWIDGET_H

#include "dataloader.h"
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QFile>
#include <future>
#include <cmath>
#include <vector>

class UsedCPUWidget : public QWidget {
    Q_OBJECT
public:
    explicit UsedCPUWidget(QWidget* parent = nullptr);
    void loading_cpu_data();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    double cpu_usage_double { 0.0 };
    double cpu_usage_percent { 0.0 };
    const int timer_update_count { 1000 };
    const QString proc_stat_path { "/proc/stat" };
    double cpu_idle_prev { 0 };
    double cpu_total_prev { 0 };
    QTimer* cpu_timer { nullptr };

    void load_settings();
    void set_timer();
    void repaint_widget();
    void set_connection() const;

signals:
    void data_loaded();
};

#endif // USEDCPUWIDGET_H
