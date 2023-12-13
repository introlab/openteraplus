#include "Utils.h"
#include <QRegularExpression>
#include <QCameraDevice>
#include <QAudioDevice>

Utils::Utils(QObject *parent) : QObject(parent)
{

}

QString Utils::generatePassword(const int &len)
{
   // Available char list
   const QString possibleCharacters("ABCDEFGHIJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz123456789$%*&!()?#");

   bool validPassword = false;
   QString password;
   while (!validPassword){
       password.clear();
       for(int i=0; i<len; ++i)
       {
           auto index = QRandomGenerator::global()->bounded(possibleCharacters.length());
           QChar nextChar = possibleCharacters.at(index);
           password.append(nextChar);
       }
       validPassword = validatePassword(password).isEmpty();
   }
   return password;
}

QList<Utils::PasswordValidationErrors> Utils::validatePassword(const QString &password)
{
    QList<Utils::PasswordValidationErrors> errors;

    if (password.length()<10){
        errors.append(PASSWORD_LENGTH);
    }

    QRegularExpression validator("[A-Z]");

    if (!validator.match(password).hasMatch()){
        errors.append(PASSWORD_NOCAPS);
    }

    validator.setPattern("[a-z]");
    if (!validator.match(password).hasMatch()){
        errors.append(PASSWORD_NONOCAPS);
    }

    validator.setPattern("[0-9]");
    if (!validator.match(password).hasMatch()){
        errors.append(PASSWORD_NODIGITS);
    }

    validator.setPattern("[^A-Za-z0-9]");
    if (!validator.match(password).hasMatch()){
        errors.append(PASSWORD_NOCHAR);
    }

    return errors;
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


    auto audio_devices = QMediaDevices::audioInputs();
    //QList<QAudioDeviceInfo> audio_devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    foreach(const QAudioDevice &input, audio_devices){
//TODO FIX LINUX
#if 0
//#ifdef Q_OS_LINUX
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
        //TODO Not sure!
        names.append(input.description());
#endif
    }
    return names;
}

QStringList Utils::getVideoDeviceNames()
{
    QStringList names;
    auto inputs = QMediaDevices::videoInputs();
    foreach (const QCameraDevice &info, inputs)
    {
        QString name = info.description();
        if (!name.contains(" IR ")) // Remove infrared cameras.
            names.append(info.description());
    }
#ifdef Q_OS_WINDOWS
    names.append("OpenTeraCam"); // Qt6 doesn't support directshow filters anymore, so manually append it...
#endif
    return names;
}

void Utils::inStringUnicodeConverter(QString *str)
{
    if (str->contains("\\u")) {
       do {
          auto idx = str->indexOf("\\u");
          QString strHex = str->mid(idx, 6);
          strHex = strHex.replace("\\u", QString());
          auto nHex = strHex.toInt(nullptr, 16);
          str->replace(idx, 6, QChar(nHex));
       } while (str->indexOf("\\u") != -1);
    }
}

QString Utils::removeAccents(const QString &s) {
    const QString diacriticLetters_ = QString::fromUtf8("ŠŒŽšœžŸ¥µÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýÿ");
    const QStringList noDiacriticLetters_ = QStringList() << "S"<<"OE"<<"Z"<<"s"<<"oe"<<"z"<<"Y"<<"Y"<<"u"<<"A"<<"A"<<"A"<<"A"<<"A"<<"A"<<"AE"<<"C"<<"E"<<"E"<<"E"<<"E"<<"I"<<"I"<<"I"<<"I"<<"D"<<"N"<<"O"<<"O"<<"O"<<"O"<<"O"<<"O"<<"U"<<"U"<<"U"<<"U"<<"Y"<<"s"<<"a"<<"a"<<"a"<<"a"<<"a"<<"a"<<"ae"<<"c"<<"e"<<"e"<<"e"<<"e"<<"i"<<"i"<<"i"<<"i"<<"o"<<"n"<<"o"<<"o"<<"o"<<"o"<<"o"<<"o"<<"u"<<"u"<<"u"<<"u"<<"y"<<"y";

    QString output = "";
    for (int i = 0; i < s.length(); i++) {
        QChar c = s[i];
        auto dIndex = diacriticLetters_.indexOf(c);
        if (dIndex < 0) {
            output.append(c);
        } else {
            QString replacement = noDiacriticLetters_[dIndex];
            output.append(replacement);
        }
    }

    return output;
}

QString Utils::removeNonAlphanumerics(const QString &s)
{
    QRegularExpression regex(QString::fromUtf8("[-`~!@#$%^&*()_â€”+=|:;<>Â«Â»,.?/{}\'\"\\[\\]\\\\]"));  // No _ and - removed

    QString rval = s;
    //return rval.remove(regex);
    rval = rval.replace(" ", "");
    return rval.replace(regex, "_");
}

QString Utils::toCamelCase(const QString &s)
{
    QStringList parts = s.split(" ", Qt::SkipEmptyParts);
    for (int i = 0; i < parts.size(); ++i)
        parts[i].replace(0, 1, parts[i][0].toUpper());

    return parts.join(" ");

}

bool Utils::isNewerVersion(QString version)
{
    QStringList versions = version.split(".");
    if (versions.count() < 3){
        return false;
    }

    float minor_version = QString(versions.at(1) + "." + versions.at(2)).toFloat();
    float current_minor = QString(QString(OPENTERAPLUS_VERSION_MINOR) + "." + QString(OPENTERAPLUS_VERSION_PATCH)).toFloat();

    //qDebug() << minor_version << current_minor;

    return versions.at(0).toInt() > QString(OPENTERAPLUS_VERSION_MAJOR).toInt() ||
           current_minor < minor_version;
}

QString Utils::formatFileSize(const int &file_size)
{
    return QLocale().formattedDataSize(file_size, 2, QLocale::DataSizeTraditionalFormat);
}

QString Utils::formatDuration(const QString &duration)
{
    QTime video_duration = QTime::fromString(duration, "h:mm:ss.zzz");
    if (!video_duration.isValid()){
        video_duration = QTime::fromString(duration, "hh:mm:ss.zzz");
        if (!video_duration.isValid()){
            video_duration = QTime::fromString(duration, "hh:mm:ss");
            if (!video_duration.isValid())
                video_duration = QTime::fromString(duration, "h:mm:ss");
        }
    }
    return video_duration.toString("hh:mm:ss");
}

