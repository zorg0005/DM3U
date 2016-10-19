#include "groups.h"
/// функции реализации без виджета

void Groups::clear()
{
    QList<GroupList *>::iterator i=grplists.begin();
    while (i!=grplists.end()) {
        QList<Group *> groups=(*i)->List;
        QList<Group *>::iterator z=groups.begin();
        while (z != groups.end()) {
            delete *z;
        }
        ///groups.clear();
        delete *i;
        ++i;
    }
    grplists.clear();
    QList<KlassGrp *>::iterator it=klasslist.begin();
    while (it!=klasslist.end()) {
        delete *it;
        ++it;
    }
    klasslist.clear();
    /// TODO send cleared();
}

// возвращает ид существующего или нового класса групп по имени
Groups::KlassGrp * Groups::getklass(QString klass)
{
    QString temp=klass.toLower();
    QList<Groups::KlassGrp *>::iterator i=klasslist.begin();
    while (i != klasslist.end()) {
        QString temp2=(*i)->displayName.toLower();
        if (temp==temp2) {
            break;
        }
        ++i;
    }
    if (i == klasslist.end()) {
        klasslist.append(new KlassGrp);
        klasslist.last()->displayName=klass;
        // смотрим, не сиротская ли группа по названию, корректируем класс
        if (klass.isEmpty() || klass.isNull()) {
            klasslist.last()->klassId=0;
        } else {
            klasslist.last()->klassId=lastid++;
        }
        //i= .last();
        /// TODO send addedklass();
    }
    return *i; // возвращаем id
}

// возвращает наименования классов групп
// теряет актуальность по сигналам addedklass() changeklass() и cleared();
QHash<Groups::KlassGrp *, QString> Groups::klassnames()
{
    QHash<Groups::KlassGrp *, QString> list;
    QList<Groups::KlassGrp *>::iterator i=klasslist.begin();
    while (i != klasslist.end()) {
        list.insert(*i,(*i)->displayName);
        ++i;
    }
    return list;
}

// заменяет createlist и findlist, возвращает или создаёт лист по указанному классу списка
Groups::GroupList * Groups::setlist(QString name, KlassGrp *stklass)
{
    // проверка ссылки на наличие в списке, заодно купируем нулевую ссылку
    QHash<Groups::KlassGrp *, QString> klases=klassnames();
    if (!klases.contains(stklass) || name.isEmpty() || name.isNull()) { // ||класс есть но имя нулевое
        name="";
        stklass=getklass(""); // определяем группу сирот, если таких нет
    }
    quint64 klass=stklass->klassId;

    QString temp=name.toLower();    // приводим к нижнему регистру для уникальности
    QString temp2;
    // проверяем наличие
    QList<Groups::GroupList *>::iterator i = grplists.begin();
    while (i != grplists.end()) {
        temp2=(*i)->Name.toLower();
        if (temp2==temp && (*i)->klassId==klass)
            break;
        else
            ++i;
    }
    if (i == grplists.end()) {
        // если нет дублей
        i=grplists.insert(i,new GroupList(name));
        (*i)->klassId=klass; // (*i) was == grplists.end() will == grplists.last()
        if (klass == 0) {
            (*i)->listId=0;
        } else {
            (*i)->listId=lastid++;
        }
        /// TODO send addedlist();
    }
    return *i;
}

// возвращает наименования списков групп определённого класса
// теряет актуальность по сигналам addedlist() changelist() и cleared();
/// если класс == 0 то получаем список всех листов (мы ведь знаем что 0 - сирота)
QHash<Groups::GroupList *, QString> Groups::getlists(quint64 klass)
{
    QHash<Groups::GroupList *, QString> list;
    QList<Groups::GroupList *>::iterator i = grplists.begin();
    while (i != grplists.end()) {
        if ((*i)->klassId==klass || klass == 0)
            list.insert(*i,(*i)->Name);
        ++i;
    }
    return list;
}

