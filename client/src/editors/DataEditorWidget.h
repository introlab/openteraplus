#ifndef DATAEDITORWIDGET_H
#define DATAEDITORWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>

#include "TeraForm.h"

#include "data/TeraData.h"

#include "managers/ComManager.h"

class DataEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DataEditorWidget(ComManager *comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~DataEditorWidget() override;

    enum EditorState{
        STATE_READY=0,
        STATE_EDITING,
        STATE_WAITING,
        STATE_LOADING
    };

    virtual void setData(const TeraData *data);
    TeraData* getData();

    virtual void saveData(bool signal=true)=0;
    virtual void undoData();
    virtual void deleteData();
    virtual bool dataIsNew();

    virtual void setReady();
    virtual void setEditing();
    virtual void setWaiting();
    virtual void setLoading();

    void setLimited(bool limited);

    bool isReady();
    bool isEditing();
    bool isWaiting();
    bool isLoading();
    bool isWaitingOrLoading();

    void refreshData();

    virtual void disconnectSignals();

    void queryDataRequest(const QString &path, const QUrlQuery &query_args = QUrlQuery());
    bool hasPendingDataRequests();
    void postDataRequest(const QString &path, const QString &query_args);
    void deleteDataRequest(const QString &path, const int &id);

    ComManager* getComManager();

private:
    virtual void updateControlsState()=0;
    virtual void updateFieldsValue()=0;
    virtual bool validateData()=0;

    QString getQueryDataName(const QString &path, const QUrlQuery &query_args);

    QList<QString>  m_requests;

    EditorState     m_editState;

    // Common controls to toggle editing state
    QAbstractButton*    m_editToggle;
    QFrame*         m_frameSave;
    QAbstractButton*    m_saveButton;
    QAbstractButton*    m_cancelButton;
    TeraForm*       m_mainForm;


protected:
    bool            m_undoing;
    bool            m_limited;
    TeraData*       m_data;
    ComManager*     m_comManager;

    QComboBox*      buildRolesComboBox();
    QComboBox*      buildRolesComboBox(const QMap<int, QString> &roles);
    QString         getRoleName(const QString& role);

    virtual void setEditorControls(TeraForm* mainForm, QAbstractButton* editToggle, QFrame* frameSave, QAbstractButton* saveButton, QAbstractButton* cancelButton);

signals:
    void dataWasChanged();

    void stateEditing();
    void stateWaiting();
    void stateReady();
    void stateLoading();

    void closeRequest();

    void dataWasDeleted();
    void dataDisplayRequest(TeraDataTypes data_type, int data_id);

public slots:

    //void setEditing(bool enabled);
    void undoOrDeleteData();

private slots:
    void queryDataReplyOK(const QString &path, const QUrlQuery &query_args);
    void postDataReplyOK(const QString &path);
    void deleteDataReplyOK(const QString &path, const int &id);
    void comDataError(QNetworkReply::NetworkError error, QString error_str, QNetworkAccessManager::Operation op, int status_code, QString path, QUrlQuery url_query);

protected slots:
    virtual void editToggleClicked();
    virtual void saveButtonClicked();
    virtual void undoButtonClicked();

};

#endif // DATAEDITORWIDGET_H
