#include "ActimetryConfigWidget.h"
#include "ui_ActimetryConfigWidget.h"


ActimetryConfigWidget::ActimetryConfigWidget(ComManager *comManager, int projectId, QString participantUuid, QString participantName, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::ActimetryConfigWidget),
    m_comManager(comManager),
    m_idProject(projectId),
    m_participantUuid(participantUuid),
    m_participantName(participantName)
{
    m_ui->setupUi(this);

    // NOTE At this state, comManager is not ready, will need to get service information before and then change state to ready.
    m_ActimetryComManager = new ActimetryComManager(comManager);

    m_ui->tabMain->setCurrentIndex(0);
    m_ui->lblWarning->hide();
    m_ui->wdgMessages->hide();


    connectSignals();
}

ActimetryConfigWidget::~ActimetryConfigWidget()
{
    delete m_ui;
    delete m_ActimetryComManager;
}

void ActimetryConfigWidget::connectSignals()
{

    connect(m_ActimetryComManager, &ActimetryComManager::readyChanged, this, &ActimetryConfigWidget::serviceReadyChanged);
    connect(m_ui->wdgMessages, &ResultMessageWidget::nextMessageShown, this, &ActimetryConfigWidget::nextMessageWasShown);
    connect(m_ActimetryComManager, &ActimetryComManager::availableAlgorithmsReceived, this, &ActimetryConfigWidget::availableAlgorithmsReceived);
    connect(m_ActimetryComManager, &ActimetryComManager::algorithmInfoReceived, this, &ActimetryConfigWidget::algorithmInfoReceived);

    connect(m_ui->btnRun, &QPushButton::clicked, this, &ActimetryConfigWidget::onRunButtonClicked);
    connect(m_ui->comboAlgorithms, &QComboBox::currentIndexChanged, this, &ActimetryConfigWidget::onComboBoxAlgorithmCurrentIndexChanged);
}

void ActimetryConfigWidget::setCurrentAlgorithmParametersInUI(const QString &algorithmKey)
{
    if (m_ui->comboAlgorithms->currentData().toString() == algorithmKey)
    {
        if (m_algorithmInformations.contains(algorithmKey))
        {
            QJsonObject algorithmInfo = m_algorithmInformations.value(algorithmKey);

            //Display information
            QString description = algorithmInfo.value("description").toString();
            QString author = algorithmInfo.value("author").toString();
            QString reference = algorithmInfo.value("reference").toString();


            QString infoText = QString("<b>Description:</b><br>%1<br><br><b>Author:</b> %2<br><br><b>Reference:</b><br>%3")
                                   .arg(description)
                                   .arg(author)
                                   .arg(reference);

            m_ui->textEdit->setHtml(infoText);

            //Get Parameters
            QJsonObject parameters_input = algorithmInfo.value("parameters").toObject();
            QJsonObject parameters_output;

            //Iterate though parameters
            foreach(const QString &paramKey, parameters_input.keys())
            {
                QJsonObject paramInfo = parameters_input.value(paramKey).toObject();
                QVariant defaultValue = paramInfo.value("default_value").toVariant();

                // Set parameters for output
                parameters_output.insert(paramKey, defaultValue.toDouble());

            }

            //Set data for parameters
            m_ui->textEditParameters->setPlainText(QString(QJsonDocument(parameters_output).toJson(QJsonDocument::Indented)));

        }
    }
}


void ActimetryConfigWidget::serviceReadyChanged(bool ready)
{
    if(ready)
    {
        // Query available algorithms
        m_ActimetryComManager->queryAvailableAlgorithms();

    }
}

void ActimetryConfigWidget::nextMessageWasShown(Message current_message)
{
    if (current_message.getMessageType()==Message::MESSAGE_NONE)
    {
        m_ui->wdgMessages->hide();
    }
    else
    {
        m_ui->wdgMessages->show();
    }
}

void ActimetryConfigWidget::availableAlgorithmsReceived(const QList<QJsonObject> &algorithms)
{
    //Iterate through list
    /*
         SAMPLE OUTPUT:
         QList(QJsonObject({"key":"Evenson2008","name":"Evenson 2008","version":"0.1"}), QJsonObject({"key":"FreedsonAdult1998","name":"Freedson Adult 1998","version":"0.1"}))
    */
    foreach(const QJsonObject &algorithm, algorithms)
    {
        QString key = algorithm.value("key").toString();
        QString name = algorithm.value("name").toString();
        QString version = algorithm.value("version").toString();

        //Add to combo box
        m_ui->comboAlgorithms->addItem(name + " (v" + version + ")", key);

        // Query more information
        m_ActimetryComManager->queryAlgorithm(key);
    }
}

void ActimetryConfigWidget::algorithmInfoReceived(const QJsonObject &algorithmInfo)
{
    /* SAMPLE INFORMATION

   QJsonObject({"author":"Simon Brière",
                "description":"Classify activity counts into various intensity levels ...",
                "key":"Evenson2008",
                "name":"Evenson 2008",
                "parameters":{"light_cutoff":{"default_value":573,"description":"light_cutoff description","max_value":15000,"min_value":0,"type":"float"},
                              "moderate_cutoff":{"default_value":1000,"description":"moderate_cutoff description","max_value":15000,"min_value":0,"type":"float"},
                              "sedentary_cutoff":{"default_value":25,"description":"sedentary_cutoff description","max_value":15000,"min_value":0,"type":"float"}},
                              "reference":"Kelly R. Evenson, Diane J. Catellier, Karminder Gill, Kristin S. Ondrak & Robert G. McMurray (2008) Calibration of two objective measures of physical activity for children, Journal of Sports Sciences, 26:14, 1557-1565, DOI: 10.1080/02640410802334196 ",
                "results":[{"description":"Minutes for sedentary activity","name":"sedentary"},{"description":"Minutes for light activity","name":"light"},{"description":"Minutes for moderate activity","name":"moderate"}],
                "unique_id":2,
                "version":"0.1"})
*/

    QString key = algorithmInfo.value("key").toString();
    m_algorithmInformations.insert(key, algorithmInfo);

    setCurrentAlgorithmParametersInUI(key);

    m_ui->btnRun->setEnabled(true);
}

void ActimetryConfigWidget::onComboBoxAlgorithmCurrentIndexChanged(int index)
{
    //Get element at index
    QString selectedAlgorithmKey = m_ui->comboAlgorithms->itemData(index).toString();

    //Query algorithm info
    if (m_algorithmInformations.contains(selectedAlgorithmKey))
    {
        //Already have information, just set it
        setCurrentAlgorithmParametersInUI(selectedAlgorithmKey);
    }
}

void ActimetryConfigWidget::onRunButtonClicked()
{
    if (m_ui->comboAlgorithms->count() == 0)
    {
        m_ui->wdgMessages->addMessage(Message(Message::MESSAGE_ERROR, tr("Aucun algorithme n'est disponible pour le traitement.")));
        return;
    }

    //Get selected algorithm
    QString selectedAlgorithmKey = m_ui->comboAlgorithms->currentData().toString();
    QString parametersText = m_ui->textEditParameters->toPlainText();

    //For now, just show a message
    m_ActimetryComManager->queryProcessing(selectedAlgorithmKey, m_participantUuid, parametersText);
}
