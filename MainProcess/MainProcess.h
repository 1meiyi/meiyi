#ifndef MAINPROCESS_H
#define MAINPROCESS_H

#include <QMainWindow>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui { class MainProcess; }
QT_END_NAMESPACE

class MainProcess : public QMainWindow
{
    Q_OBJECT

public:
    MainProcess(QWidget *parent = nullptr);
    ~MainProcess();

    void Open();
    void Clole();

private:
    Ui::MainProcess *ui;
    QProcess *m_process;
    QWidget* m_chidMainWindow;

    private slots:
    void slot_createWaitingBar();
};
#endif // MAINPROCESS_H
