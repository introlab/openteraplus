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

QStringList Utils::getAudioDeviceNames()
{
    QStringList names;
    QList<QAudioDeviceInfo> audio_devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    foreach(QAudioDeviceInfo input, audio_devices){
#ifdef Q_OS_LINUX
        // On linux, since Qt use ALSA API, we must filter the returned device list...
        if (input.deviceName().startsWith("alsa_input") || input.deviceName() == "default"){
            QString filtered_name = input.deviceName();
            if (input.deviceName() != "default"){
                QStringList name_parts = filtered_name.split(".");
                // Removes first part - usually is "alsa_input"
                if (name_parts.count()>1)
                    filtered_name = name_parts[1];

                // Split using "_" to remove first and last part, and replace others with spaces
                name_parts = filtered_name.split("_");
                if (name_parts.count()>2){
                    filtered_name = "";
                    for (int i=1; i<name_parts.count()-1; i++){
                        if (i>1)
                            filtered_name += " ";
                        filtered_name += name_parts[i];
                    }
                }else{
                    // No audio name...
                    name_parts = filtered_name.split("-");
                    if (name_parts.count()>2){
                        filtered_name = name_parts[1].replace("_", ":");
                    }
                }

            }
            names.append(filtered_name);
        }
#else
        names.append(input.deviceName());
#endif
    }
    return names;
}

QStringList Utils::getVideoDeviceNames()
{
    QStringList names;
    QList<QCameraInfo> video_devices = QCameraInfo::availableCameras();
    for (QCameraInfo camera:video_devices){
        names.append(camera.description());
    }
    return names;
}

