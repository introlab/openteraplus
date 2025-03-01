#include "TeraForm.h"
#include "ui_TeraForm.h"

#include "Logger.h"
#include "GlobalMessageBox.h"
#include "libs/AudioVideoUtils.h"

#include <QDebug>
#include <QStyledItemDelegate>

TeraForm::TeraForm(QWidget *parent, ComManager *com_man) :
    QWidget(parent),
    ui(new Ui::TeraForm)
{
    ui->setupUi(this);

    m_mainWidget = ui->toolboxMain;
    m_highlightConditionals = false;

    // Automatically sets comManager
    setComManager(com_man);
    //}
}

TeraForm::~TeraForm()
{
    /*if (m_mainWidget != ui->toolboxMain)
        m_mainWidget->deleteLater();*/
    delete ui;

}

void TeraForm::buildUiFromStructure(const QString &structure)
{
    m_widgets.clear();
    QJsonParseError json_error;

    QJsonDocument struct_info = QJsonDocument::fromJson(structure.toUtf8(), &json_error);
    if (json_error.error!= QJsonParseError::NoError){
        LOG_ERROR("Unable to parse Ui structure: " + json_error.errorString(), "TeraForm::buildUiFromStructure");
    }

    while (ui->toolboxMain->count() > 0){
        ui->toolboxMain->widget(0)->deleteLater();
        //ui->toolboxMain->removeItem(0);
        ui->toolboxMain->removeTab(0);
    }
    ui->toolboxMain->show();

    QJsonObject struct_object = struct_info.object();
    //qDebug() << struct_info.object().keys();
    if (struct_object.contains("objecttype"))
        m_objectType = struct_object["objecttype"].toString();

    // Sections
    if (struct_object.contains("sections")){
        QVariantList struct_data =struct_object["sections"].toArray().toVariantList();
        if (struct_data.count() > 1){
            int page_index = 0;
            m_mainWidget = ui->toolboxMain;
            for (const QVariant &section:std::as_const(struct_data)){
                if (section.canConvert<QVariantHash>()){
                    QVariantHash section_data = section.toHash();
                    //if (page_index>0){
                        QWidget* new_page = new QWidget(ui->toolboxMain);
                        new_page->setObjectName("pageSection" + QString::number(page_index+1));
                        new_page->setStyleSheet("QWidget#" + new_page->objectName() + "{border: 1px solid white; border-radius: 5px;}");
                        //ui->toolboxMain->addItem(new_page,"");
                        ui->toolboxMain->addTab(new_page, section_data["label"].toString());
                    //}
                    //ui->toolboxMain->setItemText(page_index, section_data["label"].toString());
                    if (section_data.contains("items")){
                        if (section_data["items"].canConvert<QVariantList>()){
                            buildFormFromStructure(ui->toolboxMain->widget(page_index), section_data["items"].toList());
                        }
                    }
                    page_index++;
                }
            }
        }else{
            if (struct_data.count() == 1){
                // Only one section - hides the header and don't use QToolBox
                ui->mainLayout->removeWidget(ui->toolboxMain);
                ui->toolboxMain->hide();
                m_mainWidget = new QFrame(this);
                ui->mainLayout->addWidget(m_mainWidget);
                QVariantHash section_data = struct_data.first().toHash();
                if (section_data.contains("items")){
                    if (section_data["items"].canConvert<QVariantList>()){
                        buildFormFromStructure(m_mainWidget, section_data["items"].toList());
                    }
                }
            }
        }
    }

}

void TeraForm::fillFormFromData(const QJsonObject &data)
{

    m_initialValues.clear();
    // Check if the object we need is part of that object or not.
    if (data.contains(m_objectType)){
        // Found it - extract data from it.
        m_initialValues = data[m_objectType].toObject().toVariantMap();
    }else{
        // If not, we suppose it is the object itself!
        m_initialValues = data.toVariantMap();
    }

    resetFormValues();

    // Set initial values for missing fields
    for (QWidget* widget:std::as_const(m_widgets)){
        QString field = m_widgets.key(widget);
        if (!m_initialValues.contains(field)){
            QVariant value;
            getWidgetValues(widget, nullptr, &value);
            m_initialValues.insert(field, value);
        }
    }
    // qDebug() << m_initialValues;

    validateFormData(false);
    emit formIsNowDirty(false);

}

void TeraForm::fillFormFromData(const QString &structure)
{
    QJsonDocument struct_info;
    QJsonParseError json_error;
    if (!structure.isEmpty()){
        struct_info = QJsonDocument::fromJson(structure.toUtf8(), &json_error);
    }else{
        struct_info = QJsonDocument::fromJson("{}", &json_error);
    }

    if (json_error.error!= QJsonParseError::NoError){
        LOG_ERROR("Unable to parse Ui structure: " + json_error.errorString(), "TeraForm::fillFormFromData");
    }

    QJsonObject struct_object = struct_info.object();
    fillFormFromData(struct_object);

}

void TeraForm::setSectionsPosition(const QTabWidget::TabPosition &position)
{
    if (dynamic_cast<QTabWidget*>(ui->toolboxMain)){
        ui->toolboxMain->setTabPosition(position);
    }
}

bool TeraForm::validateFormData(bool include_hidden)
{
    bool rval = true;
    for (QWidget* item:std::as_const(m_widgets)){
       rval &= validateWidget(item, include_hidden);
    }
    return rval;
}

QStringList TeraForm::getInvalidFormDataLabels(bool include_hidden)
{
    QStringList rval;
    for (QWidget* item:std::as_const(m_widgets)){
        if (!validateWidget(item, include_hidden)){
            rval.append(item->property("label").toString());
        }
    }
    return rval;
}

