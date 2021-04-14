#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDataStream>
#include <QMessageBox>
#include <QPushButton>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QTextCodec>
#include <QStringList>
#include <QtCharts>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QCheckBox>
#include <QIntValidator>
#define MAX_X 500
namespace Ui {
class MainDialog;
}

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MainDialog(QWidget *parent = 0);
    ~MainDialog();
    QSerialPort* serial;

private:
    Ui::MainDialog *ui;
    int count;
    int start;
    QChart* chart;
    QChartView* chartView;
    QLineSeries* lineSeries;
    QValueAxis* axisX;
    QValueAxis* axisY;
public slots:
    void onOpenButtonClicked();
    void onSendButtonClicked();
    void onSearchButtonClicked();
    void SerialRead();
    void onChartCheckChanged();
    void onClearRButtonClicked();
    void drawChartView();
    void onChartClearClicked();
signals:
    void drawChart();
private slots:
    void on_reBox_textChanged();
};

#endif // MAINDIALOG_H
