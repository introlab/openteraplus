#include "DataListWidget.h"
#include "GlobalMessageBox.h"

#include "editors/UserWidget.h"
#include "editors/SiteWidget.h"
#include "editors/DeviceWidget.h"
#include "editors/SessionTypeWidget.h"
#include "editors/DeviceSubTypeWidget.h"
#include "editors/DeviceTypeWidget.h"
#include "editors/UserGroupWidget.h"
#include "editors/ServiceWidget.h"
#include "editors/ServiceConfigWidget.h"

#include "wizards/UserWizard.h"

DataListWidget::DataListWidget(ComManager *comMan, TeraDataTypes data_type, QWidget *parent):
    DataListWidget(comMan, data_type, QUrlQuery(), parent)
{  

}

DataListWidget::DataListWidget(ComManager* comMan, TeraDataTypes data_type, QUrlQuery args, QWidget *parent ):
    DataListWidget(comMan, data_type, TeraData::getPathForDataType(data_type), args, QStringList(), parent)
{

}

DataListWidget::DataListWidget(ComManager* comMan, TeraDataTypes data_type, QUrlQuery args, QStringList extra_display_fields, QWidget *parent):
    DataListWidget(comMan, data_type, TeraData::getPathForDataType(data_type), args, extra_display_fields, parent)
{

}

DataListWidget::DataListWidget(ComManager* comMan, TeraDataTypes data_type, QString query_path, QUrlQuery args, QStringList extra_display_fields, QWidget *parent):
    QWidget(parent),
    ui(new Ui::DataListWidget),
    m_comManager(comMan),
    m_dataType(data_type),
    m_queryPath(query_path),
    m_extraDisplayFields(extra_display_fields),
    m_canEdit(false),
    m_canView(true),
    m_queryArgs(args)
{
    ui->setupUi(this);

    // Hide filters by default
    ui->frameFilter->setEnabled(false);
    ui->frameFilter->hide();

    m_editor = nullptr;
    setSearching(false);
    m_newdata = false;

    // No copy function for now
    ui->btnCopy->hide();

    connectSignals();
    updateControlsState();

    queryDataList(m_queryArgs);
}

DataListWidget::~DataListWidget()
{
    if (m_editor)
        m_editor->deleteLater();

    clearDataList();
}

void DataListWidget::setPermissions(const bool &can_view, const bool &can_edit)
{
    m_canEdit = can_edit;
    m_canView = can_view;
    updateControlsState();
}

void DataListWidget::updateDataInList(TeraData* data, bool select_item){

    QListWidgetItem* item = nullptr;
    bool already_present = false;

    // Check if we already have that item
    item = getItemForData(data);

    // If we don't, create a new one.
    if (!item){
        // Check if we have a new item that we could match
        item = new QListWidgetItem(data->getName(),ui->lstData);
        m_datamap[data] = item;

    }else{
        already_present = true;
        // Copy data to local object
        *m_datamap.key(item) = *data;
    }

    item->setIcon(QIcon(TeraData::getIconFilenameForDataType(data->getDataType())));
    QString data_name = data->getName();
    // If a parent is specified, append the parent name
    if (data->hasFieldName(data->getDataTypeName(data->getDataType()) + "_parent"))
        data_name = "[" + data->getFieldValue(data->getDataTypeName(data->getDataType()) + "_parent").toString() + "]\n" + data_name;

    // If we have extra fields to display, append them
    QString extra_field = "";
    if (!m_extraDisplayFields.isEmpty() && !m_extraInfos.contains(data->getId())){
        for (QString field:m_extraDisplayFields){
            QStringList subfields = field.split(".");
            QString subfield;
            if (subfields.count() > 1){
                field = subfields.first();
                subfield = subfields.last();
            }
            if (data->hasFieldName(field)){
                if (!extra_field.isEmpty())
                    extra_field += ", ";
                QVariant field_value = data->getFieldValue(field);

                if (field_value.canConvert(QMetaType::QVariantList)){
                    QVariantList field_values = field_value.toList();
                    QString merged_list;
                    for (QVariant field_value:field_values){
                        if (!merged_list.isEmpty())
                            merged_list += ", ";
                        // Search for subfield?
                        if (field_value.canConvert(QMetaType::QVariantMap)){
                            QVariantMap field_map = field_value.toMap();
                            field_value = field_map[subfield];
                        }
                        merged_list += field_value.toString();
                    }
                    extra_field += merged_list;
                }else{
                    extra_field += data->getFieldValue(field).toString();
                }
            }
        }

    }else{
        if (m_extraInfos.contains(data->getId())){
            extra_field = m_extraInfos[data->getId()];
        }
    }

    if (!extra_field.isEmpty()){
        m_extraInfos[data->getId()] = extra_field;
        data_name += " (" + extra_field + ")";
    }
    item->setText(data_name);
    item->setData(Qt::UserRole, data->getId());

    //Customize color and icons, if needed, according to data_type
    if (data->hasEnabledField()){
        if (data->isEnabled()){
            item->setForeground(Qt::green);
        }else{
            item->setForeground(Qt::red);
        }
    }

    /*QString color_field = TeraData::getDataTypeName(m_dataType) + "_color";
    if (data->hasFieldName(color_field)){
        item->setForeground(QColor(data->getFieldValue(color_field).toString()));
    }*/

    if (select_item){
        ui->lstData->setCurrentItem(item);
    }

    if (ui->lstData->currentItem()==item){
        // Load editor
        showEditor(data);
    }

    // Delete item if it was already there (we copied what we needed)
    if (already_present){
        delete data;
    }
}

