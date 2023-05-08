#ifndef TERAFORM_H
#define TERAFORM_H

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonObject>

#include <QVariantList>
#include <QVariantMap>

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QFrame>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QColorDialog>
#include <QDateTimeEdit>
#include <QTimeEdit>

#include <QFocusEvent>
#include <QAudioDevice>
#include <QCameraDevice>

#include "TeraData.h"
#include "managers/ComManager.h"

namespace Ui {
class TeraForm;
}

class TeraForm : public QWidget
{
    Q_OBJECT

public:
    explicit TeraForm(QWidget *parent = nullptr, ComManager* com_man = nullptr);
    ~TeraForm();


    void buildUiFromStructure(const QString& structure);
    void fillFormFromData(const QJsonObject& data);
    void fillFormFromData(const QString& structure);

    void setSectionsPosition(const QTabWidget::TabPosition &position);

    bool formHasData();
    bool formHasStructure();
    void resetFormValues();
    void setHighlightConditions(const bool& hightlight);

    bool validateFormData(bool include_hidden=false);
    QStringList getInvalidFormDataLabels(bool include_hidden=false);

    QWidget* getWidgetForField(const QString& field);
    bool setFieldValue(const QString& field, const QVariant& value);

    QVariant getFieldValue(const QString& field);
    bool getFieldDirty(const QString& field);
    bool getFieldDirty(QWidget *widget);

    void hideField(const QString& field);
    void hideFields(const QStringList& fields);
    void showField(const QString& field);

    void setFieldRequired(const QString& field, const bool& required);

    void setFieldEnabled(const QString& field, const bool& enabled);
    void setFieldsEnabled(const QStringList& fields, const bool& enabled);

    bool isDirty();

    QString getFormData(bool include_unmodified_data=false);
    QJsonDocument getFormDataJson(bool include_unmodified_data=false);
    TeraData *getFormDataObject(const TeraDataTypes data_type);

    static QColor getGradientColor(const int& lower_thresh, const int& middle_thresh, const int& higher_thresh, const int& value);

    void setComManager(ComManager *com_man);

private:
    Ui::TeraForm*                                   ui;
    QWidget*                                        m_mainWidget; // The main widget that holds the form
    QMap<QString, QWidget*>                         m_widgets;
    QHash<QWidget*, QLabel*>                        m_widgetsLabels;
    QHash<QWidget*, QFormLayout::TakeRowResult>     m_hidden_rows;
    QString                                         m_objectType;
    QVariantMap                                     m_initialValues;
    bool                                            m_highlightConditionals;
    QStringList                                     m_hiddenFields;
    bool                                            m_disabled;

    //QList<QAudioDeviceInfo>                         m_audioInputs;
    //QList<QCameraInfo>                              m_videoInputs;
    QStringList                                     m_audioInputs;
    QStringList                                     m_videoInputs;

    // Hook interface for dynamic url requests
    QMap<QWidget*, TeraDataTypes>                   m_widgetsHookRequests;
    ComManager* m_comManager;

    void buildFormFromStructure(QWidget* page, const QVariantList &structure);
    void setDefaultValues();

    QWidget* createVideoInputsWidget(const QVariantHash& structure);
    QWidget* createAudioInputsWidget(const QVariantHash& structure);
    QWidget* createArrayWidget(const QVariantHash& structure);
    QWidget* createTextWidget(const QVariantHash& structure, bool is_masked);
    QWidget* createBooleanWidget(const QVariantHash& structure);
    QWidget* createNumericWidget(const QVariantHash& structure);
    QWidget* createLabelWidget(const QVariantHash& structure);
    QWidget* createLongLabelWidget(const QVariantHash& structure);
    QWidget* createListWidget(const QVariantHash& structure);
    QWidget* createLongTextWidget(const QVariantHash& structure);
    QWidget* createColorWidget(const QVariantHash& structure);
    QWidget* createDateTimeWidget(const QVariantHash& structure);
    QWidget* createDurationWidget(const QVariantHash& structure);

    void loadAudioInputs();
    void loadVideoInputs();

    void checkConditions(QWidget* item_triggering = nullptr);
    void checkConditionsForItem(QWidget* item, QWidget* item_triggering = nullptr);
    bool hasHookCondition(QWidget* item);
    void setWidgetVisibility(QWidget* widget, QWidget *linked_widget, bool visible);
    bool getWidgetValues(QWidget *widget, QVariant *id, QVariant* value);
    QVariant getWidgetValue(QWidget* widget);
    void setWidgetValue(QWidget* widget, const QVariant& value);
    void setWidgetRequired(QWidget* item_widget, QLabel* item_label, const bool& required);
    void updateWidgetChoices(QWidget* widget, const QList<TeraData> values);
    bool validateWidget(QWidget* widget, bool include_hidden=false);

    static qreal doLinearInterpolation(const qreal &p1, const qreal &p2, const qreal &value);

    bool eventFilter(QObject* object, QEvent* event);

private slots:
    void widgetValueChanged();
    void colorWidgetClicked();
    void longLabelButtonClicked();

    // Hooks
    void hookReplyReceived(TeraDataTypes data_type, QList<TeraData> datas);

public slots:

    void setDisabled(bool disable);
    void setEnabled(bool enable);

signals:
    void widgetValueHasChanged(QWidget* widget, QVariant value);
    void widgetValueHasFocus(QWidget* widget);
    void formIsNowDirty(bool dirty);

};

#endif // TERAFORM_H
