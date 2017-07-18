
#include "mainwindow.h"
#include "ui_mainwindow.h"

//#define MESSAGE
//    Dialog::tr("<p>Message boxes have a caption, a text, "
//               "and any number of buttons, each with standard or custom texts."
//               "<p>Click a button to close the message box. Pressing the Esc button "
//               "will activate the detected escape button (if any).")
//#define MESSAGE_DETAILS
//    Dialog::tr("If a message box has detailed text, the user can reveal it "
//               "by pressing the Show Details... button.")


// TODO
/*
//steps
//NA this is in C#, will need windows service or...: import data from excel and insert into components table
//redo in PHP TODO initialize audits table for cycle 1
//done distribute each record that matches audits for cycle1         done //next move to seperate class
//user counts cycle1                                            user function
//done app updates db                                                done //next move to seperate class
//done ***initialize for cycle2 if !match
//done ***Get next item to audit
//done INSERT for cycle2
//TODO ***distribute to different user for cycle2
//user counts cycle2
//TODO show when no other audits are available
//TODO cycle 3
//TODO prioritize by Location 1st 2 letters
//TODO cycle2 assigned to diff user than cycle1 for that audit item
//done validate count input is + double
//NA tab with all audits; Jeff said not to do this
*/

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    //ui->tabList->setVisible(false);
    //ui->btnSkip->setVisible(false);
    //TEMPS
    userId = 1;
    strCurrentUserId = "1";
    strCurrentUserName = "Test User";
    //    currentAuditTypeId = 0;
    //    currentAuditId = 0;
    //    strCurrentAuditId = "";
    //    strCurrentTime = "";

    ui->lblUserId->setText(strCurrentUserId);
    ui->lblUserName->setText(strCurrentUserName);
    numEntries = 0;
    isMatched = 0;
    GetNextItem();
}

void MainWindow::GetNextItem()
{
    ClearFormItem();
    currentAuditId = 0;
    numEntries = 0;
    isMatched = 0;

    if (!cycleCountDb.open())
    {
        connOpen();
    }

    QSqlQuery query;
    //test, else below: if no cycle1's get cycle2's
    //TODO do query count on quantity of cycle1's ; and if no records get cycle2, ...

    if (query.exec("SELECT components.componentId, components.itemCode, components.location, components.description, audits.auditId "
                   "FROM components INNER JOIN audits ON audits.componentId = components.componentId WHERE audits.assigned = 0 "
                   "AND audits.auditTypeId = 1 LIMIT 1"))
    {
        while (query.next())
        {

            qDebug() << query.value(0) << query.value(1) << query.value(2) << query.value(3)<<query.value(4);

            currentComponentId = query.value(0).toInt();
            ui->lblItemCode->setText(query.value(1).toString());
            ui->lblLocation->setText(query.value(2).toString());
            ui->lblDescription->setText(query.value(3).toString());
            strCurrentAuditId = query.value(4).toString();
            currentAuditId = query.value(4).toInt();
            currentAuditTypeId = 1;

            //TODO cleaner way to do this
            strItemCode = (query.value(1).toString());
            strLoc = (query.value(2).toString());
        }

        AssignAuditItem(currentAuditId);
    }
    else
    {   //get cycle2 if no cycle1's
        //TODO add logic so that cycle2 is a different user than cycle1
        if (query.exec("SELECT components.componentId, components.itemCode, components.location, components.description, audits.auditId "
                       "FROM components INNER JOIN audits ON audits.componentId = components.componentId WHERE audits.assigned = 0 "
                       "AND audits.auditTypeId = 2 "
                       "LIMIT 1"))
            //AND userId != ? currentUserId  change to query.prepare
        {

            while (query.next())
            {
                currentComponentId = query.value(0).toInt();
                ui->lblItemCode->setText(query.value(1).toString());
                ui->lblLocation->setText(query.value(2).toString());
                ui->lblDescription->setText(query.value(3).toString());
                currentAuditId = query.value(4).toInt();
                currentAuditTypeId = 2;
            }
            AssignAuditItem(currentAuditId);
        }
    }
}
void MainWindow::AssignAuditItem(int m_currentAuditId)
{
    if (cycleCountDb.open())
    {
        QSqlQuery query;
        query.prepare("UPDATE audits SET assigned = 1, auditTypeId = ?, userId = ? "
                      "WHERE auditId = ?");

        query.addBindValue(userId);
        query.addBindValue(currentAuditTypeId);
        query.addBindValue(m_currentAuditId);

        qDebug()<<"userId: "<<userId<<", currentAuditTypeId: "<<currentAuditTypeId<<", currentAuditId: "<<m_currentAuditId;


        if(query.exec())
        {
            qDebug() << "assigned item to user: " << userId;
        }
        else
        {
            qDebug() << "assignment update failed";
        }
    }
    else
    {
        qDebug() << "AssignAuditItem() Database not open";
    }

}