QWidget *TeraForm::getWidgetForField(const QString &field)
{
    if (m_widgets.contains(field))
        return m_widgets[field];

    return nullptr;
}

bool TeraForm::setFieldValue(const QString &field, const QVariant &value)
{
    bool rval = false;

    if (m_widgets.contains(field)){
        setWidgetValue(m_widgets[field], value);
        rval = true;
    }

    return rval;
}

QVariant TeraForm::getFieldValue(const QString &field)
{
    QVariant rval;

    if (m_widgets.contains(field)){
        getWidgetValues(m_widgets[field], nullptr, &rval);
    }
    return rval;
}

bool TeraForm::getFieldDirty(const QString &field)
{
    if (m_widgets.contains(field)){
        return getFieldDirty(m_widgets[field]);
    }
    return false;
}

bool TeraForm::getFieldDirty(QWidget *widget)
{
    // Read only fields are never dirty
    if (widget->property("readonly").isValid()){
        if (widget->property("readonly").toBool()){
            //qDebug() << "Readonly widget never dirty!";
            return false;
        }
    }

    if (std::find(m_widgets.cbegin(), m_widgets.cend(), widget) != m_widgets.cend()){

        QString widget_id = m_widgets.key(widget);
        if (dynamic_cast<QLabel*>(widget)){
            return false; // QLabel are never dirty
        }
        QVariant value, id;
        getWidgetValues(widget, &id, &value);
        if (!id.isNull() && id.isValid())
            value = id;

        if (dynamic_cast<QDateTimeEdit*>(widget)){
            // Datetime must be checked as string as they are stored as such...
            value = value.toDateTime().toString(Qt::DateFormat::ISODateWithMs);
            if (m_initialValues.contains(widget_id)){
                return !m_initialValues[m_widgets.key(widget)].toString().startsWith(value.toString());
            }
        }

        if (m_initialValues.contains(widget_id))
            return m_initialValues[m_widgets.key(widget)] != value;

        // No initial value. So dirty if not empty!
        return !value.toString().isEmpty();
    }
    return false;
}

void TeraForm::hideField(const QString &field)
{
    QWidget* widget = getWidgetForField(field);
    if (widget){
        setWidgetVisibility(widget, nullptr, false);
        checkConditions(widget);
        // Disable condition
        /*if (widget->property("condition").isValid() && !hasHookCondition(widget)){
            widget->setProperty("_condition", widget->property("condition"));
            widget->setProperty("condition", QVariant());
        }*/
    }
}

void TeraForm::showField(const QString &field)
{
    QWidget* widget = getWidgetForField(field);
    if (widget){
        setWidgetVisibility(widget, nullptr, true);
        // Enable condition
        /*if (widget->property("_condition").isValid()){
            widget->setProperty("condition", widget->property("_condition"));
            widget->setProperty("_condition", QVariant());
        }*/
        checkConditions(widget);
    }
}

void TeraForm::hideFields(const QStringList &fields)
{
    // Hides the fields if already hidden
    foreach(QString field, fields){
        hideField(field);
    }

    // Stores the list if those fields appear later
    m_hiddenFields = fields;
}

void TeraForm::setFieldRequired(const QString &field, const bool &required)
{
    QWidget* widget = getWidgetForField(field);
    if (widget){
        QLabel* widget_label = m_widgetsLabels[widget];
        setWidgetRequired(widget, widget_label, required);
    }

}

void TeraForm::setFieldEnabled(const QString &field, const bool &enabled)
{
    QWidget* widget = getWidgetForField(field);
    if (widget){
        widget->setEnabled(enabled);
    }
}

void TeraForm::setFieldsEnabled(const QStringList &fields, const bool &enabled)
{
    for(const QString &field:fields){
        setFieldEnabled(field, enabled);
    }
}

bool TeraForm::isDirty()
{
    bool dirty = false;
    for(QWidget* wdg: std::as_const(m_widgets)){
        if (getFieldDirty(wdg)){
            dirty = true;
            break;
        }
    }
    return dirty;
}

QString TeraForm::getFormData(bool include_unmodified_data)
{
    QString data;
    QJsonDocument document = getFormDataJson(include_unmodified_data);
    data = document.toJson();
    return data;
}

QJsonDocument TeraForm::getFormDataJson(bool include_unmodified_data)
{
    QJsonDocument document;
    QJsonObject data_obj;
    QJsonObject base_obj;

    for(QWidget* wdg:std::as_const(m_widgets)){
        QString field = m_widgets.key(wdg);
        QVariant value, id;
        getWidgetValues(wdg, &id, &value);
        if (!id.isNull() && id.isValid())
            value = id;
        // Include only modified fields or ids
        if ((!include_unmodified_data && getFieldDirty(field))
                || field.startsWith("id_") || include_unmodified_data){
            QJsonValue json_value = QJsonValue::fromVariant(value);
            if (field.startsWith("id_")){
                // Force value into int
                json_value = QJsonValue::fromVariant(value.toInt());
            }
            data_obj.insert(field, json_value);
        }
    }

    if (!data_obj.isEmpty()){
        base_obj.insert(m_objectType, data_obj);
        document.setObject(base_obj);
    }

    return document;
}

TeraData* TeraForm::getFormDataObject(const TeraDataTypes data_type)
{
    TeraData* rval = new TeraData(data_type);
    for(QWidget* wdg:std::as_const(m_widgets)){
        QString field = m_widgets.key(wdg);
        QVariant value, id;
        if (getWidgetValues(wdg, &id, &value)){
            if (!id.isNull() && id.isValid())
                value = id;
            rval->setFieldValue(field, value);
        }
    }
    return rval;
}