void DataListWidget::deleteDataFromList(TeraData *data)
{
    // Remove list item
    QListWidgetItem* list_item = m_datamap[data];
    if (list_item){
        delete ui->lstData->takeItem(ui->lstData->row(list_item));
        m_datamap.remove(data);
    }else{
        LOG_WARNING("Can't find ListWidgetItem for " + data->getName(), "DataListWidget::deleteDataFromList");
    }

    data->deleteLater();

    if (ui->lstData->count() == 0){
        // No more data in list - hide editor
        showEditor(nullptr);
    }
}

void DataListWidget::showEditor(TeraData *data)
{
    if (m_editor){
        ui->wdgEditor->layout()->removeWidget(m_editor);
        m_editor->disconnectSignals();
        m_editor->deleteLater();
        m_editor = nullptr;
    }

    // No data to display - return!
    if (!data)
        return;

    switch(data->getDataType()){
        case TERADATA_USER:
            m_editor = new UserWidget(m_comManager, data);
        break;
        case TERADATA_SITE:
            m_editor = new SiteWidget(m_comManager, data);
        break;
        case TERADATA_DEVICE:
            m_editor = new DeviceWidget(m_comManager, data);
        break;
        case TERADATA_SESSIONTYPE:
            m_editor = new SessionTypeWidget(m_comManager, data);
        break;
        case TERADATA_DEVICESUBTYPE:
            m_editor = new DeviceSubTypeWidget(m_comManager, data);
        break;
        case TERADATA_DEVICETYPE:
            m_editor = new DeviceTypeWidget(m_comManager, data);
        break;
        case TERADATA_USERGROUP:
            m_editor = new UserGroupWidget(m_comManager, data);
        break;
        case TERADATA_SERVICE:
            m_editor = new ServiceWidget(m_comManager, data);
        break;
        default:
            LOG_ERROR("Unhandled datatype for editor: " + TeraData::getDataTypeName(data->getDataType()), "DataListWidget::showEditor()");
            return;
    }

    if (m_editor){
        connect(m_editor, &DataEditorWidget::dataWasDeleted, this, &DataListWidget::editor_dataDeleted);
        connect(m_editor, &DataEditorWidget::dataWasChanged, this, &DataListWidget::editor_dataChanged);
    }
    ui->wdgEditor->layout()->addWidget(m_editor);
}

void DataListWidget::setFilterText(const QString &text)
{
    ui->lblFilter->setText(text);
    if (m_canView){
        ui->frameFilter->setEnabled(true);
        ui->frameFilter->setVisible(true);
    }

}

/*void DataListWidget::itemClicked(QListWidgetItem *item){
    if (m_editor){
        if (m_editor->isEditing() && last_item!=NULL){
            lstData->setCurrentItem(last_item);
        }
    }
}*/