Groups::Group *Groups::addgroup(Groups::GroupList *stlist, QString dispName, QString name)
{
    // проверка ссылки на наличие в списке, заодно купируем нулевую ссылку
    QHash<Groups::GroupList *, QString> list=getlists(0);
    if (!list.contains(stlist)) {
        stlist=setlist("", nullptr); // определяем группу сирот, если таких нет
    }
    quint64 listid=stlist->listId;

    // проверяем наличие дублей
    QString temp=name.toLower();    // приводим к нижнему регистру для уникальности
    QString temp2=dispName.toLower();
    QString temp3, temp4;
    QList<Groups::Group *>::iterator i=stlist->List.begin();
    while (i != stlist->List.end()) {
        temp3=(*i)->displayName.toLower();
        temp4=(*i)->idName.toLower();
        if (temp3==temp2 && temp4==temp) {
            // если всё совпало возвращаем группу
            break;
        }
        /// TODO
        /// когда name одинаковые а dispName разные это как-то терпимо в обработке данных приложений участвуют name
        /// но вносит путаницу при восстановлении dispName к name (TODO внести в интерфейс поиск и объединение dispName одинаковых name)
        /// но когда dispName одинаковый для разных name - это не допустимо, выход в переделке dispName.
        if (temp3==temp2 && temp4!=temp){/// || (temp3!=temp2 && temp4==temp) {  // смотря что выпадет вперёд, а если есть в разных записях и совпадение по name и по dispName (такое возможно в случае сбоя)
            /// TODO если что-то совпало выдаём предупреждение и варианты решения
            /// 1. отредактировать существующую группу *** TODO отрабатывает по умолчанию эта
            /// 2. добавить новую с индексом ДБЛ
            /// 3. отказаться от операции
            /// 4. принять данные существующей группы
            // меняем текущее отображаемое имя
            (*i)->displayName=(*i)->displayName+QString(" (\"")+(*i)->idName+QString("\")");
            // меняем новое
            dispName=dispName+QString(" (\"")+name+QString("\")");
            // проверяем список дальше
        } else if (temp3!=temp2 && temp4==temp) {
            // совпало только name, модифицируем текущее отображаемое имя
            (*i)->displayName=(*i)->displayName+QString("\\")+dispName;
            break; // возвращаем группу
        }
            ++i;
    }
    if (i == stlist->List.end()) {
        // новая группа
        i=stlist->List.insert(i,new Group);     // добавляем в фильтрованный список
        stlist->List.last()->grpId=lastid++;
        groups.append(stlist->List.last()); // скидываем в общий список
        (*i)->displayName=dispName;
        (*i)->idName=name;
        ///grp->grplist=stlist;
        (*i)->listId=listid;
    }

    /// TODO send changedgroup();
    return (*i);
}

// создание и редактирование группы
//// !!!!!!!! в дальнейшем останется только редактирование TODO
Groups::Group *Groups::setgroup(Groups::GroupList * stlist, QString dispName, QString name)
{
    // проверка ссылки на наличие в списке, заодно купируем нулевую ссылку
    QHash<Groups::GroupList *, QString> list=getlists(0);
    if (!list.contains(stlist)) {
        stlist=setlist("", nullptr); // определяем группу сирот, если таких нет
    }
    quint64 listid=stlist->listId;

    // проверяем наличие дублей
    QString temp=name.toLower();    // приводим к нижнему регистру для уникальности
    QString temp2=dispName.toLower();
    QString temp3, temp4;
    QList<Groups::Group *>::iterator i=stlist->List.begin();
    while (i != stlist->List.end()) {
        temp3=(*i)->displayName.toLower();
        temp4=(*i)->idName.toLower();
        if (temp3==temp2 || temp4==temp) {
            // если что-то совпало считаем это редактированием
            break;
        } else
            ++i;
    }
    if (i == stlist->List.end()) {
        // новая группа
        i=stlist->List.insert(i,new Group);     // добавляем в фильтрованный список
        stlist->List.last()->grpId=lastid++;
        groups.append(stlist->List.last()); // скидываем в общий список
    }
    Group *grp=*i;

    grp->displayName=dispName;
    grp->idName=name;
    ///grp->grplist=stlist;
    grp->listId=listid;
    /// TODO send changedgroup();
    return grp;
}

// возвращает список групп определённого списка
// теряет актуальность по сигналам changedgroup() changelist() и cleared();
/// если grplist == 0 то получаем список всех групп (мы ведь знаем что 0 - сирота) определённого класса
/// klass == 0 получаем список групп всех классов (кроме зарезервированных)
QHash<Groups::Group *, QString> Groups::getgroupnames(quint64 klass, Groups::GroupList * grplist)
{
    return getgroups(klass, grplist, 1);
}