QColor TeraForm::getGradientColor(const int &lower_thresh, const int &middle_thresh, const int &higher_thresh, const int &value)
{
    QColor lower_color = QColor(Qt::green).toHsv();
    QColor middle_color = QColor(213, 197, 0); //QColor(232, 97, 0).toHsv(); // orange
    QColor higger_color = QColor(123, 100, 6); //QColor(Qt::red).toHsv();

    QColor grad_color;
    qreal hue;
    qreal sat;
    qreal val;

    int current_value = value;

    if (current_value < lower_thresh)
        current_value = lower_thresh;
    if (current_value > higher_thresh)
        current_value = higher_thresh;

    if (current_value <= middle_thresh){
        qreal ratio = static_cast<qreal>(current_value-lower_thresh) / middle_thresh;
        hue = doLinearInterpolation(lower_color.hueF(), middle_color.hueF(), ratio);
        sat = doLinearInterpolation(lower_color.saturationF(), middle_color.saturationF(), ratio);
        val = doLinearInterpolation(lower_color.valueF(), middle_color.valueF(), ratio);
    }else{
        qreal ratio = static_cast<qreal>(current_value-middle_thresh) / static_cast<qreal>(higher_thresh-middle_thresh);
        hue = doLinearInterpolation(middle_color.hueF(), higger_color.hueF(), ratio);
        sat = doLinearInterpolation(middle_color.saturationF(), higger_color.saturationF(), ratio);
        val = doLinearInterpolation(middle_color.valueF(), higger_color.valueF(), ratio);
    }

    grad_color.setHsvF(hue, sat, val);

    return grad_color;

}

void TeraForm::setComManager(ComManager *com_man)
{
    m_comManager = com_man;
    if (m_comManager){
        connect(m_comManager, &ComManager::dataReceived, this, &TeraForm::hookReplyReceived);
    }
}


bool TeraForm::formHasData()
{
    return !m_initialValues.isEmpty();
}

bool TeraForm::formHasStructure()
{
    return !m_widgets.isEmpty();
}

void TeraForm::resetFormValues()
{
    QStringList keys = m_initialValues.keys();
    for (const QString& field:std::as_const(keys)){
        if (m_widgets.contains(field)){
            setWidgetValue(m_widgets[field], m_initialValues[field]);
            checkConditions(m_widgets[field]);
        } else{
           //LOG_WARNING("No widget for field: " + field, "TeraForm::resetFormValues");
        }
    }

}

void TeraForm::setHighlightConditions(const bool &hightlight)
{
    m_highlightConditionals = hightlight;
}

void TeraForm::buildFormFromStructure(QWidget *page, const QVariantList &structure)
{
    QFormLayout* layout;
    if (!page->layout()){
        layout = new QFormLayout(page);
        //layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
        //layout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
        layout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
        layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        layout->setVerticalSpacing(3);

    }else{
        layout = static_cast<QFormLayout*>(page->layout());
    }

    for (const QVariant &item:structure){
        if (item.canConvert<QVariantHash>()){
            QVariantHash item_data = item.toHash();
            QString item_id = item_data["id"].toString();
            QWidget* item_widget = nullptr;
            QLabel* item_label = new QLabel(item_data["label"].toString());
            item_label->setAlignment(Qt::AlignVCenter);
            item_label->setStyleSheet("QLabel{color: #b6ddf8;}");
            //item_label->setStyleSheet("QLabel{min-height: 25px;}");

            // Build widget according to item type
            QString item_type = item_data["type"].toString().toLower();
            if (item_type == "videoinputs"){
                item_widget = createVideoInputsWidget(item_data);
            }
            else if (item_type == "audioinputs"){
                item_widget = createAudioInputsWidget(item_data);
            }
            else if (item_type == "array"){
                item_widget = createArrayWidget(item_data);
            }
            else if (item_type == "text"){
                item_widget = createTextWidget(item_data, false);
            }
            else if (item_type == "password"){
                item_widget = createTextWidget(item_data, true);
            }
            else if (item_type == "boolean"){
                item_widget = createBooleanWidget(item_data);
            }
            else if (item_type == "numeric"){
                item_widget = createNumericWidget(item_data);
            }
            else if (item_type == "hidden"){
                item_widget = createLabelWidget(item_data);
            }
            else if (item_type == "checklist"){
                item_widget = createListWidget(item_data);
            }
            else if (item_type == "longtext" || item_type == "json"){ // Json here, for now...
                item_widget = createLongTextWidget(item_data);
            }
            else if (item_type == "label"){
                item_widget = createLabelWidget(item_data);
            }
            else if (item_type == "longlabel"){
                item_widget = createLongLabelWidget(item_data);
            }
            else if (item_type == "color"){
                item_widget = createColorWidget(item_data);
            }
            else if (item_type == "datetime"){
                item_widget = createDateTimeWidget(item_data);
            }
            else if (item_type == "duration"){
                item_widget = createDurationWidget(item_data);
            }


            if (item_widget){
                // Set widget properties
                if (item_data.contains("_order"))
                    item_widget->setProperty("_order", item_data["_order"].toInt());
                if (item_data.contains("label"))
                    item_widget->setProperty("label", item_data["label"].toString());
                if (item_data.contains("id"))
                    item_widget->setProperty("id", item_id);

                // Required?
                setWidgetRequired(item_widget, item_label, item_data.contains("required"));

                if (item_data.contains("condition")){
                    item_widget->setProperty("condition", item_data["condition"]);
                    if (m_highlightConditionals)
                        item_label->setStyleSheet("background-color:rgb(70,70,70);");
                }
                if (item_data.contains("readonly")){
                    item_widget->setProperty("readonly", item_data["readonly"].toBool());
                    item_widget->setDisabled(item_data["readonly"].toBool());
                }
                item_widget->setProperty("item_type", item_type);

                item_widget->setMinimumHeight(25);

                // Add widget to layout
                layout->addRow(item_label, item_widget);

                // Add widget to list
                m_widgets[item_data["id"].toString()] = item_widget;
                m_widgetsLabels[item_widget] = item_label;

                // Remove row if hidden
                if (item_type == "hidden"){
                    setWidgetVisibility(item_widget, nullptr, false);
                }

                // Check if is in "hidden fields" list
                if (m_hiddenFields.contains(item_id)){
                    hideField(item_id);
                }

                // Install event filter if needed to process some specific events
                item_widget->installEventFilter(this);

            }else{
                LOG_WARNING("Unknown item type: " + item_type, "TeraForm::buildFormFromStructure");
                item_label->deleteLater();
            }
        }
    }

    // Set layout alignement
    layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    layout->update();
    page->setLayout(layout);

    // Set default values
    setDefaultValues();
    checkConditions();
    validateFormData(true);

    page->setDisabled(m_disabled);
    updateRequiredWidgetsLabel();

}