void MainWindow::on_btnSubmit_clicked()
{
    //void MainWindow::SubmitData(QString m_floorCount, int m_currentAuditId, QString m_loc, QString m_item)

    QString m_loc = ui->lblLocation->text();
    QString m_item = ui->lblItemCode->text();
    floorCount = ui->txtQuantity->text();

    //validate count is double
    try
    {
        double value = std::stod(floorCount.toStdString());

        qDebug() << "string value as double: " << value;
        //TODO done but test after entering nan check for match count

        if(value>=0)
        {
            if (numEntries == 0)
            {
                if(CheckMatch(value, m_loc, m_item))
                    SubmitDataMatched(value, currentAuditId);
                else //give error msgBox
                {
                    QMessageBox messageBox;
                    messageBox.critical(0,"Error","Count Did Not Match Server - Please Recount");
                    messageBox.setFixedSize(500,300);
                    messageBox.show();

                    numEntries ++;
                    ui->txtQuantity->setText("");
                    ui->txtQuantity->setFocus();

                    //SubmitDataNoMatch(value, currentAuditId);
                }
            }
            else
                SubmitDataNoMatch(value, currentAuditId);
        }
        else
        {
            ErrorMessageBox("Quantity Must Be 0 Or Greater !");
            ui->txtQuantity->setText("");
            ui->txtQuantity->setFocus();
        }
    }
    catch(std::exception& e)
    {
        qDebug() << "Could not convert string to double";

        ErrorMessageBox("Quantity Must Be A Number !");
        numEntries = 0;

        ui->txtQuantity->setText("");
        ui->txtQuantity->setFocus();
    }
}


bool MainWindow::CheckMatch(int m_floorCount, QString m_loc, QString m_item)
{
    //TODO when able to get live data convert this to diff source; and verify loc and item are paramaters used in as400
    //TODO handle if no results from query
    //TODO need to check if duplicate entries for same item and same location; for 4Wall need to accumulate same items???

    bool isMatched = false;
    int as400Count = -1;

    QSqlQuery queryServerItemQuantity;
    queryServerItemQuantity.prepare("SELECT quantity FROM as400_data WHERE Loc = ? AND Item = ?");
    queryServerItemQuantity.addBindValue(m_loc);
    queryServerItemQuantity.addBindValue(m_item);

    queryServerItemQuantity.exec();

    while (queryServerItemQuantity.next())
    {
        //TODO check if more than 1 result is found
        as400Count = queryServerItemQuantity.value(0).toInt();

        if(as400Count == m_floorCount)
        {
            isMatched = true;
            return isMatched;
        }
        else
        {
            isMatched = false;
            return isMatched;
        }
    }
    return isMatched;
}