// возвращает список групп определённого списка
// теряет актуальность по сигналам changedgroup() changelist() и cleared();
/// если grplist == 0 то получаем список всех групп (мы ведь знаем что 0 - сирота) определённого класса
/// klass == 0 получаем список групп всех классов (кроме зарезервированных)
QHash<Groups::Group *, QString> Groups::getgroupids(quint64 klass, Groups::GroupList * grplist)
{
    return getgroups(klass, grplist, 2);
}

// возвращает список групп определённого списка
/// если grplist == 0 то получаем список всех групп (мы ведь знаем что 0 - сирота) определённого класса
/// klass == 0 получаем список групп всех классов (кроме зарезервированных)
/// если моде == 1 - получаем имена групп, если == 2 - внешние идентификаторы.
QHash<Groups::Group *, QString> Groups::getgroups(quint64 klass, Groups::GroupList * grplist, int mode)
{
    // проверка ссылки на наличие в списке, заодно купируем нулевую ссылку
    QHash<Groups::GroupList *, QString> list=getlists(0);
    if (!list.contains(grplist)) {
        grplist=nullptr; /// такой ссылки нет, значит берём группы со всех листов
    }
    QHash<Groups::Group *, QString> listgroups; // указатель на структуру группы и отображаемое имя
    QList<Groups::GroupList *>::iterator i = grplists.begin();
    while (i != grplists.end()) {
        if ((((*i)->klassId==klass && grplist==nullptr) || klass == 0) || (grplist==(*i) && (*i)->klassId==klass)) {
            QList<Groups::Group *>::iterator it = (*i)->List.begin();
            while (it != (*i)->List.end()) {
                if (mode == 1)
                    listgroups.insert(*it,(*it)->displayName);
                else if (mode == 2)
                        listgroups.insert(*it,(*it)->idName);
                ++it;
            }
        }
        ++i;
    }
    return listgroups;
}

quint64 Groups::bind(quint64 oldextid)
{
    if (oldextid) oldextid=oldextid; /// TODO заглушка от ворнингов
    return extid++;
}

void Groups::unbind(quint64 extid)
{
    if (extid) extid=extid; /// TODO заглушка от ворнингов
    ;
}

bool Groups::savebase()
{
    /// TODO mySQL
    if (baseDriver=="MYSQL") {
        ;
    }
    // По умолчанию baseDriver=="TXT"
    QString AppPath=qApp->applicationDirPath();
    QDir basedir(AppPath+QString("/base")); /// TODO вынести в настройки
    if (!basedir.exists()) {
        basedir.mkpath(basedir.path()); /// TODO проверка результата
    }

    /// TODO Делаем бэкап для отката?
    // открываем файлы
    QString filename=AppPath;
    filename+="/base/groups.dat";   /// TODO вынести в настройки
    QFile fout(filename);
    bool fopen=fout.open(QIODevice::WriteOnly);
    if (!fopen) {
        QMessageBox::critical(QApplication::activeWindow(),tr("Ошибка!!!"),tr("Файл не открывается.") + fout.errorString());  // ::about(
        return false;
    }
    QTextStream out(&fout);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);
    // заголовок и служебные строки
    out << "#TXTBASEGROUPS" << endl;
    out << "[CLASSES]" << endl;
    QList<Groups::KlassGrp *>::iterator i=klasslist.begin();
    while (i != klasslist.end()) {
        out << (*i)->klassId << "\t" << (*i)->displayName << endl;
        ++i;
    }
    out << "[LISTS]" << endl;
    QList<Groups::GroupList *>::iterator it = grplists.begin();
    while (it != grplists.end()) {
        out << (*it)->listId << "\t" << (*it)->klassId << "\t" << (*it)->Name << endl;
        ++it;
    }
    out << "[GROUPS]" << endl;
    QList<Groups::Group *>::iterator ite = groups.begin();
    while (ite != groups.end()) {
        out << (*ite)->grpId << "\t" << (*ite)->listId << "\t" << (*ite)->idName << "\t" << (*ite)->displayName << endl;
        ++ite;
    }
    out << "[PARAMETERS]" << endl;
    out  << "LASTID\t" << lastid << endl \
         << "EXTID\t" << extid \
         << endl;