void TeraForm::setDefaultValues()
{
    for (QWidget* item:std::as_const(m_widgets)){
        if (QComboBox* combo = dynamic_cast<QComboBox*>(item)){
            combo->setCurrentIndex(0);
        }
    }
}

QWidget *TeraForm::createVideoInputsWidget(const QVariantHash &structure)
{
    Q_UNUSED(structure)
    QComboBox* item_combo = new QComboBox();
    item_combo->setItemDelegate(new QStyledItemDelegate(item_combo));

    // Add empty item
    item_combo->addItem("", "");

    // Query webcams on the system
    if (m_videoInputs.isEmpty())
        loadVideoInputs();
    /*for (QCameraInfo camera:m_videoInputs){
        //item_combo->addItem(camera.description(), camera.deviceName());
        item_combo->addItem(camera.description(), camera.description());
    }*/
    for (const QString &camera:std::as_const(m_videoInputs)){
        item_combo->addItem(camera, camera);
    }

    // Using old-style connect since SLOT has less parameter and not working with new-style connect
    connect(item_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(widgetValueChanged()));

    return item_combo;
}

QWidget *TeraForm::createAudioInputsWidget(const QVariantHash &structure)
{
    Q_UNUSED(structure)
    QComboBox* item_combo = new QComboBox();
    item_combo->setItemDelegate(new QStyledItemDelegate(item_combo));

    // Add empty item
    item_combo->addItem("", "");

    // Query audio inputs on the system
    if (m_audioInputs.isEmpty())
        loadAudioInputs();
    /*for (QAudioDeviceInfo input:m_audioInputs){
        item_combo->addItem(input.deviceName(), input.deviceName());
    }*/
    for (const QString &input:std::as_const(m_audioInputs)){
        item_combo->addItem(input, input);
    }

    // Using old-style connect since SLOT has less parameter and not working with new-style connect
    connect(item_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(widgetValueChanged()));

    return item_combo;
}

QWidget *TeraForm::createArrayWidget(const QVariantHash &structure)
{
    QComboBox* item_combo = new QComboBox();
    item_combo->setItemDelegate(new QStyledItemDelegate(item_combo));

    // Add empty item
    item_combo->addItem("", "");

    if (structure.contains("values")){
        if (structure["values"].canConvert<QVariantList>()){
            QVariantList list = structure["values"].toList();
            for (const QVariant &value:std::as_const(list)){
                if (value.canConvert<QVariantHash>()){
                    QVariantHash item_data = value.toHash();
                    item_combo->addItem(item_data["value"].toString(), item_data["id"].toString());
                }
            }
        }

    }

    // Using old-style connect since SLOT has less parameter and not working with new-style connect
    connect(item_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(widgetValueChanged()));

    // Hide combo box if empty
    if (item_combo->count() == 1){
        hideField(structure["id"].toString());
    }

    return item_combo;
}

QWidget *TeraForm::createTextWidget(const QVariantHash &structure, bool is_masked)
{
    Q_UNUSED(structure)
    QLineEdit* item_text = new QLineEdit();

    if (is_masked){
        item_text->setEchoMode(QLineEdit::Password);
    }

    if (structure.contains("max_length")){
        item_text->setMaxLength(structure["max_length"].toInt());
    }

    item_text->setAlignment(Qt::AlignVCenter);

    connect(item_text, &QLineEdit::textChanged, this, &TeraForm::widgetValueChanged);
    return item_text;
}

QWidget *TeraForm::createBooleanWidget(const QVariantHash &structure)
{
    Q_UNUSED(structure)
    QCheckBox* item_check = new QCheckBox();
    item_check->setCursor(Qt::PointingHandCursor);

    connect(item_check, &QCheckBox::clicked, this, &TeraForm::widgetValueChanged);

    if (structure.contains("default")){
        item_check->setChecked(structure["default"].toBool());
    }

    return item_check;
}

QWidget *TeraForm::createNumericWidget(const QVariantHash &structure)
{
    QSpinBox* item_spin = new QSpinBox();

    if (structure.contains("minimum")){
        item_spin->setMinimum(structure["minimum"].toInt());
    }else {
        item_spin->setMinimum(0);
    }

    if (structure.contains("maximum")){
        item_spin->setMaximum(structure["maximum"].toInt());
    }else{
        item_spin->setMaximum(99999);
    }

    // Using old-style connect since SLOT has less parameter and not working with new-style connect
    connect(item_spin, SIGNAL(valueChanged(int)), this, SLOT(widgetValueChanged()));

    return item_spin;
}

