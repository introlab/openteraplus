#ifndef TESTSWIDGET_H
#define TESTSWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QKeyEvent>

#include "managers/ComManager.h"

#include "GlobalMessageBox.h"
#include "TableDateWidgetItem.h"

#include "data/Message.h"
#include "TeraTest.h"
#include "Utils.h"

namespace Ui {
class TestsWidget;
}

class TestsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TestsWidget(ComManager* comMan = nullptr, QWidget *parent = nullptr);
    ~TestsWidget();

    void setComManager(ComManager* comMan);
    void setAssociatedProject(const int &id_project);

    bool isEmpty() const;
    void clearTests();
    void enableNewTests(const bool &enable);

    void displayTestsForSession(const int &id_session);
    void displayTestsForParticipant(const int &id_participant);

private:
    typedef enum {
        VIEWMODE_UNKNOWN,
        VIEWMODE_SESSION,
        VIEWMODE_PARTICIPANT,
        VIEWMODE_PROJECT
    }
    ViewMode;

    typedef enum {
        TEST_NAME_COLUMN=0,
        TEST_SESSION_COLUMN=1,
        TEST_TYPE_COLUMN=2,
        TEST_STATUS_COLUMN=3,
        TEST_DATE_COLUMN=4,
        TEST_OWNER_COLUMN=5}
    TestColumn;

    Ui::TestsWidget *ui;

    ComManager*         m_comManager;

    int                 m_idProject;
    int                 m_idSession;
    ViewMode            m_viewMode;

    QUrlQuery           m_dataQuery;

    QMap<QString, QTableWidgetItem*>    m_tableTests;

    QMap<QString, TeraData*>            m_tests;

    void connectSignals();

    bool eventFilter(QObject* o, QEvent* e);

    void updateTest(const TeraData& test, const int& id_participant = -1, const bool& emit_count_update_signal=true);

    void setViewMode(const ViewMode& new_mode);
    void setLoading(const bool &loading, const QString &msg = QString(), const bool &hide_ui = false);

    void showTestSummary(const QString& test_uuid);

    void updateTestsCountLabel();


private slots:
    void processTestsReply(QList<TeraData> tests, QUrlQuery reply_query);
    void processServicesReply(QList<TeraData> services, QUrlQuery reply_query);

    void comDeleteDataReply(QString path, int id);

    void nextMessageWasShown(Message current_message);

    void on_btnDelete_clicked();

    void on_tableTests_itemSelectionChanged();

signals:
    void testCountChanged(int new_count);

};

#endif // TESTSWIDGET_H