void DataListWidget::connectSignals()
{
    connect(m_comManager, &ComManager::waitingForReply, this, &DataListWidget::com_Waiting);
    connect(m_comManager, &ComManager::deleteResultsOK, this, &DataListWidget::deleteDataReply);

    // Connect correct signal according to the datatype
    TeraDataTypes expectedDataType = TeraData::getDataTypeFromPath(m_queryPath);
    if (expectedDataType != m_dataType) // Also connect the indirect signal for that data type
        connect(m_comManager, ComManager::getSignalFunctionForDataType(expectedDataType), this, &DataListWidget::setDataList);
    connect(m_comManager, ComManager::getSignalFunctionForDataType(m_dataType), this, &DataListWidget::setDataList);

    connect(ui->txtSearch, &QLineEdit::textChanged, this, &DataListWidget::searchChanged);
    //connect(ui->btnClear, &QPushButton::clicked, this, &DataListWidget::clearSearch);
    connect(ui->lstData, &QListWidget::currentItemChanged, this, &DataListWidget::lstData_currentItemChanged);
    connect(ui->btnNew, &QPushButton::clicked, this, &DataListWidget::newDataRequested);
    connect(ui->btnDelete, &QPushButton::clicked, this, &DataListWidget::deleteDataRequested);
}

void DataListWidget::queryDataList()
{
    QString query_path = TeraData::getPathForDataType(m_dataType);
    clearSearch();

    if (!query_path.isEmpty()){
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_LIST, "true");
        m_comManager->doQuery(query_path, args);
    }
}

void DataListWidget::queryDataList(QUrlQuery args)
{
    //QString query_path = TeraData::getPathForDataType(m_dataType);
    clearSearch();

    if (!m_queryPath.isEmpty()){
        if (args.isEmpty()) // "list" parameter must be appended manually if we provide an args list
            args.addQueryItem(WEB_QUERY_LIST, "true");
        m_comManager->doQuery(m_queryPath, args);
    }
}

void DataListWidget::updateControlsState()
{
    ui->frameFilter->setVisible(m_canView && ui->frameFilter->isEnabled());
    ui->frameButtons->setVisible(m_canEdit);
}

TeraData *DataListWidget::getCurrentData()
{
    if (m_datamap.key(ui->lstData->currentItem())){
        return m_datamap.key(ui->lstData->currentItem());
    }
    return nullptr;
}

QListWidgetItem *DataListWidget::getItemForData(TeraData *data)
{
    // Simple case - we have the data into direct mapping
    if (m_datamap.contains(data))
        return m_datamap[data];

    // Less simple case - the pointers are not the same, but we might be referencing an object already present.
    if (!data->isNew()){
        for (TeraData* current_data:m_datamap.keys()){
            if (*current_data == *data){
                return m_datamap[current_data];
            }
        }

        // Not found - try to find an item which is new but with the same name
        for (TeraData* current_data:m_datamap.keys()){
            if (current_data->isNew() && current_data->getName() == data->getName()){
                m_newdata=false;
                return m_datamap[current_data];
            }
        }
    }else{
        // We have a new item - try and match.
        for (TeraData* current_data:m_datamap.keys()){
            if (current_data->isNew()){
                return m_datamap[current_data];
            }
        }
    }

    // No match.
    return nullptr;

}

void DataListWidget::clearDataList(){
    ui->lstData->clear();

    for (TeraData* data:m_datamap.keys()){
        delete data;
    }
    m_datamap.clear();
}

void DataListWidget::com_Waiting(bool waiting){
    /*TeraData* current_item = getCurrentData();
    if (current_item){
        if (current_item->isNew()){
            waiting = true;
        }
    }*/
    //this->setDisabled(waiting);
    if (m_newdata)
        waiting = true;
    ui->frameItems->setDisabled(waiting);
}


void DataListWidget::deleteDataReply(QString path, int id)
{
    if (id==0)
        return;

    if (path == TeraData::getPathForDataType(m_dataType)){
        // An item that we are managing got deleted
        for (TeraData* data:m_datamap.keys()){
            if (data->getId() == id){
                deleteDataFromList(data);
                break;
            }
        }

    }
}

void DataListWidget::setDataList(QList<TeraData> list)
{
    TeraDataTypes expectedDataType = TeraData::getDataTypeFromPath(m_queryPath);
    for (int i=0; i<list.count(); i++){
        if (list.at(i).getDataType() == expectedDataType || list.at(i).getDataType() == m_dataType){
            TeraData* item_data = new TeraData(list.at(i));
            if (expectedDataType != m_dataType && list.at(i).getDataType() == expectedDataType){
                // We had some objects from a different URL - switch the data type.
                item_data->setDataType(m_dataType);
                if (!item_data->hasNameField() || item_data->getId() == 0){
                    // Wrong item format - this might happen if the editor queried similar infos.
                    delete item_data;
                    continue;
                }

            }
            updateDataInList(item_data);
        }
    }
}