void MainWindow::SubmitDataMatched(double value, int currentAuditId)
{
    strCurrentTime = QDateTime::currentDateTime().toString("MM:dd:yyyy-hh:mm:ss");

    //update as matched
    QSqlQuery isMatchedQuery;
    isMatchedQuery.prepare("UPDATE audits SET completed = 1, matched = 1, timestamp = ?, count = ? "
                           "WHERE auditId = ?");

    isMatchedQuery.addBindValue(strCurrentTime);
    isMatchedQuery.addBindValue(value);
    isMatchedQuery.addBindValue(currentAuditId);

    if(isMatchedQuery.exec())
    {
        ui->txtMessageBox->append("Count Uploaded for Item: " + ui->lblItemCode->text() + " " + strCurrentTime);
        //numEntries = 0;
        GetNextItem();
    }
    else
    {
        //TODO record this to error file so update can be done manually
        qDebug()<< "isMatchedQuery failed for auditId: " <<currentAuditId << "count value: "<<value;
    }

    //if any cycle1 get c1, else if any c2 get c2, else messagebox no items remain



    /*
            if numEntries = 0
            {

                if match = 1
                    update match = 1, completed = 1
                else //match = 0
                    update set match = 0, completed = 1
            }
            else
            {
                show error
                numEntries ++;
                getCountData()
*/
    /*
            QSqlQuery isMatchedQuery;
            isMatchedQuery.prepare("UPDATE audits SET completed = 1, matched = 1, timestamp = :timeStamp, count = :count "
                                   "WHERE auditId = :auditId");

            isMatchedQuery.bindValue(":timeStamp", QDateTime::currentDateTime().toString("MM:dd:yyyy-hh:mm:ss"));
            isMatchedQuery.bindValue(":count", m_floorCount);
            isMatchedQuery.bindValue(":auditid", m_currentAuditId);
*/


    /*
    //check if matches as400 quantity
    if(CheckMatch(m_floorCount.toInt(), strLoc, strItemCode))
    {
        //update if isMatched = true

        if(isMatched)
        {
            QSqlQuery isMatchedQuery;
            isMatchedQuery.prepare("UPDATE audits SET completed = 1, matched = 1, timestamp = :timeStamp, count = :count "
                                   "WHERE auditId = :auditId");

            isMatchedQuery.bindValue(":timeStamp", QDateTime::currentDateTime().toString("MM:dd:yyyy-hh:mm:ss"));
            isMatchedQuery.bindValue(":count", m_floorCount);
            isMatchedQuery.bindValue(":auditid", m_currentAuditId);

            //        qDebug()<<m_floorCount<<"**********"<<m_currentAuditId;
            if(isMatchedQuery.exec())
            {
                numEntries = 0;
                qDebug()<<"query UPDATE line 264 successful";
                qDebug() << "time: " << strCurrentTime << "count: " << m_floorCount << "audit id" << currentAuditId;
            }
            else
            {
                qDebug()<<"query line 264 failed";
            }
        }


    }
    else
    {
        //only want to warn on first noMatch otherwise let it go
        if(numEntries = 0)  //TODO reset this
        {

            ErrorMessageBox("Count Did Not Match - Verify Count and Reenter");
            numEntries += 1;
            isMatched = false;
            ui->txtQuantity->setText("");
            ui->txtQuantity->setFocus();
            //need to get 2nd read before going to this
            //set isMatched bool at class level and use it to determine UPDATE query
            //UpdateNoMatch(m_floorCount, m_currentAuditId);
        }
        else
        {
            //calling itself
            //SubmitData();
            numEntries ++;
            //call insert noMatched
            SubmitData(m_floorCount, m_currentAuditId);
            //UpdateNoMatch(m_floorCount, m_currentAuditId);

            //numEntries = 0;
        }
    }
    */
}

