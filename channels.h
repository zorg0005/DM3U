#ifndef CHANNELS_H
#define CHANNELS_H

#include <QtWidgets>
#include <QtNetwork>
#include <QtEvents>
#include <Qt>

class MainWindow;

namespace Ui {
class Channels;
}

class ChannelsModel;

class Channels : public QWidget
{
    Q_OBJECT

public:
    explicit Channels(QWidget *parent = 0, MainWindow *mainWindow = 0);
    ~Channels();

    MainWindow *mw;

    // представдение данных
    QAbstractItemModel *aim_epg;
    // структура источника ЕПГ
    struct EpgInfo; /// TODO del after del deprecated
    ///QAbstractListModel alm_sepg;
    enum {STNOSOURCE, STEPGPORTAL, STFILE};
    struct EpgSources {
        QString epgsourcename; // название источника EPG
        QString urlepg; // урл источника
        QString turlprog; // шаблон для телепрограммы
        bool    valid=false; // прочитаны данные или нет
        QList<EpgInfo *> epgchannels; /// TODO deprecated // каналы с ЕПГ
        QDateTime lastupdate; /// TODO дата от протухания
        int sourceType; // тип источника EPG: 0-nosource, 1-epg portal, 2-file
    };
    QList<EpgSources *> listepgs; // список источников
    EpgSources *curEPGs=nullptr;    // текущий выбранный источник (nullptr - все источники)
    // структура информации EPG
    struct EpgInfo {    /// EXTINFO
        EpgSources *epgsource; // источник EPG
        QString epgsourcename; /// TODO deprecated // название источника EPG
        QString cn; // название версии канала по EPG
        QString cndescription; // описание канала
        QString id; // ID EPG
        QString urlcnepg; // урл EPG по каналу
        QStringList groups; // // категории из ЕПГ
        QString urlicon; // урл на иконку канала из источника ЕПГ
    };
    // структура информации по каналу
    struct channelinfo {
        QString cn; // общее название версии канала для всех EPG (собирается со всех EPG или правится пользователем)
        QString cndescription; // описание канала  (утверждается из EPG пользователем)
        QList<EpgInfo *> epgl; // список версий EPG
        QStringList icons; // быстрый список иконок для канала (собирается из EPG и добавляется пользователем)
        QStringList groups; // выбранные категории из ЕПГ и добавленные пользователем
        QString icon; // ссылка на выбранную иконку (это может быть локальный файл) (из быстрого списка channelinfo)
    };
    /// **** в редакторе в режиме расстановки EPG выбирается канал по имени версии,
    /// список имён составляется на основе наличия ID для выбранного источника EPG
    QList<channelinfo *> chanlist; // список каналов

    channelinfo *setChannel(QString cn, bool *exists);  // получаем канал по имени, если нет - Проверка на наличие и добавление канала по имени,
    EpgSources *setEPG(QString sn,bool *exists);  // получаем источник по имени, если нет - Проверка на наличие и добавление источника по имени,
    //EpgInfo *setEPG(QString urlepg, QString cn); /*проверка по источнику EPG, получение или добавление EPG //*/ ///DEPRECATED
    //void bindEPG(channelinfo *ch, EpgInfo *epg); /// deprecated (используем ch->epgl.append(epg);)
    void attachDoubleChannel(channelinfo *cn, channelinfo *dbl); // поглощение(слияние) дублей

    QString baseDriver="TXT";
    ///QList<EpgInfo *> techlistepginfo; deprecated
    QHash<EpgInfo *, quint64> epglink; /// для связи струкур из памяти в базу при сохранении
    QHash<EpgSources *, quint64> sourcelink; /// для связи струкур из памяти в базу при сохранении
    //QHash<EpgInfo const &, quint64> epglinknew; /// TODO для копирования связи струкур из памяти в базу при сохранении
    quint64 maxvalue=1; // последний ID из базы, 0 - сигнализирует об отсутствии связи в epglink (default value)
    ///QHash<int, EpgInfo *> baselink; /// зеркало epglink для связи струкур из базы в память при загрузке базы
    //QHash<int, channelinfo *> chanlink; // для связи струкур в базе и в памяти при сохранении базы и загрузке

    /// общий список (база) каналов формируется из нескольких источников:
    /// EPG-источники
    /// плэйлисты пользователей (TODO необходимо отличать телеканалы от других мультимедийных источников)
    /// сначала делается поиск по совпадению имени, если не найдено оформляется как новый канал
    /// в интерфейсе должен быть механизм объединения одинаковых каналов если они вдруг посчитались как разные от разных источников
    /// имена для сверки источников EPG хранятся в списке структур EpgSources - listepgs,
    /// listepgs используется для работы с порталами EPG
    //QAbstractItemModel *channelsModel;
    ChannelsModel *channelsModel;

    QComboBox *cb;

    void getEPGID(EpgSources *epgsource);
    void readChannelInfo();
    void writeChannelInfo();
    QByteArray getUrl(QUrl url);
    void downloadFile(QUrl url, QString filename);

    bool savebase();
    bool openbase();
private:
    Ui::Channels *ui;

    // структура дополнительных параметров для сетевых запросов
    struct networkReplyes {
        QNetworkAccessManager *nam;
        QString filename;
        QString path;
        QUrl url;
    };
    QHash<QNetworkReply *,networkReplyes> netReplyes;
    QNetworkAccessManager *nam;
    bool eventFilter(QObject *target, QEvent *event);

public slots:
    void setEPGbox();   // прописываются итемы с родительского окна
    //void getEPG(int);  // получение данных по каналу для выбранного источника
    bool readEPGs(EpgSources *epgsource);    // получение всех EPG данных с выбранного источника
    void comBox_currentIndexChanged(int index);
    void saveReply();
    //void dragEnabled(int r, int c);
private slots:
    //void on_comboBox_currentIndexChanged(int index);
};

class ChannelsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ChannelsModel(Channels *dstore, QObject *parent=0);
    //~groupClassModel();

    void startreset();
    void endreset();

    int rowCount(const QModelIndex &parent = QModelIndex()) const; // мы должны реализовать только две функции: rowCount(), возвращающую количество строк в модели, и data(), возвращающую элемент данных, соответствующий определенному модельному индексу.
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const; // Хорошо работающие модели также реализуют headerData() для получения представлениями деревьев и таблиц чего-либо для отображения в их заголовках.
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild);

    Channels *datastore;
protected:

private:
    //QString currentepgsource; use curEPGs
    /// PNUM - порядковый номер
    /// GENNAME - общее название канала
    /// ID - ID по источнику EPG
    /// EPGNAME - название канала из источника EPG
    /// ICON - выбранная иконка
    /// GROUPS - список групп
    /// DES - описание канала
    //int colcount=6;
    enum {PNUM, GENNAME, DES, ID, EPGNAME, ICON, GROUPS, colcount};
    //QList<QString> klassnames;  /// TODO теряет актуальность при изменениях

private slots:
    //void changeKlassnames();
};

#endif // CHANNELS_H
