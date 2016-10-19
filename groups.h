/// База данных групп (категорий) и их связей.
/// Концепт:
/// 1. Один класс групп может содержать несколько принадлежащих ему списов групп, список групп может принадлежать только одному классу.
/// 2. Один список групп может содержать несколько принадлежащих ему групп. Одна группа может принадлежать только одному списку.
/// 3. Одна группа может содержать несколько значений, значения могут принадлежать разным группам.
/// Пример:
///                 class           class           class               class
///                 (channels)      (channels)      (categories)        (categories)
/// channel name    |grplist (prov1)|grplist (prov2)| grplist (categ)   | grplist (quality) |
/// 1channel        | group (SAT)   | group (AOL)   | group (sport)     | group (HD)        |
/// 2channel        | group (AOL)   |               | group (entertain) | group (SD)        |
/// 3channel        | group (SAT)   |               | group (sport)     | group (SD)        |
/// 4channel        | group (RAI)   | group (SAT)   | group (advent)    | group (HD)        |
/// ///
/// TODO TODO TODO TODO TODO
/// всё это не то, V2 предусмотрит другой концепт.
/// Есть списки групп один к одному, в одном списке членство может быть в одной только группе.
/// Списки один ко многим, в списке членство может быть в нескольких группах.
/// в новом концепте ID ид будет в памяти динамическим и будет использоваться только для сохранения в базе.
/// название класса или группы это уже объект, ссылка уникальна и будет ключём для ID.
/// TODO TODO TODO TODO TODO
/// ///
/// Обычно внешние объекты не знают ничего о группах и категориях, это мы вечно сортируем, распихиваем в группы, категории.
/// Класс хранит только связь идентификатора внешней связи с идентификатором комплекса элементов групп.
/// Идентификатор внешней связи также должен иметь соответствие хэшу внешнего объекта, чтобы распознать его, либо его части, иначе смысла хранить его нет.
/// Класс предоставляет модели
/// каждый внешний элемент для разбивки по группам должен получить уникальный идентификатор внешней связи
/// каждое сочетание групп (категорий) имеет свой уникальный идентификатор, который связывается с внешним идентификатором.
/// первичный список элементов групп, может являться как самостоятельным списком, так и быть родителем подгрупп (подкатегорий)
/// Класс группы - не могут быть смешаны группы разных классов, например группы категорий и группы значений.
/// состав и названия элементов сначала неопределены и определяются пользователем.
/// * createlist(QString) - создание именованного списка групп, на выходе уникальный идентификатор списка (listid)
/// int addgroup(listid, QString, QString) - создаётся новый элемент - группа (категория), на выходе уникальный идентификатор элемента (grpid), на входе название (расшифровка) и идентификатор.
/// если listid нулевой, создаётся группа без привязки к листу (на самом деле "нулевой" лист создаётся)
/// int bind() - создание и получение уникального идентификатора внешней связи (extid). Сначала выполняется привязка внешнего объекта, только затем выполняются операции с его группами.
/// savebase() - сохранение базы групп
/// loadbase() - ЗАГРУЗКА базы
/// int setgroups(int extid, QList) - привязка идентификатора к группам (по grpid), если группа из листа не найдена, добавляется новая.
/// На выходе идентификатор комплекса групп (grpid), на входе внешняя связь(не обязательно) и перечень её групп.
/// QList getlists(int extid) - получение списков групп через привязку
/// QList getgroups(int listid) - получение списка элементов групп через идентификатор
/// int translate(extid, listid, listid) - перепривязка внешнего объекта к другому комплексу групп, на выходе идентификатор элемента из нового комплекса
/// int findgroup(listid, QString) - поиск группы по названию
/// swap(listid, grpid, grpid) - перестановка местами элементов в списке

