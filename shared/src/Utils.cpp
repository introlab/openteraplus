#include "Utils.h"

Utils::Utils(QObject *parent) : QObject(parent)
{

}

QString Utils::generatePassword(const int &len)
{
   // Available char list
   const QString possibleCharacters("ABCDEFGHIJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz123456789$%*&!()?#");

   QString password;
   for(int i=0; i<len; ++i)
   {
       quint32 index = QRandomGenerator::global()->bounded(possibleCharacters.length());
       QChar nextChar = possibleCharacters.at(index);
       password.append(nextChar);
   }
   return password;
}

QString Utils::getMachineUniqueId()
{
    QByteArray direct_machine_id = QSysInfo::machineUniqueId();
    QString machine_id = "";

    if (!direct_machine_id.isEmpty()){
        machine_id = QString::fromUtf8(direct_machine_id);
    }else{
        // This OS doesn't provide a unique machine id. We could browse through hardware, but, much simply,
        // we generate a UUID that we store using QSettings

        QSettings settings;
        machine_id = settings.value("general/machine_id").toString();
        if (machine_id.isEmpty()){
            machine_id = QUuid::createUuid().toString();
            settings.setValue("general/machine_id", machine_id);
        }

    }

    return machine_id;
}