return true;
}

bool Groups::openbase()
{
    /// TODO mySQL
    if (baseDriver=="MYSQL") {
        ;
    }
    // По умолчанию baseDriver=="TXT"
    QString AppPath=qApp->applicationDirPath();
    QDir basedir(AppPath+QString("/base"));
    if (!basedir.exists()) {
        basedir.mkpath(basedir.path()); /// TODO проверка результата
        /// генерируем новую базу ?
    }

    // открываем файлы
    QString filename=AppPath;
    filename+="/base/groups.dat";
    QFile fin(filename);
    bool fopen=fin.open(QIODevice::ReadOnly);
    if (!fopen) {
        ///QMessageBox::critical(QApplication::activeWindow(),tr("Ошибка!!!"),tr("Файл не открывается.") + fin.errorString());  // ::about(
        /// будем создавать новый
        return false;
    }
    QTextStream in(&fin);
    QString temp=in.readLine();
    if (temp!="#TXTBASEGROUPS") { // страхуемся, проверяем что первая строка принадлежит базе
        ///QMessageBox::critical(QApplication::activeWindow(),tr("Ошибка!!!"),tr("База данных для групп не обнаружена!");  // ::about(
        /// будет создаваться с нуля
        return false;
    }
    temp=in.readLine();
    quint64 oldlistid=0-1;
    Groups::GroupList *gls;
    QStringList fields;
    enum mode_phase{ CLASSES, LISTS, GROUPS, PARAMETERS};
    int mode=999;
    while (!temp.isNull()) {
        fields=temp.split("\t");
        // селектор секций
        if (fields.size()==1) { // если одна фраза то это новая секция
            if (temp=="[CLASSES]") { // переключаем моду
                mode=CLASSES;
            } else {
                if (temp=="[LISTS]") { // переключаем моду
                    mode=LISTS;
                } else {
                    if (temp=="[GROUPS]") { // переключаем моду
                        mode=GROUPS;
                    } else {
                        if (temp=="[PARAMETERS]") { // переключаем моду
                            mode=PARAMETERS;
                        }
                    }
                }
            }
            temp=in.readLine();
            continue;
        }
        // обработчик секций
        switch (mode) {
            case GROUPS: {
                Group *grp=new Group;
                // прописываем с индексов по порядку сохранения
                grp->grpId=fields.at(0).toULongLong();
                grp->listId=fields.at(1).toULongLong();
                grp->idName=fields.at(2);
                grp->displayName=fields.at(3);
                /// ищем список группы // значение уже определено, благодаря порядку сохранения 1. Классы -> 2.Списки -> 3.Группы
                if (oldlistid != grp->listId) { // если не совпадает с предыдущим списком
                    QList<Groups::GroupList *>::iterator it = grplists.begin();
                    while (it != grplists.end()) {          /// TODO медленный цикл (вопрос о необходимости в переменной grplist)
                        if ((*it)->listId == grp->listId) {
                            oldlistid=(*it)->listId;    // находим список которому принадлежит группа
                            gls=*it;
                            break;
                        }
                        ++it;
                    }
                    /// TODO проверка на найденность
                }
                // вносим в необходимые структуры
                gls->List.append(grp); // вносим группу в список
                ///grp->grplist=gls;
                groups.append(grp);
                break;
            }
            case LISTS: {
                GroupList *gl=new GroupList;
                // прописываем с индексов по порядку сохранения
                gl->listId=fields.at(0).toULongLong();
                gl->klassId=fields.at(1).toULongLong();
                gl->Name=fields.at(2);
                //gl->List // определяем список, которого ещё нет
                grplists.append(gl);
                break;
            }
            case CLASSES: {
                KlassGrp *kg=new KlassGrp;
                // прописываем с индексов по порядку сохранения
                kg->klassId=fields.at(0).toULongLong();
                kg->displayName=fields.at(1);
                //
                klasslist.append(kg);
                break;
            }
            case PARAMETERS: {
                if (fields.at(0)=="LASTID") lastid=fields.at(1).toULongLong();
                if (fields.at(0)=="EXTID") extid=fields.at(1).toULongLong();
                break;
            }
            default: {
                break;
            }
        }
        /// TODO send loadbase(); or changeklass() + changedgroup() + changelist()

        temp=in.readLine();
    }

    return true;
}