void MainWindow::SubmitDataNoMatch(double value, int currentAuditId)
{
    //  if(numEntries == 1 && isMatched == false)
    //update as no match
    strCurrentTime = QDateTime::currentDateTime().toString("MM:dd:yyyy-hh:mm:ss");
    QSqlQuery noMatchedQuery;
    noMatchedQuery.prepare("UPDATE audits SET completed = 1, matched = 0, timestamp = ?, count = ? "
                           "WHERE auditId = ?");

    noMatchedQuery.addBindValue(strCurrentTime);
    noMatchedQuery.addBindValue(value);
    noMatchedQuery.addBindValue(currentAuditId);

    if(noMatchedQuery.exec())
    {
        ui->txtMessageBox->append("Count Uploaded for Item: " + ui->lblItemCode->text() + " " + strCurrentTime);
        QSqlQuery queryInsert;
        queryInsert.prepare("INSERT INTO audits(componentId, auditTypeId, assigned) VALUES (?,2,0)");
        queryInsert.addBindValue(currentComponentId);
        if(queryInsert.exec())
        {
            qDebug() << "No Match insert succeded";
        }
        else
        {
            qDebug()<< "updateNoMatchQuery INSERT failed ";
        }
        GetNextItem();
    }
    /*
//    if(updateNoMatchQuery.exec())
//    {
//        QSqlQuery queryInsert;
//        queryInsert.prepare("INSERT INTO audits(componentId, auditTypeId, assigned) VALUES (?,2,0)");
//        queryInsert.addBindValue(currentComponentId);
//        if(queryInsert.exec())
//        {
//            qDebug() << "No Match insert succeded";
//        }
//        else
//        {
//            qDebug()<< "updateNoMatchQuery INSERT failed ";
//        }

//    }
*/
    else
    {
        //TODO record this to error file so update can be done manually; and no cycle2 created
        qDebug()<< "noMatchUpdate failed for auditId: " <<currentAuditId << "count value: "<<value;
    }
}

//void MainWindow::UpdateNoMatch(QString m_floorCount, int m_currentAuditId)
//{
//    QSqlQuery updateNoMatchQuery;
//    updateNoMatchQuery.prepare("UPDATE audits SET assigned = 0, completed = 1, matched = 0, timestamp = ?, count = ? WHERE auditId = ?");
//    updateNoMatchQuery.addBindValue(strCurrentTime);
//    updateNoMatchQuery.addBindValue(m_floorCount);
//    updateNoMatchQuery.addBindValue(m_currentAuditId);

//    if(updateNoMatchQuery.exec())
//    {
//        QSqlQuery queryInsert;
//        queryInsert.prepare("INSERT INTO audits(componentId, auditTypeId, assigned) VALUES (?,2,0)");
//        queryInsert.addBindValue(currentComponentId);
//        if(queryInsert.exec())
//        {
//            qDebug() << "No Match insert succeded";
//        }
//        else
//        {
//            qDebug()<< "updateNoMatchQuery INSERT failed ";
//        }

//    }
//    GetNextItem();
//}

void MainWindow::ClearFormItem()
{
    ui->lblItemCode->setText("");
    ui->lblLocation->setText("");
    ui->lblDescription->setText("");
    ui->txtQuantity->setText("");
}

void MainWindow::ErrorMessageBox(QString m_message)
{
    QMessageBox messageBox;
    messageBox.critical(0,"Error",m_message);
    messageBox.setFixedSize(500,300);
    messageBox.show();

    numEntries ++;
    ui->txtQuantity->setText("");
    ui->txtQuantity->setFocus();
}

bool MainWindow::connOpen()
{
    cycleCountDb = QSqlDatabase::addDatabase("QMYSQL");
    cycleCountDb.setHostName("127.0.0.1");
    cycleCountDb.setUserName("rwtom");
    cycleCountDb.setPassword("123456");
    cycleCountDb.setDatabaseName("cyclecounts2");

    if(!cycleCountDb.open())
    {
        qDebug()<<"failed to open DB ";
        return false;
    }
    else
    {
        qDebug()<<"connected";
        return true;
    }
}


void MainWindow::connClose()
{
    cycleCountDb.close();
    cycleCountDb.removeDatabase(QSqlDatabase::defaultConnection);
}

void MainWindow::on_btnClear_clicked()
{
    ui->txtMessageBox->clear();
}

MainWindow::~MainWindow()
{
    if(cycleCountDb.open())
        connClose();
    delete ui;
}