QWidget *TeraForm::createLabelWidget(const QVariantHash &structure)
{
    Q_UNUSED(structure)
    QLabel* item_label = new QLabel();

    item_label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    //item_label->setWordWrap(true);

    //item_label->setHidden(is_hidden);

    return item_label;
}

QWidget *TeraForm::createLongLabelWidget(const QVariantHash &structure)
{
    QPushButton* item_button = new QPushButton();
    item_button->setText(tr("Afficher"));
    item_button->setIcon(QIcon(":/icons/view_on.png"));
    item_button->setCursor(Qt::PointingHandCursor);

    // Connect signal
    connect(item_button, &QPushButton::clicked, this, &TeraForm::longLabelButtonClicked);

    return item_button;

}

QWidget *TeraForm::createListWidget(const QVariantHash &structure)
{
    QListWidget* item_list = new QListWidget();

    if (structure.contains("values")){
        // TODO.
    }

    return item_list;
}

QWidget *TeraForm::createLongTextWidget(const QVariantHash &structure)
{
    Q_UNUSED(structure)
    QTextEdit* item_text = new QTextEdit();

    /*QFontMetrics metrics(item_text->font());
    int row_height = metrics.lineSpacing();
    item_text->setFixedHeight(4.5 * row_height);*/

    //item_text->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    connect(item_text, &QTextEdit::textChanged, this, &TeraForm::widgetValueChanged);

    return item_text;
}

QWidget *TeraForm::createColorWidget(const QVariantHash &structure)
{
    Q_UNUSED(structure)
    QPushButton* item_btn = new QPushButton();

    item_btn->setFlat(true);
    item_btn->setProperty("color", "#FFFFFF");
    item_btn->setStyleSheet("background-color: #FFFFFF;min-width: 32px;");
    item_btn->setCursor(Qt::PointingHandCursor);

    // Connect signal
    connect(item_btn, &QPushButton::clicked, this, &TeraForm::colorWidgetClicked);

    return item_btn;
}

QWidget *TeraForm::createDateTimeWidget(const QVariantHash &structure)
{
    Q_UNUSED(structure)
    QDateTimeEdit* item_dt = new QDateTimeEdit();
    item_dt->setDisplayFormat("dd MMMM yyyy - hh:mm");
    //item_dt->setCalendarPopup(true);

    // Hide by default if read only
    if (structure.contains("readonly")){
        if (structure["readonly"].toBool() == true){
            m_hiddenFields.append(structure["id"].toString());
        }
    }

    connect(item_dt, &QDateTimeEdit::dateTimeChanged, this, &TeraForm::widgetValueChanged);

    return item_dt;

}

QWidget *TeraForm::createDurationWidget(const QVariantHash &structure)
{
    Q_UNUSED(structure)
    QTimeEdit* item_t = new QTimeEdit();
    item_t->setDisplayFormat("hh:mm:ss");

    connect(item_t, &QTimeEdit::timeChanged, this, &TeraForm::widgetValueChanged);

    return item_t;
}

void TeraForm::loadAudioInputs()
{
    m_audioInputs = AudioVideoUtils::getAudioDeviceNames();// QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
}

void TeraForm::loadVideoInputs()
{
    m_videoInputs = AudioVideoUtils::getVideoDeviceNames(); //QCameraInfo::availableCameras();
}

void TeraForm::checkConditions(QWidget *item_triggering)
{
    for (QWidget* item:std::as_const(m_widgets)){
        if (!item)
            continue;
        if (item == item_triggering)
            continue;
        checkConditionsForItem(item, item_triggering);
    }
}

void TeraForm::checkConditionsForItem(QWidget *item, QWidget *item_triggering)
{
    if (item->property("condition").isValid()){
        // Item has a condition
        if (item->property("condition").canConvert<QVariantHash>()){
            QVariantHash condition = item->property("condition").toHash();
            QString check_id = condition["item"].toString();
            if (!check_id.isNull()){
                QWidget* check_item = m_widgets[check_id];
                if (check_item){
                    if (check_item->property("condition").isValid())
                        checkConditionsForItem(check_item);
                    if (check_item->isHidden() && check_item->property("item_type").toString()!="hidden"){
                        setWidgetVisibility(item, check_item, false);
                        return;
                    }

                    // Check if condition is met or not for that item
                    QString op = condition["op"].toString();
                    QVariant value = condition["condition"];
                    QVariant hook = condition["hook"].toString();
                    QVariant sender_index;
                    QVariant sender_value;
                    getWidgetValues(check_item, &sender_index, &sender_value);

                    bool condition_met = false;
                    //TODO: Other operators...
                    if (op == "="){
                        if (sender_index == value || sender_value == value){
                            condition_met = true;
                        }else{
                            if (value.toString() == "changed"){
                                // Trigger hook when value was changed
                                condition_met = true;
                            }
                        }
                    }
                    if (op == "<>"){
                        if (sender_index != value || sender_value != value){
                            condition_met = true;
                        }
                    }
                    if (op.toUpper() == "NOT NULL"){
                        if ((!sender_index.isValid() && !sender_value.isValid()) || (!sender_value.isValid() || (sender_value.typeId() == QMetaType::QString && !sender_value.toString().isEmpty()))){
                            condition_met = true;
                        }
                    }
                    if (op.toUpper() == "CONTAINS"){
                        if (sender_index == value || sender_value.toString().contains(value.toString())){
                            condition_met = true;
                        }
                    }

                    // Hide/show that item
                    setWidgetVisibility(item, check_item, condition_met);

                    // We have a hook requesting data for that specific widget...
                    if (!hook.isNull() || hook.isValid()){
                        if (item_triggering == check_item){
                            if (sender_index.toString() != ""){
                                if (m_comManager){
                                    QStringList target_hook = hook.toString().split("?");
                                    if (target_hook.length()==2){
                                        if (!m_widgetsHookRequests.contains(item)){
                                            m_widgetsHookRequests.insert(item, TeraData::getDataTypeFromPath(target_hook.first()));
                                            QUrlQuery args;
                                            args.addQueryItem(target_hook.last().replace("=",""), sender_index.toString());
                                            m_comManager->doGet(target_hook.first(), args);
                                        }
                                    }
                                }else{
                                    LOG_ERROR("Unable to process hook - no linked to comManager!", "TeraForm::checkConditionsForItem");
                                }

                            }
                        }
                    }

                }
            }
        }
    }

}