/// алгоритм действий программы пользующейся классом после запуска рекомендуется такой:
/// 1. проверка наличия классов групп, что список загруженной базы не пустой
/// 2. сверка наличия и регистрация обязательных предопределённых классов групп ****(получаем указатели на структуры)
/// 3. добавление из базы в программу классов, которые были созданы в процессе работы программы ****(получаем указатели на структуры)
/// 4. проверка наличия списков групп из базы
/// 5. сверка наличия и регистрация предопределённых списков групп ****(получаем указатели на структуры)
/// 6. добавление из базы в программу списков групп, которые были созданы в процессе работы программы ****(получаем указатели на структуры)
/// 7. проверка наличия групп из базы
/// 8. сверка наличия и регистрация предопределённых групп  ****(получаем указатели на структуры)
/// 9. добавление из базы в программу групп, которые были созданы в процессе работы программы  ****(получаем указатели на структуры)
/// 10. регистрация предопределённого списка трансляции и дополнение(корректировка?) его из базы    ****()
/// и т.д.
/// Можно использовать часть алгоритма, например графический интерфейс класса использует в работе алгоритм - 3,6,9,10
/// Есть и другие способы работы с классом которые уменьшают степень целесообразности его использования:
/// например только регистрация по алгоритму [clear()],2,5,8, или использование внутренних структур минуя функции.
/// ///
/// далее, для работы с какой либо группой, приложение должно знать - класс группы и список группы, либо их предполагать.
/// для сохранения соответствия с группами, в файлах внутреннего формата хранения данных приложения, рекомендуется использовать -*-( grpId )-*-
/// в выходных файлах внешнего формата хранения используется - ( idName ).
/// Для установки соответствий группам из таких файлов, необходимо знание класса и списка групп, используемые при сохранении этих файлов.
/// Приложение должно зараннее их определить по другим признакам и условиям.

/// в процессе работы алгоритм работы с привязкой объектов рекомендуется следующий:
/// 1. получаем для нового объекта его внешний групповой идентификатор, если объект старый, загружаем идентификатор из базы
/// 2. ... TODO?
/// !!! Необходимо помнить, что получаемые в приложении списки "протухают" (теряют актуальность), и необходимо их актуализировать через ловлю сигналов в приложении.

#ifndef GROUPS_H
#define GROUPS_H

#include <QtWidgets>

namespace Ui {
class Form;
}

class Groups : public QWidget
{
    Q_OBJECT

public:
    explicit Groups(QWidget *parent);
    ~Groups();
    Ui::Form *ui;

    bool event(QEvent *event);
    bool eventFilter(QObject *target, QEvent *event);

    QString baseDriver="TXT"; /// TODO MySQL
    quint64 lastid=10;   // 0 - зарезервирован для "сирот", как для класса так и для списка. "Сиротский" ид - это ошибка
                        /// !! возможно сократить внутреннее использование и применять динамически для сохранения и чтения базы?
                        /// этого делать не надо чтобы не потерять сцепку между версиями разных частей сохранений, например базы и файлов данных внутреннего формата программы
    quint64 extid=10;    // 0 - резерв, ошибка
    ///bool siroty=false;          // индикатор наличия сирот

    struct GroupList;

    struct Group{
        //QString *klass;     // класс группы# no, достаточно класса списка
        QString displayName;    // отображаемое имя (напр: "0 - Общие")
        QString idName;     // идентификатор группы внешнего формата, имя используемое в обработке данных и сохранениях (напр: "0")
        ///GroupList *grplist;  // принадлежность к списку
        quint64 listId;
        quint64 grpId;
    };

    struct GroupList{
        quint64 klassId;     // класс к которому относятся группы списка
        quint64 listId;
        QString Name;           // имя списка
        QList<Group *> List;    // фильтрованный список, указатели на Group
        GroupList() {;} //конструктор по умолчанию
        GroupList(QString name) {Name=name;}
    };

    struct KlassGrp{
        QString displayName;
        quint64 klassId;
    };

    QList<Group *> groups;          // список всех групп, указатели на Group
    QList<GroupList *> grplists;    // групповые списки, указатели на GroupList
    QList<KlassGrp *> klasslist;    // списки классов групп (например: разные плэйлисты - группы каналов, или разные спецификации групп - категории каналов)

    /// список трансляции, это "горизонтальный" список, который определяет соответствие элементов "вертикальных" списков
    /// "вертикальными" списками являются списки групп. Список трансляции это тот же вертикальный список но с зарезервированным
    /// классом - 1. По сути мы пришли к представлению плоской таблицы ссылочного типа. Вертикальные списки определяют значения столбцов,
    /// Списки класса 1 определяют строки, остальные значения классов определяют базы данных для вертикальных списков.
    /// таким образом, раз для списков трансляции (строк) не определяется принадлежность к другим классам, значит они могут содержать
    /// ссылки на другие "базы" - делать трансляцию между списками разных классов.
    QList<GroupList *> translists;    // списки трансляций

    //struct
    QHash<quint64, quint64> extlinks; // <extid, extold> ** список внешних связей

