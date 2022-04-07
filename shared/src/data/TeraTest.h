#ifndef TERATEST_H
#define TERATEST_H

#include <QObject>

class TeraTest : public QObject
{
    Q_OBJECT
public:
    enum TestStatus{
        TEST_STATUS_INCOMPLETE = 0,
        TEST_STATUS_INPROGRESS = 1,
        TEST_STATUS_COMPLETE = 2
    };

    explicit TeraTest(QObject *parent = nullptr);


    static QString getTestStatusName(const TestStatus& status);
    static QString getTestStatusName(const int& status);

    static QString getTestStatusColor(const TestStatus& status);
    static QString getTestStatusColor(const int& status);
signals:

};

#endif // TERATEST_H