bool TeraForm::hasHookCondition(QWidget *item)
{
    // Check if any item a hook condition on it
    if (item->property("condition").isValid()){
        // Item has a condition
        if (item->property("condition").canConvert<QVariantHash>()){
            QVariantHash condition = item->property("condition").toHash();
            if (condition.contains("hook"))
                return true;
        }
    }

    return false;
}

void TeraForm::setWidgetVisibility(QWidget *widget, QWidget *linked_widget, bool visible)
{

    if (widget->parentWidget()){
        if (QFormLayout* form_layout = dynamic_cast<QFormLayout*>(widget->parentWidget()->layout())){
            if (visible){
                // Check if is a removed row
                if (m_hidden_rows.contains(widget)){
                    QFormLayout::TakeRowResult row = m_hidden_rows[widget];
                    int parent_row;
                    if (linked_widget){
                        form_layout->getWidgetPosition(linked_widget, &parent_row, nullptr);
                    }else {
                        //form_layout->getWidgetPosition(ui->toolboxMain, &parent_row, nullptr);
                        form_layout->getWidgetPosition(m_mainWidget, &parent_row, nullptr);
                     }

                    // Ensure the item is at the correct row order
                    if (widget->dynamicPropertyNames().contains("_order")){
                        //qDebug() << "Showing " <<  m_widgets.key(widget) << ", order = " << widget->property("_order").toInt();
                        for (int row = 0; row<form_layout->rowCount(); row++){
                            if (form_layout->itemAt(row, QFormLayout::FieldRole)){
                                QWidget* form_widget = form_layout->itemAt(row, QFormLayout::FieldRole)->widget();
                                if (form_widget->dynamicPropertyNames().contains("_order")){
                                     //qDebug() << "Comparing to " <<  m_widgets.key(form_widget) << ", order = " << form_widget->property("_order").toInt();
                                    if (widget->property("_order").toInt() < form_widget->property("_order").toInt()){
                                        //qDebug() << "YES";
                                        parent_row = row - 1;
                                        break;
                                    }
                                    //qDebug() << "NO";
                                }
                            }
                        }
                        if (parent_row <0){
                            // No before any present widget, insert last!
                            parent_row = form_layout->rowCount();
                        }
                    }
                    form_layout->insertRow(parent_row+1, row.labelItem->widget(), row.fieldItem->widget());
                    row.labelItem->widget()->show();
                    row.fieldItem->widget()->show();
                    m_hidden_rows.remove(widget);
                }
            }else{
                if (!m_hidden_rows.contains(widget)){
                    m_hidden_rows[widget] = form_layout->takeRow(widget);
                    widget->hide();
                    m_hidden_rows[widget].labelItem->widget()->hide();
                }
            }
        }
    }

}

bool TeraForm::getWidgetValues(QWidget* widget, QVariant *id, QVariant *value)
{
/*    if (id)
        *id = QVariant();*/

    if (QComboBox* combo = dynamic_cast<QComboBox*>(widget)){
        if (id)
            *id = combo->currentData();
        *value = combo->currentText();
        return true;
    }

    if (QLineEdit* text = dynamic_cast<QLineEdit*>(widget)){
        *value = text->text();
    }

    if (QTextEdit* text = dynamic_cast<QTextEdit*>(widget)){
        *value = text->toPlainText();
        return true;
    }

    if (QCheckBox* check = dynamic_cast<QCheckBox*>(widget)){
        *value = check->isChecked();
        return true;
    }

    if (QSpinBox* spin = dynamic_cast<QSpinBox*>(widget)){
        *value = spin->value();
        return true;
    }

    if (QLabel* label = dynamic_cast<QLabel*>(widget)){
        *value = label->text();
    }

    if (QPushButton* btn = dynamic_cast<QPushButton*>(widget)){
        if (btn->property("color").isValid()){
            *value = btn->property("color").toString();
        }
        if (btn->property("display_text").isValid()){
            *value = btn->property("display_text").toString();
        }
    }

    if (QTimeEdit* te = dynamic_cast<QTimeEdit*>(widget)){
        if (te->property("item_type") == "duration"){
            *value = QTime(0,0).secsTo(te->time());
        }else{
            *value = te->time();
        }
        return true;
    }

    if (QDateTimeEdit* dt = dynamic_cast<QDateTimeEdit*>(widget)){
        if (dt->dateTime().isValid() && dt->dateTime().date().year()>2000){
            *value = dt->dateTime();
        }else{
            return false; // Invalid date
        }
        return true;
    }


    if (value->canConvert<QString>()){
        bool ok;
        QString string_val = value->toString();
        string_val.toInt(&ok);
        if (ok){
            *value = string_val.toInt();
        }

        if (string_val.startsWith("{")){
            // Maybe JSON, so try to keep that format if possible
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(string_val.toUtf8(), &err);
            if (err.error == QJsonParseError::NoError){
                *value = doc;
            }else{
                return false;
            }
        }

    }
    return true;
}

