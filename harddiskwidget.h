#ifndef HARDDISKWIDGET_H
#define HARDDISKWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QStorageInfo>

class HardDiskWidget : public QWidget {
    Q_OBJECT
public:
    explicit HardDiskWidget(QWidget* parent = nullptr);
    void loading_hard_disks_info();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    double total_size { 0 };
    double free_size { 0 };
    double avail_size { 0 };
    double used_size { 0 };
    QTimer* hdd_timer { nullptr };
    const int timer_count { 1000 };

    void load_settings();
    void set_timer();
    void repaint_widget();
    void set_connection() const;

signals:
    void data_loaded();
};

#endif // HARDDISKWIDGET_H
