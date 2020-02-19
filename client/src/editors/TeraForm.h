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

#include <QtMultimedia/QCameraInfo>
#include <QtMultimedia/QAudioDeviceInfo>

#include "TeraData.h"
#include "ComManager.h"

namespace Ui {
class TeraForm;
}

class TeraForm : public QWidget
{
    Q_OBJECT

public:
    explicit TeraForm(QWidget *parent = nullptr);
    ~TeraForm();

    void buildUiFromStructure(const QString& structure);
    void fillFormFromData(const QJsonObject& data);
    void fillFormFromData(const QString& structure);
    bool formHasData();
    void resetFormValues();
    void setHighlightConditions(const bool& hightlight);

    bool validateFormData(bool include_hidden=false);
    QStringList getInvalidFormDataLabels(bool include_hidden=false);

    QWidget* getWidgetForField(const QString& field);
    bool setFieldValue(const QString& field, const QVariant& value);
    QVariant getFieldValue(const QString& field);
    void hideField(const QString& field);

    QString getFormData(bool include_unmodified_data=false);
    QJsonDocument getFormDataJson(bool include_unmodified_data=false);
    TeraData *getFormDataObject(const TeraDataTypes data_type);

    static QColor getGradientColor(const int& lower_thresh, const int& middle_thresh, const int& higher_thresh, const int& value);

private:
    Ui::TeraForm*                                   ui;
    QMap<QString, QWidget*>                         m_widgets;
    QMap<QWidget*, QFormLayout::TakeRowResult>      m_hidden_rows;
    QString                                         m_objectType;
    QVariantMap                                     m_initialValues;
    bool                                            m_highlightConditionals;

    // Hook interface for dynamic url requests
    QMap<QWidget*, TeraDataTypes>                   m_widgetsHookRequests;
    ComManager* m_comManager;

    void buildFormFromStructure(QWidget* page, const QVariantList &structure);
    void setDefaultValues();

    QWidget* createVideoInputsWidget(const QVariantMap& structure);
    QWidget* createAudioInputsWidget(const QVariantMap& structure);
    QWidget* createArrayWidget(const QVariantMap& structure);
    QWidget* createTextWidget(const QVariantMap& structure, bool is_masked);
    QWidget* createBooleanWidget(const QVariantMap& structure);
    QWidget* createNumericWidget(const QVariantMap& structure);
    QWidget* createLabelWidget(const QVariantMap& structure);
    QWidget* createListWidget(const QVariantMap& structure);
    QWidget* createLongTextWidget(const QVariantMap& structure);
    QWidget* createColorWidget(const QVariantMap& structure);
    QWidget* createDateTimeWidget(const QVariantMap& structure);
    QWidget* createDurationWidget(const QVariantMap& structure);

    void checkConditions(QWidget* item_triggering = nullptr);
    void checkConditionsForItem(QWidget* item, QWidget* item_triggering = nullptr);
    void setWidgetVisibility(QWidget* widget, QWidget *linked_widget, bool visible);
    void getWidgetValues(QWidget *widget, QVariant *id, QVariant* value);
    QVariant getWidgetValue(QWidget* widget);
    void setWidgetValue(QWidget* widget, const QVariant& value);

    void updateWidgetChoices(QWidget* widget, const QList<TeraData> values);

    bool validateWidget(QWidget* widget, bool include_hidden=false);

    static qreal doLinearInterpolation(const qreal &p1, const qreal &p2, const qreal &value);

private slots:
    void widgetValueChanged();
    void colorWidgetClicked();

    // Hooks
    void hookReplyReceived(TeraDataTypes data_type, QList<TeraData> datas);

signals:
    void widgetValueHasChanged(QWidget* widget, QVariant value);

};

#endif // TERAFORM_H
