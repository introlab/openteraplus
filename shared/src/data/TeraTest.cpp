#include "TeraTest.h"

TeraTest::TeraTest(QObject *parent)
    : QObject{parent}
{

}

QString TeraTest::getTestStatusName(const TestStatus &status)
{
    switch(status){
        case TEST_STATUS_INCOMPLETE:
            return tr("Incomplet");
        break;
        case TEST_STATUS_INPROGRESS:
            return tr("En cours");
        break;
        case TEST_STATUS_COMPLETE:
            return tr("Complété");
        break;
    }

    return tr("Inconnu");
}

QString TeraTest::getTestStatusName(const int &status)
{
    return getTestStatusName(static_cast<TestStatus>(status));
}

QString TeraTest::getTestStatusColor(const TestStatus &status)
{
    switch(status){
    case TEST_STATUS_INCOMPLETE:
        return "red";
    case TEST_STATUS_COMPLETE:
        return "lightgreen";
    case TEST_STATUS_INPROGRESS:
        return "yellow";
    }
    return "black";
}

QString TeraTest::getTestStatusColor(const int &status)
{
    return getTestStatusColor(static_cast<TestStatus>(status));
}
