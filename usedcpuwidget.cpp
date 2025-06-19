#include "usedcpuwidget.h"

UsedCPUWidget::UsedCPUWidget(QWidget* parent)
    : QWidget { parent }
{
    load_settings();
    set_connection();
    set_timer();
}

void UsedCPUWidget::load_settings()
{
    // Load default widget settings
    QPalette pal = palette();
    const QBrush background_brush(Qt::darkMagenta, Qt::CrossPattern);
    pal.setBrush(QPalette::Window, background_brush);
    setAutoFillBackground(true);
    setPalette(pal);
}

void UsedCPUWidget::loading_cpu_data()
{

    std::future<QStringList> future = std::async(std::launch::async, [this]() {
        const QScopedPointer<Dataloader> d_loader(
            new Dataloader(proc_stat_path));
        auto data_list = d_loader->loading_linux_proc_cpu_data();
        return data_list;
    });
    QStringList cpu_info_list;
    try {
        cpu_info_list = future.get();
    } catch (std::exception& excep) {
        cpu_timer->stop();
        const QString warn_message {
            tr("Error happens during cpu data list loading: ") + excep.what()
        };
        qCritical() << warn_message;
    }

    constexpr int min_cpu_file_categories { 7 };
    if (cpu_info_list.size() < min_cpu_file_categories) {
        cpu_timer->stop();
        const QString warn_message { tr(
            "Error! Not enough data in the cpu list") };
        qCritical() << warn_message;
        QMessageBox::warning(nullptr, tr("Warning!"), warn_message,
                             QMessageBox::Ok);
    } else {
        double cpu_total_current { 0.0 };
        for (int i = 1; i < cpu_info_list.size(); ++i) {
            cpu_total_current += cpu_info_list[i].toDouble();
        }
        const double cpu_idle_current = cpu_info_list[4].toDouble();
        const double dif_cpu_idle = cpu_idle_current - cpu_idle_prev;
        const double dif_total_cpu = cpu_total_current - cpu_total_prev;
        constexpr double one_hundred_percent { 100. };
        constexpr int round_multiplier { 100 };
        cpu_usage_double = (dif_total_cpu - dif_cpu_idle) / dif_total_cpu;
        cpu_usage_percent = cpu_usage_double * one_hundred_percent;
        cpu_usage_percent = std::ceil(cpu_usage_percent * round_multiplier)
            / round_multiplier;
        cpu_idle_prev = cpu_idle_current;
        cpu_total_prev = cpu_total_current;
        emit data_loaded();
    }
}

void UsedCPUWidget::set_timer()
{
    cpu_timer = new QTimer(this);
    connect(cpu_timer, &QTimer::timeout, this,
            &UsedCPUWidget::loading_cpu_data);
    cpu_timer->start(timer_update_count);
}

void UsedCPUWidget::repaint_widget()
{
    repaint();
}

void UsedCPUWidget::set_connection() const
{
    connect(this, &UsedCPUWidget::data_loaded, this,
            &UsedCPUWidget::repaint_widget);
}

void UsedCPUWidget::paintEvent(QPaintEvent* event)
{
    const QScopedPointer<QPainter> painter(new QPainter(this));
    QPen pen(Qt::cyan);
    pen.setWidth(2);
    painter->setPen(pen);
    const QBrush brush(Qt::blue);
    painter->setBrush(brush);
    const int width = this->width();
    const int height = this->height();
    QFont default_font = this->font();
    default_font.setBold(true);
    painter->setFont(default_font);
    const QString text(QString::number(cpu_usage_percent) + " %");

    const QRect used_mem_rect(0, 0, static_cast<int>(width * cpu_usage_double),
                              height);
    painter->drawRect(used_mem_rect);
    painter->drawText(used_mem_rect, Qt::AlignCenter | Qt::TextWordWrap, text);
}