void DataListWidget::editor_dataDeleted()
{
    // Remove data from list
    deleteDataFromList(getCurrentData());

    m_newdata = false;
    ui->lstData->setCurrentRow(-1);
    showEditor(nullptr);
}

void DataListWidget::editor_dataChanged()
{
    QListWidgetItem* item = getItemForData(m_editor->getData());

    if (item){
        item->setText(m_editor->getData()->getName());
        // Copy data to local object
        *m_datamap.key(item) = *m_editor->getData();
    }
}

void DataListWidget::searchChanged(QString new_search){
    Q_UNUSED(new_search)
    // Check if search field is empty
    if (ui->txtSearch->text().count()==0){
        setSearching(false);
        // Display back all items
        for (int i=0; i<ui->lstData->count();i++){
            ui->lstData->item(i)->setHidden(false);
        }
        if (m_editor && ui->lstData->selectedItems().count()>0)
            m_editor->setVisible(true);
        return;
    }

    if (!m_searching){
        setSearching(true);
    }

    // Apply the search filter
    QList<QListWidgetItem*> found = ui->lstData->findItems(ui->txtSearch->text(),Qt::MatchContains);
    for (int i=0; i<ui->lstData->count();i++){
        if (found.contains(ui->lstData->item(i))){
            if (ui->lstData->item(i)->isSelected()){
                if (m_editor)
                    m_editor->setVisible(true);
            }
            ui->lstData->item(i)->setHidden(false);

        }else{
            if (ui->lstData->item(i)->isSelected()){
                if (m_editor)
                    m_editor->setVisible(false);
            }
            ui->lstData->item(i)->setHidden(true);
        }
    }

}

void DataListWidget::setSearching(bool search){
    if (search){
        m_searching = true;
        ui->txtSearch->setStyleSheet("color:white;");
        //ui->btnClear->setVisible(true);
    }else{
        m_searching=false;
        ui->txtSearch->setStyleSheet("color:rgba(255,255,255,50%);");
        //ui->btnClear->setVisible(false);
    }
}

void DataListWidget::clearSearch(){
    ui->txtSearch->setText("");
}


void DataListWidget::lstData_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)
    if (!current)
        return;

    if (!m_canView) // Check if the user can see the details of that item
        return;

    TeraData* current_data = m_datamap.keys(current).first();

    if (current_data->isNew())
        return;

    // Query full data for that data item
    QUrlQuery args;
    args.addQueryItem(current_data->getIdFieldName(), QString::number(current_data->getId()));
    m_comManager->doQuery(TeraData::getPathForDataType(current_data->getDataType()), args);
}

void DataListWidget::newDataRequested()
{
    clearSearch();

    TeraData* new_data = new TeraData(m_dataType);
    // Check if we have some filter information we need to set in the object passed to the editor
    //if (ui->radBtnFilter->isChecked()){
        for (int i=0; i<m_queryArgs.queryItems().count(); i++){
            if (m_queryArgs.queryItems().at(i).first.startsWith("id_")){
                new_data->setFieldValue(m_queryArgs.queryItems().at(i).first, m_queryArgs.queryItems().at(i).second);
            }
        }

    //}

    new_data->setId(0);
    m_newdata = true;
    updateDataInList(new_data, true);
    /*UserWizard new_wizard(m_comManager);
    new_wizard.exec();*/

}

void DataListWidget::deleteDataRequested()
{
    if (!ui->lstData->currentItem())
        return;

    GlobalMessageBox diag;
    QMessageBox::StandardButton answer = diag.showYesNo(tr("Suppression?"),
                                                        tr("Êtes-vous sûrs de vouloir supprimer """) + ui->lstData->currentItem()->text() + """?");
    if (answer == QMessageBox::Yes){
        // We must delete!
        m_comManager->doDelete(TeraData::getPathForDataType(m_dataType), m_datamap.key(ui->lstData->currentItem())->getId());
    }
}

void DataListWidget::copyDataRequested()
{

}

void DataListWidget::on_radBtnFilter_toggled(bool checked)
{
    if (checked){
        clearDataList();
        queryDataList(m_queryArgs);
    }
}

void DataListWidget::on_radBtnAll_toggled(bool checked)
{
    if (checked){
        clearDataList();
        queryDataList();
    }
}
