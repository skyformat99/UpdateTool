#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include "common/inofmanaging.h"
#include "seriorport/myseriorport.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_12_clicked();
    void on_comboBox_port_currentIndexChanged(int index);
    void on_pushButton_port_clicked();
    void on_selectfileBtn_clicked();
    void on_startBtn_clicked();

private:
    void initPortCombox(QStringList &portList);
    int getIndex(QStringList &list ,QString &str);
    void initButtonText();
    void sleep(unsigned int msec);
    void sendUpdateCmd();
    bool responseUpdate();
    bool responseSendFile(int num);
    void sendFile();
    int getPacketNum(int bytes);

private:
    Ui::MainWindow *ui;
    MySeriorport *myPort;
    QString mUpdateFile;
    QString mCurrentPort;
};

#endif // MAINWINDOW_H
