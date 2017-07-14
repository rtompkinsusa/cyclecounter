#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QDate>
#include <QInputDialog>
#include <QDoubleValidator>
#include <QWidget>
#include <QMessageBox>


namespace Ui {
class MainWindow;
}
class QtSql;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void SubmitDataMatched(double value, int m_currentAuditId);
    void SubmitDataNoMatch(double m_floorCount, int m_currentAuditId);
    void GetNextItem();
    QSqlDatabase cycleCountDb;
    void connClose();
    bool connOpen();
    bool isMatched;
    int numEntries;


    bool CheckMatch(int m_floorCount, QString m_loc, QString m_item);
    void AssignAuditItem(int m_currentAuditId);
    void ClearFormItem();
    void ForceRecount();


    void ErrorMessageBox(QString m_message);

    void UpdateNoMatch(QString m_floorCount, int m_currentAuditId);
private slots:

    void on_btnClear_clicked();

    void on_btnSubmit_clicked();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    bool connected;
    bool isLoggedIn;
    int currentComponentId;
    int currentAuditId;
    QString floorCount;
    int userId;
    int currentAuditTypeId;
    QString strCurrentTime;
    QString strCurrentUserId;
    QString strCurrentUserName;
    QString strCurrentAuditId;
    QString strLoc;
    QString strItemCode;
    //QString m_loc;
    QLabel *informationLabel;


};

#endif