groupClassModel::groupClassModel(Groups *dstore, QObject *parent) : QAbstractItemModel(parent)
{
    datastore=dstore;
    //QHash<Groups::KlassGrp *, QString> klasses=dstore->klassnames();
    //klassnames=klasses.values();
    //changeKlassnames();
}

int groupClassModel::rowCount(const QModelIndex &parent) const
{
    qDebug() << parent;  /// TODO заглушка от ворнингов
    return datastore->klasslist.size();
}

int groupClassModel::columnCount(const QModelIndex &parent) const
{
    qDebug() << parent;  /// TODO заглушка от ворнингов
    return 1;   // у нас только один список классов групп (значит одна колонка)
}

QVariant groupClassModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= datastore->klasslist.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        //return klassnames.at(index.row());
        Groups::KlassGrp *temp=datastore->klasslist.at(index.row());
        return temp->displayName;
    }
    else {
        return QVariant();
    }
}

bool groupClassModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {  // для булевых значений Qt::CheckStateRole
        //klassnames.replace(index.row(), value.toString()); а кто будет исходные данные менять?
        ////qDebug() << "=" << index.row() ;
        Groups::KlassGrp *temp=datastore->klasslist.at(index.row());
        temp->displayName=value.toString();
        emit dataChanged(index, index); // Модель должна дать знать представлениям, что данные изменены. Изменился только один элемент данных, в сигнале диапазон элементов ограничен одним модельным индексом.
        return true;
    }
    return false;
}

QVariant groupClassModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
             return QVariant();

    if (orientation == Qt::Horizontal) {
        //return QString("Column %1").arg(section);
        return QString(tr("Классы групп"));
    }
    else {
        return QString("Row %1").arg(section);
    }
}

Qt::ItemFlags groupClassModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;  // для булевых значений Qt::ItemIsUserCheckable;
}

QModelIndex groupClassModel::index(int row, int column, const QModelIndex &parent) const
{
    qDebug() << parent;  /// TODO заглушка от ворнингов
    //if (!parent.isValid() || row==-1 || column == -1)
    ///qDebug() << row;
    if (row==-1 || row>=datastore->klasslist.size() || column == -1)
        return QModelIndex();

    ////qDebug() << row << datastore->klasslist.size();
    Groups::KlassGrp *temp=datastore->klasslist.at(row);
    //return hasIndex(row, column, parent) ? createIndex(row, column, (void*)&temp->displayName) : QModelIndex();
    return createIndex(row, column, (void*)&temp->displayName);
}

QModelIndex groupClassModel::parent(const QModelIndex &index) const
{
    qDebug() << index;  /// TODO заглушка от ворнингов
    //return createIndex(row, column, (void*)&temp->displayName)
    return QModelIndex();
}//*/

bool groupClassModel::insertRows(int position, int rows, const QModelIndex &index)
{
    qDebug() << index;  /// TODO заглушка от ворнингов
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        datastore->getklass(""); // добавляем пустую строку
        if (!datastore->klasslist.isEmpty()) {
            datastore->klasslist.move(datastore->klasslist.size()-1,position);    // перемещаем в позицию вставки
        } else {
            //
        }
        //datastore->klasslist.insert();
        //stringList.insert(position, "");
    }

    endInsertRows();
    return true;
}

bool groupClassModel::removeRows(int position, int rows, const QModelIndex &index)
{
    qDebug() << index;  /// TODO заглушка от ворнингов
    beginRemoveRows(QModelIndex(),position,position+rows-1);
    for (int row = 0; row < rows; ++row) {
        Groups::KlassGrp *temp=datastore->klasslist.at(position);
        delete temp;
        datastore->klasslist.removeAt(position);
    }
    endRemoveRows();
    return true;
}

/*void groupClassModel::changeKlassnames()
{
    QHash<Groups::KlassGrp *, QString> klasses=datastore->klassnames(); /// TODO для быстроты реализовать свой цикл перебора и заполнения klassnames
    klassnames=klasses.values();
    ///emit
}//*/