QVariant TeraForm::getWidgetValue(QWidget *widget)
{
    QVariant value;
    QVariant id;

    getWidgetValues(widget, &id, &value);

    if (!id.isNull() || id.isValid())
        value = id;

    return value;

}

void TeraForm::setWidgetValue(QWidget *widget, const QVariant &value)
{
    widget->setProperty("last_value", value);
    if (QComboBox* combo = dynamic_cast<QComboBox*>(widget)){
        int index = combo->findText(value.toString());
        if (index>=0){
            combo->setCurrentIndex(index);
        }else{
            // Check if the value matches a data field
            index = combo->findData(value);
            if (index>=0){
                combo->setCurrentIndex(index);
            }else{
                // Check if we have a number, if so, suppose it is the index
                if (value.canConvert<int>()){
                    index = value.toInt();
                    if (index>=0 && index+1<combo->count()){
                        combo->setCurrentIndex(index+1);
                        return;
                    }
                }
                LOG_WARNING("Item not found in ComboBox "+ combo->objectName() + " for item " + value.toString(), "TeraForm::setWidgetValue");
            }
        }
        return;
    }

    if (QLineEdit* text = dynamic_cast<QLineEdit*>(widget)){
        text->setText(value.toString());
        return;
    }

    if (QTextEdit* text = dynamic_cast<QTextEdit*>(widget)){
        if (value.canConvert<QVariantMap>() || value.canConvert<QVariantHash>()){
            QVariantHash data = value.toHash();
            QJsonDocument doc;
            doc.setObject(QJsonObject::fromVariantHash(data));
            text->setText(doc.toJson(QJsonDocument::Compact));
            return;
        }
        text->setText(value.toString());
        return;
    }

    if (QLabel* label = dynamic_cast<QLabel*>(widget)){
        label->setText(value.toString());
        return;
    }

    if (QCheckBox* check = dynamic_cast<QCheckBox*>(widget)){
        check->setChecked(value.toBool());
        return;
    }

    if (QSpinBox* spin = dynamic_cast<QSpinBox*>(widget)){
        spin->setValue(value.toInt());
        return;
    }

    if (QPushButton* btn = dynamic_cast<QPushButton*>(widget)){
        if (value.toString().startsWith("#")){
            btn->setProperty("color", value.toString());
            btn->setStyleSheet(QString("background-color: " + value.toString() + ";min-width: 32px;"));
            return;
        }else{
            btn->setProperty("display_text", value.toString());
            return;
        }

    }

    if (QTimeEdit* te = dynamic_cast<QTimeEdit*>(widget)){
        QTime time_value = value.toTime();

        if (!time_value.isValid()){
            int time_s = value.toInt();
            time_value = QTime(0,0).addSecs(time_s);
        }
        te->setTime(time_value);
        return;
    }

    if (QDateTimeEdit* dt = dynamic_cast<QDateTimeEdit*>(widget)){
        if (value.canConvert<QString>()){
            if (value.toString().isEmpty()){
                widget->hide();
                return;
            }
        }

        QDateTime time_value = value.toDateTime().toLocalTime();
        if (!time_value.isValid()){
            unsigned int time_s = value.toUInt();
            // Consider we have a UNIX timestamp
            time_value = QDateTime::fromSecsSinceEpoch(time_s);
        }

        if (time_value.isValid() && time_value.date().year()>2000){
            dt->setDateTime(time_value);
            setWidgetVisibility(widget, nullptr, true);
        }else{
            setWidgetVisibility(widget, nullptr, false);
        }
        return;
    }



    LOG_WARNING("Unhandled widget: "+ QString(widget->metaObject()->className()) + " for item " + value.toString(), "TeraForm::setWidgetValue");
}

void TeraForm::setWidgetRequired(QWidget *item_widget, QLabel *item_label, const bool &required)
{
    if (!item_widget || !item_label)
        return;

    item_widget->setProperty("required", required);
    item_label->setProperty("required", required);
    item_label->setProperty("label", item_label->text());
    /*if (required){
        item_label->setText("<font color=red>*</font> " + item_label->text());
    }else{
        item_label->setText("  " + item_label->text());
    }*/
}

void TeraForm::updateWidgetChoices(QWidget *widget, const QList<TeraData> values)
{

    if (QComboBox* combo = dynamic_cast<QComboBox*>(widget)){
        // Check if we need to update something first...
        /*if (combo->count()>1 && !values.isEmpty()){
            if (combo->itemData(1) == values.first().getId())
                return; // Nothing to update...
        }*/
        combo->clear();
        combo->addItem("", "");

        for (const TeraData &value:values){
            combo->addItem(value.getName(), value.getId());
        }

        if (combo->count() == 1){
            // Hide field value if empty
            hideField(m_widgets.key(combo));
        }

        return;
    }
    if (widget)
        LOG_WARNING("Unhandled widget: "+ QString(widget->metaObject()->className()), "TeraForm::updateWidgetValues");
}

bool TeraForm::validateWidget(QWidget *widget, bool include_hidden)
{

    bool rval = true;

    if (!widget)
        return false;

    if (widget->isVisibleTo(widget->parentWidget()) || include_hidden){
        if (widget->property("required").isValid()){
            if (widget->property("required").toBool()){
                QVariant id, value;
                getWidgetValues(widget, &id, &value);
                //qDebug() << widget->metaObject()->className() << " - " << m_widgets.key(widget) << " - " << value;
                if (value.isNull() || !value.isValid() || value.toInt()==-1 || value.toString().isEmpty()){
                    rval = false;
                }
            }
        }
    }

    if (rval){
        if (!dynamic_cast<QPushButton*>(widget)) // Ignore push button in validation
            widget->setStyleSheet("");
    }else{
        widget->setStyleSheet("background-color: #ffaaaa;");
        /*if (!dynamic_cast<QComboBox*>(widget)){
           widget->setStyleSheet("background-color: #ffaaaa;");
        }else{
           widget->setStyleSheet("background-color: #ffaaaa; color:white;");
        }*/
    }
    return rval;
}