    void clear();
    /// TODO? void restorebase(); // scan base, restore lastid,
    KlassGrp *getklass(QString klass); // определение класса по названию, если такого нет создаётся новый
    QHash<KlassGrp *, QString> klassnames();
    GroupList *setlist(QString name, KlassGrp *stklass);   //возвращает или создаёт лист по указанному классу списка (необходимо разделение функций создания и редактирования от получения)
    QHash<GroupList *, QString> getlists(quint64 klass);
    Group* addgroup(GroupList *stlist, QString dispName, QString name);    /// addgroup to list добавление группы (+проверка на дубли)
    Group* setgroup(GroupList *stlist, QString dispName, QString name);    /// создание и редактирование группы
    QHash<Group *, QString> getgroupnames(quint64 klass, GroupList *grplist);  // получение списка отображаемых имён групп
    QHash<Group *, QString> getgroupids(quint64 klass, GroupList *grplist);    // получение списка идентификаторов групп внешнего формата
    // хранение в базе внешних связей плохая идея, и вообще всего того что обрабатывается внешне
    /// при загрузке базы идентификаторы внешней связи обновляются, старый высвобождается и присваивается новый, если старый 0, вырабатывается новый, без регистрации соответствия со старым,
    /// после загрузки должна осуществляться перепривязка объектов, чтобы уникальные идентификаторы не кончились, если не делается освобождение идентификаторов.
    /// Перепривязка порождает недостаток, необходимо перепривязывать все внешние объекты, это может быть затратно.
    quint64 bind(quint64 oldextid); // получаем уникальный ID для привязываемого объекта
    void unbind(quint64 extid);
    bool savebase();    // TXT жёстко привязано к каталогу приложения ./base
    bool openbase();    // TXT жёстко привязано к каталогу приложения ./base

    /// GUI ///
    QAbstractItemModel *groupklassmodel;

private slots:

    /// GUI ///
    //void on_toolButton_clicked();
    void comboBoxDeleteItem();
    void comboBoxEditItem();
    void on_comboBox_2_currentIndexChanged(int index);

private:
    QHash<Group *, QString> getgroups(quint64 klass, GroupList *grplist, int mode);

    /// TODO добавляем сигналы для уведомления изменений в списках, что добавили, что переименовали send renamedxxx, addedxxx
    ///
};

class groupClassModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    groupClassModel(Groups *dstore, QObject *parent=0);
    //~groupClassModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const; // мы должны реализовать только две функции: rowCount(), возвращающую количество строк в модели, и data(), возвращающую элемент данных, соответствующий определенному модельному индексу.
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const; // Хорошо работающие модели также реализуют headerData() для получения представлениями деревьев и таблиц чего-либо для отображения в их заголовках.
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());

protected:

private:
    Groups *datastore;
    //QList<QString> klassnames;  /// TODO теряет актуальность при изменениях

private slots:
    //void changeKlassnames();
};

/// стиль рисующий заданную фразу если индекс == -1
class ComboBoxProxyStyle : public QProxyStyle
{
    Q_OBJECT
    Q_PROPERTY(QString placeHolder READ placeHolder WRITE setPlaceHolder)
public:
    ComboBoxProxyStyle(const QString& placeHolder)
    {
        setPlaceHolder(placeHolder);
    }

    virtual void drawControl(ControlElement element,
                             const QStyleOption* option,
                             QPainter* painter, const QWidget* widget) const
    {
        QStyleOption* opt = const_cast<QStyleOption*>(option);

        if (element == QStyle::CE_ComboBoxLabel) {
            QWidget* w = const_cast<QWidget*>(widget);
            QComboBox* cbx = qobject_cast<QComboBox*>(w);
            QStyleOptionComboBox* cb =
                            qstyleoption_cast<QStyleOptionComboBox*>(opt);
            if (cb && cbx) {

                if (-1 == cbx->currentIndex()) {
                    QPalette pal = cb->palette;
                    pal.setBrush(QPalette::Text, pal.mid());
                    cb->currentText = placeHolder();
                    cb->palette = pal;
                }
            }
        }

        QProxyStyle::drawControl(element, opt, painter, widget);
    }

    virtual void drawItemText(QPainter *painter,
                              const QRect &rect,
                              int flags,
                              const QPalette &pal,
                              bool enabled,
                              const QString &text,
                              QPalette::ColorRole textRole) const
    {
        if (placeHolder() == text) {
            textRole = QPalette::Text;
        }
        QProxyStyle::drawItemText(painter, rect, flags, pal, enabled, text, textRole);
    }
    QString placeHolder() const {return m_placeHolder;}
    void setPlaceHolder(const QString& text) {m_placeHolder = text;}

private:
    QString m_placeHolder;
};

#endif // GROUPS_H
