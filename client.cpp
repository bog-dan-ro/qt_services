#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QtAndroid>
#include <QDebug>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#include <QAndroidIntent>
#include <QAndroidServiceConnection>
#include <QAndroidBinder>
#include <QAndroidParcel>
#include <QColor>
#include <QDir>
#include <QTimer>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

class MyBinder : public QAndroidBinder
{
public:
    bool onTransact(int code, const QAndroidParcel &data, const QAndroidParcel &reply, QAndroidBinder::CallType flags) override
    {
        qDebug() << " ~~ client: onTransact " << code << data.readVariant() << int(flags);
        reply.writeData("Yupee");
        return true;
    }
};

class MyConnection : public QAndroidServiceConnection
{
    // QAndroidServiceConnection interface
public:
    void onServiceConnected(const QString &name, const QAndroidBinder &serviceBinder) override
    {
        qDebug() << "~~ onServiceConnected" << name;
        // test the API

        {
            // test binder
            QAndroidParcel sendData, replyData;
            sendData.writeBinder(m_binder);
            serviceBinder.transact(1, sendData, &replyData);
            qDebug() << replyData.readVariant();
        }
        {
            // test file handler
            QByteArray fileName = QDir::homePath().toLatin1() + "/test";
            QFile f(fileName);
            if (f.open(QIODevice::WriteOnly)) {
                f.resize(0);
                f.write("Test FD Data");
                f.close();
                int fd = open(fileName.constData(), O_RDWR);
                QAndroidParcel sendData, replyData;
                sendData.writeFileDescriptor(fd);
                serviceBinder.transact(2, sendData, &replyData);
                qDebug() << replyData.readVariant();
                close(fd);
            }
        }

        {
            // test QVariant
            QAndroidParcel sendData, replyData;
            QVariantHash testData;
            QVariantMap testMap;
            testMap["1"] = QVariant("one");
            testMap["2"] = QVariant("two");
            testData["0"] = testMap;
            testData["QString"] = QString("QString");
            testData["QColor"] = QColor(Qt::red);
            sendData.writeVariant(testData);
            serviceBinder.transact(3, sendData, &replyData);
            qDebug() << replyData.readVariant();
        }

        {
            // test any data
            QAndroidParcel sendData, replyData;
            sendData.writeData("Here goes any data we like");
            serviceBinder.transact(4, sendData, &replyData);
            qDebug() << replyData.readVariant();
        }
    }

    void onServiceDisconnected(const QString &name) override
    {
        qDebug() << "~~ onServiceDisconnected" << name;
    }

private:
    MyBinder m_binder;
};

struct TestStaticClient
{
    ~TestStaticClient()
    {
        qWarning() << "~~~~ ~TestStaticClient()" << a;
    }
    int a = 0;
};

static TestStaticClient test;

int main(int argc, char *argv[])
{
    test.a = 100;
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));

    MyConnection connection;
    qDebug() << "~~ try to bind service";
    qDebug() << QtAndroid::bindService(QAndroidIntent(QtAndroid::androidActivity(), "com.kdab.training.MyService"),
                                       connection, QtAndroid::BindFlag::AutoCreate);

    QTimer::singleShot(10000, [&app]{app.quit();});
    return app.exec();
}
