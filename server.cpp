#include <QAndroidService>
#include <QAndroidBinder>
#include <QAndroidParcel>
#include <QDebug>
#include <QTimer>
#include <QFile>

class MyBinder : public QAndroidBinder
{
public:
    bool onTransact(int code, const QAndroidParcel &data, const QAndroidParcel &reply, QAndroidBinder::CallType flags) override
    {
        qDebug() << "~~~  onTransact" << code << int(flags);
        switch (code) {
        case 1: {
            QAndroidBinder binder = data.readBinder();
            qDebug() << "~~~  onTransact client sent its binder !";

            // use it to sendback some data
            QAndroidParcel someData, myreply;
            someData.writeVariant(QVariant("Data from server"));
            binder.transact(2, someData, &myreply);
            qDebug() << "~~~  onTransact reply for client's binder" << myreply.readData();
            reply.writeVariant(11);
            }
            break;
        case 2: {
            int fd = data.readFileDescriptor();
            qDebug() << "~~~  onTransact client sent a fileHandle" << fd;
            QFile f;
            f.open(fd, QIODevice::ReadOnly, QFile::DontCloseHandle);
            f.seek(0);
            qDebug() << "~~~  onTransact fileHandle content " << f.readAll();
            reply.writeVariant(22);
            }
            break;
        case 3:
            qDebug() << "~~~  onTransact client sent a QVariant" << data.readVariant();
            reply.writeVariant(33);
            break;
        default:
            qDebug() << "~~~  onTransact client sent unknow data" << data.readData();
            reply.writeVariant(555);
            break;
        }
        return true;
    }
};

struct TestStaticServer
{
    ~TestStaticServer()
    {
        qWarning() << "~~~~ ~TestStaticServer()" << a;
    }
    int a = 0;
};

static TestStaticServer test;

class MyService : public QAndroidService
{
public:
    MyService(int &argc, char **argv) : QAndroidService(argc, argv) {}

    QAndroidBinder *onBind(const QAndroidIntent &/*intent*/) override
    {
        qDebug() << "~~~ create onBind !!!";
        return new MyBinder();
    }
};

int main(int argc, char *argv[])
{
    test.a = 10;
    qDebug() << "~~~ I'm alive !!!";
    // MyService app(argc, argv);
    QAndroidService app(argc, argv, [](const QAndroidIntent &){ return new MyBinder{};});
    QTimer::singleShot(15000, [&app]{app.quit();});
    return app.exec();
}