qreal TeraForm::doLinearInterpolation(const qreal &p1, const qreal &p2, const qreal &value)
{
    return (p1 + (p2-p1)*value);
}

void TeraForm::updateRequiredWidgetsLabel()
{
    for (QLabel* label: m_widgetsLabels){
        if (m_disabled){
            if (label->property("label").isValid()){
                label->setText(label->property("label").toString());
            }
        }else{
            for (QLabel* label: m_widgetsLabels){
                if (label->property("required").toBool()){
                    label->setText("<font color=red>*</font> " + label->property("label").toString());
                }else{
                    label->setText("  " + label->property("label").toString());
                }
            }
        }
    }
}

bool TeraForm::eventFilter(QObject *object, QEvent *event)
{
    if(event->type() == QEvent::FocusIn) {
        QFocusEvent* focus_event = dynamic_cast<QFocusEvent*>(event);
        if (focus_event->reason() != Qt::ActiveWindowFocusReason){
            //qDebug() << object << " has focus now!";
            QWidget* widget = dynamic_cast<QWidget*>(object);
            if (widget)
                emit widgetValueHasFocus(widget);
        }
        return false; // lets the event continue to the edit
    }
    return false;
}

void TeraForm::widgetValueChanged()
{
    // This will work only if the sender is in the same thread, which is always the case here.
    QObject* sender = QObject::sender();
    if (!sender)
        return;

    QWidget* sender_widget = dynamic_cast<QWidget*>(sender);
    if (sender_widget){
        //qDebug() << m_widgets.key(sender_widget) << " - " << sender_widget->property("last_value");
        QVariant current_value = getWidgetValue(sender_widget);
        if (!sender_widget->property("last_value").isValid() || sender_widget->property("last_value") != current_value){
            validateWidget(sender_widget);
            sender_widget->setProperty("last_value", current_value);
            emit widgetValueHasChanged(sender_widget, getWidgetValue(sender_widget));
            checkConditions(sender_widget);
        }else{
            // If item has a hook, always trigger it!
            /*if (hasHookCondition(sender_widget))
                checkConditions(sender_widget);*/

        }
        emit formIsNowDirty(isDirty());
    }
}

void TeraForm::colorWidgetClicked()
{
    QObject* sender = QObject::sender();
    if (!sender)
        return;

    QPushButton* sender_widget = dynamic_cast<QPushButton*>(sender);

    QColorDialog diag;
    QColor color;

    color = diag.getColor(QColor(sender_widget->property("color").toString()),nullptr,tr("Choisir la couleur"));

    if (color.isValid()){
        sender_widget->setProperty("color", color.name());
        // Display current color
        sender_widget->setStyleSheet(QString("background-color: " + color.name() + ";"));
    }
}

void TeraForm::longLabelButtonClicked()
{
    QObject* sender = QObject::sender();
    if (!sender)
        return;

    QPushButton* sender_widget = dynamic_cast<QPushButton*>(sender);
    QString text = sender_widget->property("display_text").toString();

    if (!text.isEmpty()){
        GlobalMessageBox msgbox(this);
        msgbox.showInfo(m_widgetsLabels.value(sender_widget)->text(), text);
    }
}

void TeraForm::hookReplyReceived(TeraDataTypes data_type, QList<TeraData> datas)
{
    if (m_widgetsHookRequests.isEmpty())
        return;

    // Check if we have a widget waiting for a reply from a query
    //if (m_widgetsHookRequests.values().contains(data_type)){
    if (std::find(m_widgetsHookRequests.cbegin(), m_widgetsHookRequests.cend(), data_type) != m_widgetsHookRequests.cend()){
        QWidget* target_widget = m_widgetsHookRequests.key(data_type);
        // Update widget values
        updateWidgetChoices(target_widget, datas);

        // Update widget value if value was already set
        if (m_initialValues.contains(m_widgets.key(target_widget)))
            setWidgetValue(target_widget, m_initialValues[m_widgets.key(target_widget)]);

        m_widgetsHookRequests.remove(target_widget);
    }
}

void TeraForm::setDisabled(bool disable)
{
    // Disable only the contents of pages, not the toolbox itself
    if (m_mainWidget == ui->toolboxMain){
        for (int i=0; i<ui->toolboxMain->count(); i++){
            ui->toolboxMain->widget(i)->setDisabled(disable);
        }
    }else{
        if (m_mainWidget)
            m_mainWidget->setDisabled(disable);
    }
    m_disabled = disable;

    // Hide "required" labels indicators
    updateRequiredWidgetsLabel();

    //QLabel* widget_label = m_widgetsLabels[widget];
    //QWidget::setDisabled(disable);

}

void TeraForm::setEnabled(bool enable)
{
    // Enable only the contents of pages, not the toolbox itself
    if (m_mainWidget == ui->toolboxMain){
        for (int i=0; i<ui->toolboxMain->count(); i++){
            ui->toolboxMain->widget(i)->setEnabled(enable);
        }
    }else{
        if (m_mainWidget)
            m_mainWidget->setEnabled(enable);
    }
    m_disabled = !enable;
    //QWidget::setEnabled(enable);
    // Show "required" labels indicators
    updateRequiredWidgetsLabel();
}
