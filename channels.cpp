#include "channels.h"
#include "ui_channels.h"
#include "mainwindow.h"
#include "groups.h"

int test=0;

Channels::Channels(QWidget *parent, MainWindow *mainWindow) :
    QWidget(parent),
    ui(new Ui::Channels)
{
    ui->setupUi(this);
    //Ui::MainWindow *mw;//=(Ui::MainWindow *)parent;
    mw=mainWindow;
    cb=ui->comboBox;

    QStatusBar *SB=new QStatusBar(this);
    ui->verticalLayout->addWidget(SB);
    SB->showMessage("Hi!!!");

    nam=new QNetworkAccessManager;

    //ui->comboBox->setModel(mainWindow->epgcb->model());
    /// заполняем список известных источников EPG /// TODO in parameters
    Channels::EpgSources *tmp=new EpgSources;
    listepgs.append(tmp);
    listepgs.last()->epgsourcename=QString("vsetv.com");
    listepgs.last()->turlprog=QString("http://www.vsetv.com/schedule_channel_1079_day_2016-05-25.html");
    listepgs.last()->urlepg=QString("http://www.vsetv.com/channels.html");
    listepgs.last()->sourceType=STEPGPORTAL;
    // структура готова добавляем элемент
    QVariant variant;
    variant.setValue(static_cast<void *>(tmp));
    ui->comboBox->addItem("vsetv.com",variant);
    //
    tmp=new EpgSources();
    listepgs.append(tmp);
    listepgs.last()->epgsourcename=QString("s-tv.ru");
    listepgs.last()->turlprog=QString(""); /// пока такой задачи нет
    listepgs.last()->urlepg=QString("http://www.s-tv.ru/tv/");
    listepgs.last()->sourceType=STEPGPORTAL;
    variant.setValue(static_cast<void *>(tmp));
    ui->comboBox->addItem("s-tv.ru",variant);
    //
    listepgs.append(new EpgSources);
    listepgs.last()->epgsourcename=QString("teleguide.info");
    listepgs.last()->turlprog=QString(""); /// пока такой задачи нет
    listepgs.last()->urlepg=QString("http://teleguide.info/kanals.html");
    listepgs.last()->sourceType=STEPGPORTAL;
    variant.setValue(static_cast<void *>(listepgs.last()));
    ui->comboBox->addItem("teleguide.info",variant);
    //
    connect(ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(comBox_currentIndexChanged(int)));
    // подключаем слоты активации drag&drop
    ///connect(ui->tableWidget,SIGNAL(cellPressed(int,int)),this,SLOT(dragEnabled(int,int)));

    // регистрация фильтра событий для таблицы
    // После регистрации фильтра, все события, которые предназначены объектам сначала попадут в обработчик Channels::eventFilter() - this.
    ui->tableView->installEventFilter(this);

    /// создаём хранилище списка каналов и EPG
    // chanlist уже объявлен

    channelsModel=new ChannelsModel(this,0);
    if (!openbase()){
        QMessageBox::critical(QApplication::activeWindow(),tr("Ошибка!!!"),tr("Ошибка открытия базы каналов!"));
    }
    ui->tableView->setModel(channelsModel);

    /// TODO теперь надо бы пройтись по источникам для заполнения базы если она пустая
    for (int i=0; ui->comboBox->count()>i;i++) {
        comBox_currentIndexChanged(i);
    }
    ui->tableView->resizeColumnToContents(0);  // ресайзим номерную колонку по заполнению
    // ставим первый
    if (ui->comboBox->count()>0)
        ui->comboBox->setCurrentIndex(0);
}

Channels::~Channels()
{
    delete ui;
}

Channels::channelinfo *Channels::setChannel(QString cn, bool *exists)
{
    // проверяем наличие данного канала
    *exists=false;
    channelinfo *temp;
    EpgInfo* tepg;
    //bool duble=false;
    foreach (temp, chanlist) {
        // проверка по названию
        ///if (temp->cn==cn) { /// TODO версия с учётом регистра
        if (temp->cn.toLower()==cn.toLower()) { /// TODO версия без учёта регистра
            *exists=true;
            return temp;
        }
        // по имени не совпало, проверка по именам из источников
        foreach (tepg, temp->epgl) {
            if (tepg->cn.toLower()==cn.toLower()) { /// TODO версия с учётом регистра
                *exists=true;
                return temp;
            }
        }
    }
    Channels::channelinfo *nc=new Channels::channelinfo();
    chanlist.append(nc);
    nc->cn=cn;
    return nc;
}

Channels::EpgSources *Channels::setEPG(QString sn, bool *exists)
{
    // проверяем наличие источника
    *exists=false;
    EpgSources *temp;
    foreach (temp, listepgs) {
        // проверяем по названию
        if (temp->epgsourcename.toLower()==sn.toLower()) { /// TODO? версия с учётом регистра
            *exists=true;
            return temp;
        }
    }
    temp=new EpgSources();
    listepgs.append(temp);
    temp->epgsourcename=sn;
    return temp;
}

/*Channels::EpgInfo *Channels::setEPG(QString urlepg, QString cn)
{
    //доступ к списку есть только через список каналов и epgsources
    //поехали
    channelinfo *temp;
    EpgInfo* tepg;
    // проверка на дубли epg
    foreach (temp, chanlist) {
        foreach (tepg, temp->epgl) {
            if (tepg->cn==cn) {
                return temp;
            }
        }
    }
    Channels::channelinfo *nc=new Channels::channelinfo();
    chanlist.append(nc);
    nc->cn=cn;
    return nc;
}//*/

void Channels::setEPGbox()
{
    //QComboBox  *cb = qobject_cast<QComboBox*>(sender());
    //ui->comboBox->setModel(cb->model());
}

// проверка наличия и запуск получения EPG в хранилище
bool Channels::readEPGs(EpgSources *epgsource)
{
    qint64 d=1; /// TODO in parameters
    // получаем текущий список каналов и epgID c указанного сайта
    if (!curEPGs->lastupdate.isValid() or // проверка на валидность если база новая
            //!epgsource->valid or // проверяем читали мы уже epgid или нет
            curEPGs->lastupdate.daysTo(QDateTime::currentDateTime())>d  //in days // if (curEPGs->lastupdate.secsTo(QDateTime::currentDateTime())/60 > h) { //in hours
            ) {
        epgsource->epgchannels.clear(); // очищаем
        Channels::getEPGID(epgsource);
    }
    /// TODO проверка завершения чтения без ошибки
    epgsource->valid=true;
    /// else false;
    /// return false;
    return true;
}

/*void Channels::on_comboBox_currentIndexChanged(int index)
{

}//*/

void Channels::comBox_currentIndexChanged(int index)
{
    if (index) index=index; /// TODO заглушка от ворнингов
    curEPGs=static_cast<Channels::EpgSources *>(ui->comboBox->currentData().value<void *>());
    bool res=false;
    res=readEPGs(curEPGs);
    if (res) {
        res=res;  /// TODO заглушка от ворнингов
    }
    // очищаем от старого или другого EPG
    //mw->epgchModel.clear();
    // запихиваем список в combobox 4
    /// TODO if (listepgs->at(index).valid)
    /*/ получаем хранилище списка
    QHashIterator<QString, QString> i(*tst2);
    while(i.hasNext()) {
        i.next();
        epgchModel.appendRow(new QStandardItem(i.value()));
    }
    epgchModel.sort(0);//*/
    /// TODO переопределяем колонку EPG со старых на новые ID
    // чтобы инициировать новое фильтрованное отображение модели в таблице
    channelsModel->endreset();
    /// TODO установим фокус на текущую ячейку заново
}

// получение и парсинг из EPG источника
void Channels::getEPGID(EpgSources *epgsource)
{
    //EpgSources *epgsource=listepgs.at(idx);
    QUrl url=epgsource->urlepg;

    QByteArray bytes=getUrl(url);

    // парсинг полученой страницы
    // проверяем какая установлена кодировка на странице
    QRegExp exp("\\bcharset=([^\"> ]+)\\b");
    QString string;
    if ((exp.indexIn(bytes)) != -1) {
        QString charset=exp.cap(1);
        QTextCodec *codec = QTextCodec::codecForName(charset.toLocal8Bit().data());
        string = codec->toUnicode(bytes);
    } else { // используем без перекодировки
        QString temp(bytes);
        string=temp;
    }

    // проверка каталога для иконок
    QString AppPath=qApp->applicationDirPath();
    //QString iconsdir=AppPath+QString("/ch_icons");
    QDir iconsdir(AppPath+QString("/ch_icons"));
    if (!iconsdir.exists()) {
        iconsdir.mkpath(iconsdir.path()); /// TODO проверка результата
    }
    /// далее парсинг в зависимости от источника
    /// string - скачанный html
    EpgInfo *tmp;
    // готовим модель к заполнению
    ///channelsModel->beginInsertRows(QModelIndex(),0,100);
    channelsModel->startreset();
    if (epgsource->epgsourcename=="vsetv.com") {
        exp.setPattern("value=channel_([0-9]+)>([^<]+)");
        //QRegExp expico("");
        int pos=0;
        while ((pos=exp.indexIn(string,pos)) != -1) {
            //epgid[exp.cap(1)]=exp.cap(2);
            tmp=new EpgInfo;
            tmp->cn=exp.cap(2);
            /*tmp->cndescription= //*/ ///TODO
            tmp->epgsourcename=epgsource->epgsourcename;
            tmp->epgsource=epgsource;
            /*tmp->groups//*/ ///TODO
            tmp->id=exp.cap(1);
            /*/tmp->urlcnepg//*/ ///TODO
            tmp->urlicon="http://www.vsetv.com/pic/channel_logos/" + tmp->id + ".gif";
            // Последним выполняется копирование tmp
            epgsource->epgchannels.append(tmp);
            // скачивание картинки
            QString filename(iconsdir.path() + QString("/") + tmp->id + QString(".gif"));
            if (mw->forceLogoDownload || !iconsdir.exists(filename))  {
                url.setUrl(tmp->urlicon);
                downloadFile(url,filename);
            }
            /*bytes=getUrl(url);
            if (!bytes.isEmpty()) {
                QFile file(filename);
                file.open(QIODevice::WriteOnly);
                file.write(bytes);
                file.close();
            }//*/

            ///### ++channelinfo ###///
            bool exists; // пока не используется
            channelinfo *ci=setChannel(tmp->cn,&exists);
            if (ci->cndescription.isEmpty())
                ci->cndescription=tmp->cndescription;
            if (ci->icon.isEmpty())
                ci->icon=filename;
            ci->icons.append(filename);
            ci->epgl.append(tmp);
            ci->groups << tmp->groups;
            ///### ###///

            pos += exp.matchedLength();
        }
    }
    if (epgsource->epgsourcename=="s-tv.ru") {
        // определяем позицию группы
        int grpos=0;
        QRegExp grpexp("группу\" />([^</]*)</div>");
        QString grpname("");
        // определяем позицию первой группы
        grpos=grpexp.indexIn(string,grpos);
        // определяем позицию канала
        int pos=0;
        exp.setPattern("href=\"/tv/([^/]*)/\"><img src=\"([^<>]*)\" alt=\"([^<>]*)\" /><span>([^></]*)</span>");
        while ((pos=exp.indexIn(string,pos)) != -1) {
            tmp=new EpgInfo;
            tmp->cn=exp.cap(4);
            /*tmp->cndescription= //*/ ///TODO
            /*/tmp->urlcnepg//*/ ///TODO
            tmp->id=exp.cap(1);
            tmp->urlicon=exp.cap(2);
            tmp->epgsource=epgsource;
            tmp->epgsourcename=epgsource->epgsourcename; ///depr
            // проверяем достигнута ли позиция следующей группы (категории)
            if (pos>grpos) {
                // запоминаем имя текущей группы
                grpname=grpexp.cap(1);
                // определяем позицию следующей группы
                grpos+=grpexp.matchedLength();
                grpos=grpexp.indexIn(string,grpos);
                if (grpos == -1) {
                    // нет следующей группы, остальные каналы принадлежат последней группе
                    grpos=string.length();
                } /*else {
                    // запоминаем имя следующей группы
                    grpname=grpexp.cap(1);
                }//*/
            }
            if (!grpname.isEmpty()) {
                tmp->groups.append(grpname);    // добавляем в список групп канала
                /// GROUPS
                mw->groups->addgroup(mw->lstKat,grpname,grpname); // добавляем в список групп
            }
            // Последним выполняется копирование tmp (обработка заполненной структуры)
            epgsource->epgchannels.append(tmp);
            // скачивание картинки
            QString filename(iconsdir.path() + QString("/") + tmp->id + QString(".jpg"));
            if (mw->forceLogoDownload || !iconsdir.exists(filename))  {
                url.setUrl(tmp->urlicon);
                downloadFile(url,filename);
            }

            ///### ++channelinfo ###///
            bool exists; /*/ пока не используется /*/ /// TODO проверка на совпадение или нет данных из источника (например вдруг имена одинаковые, а epgID разные)
            channelinfo *ci=setChannel(tmp->cn,&exists);
            if (ci->cndescription.isEmpty())
                ci->cndescription=tmp->cndescription;
            if (ci->icon.isEmpty())
                ci->icon=filename;
            ci->icons.append(filename);
            ci->epgl.append(tmp);
            ci->groups << tmp->groups;
            ///### ###///

            pos += exp.matchedLength();
        }
    }
    if (epgsource->epgsourcename=="teleguide.info") {
        // определяем позицию группы
        /// пока не обрабатывается, т.к. на сайте бардак
        int grpos=0;
        QRegExp grpexp("ByPrefix\\('','([\\d])',this\\);\"><span>([^/]+)</span>");
        QHash<QString, QString> grps;
        while ((grpos=grpexp.indexIn(string,grpos)) != -1) {
            // формируем список групп
            grps[grpexp.cap(1)]=grpexp.cap(2);
            grpos += grpexp.matchedLength();
        }
        /*tmp->groups//*/ ///TODO
        int pos=0;
        exp.setPattern("<a href=\"/kanal([\\d]+).html\" title=\"([^/]+)\"alt=.{0,}(</a>&nbsp;([^/]+)</div>|programm_group)");
        exp.setMinimal(true);   // здесь нельзя жадничать в .{0,}
        while ((pos=exp.indexIn(string,pos)) != -1) {
            tmp=new EpgInfo;
            tmp->cn=exp.cap(2);
            tmp->id=exp.cap(1);
            tmp->epgsource=epgsource;
            tmp->epgsourcename=epgsource->epgsourcename;
            tmp->cndescription=exp.cap(4).replace(QRegExp("[\\n\\r]"), " ");
            /*/tmp->urlcnepg//*/ ///TODO
            //tmp->urlicon=exp.cap(2);
            /// TODO проверяем принадлежность группе (категории)
            // Последним выполняется копирование tmp (обработка заполненной структуры)
            epgsource->epgchannels.append(tmp);
            /*/ скачивание картинки
            QString filename(iconsdir.path() + QString("/") + tmp->id + QString(".jpg"));
            if (mw->forceLogoDownload || !iconsdir.exists(filename))  {
                url.setUrl(tmp->urlicon);
                downloadFile(url,filename);
            }//*/

            ///### ++channelinfo ###///
            bool exists; // пока не используется
            channelinfo *ci=setChannel(tmp->cn,&exists);
            if (ci->cndescription.isEmpty())
                ci->cndescription=tmp->cndescription;
            /*if (ci->icon.isEmpty())
                ci->icon=filename;//*/
            //ci->icons.append(filename);
            ci->epgl.append(tmp);
            ci->groups << tmp->groups;
            ///### ###///

            pos += exp.matchedLength();
        }
    }
    epgsource->lastupdate=QDateTime::currentDateTime();
    // закончили подготовку модели
    channelsModel->endreset();
    ///channelsModel->endResetModel();
}

// ждущая функция, если url показывает на ошибку то QByteArray.isEmpty()==true
QByteArray Channels::getUrl(QUrl url)
{
    ///QNetworkAccessManager nam;
    QNetworkReply *reply = nam->get(QNetworkRequest(url));

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()),&loop, SLOT(quit()));
    loop.exec();
    /// Ждать не будем, закинем запрос и натравим функцию обработки
    ///connect(nam,SIGNAL(finished(QNetworkReply*)),this,SLOT(parseReply(QNetworkReply*)));

    QByteArray bytes;
    // Не произошло-ли ошибки?
    int error = reply->error();
    switch (error)
    {
    case QNetworkReply::NoError:
        {
        /// редирект
        /// if reply->attribute(QNetworkRequest::Attribute)==QNetworkRequest::RedirectionTargetAttribute
        // Читаем ответ от сервера
        bytes = reply->readAll();
        ///QString string(bytes);

        // Выводим ответ на экран
        //qDebug() << string;
        ///ui->textEdit->append(string);
        break;
        }
    case QNetworkReply::ContentNotFoundError:
    case QNetworkReply::ContentAccessDenied:
       {
       int http_status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
       // handle HTTP status.
       if (http_status_code) /// TODO заглушка от ворнингов
           http_status_code=http_status_code;
       break;
       }
    default:
       // handle network error.
        {
        // обрабатываем ошибку
        //qDebug() << reply->errorString();
        // ui->textEdit->append(reply->errorString());
        QMessageBox::critical(QApplication::activeWindow(),tr("Ошибка!!!"),tr("Оширка получения данных сайта EPG."));  // ::about(
        }
    }
    delete reply;
    return bytes;
}

void Channels::downloadFile(QUrl url, QString filename)
{
    ///QNetworkAccessManager *nam=new QNetworkAccessManager;
    //nam->get(QNetworkRequest(url));
    QNetworkReply *reply; // nam.get(QNetworkRequest(url));
    networkReplyes nr;
    nr.filename=filename;
    nr.url=url;
    // оформляем дополнительные параметры
    // netReplyes.append(*(new networkReplyes));
    // netReplyes.last().filename=filename;
    // netReplyes.last().nam=nam;
    ///QEventLoop loop;
    ///QObject::connect(reply, SIGNAL(finished()),&loop, SLOT(quit()));
    ///loop.exec();
    /// Ждать не будем, закинем запрос и натравим функцию обработки
    //connect(nam,SIGNAL(finished(QNetworkReply*)),this,SLOT(saveReply(QNetworkReply*)));
    reply=nam->get(QNetworkRequest(url));
    netReplyes[reply]=nr;
    connect(reply, SIGNAL(finished()),this,SLOT(saveReply()));
    /// TODO connect(reply, SIGNAL(readyRead()), SLOT(readData()));
    //*/
}

/// Если событие пришло от клавиатуры, то выполняется приведение к типу QKeyEvent и проверяется -- какая клавиша нажата.
/// Если нажата клавиша "пробел", то вызывается функция ... и возвращается результат true, сообщая Qt о том, что событие обработано.
/// Если вернуть false, то Qt передаст событие объекту назначения.
/// Если событие порождено не клавишей "пробел", то управление передается функции eventFilter() базового класса.
bool Channels::eventFilter(QObject *target, QEvent *event)
{
    // событие для таблицы
    if (target==ui->tableView) {
        // keyboard
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = (QKeyEvent *)event;
            if (keyEvent->key() == Qt::Key_Space) {
              ;
              //return true;
            }
        }
        /// включение драг энд дроп реализовано в субклассе таблицы
        /*/ mouse нажали кнопку
        ///qDebug() << event->type();
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*> (event);
            if (mouseEvent->button() == Qt::LeftButton) {
                // включаем драг и дроп если ячейка выделена
                if (ui->tableWidget->itemAt(mouseEvent->pos())->isSelected()) {
                    ui->tableWidget->setDragDropMode(QAbstractItemView::DragDrop);
                }
            }
        }
        // отпустили клаву мыши
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*> (event);
            if (mouseEvent->button() == Qt::LeftButton) {
                ;
            }
        }//*/
    }
    //return QWidget::eventFilter(target, event);
    //return false;
}

bool Channels::savebase()
{
    /// TODO mySQL
    if (baseDriver=="MYSQL") {
        return true;
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
    filename+="/base/channels.dat";   /// TODO вынести в настройки
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
    out << "#TXTBASECHANNELS" << endl;
    /// помимо статически определённых в программе источников есть динамически создаваемые определяемые источники
    out << "[EPGSOURCES]" << endl; //
    QList<EpgSources *>::iterator is=listepgs.begin();
    while (is != listepgs.end()) {
        // определяем наличие в списке связи с базой
        quint64 value=sourcelink.value(*is);
        if (value==0) {
            value=++maxvalue;
            sourcelink.insert((*is),value);
        }
        out << value << "\t";
        // одиночные переменные
        out << (*is)->epgsourcename << "\t" << (*is)->urlepg << "\t" << (*is)->turlprog
            << "\t" << (*is)->lastupdate.toString() << "\t" << (*is)->sourceType
            << endl;
        ++is;
    }
    out << "[CHANNELS]" << endl;
    QList<channelinfo *>::iterator i=chanlist.begin();
    while (i != chanlist.end()) {
        // одиночные переменные
        out << (*i)->cn << "\t" << (*i)->cndescription << "\t" << (*i)->icon << endl;

        // списки иконок
        QString line;
        for (int a = 0; a < (*i)->icons.size(); a++){
            if (a>0) {
                line +="\t";
            }
            line +=(*i)->icons.at(a);
        }
        out << line << endl;

        // списки групп
        line.clear();
        for (int a = 0; a < (*i)->groups.size(); ++a){
            if (a>0) {
                line +="\t";
            }
            line +=(*i)->groups.at(a);
        }
        out << line << endl;

        // список источников канала
        // определяем максимальный идентификатор связи
        //quint64 maxvalue=(--epglink.constEnd()).value(); // берём из последнего элемента (TODO а если не последний?)
        quint64 value;
        for (int a = 0; a < (*i)->epgl.size(); ++a){
            // проверяем наличие связи с базой
            //const Channels::EpgInfo &key=(*i).epgl.at(a);
            value=epglink.value((*i)->epgl.at(a));
            if (value==0) {
                value=++maxvalue;
                epglink.insert((*i)->epgl.at(a),value);
            }
            // пишем только идентификаторы в базу
            /// TODO делаем копию в epglinknew
            if (a>0) {
                out << "\t";
            }
            out << value;
        }
        out << endl;

        ++i;
    }
    /// !!!!!! ДЕЛАЕТСЯ ТОЛЬКО ПОСЛЕ СОХРАНЕНИЯ КАНАЛОВ где актуализируется epglink
    /// у нас нет общего списка epginfo
    out << "[EPGLIST]" << endl;
    // считываем ссылки из epglink
    /// TODO считываем ссылки из epglinknew
    QHash<EpgInfo *, quint64>::iterator it = epglink.begin();
    while (it != epglink.end()) {
        EpgInfo *key=it.key();
        // определяем связь(ID) с источником
        quint64 sourceID=sourcelink.value(key->epgsource);
        // одиночные переменные
        out << it.value() << "\t" << sourceID << "\t" << key->cn << "\t";
        out << key->cndescription << "\t" << key->id << "\t" << key->urlcnepg << "\t";
        out << key->urlicon << "\t" << endl;

        // списки групп
        for (int a = 0; a < key->groups.size(); ++a){
            if (a>0) {
                out << "\t";
            }
            out << key->groups.at(a);
        }
        out << endl;

        ++it;
    }

return true;
}

bool Channels::openbase()
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
    filename+="/base/channels.dat";
    QFile fin(filename);
    bool fopen=fin.open(QIODevice::ReadOnly);
    if (!fopen) {
        ///QMessageBox::critical(QApplication::activeWindow(),tr("Ошибка!!!"),tr("Файл не открывается.") + fin.errorString());  // ::about(
        /// будем создавать новый
        return false;
    }
    QTextStream in(&fin);
    QString temp=in.readLine();
    if (temp!="#TXTBASECHANNELS") { // страхуемся, проверяем что первая строка принадлежит базе
        ///QMessageBox::critical(QApplication::activeWindow(),tr("Ошибка!!!"),tr("База данных для групп не обнаружена!");  // ::about(
        /// будет создаваться с нуля
        return false;
    }
    temp=in.readLine();
    ///***quint64 oldlistid=0-1;
    ///***Groups::GroupList *gls;
    QStringList fields;
    enum mode_phase{ EPGSOURCES, CHANNELS, EPGLIST, PARAMETERS};
    int mode=-1;
    while (!temp.isNull()) {
        // обрабатываем новый блок, блок может быть из нескольких строк, первая строка в блоке должна иметь несколько слов, иначе новая секция
        fields=temp.split("\t");
        // селектор секций
        if (fields.size()==1) { // если одна фраза то это новая секция
            if (temp=="[EPGSOURCES]") { // переключаем моду
                mode=EPGSOURCES;
            } else {
                if (temp=="[CHANNELS]") { // переключаем моду
                    mode=CHANNELS;
                } else {
                    if (temp=="[EPGLIST]") { // переключаем моду
                        mode=EPGLIST;
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
            case EPGSOURCES: {
                bool exists;
                EpgSources *epgs=setEPG(fields.at(1),&exists);
                // прописываем с индексов по порядку сохранения (мы могли переопределить данные)
                epgs->urlepg=fields.at(2);
                epgs->turlprog=fields.at(3);
                epgs->lastupdate=QDateTime::fromString(fields.at(4));
                epgs->sourceType=fields.at(5).toInt();
                // вносим в список
                /// sourcelink для значений определённых в программе тоже не определён
                quint64 value=fields.at(0).toULongLong();
                sourcelink.insert(epgs,value);
                if (maxvalue<value)
                    maxvalue=value;
                if (!exists) { /// TODO заглушка для ворнингов
                    exists=true; /// TODO заглушка для ворнингов
                }
                break;
            }
            case CHANNELS: {
                // начинаем обрабатывать блок
                bool exists;
                channelinfo *chi=setChannel(fields.at(0),&exists);
                // прописываем с индексов по порядку сохранения
                chi->cndescription=fields.at(1);
                if (fields.size()>2) /// TODO (этого не должно быть) временно проверяем наличие последнего поля (может не быть)
                    chi->icon=fields.at(2);
                // читаем строку списка иконок
                temp=in.readLine();
                fields=temp.split("\t");
                // обрабатываем строку списка иконок
                for (int i=0; i<fields.size();i++) {
                    chi->icons.append(fields.at(i));
                }
                // читаем строку групп
                temp=in.readLine();
                fields=temp.split("\t");
                // обрабатываем строку групп
                for (int i=0; i<fields.size();i++) {
                    chi->groups.append(fields.at(i));
                }
                // читаем строку источников EPG каналов
                temp=in.readLine();
                fields=temp.split("\t");
                // обрабатываем строку источников
                for (int i=0; i<fields.size();i++) {
                    EpgInfo *epg=new EpgInfo();         /// TODO проверка на наличие *(пока предполагается уникальность)
                    chi->epgl.append(epg);
                    quint64 value=fields.at(i).toULongLong();
                    epglink.insert(epg,value); // список пуст, повторений по value не должно быть при загрузке
                    if (maxvalue<value)
                        maxvalue=value;
                }
                break;
            }
            case EPGLIST: {
                /* начинаем обрабатывать блок */ /// epglink к этому времени уже прочитан и создан.
                /// читаем из файла находим в списке. TODO Могут быть две гипотетические ошибки, 1) в файле есть, в списке нет. 2) в списке есть в файле нет. (пока не обрабатываются)
                EpgInfo *epgi=epglink.key(fields.at(0).toULongLong());      ///получается самая долгая процедура
                EpgSources *epgs=sourcelink.key(fields.at(1).toULongLong()); //получается самая долгая процедура
                epgi->epgsource=epgs;
                epgi->cn=fields.at(2);
                epgi->cndescription=fields.at(3);
                epgi->id=fields.at(4);
                epgi->urlcnepg=fields.at(5);
                epgi->urlicon=fields.at(6);
                // читаем строку групп
                temp=in.readLine();
                fields=temp.split("\t");
                // обрабатываем строку групп
                for (int i=0; i<fields.size();i++) {
                    epgi->groups.append(fields.at(i));
                }
                /*QHash<EpgInfo *, quint64>::iterator it = epglink.begin();
                while (it != epglink.end()) {
                    EpgInfo *key=it.key();
                    fields.at(0).toULongLong();
                }//*/
                break;
            }
            case PARAMETERS: {
                //if (fields.at(0)=="LASTID") lastid=fields.at(1).toULongLong();
                //if (fields.at(0)=="EXTID") extid=fields.at(1).toULongLong();
                break;
            }
            default: {
                break;
            }
        }

        temp=in.readLine();
    }

    return true;
}

// сохранение скачанного в файл
void Channels::saveReply()
{
    QNetworkReply* reply=qobject_cast<QNetworkReply*>(sender());
    //QMutex mutex; //при обращении к ui->
    //mutex.lock();

    networkReplyes nr=netReplyes.take(reply);
    QString filename=nr.filename;
    //QString filename = reply->url().toString();
    //filename.remove(0, fileName.lastIndexOf('/') + 1); // оставить только имя файла

    // ищем наш запрос
    //QNetworkAccessManager *nam=reply->manager();
    /*int cnt=netReplyes.size();
    int idx=-1;
    //QString filename("");
    for (int i=0;i<cnt;i++) {
        if (netReplyes.at(i).nam==nam) {
            filename=netReplyes.at(i).filename;
            idx=i;
            break;
        }
    }
    if (idx==-1) {
        // фиксируем ошибку
        QMessageBox::critical(QApplication::activeWindow(),tr("Ошибка!!!"),tr("Неизвестная сессия получения данных сайта EPG."));
    } else {
        netReplyes.removeAt(idx);
    }//*/

    QByteArray *bytes=new QByteArray;
    // Не произошло-ли ошибки?
    int error = reply->error();
    switch (error)
    {
    case QNetworkReply::NoError:
        {
        /// редирект
        /// if reply->attribute(QNetworkRequest::Attribute)==QNetworkRequest::RedirectionTargetAttribute
        // Читаем ответ от сервера
        *bytes = reply->readAll();
        ///QString string(bytes);

        // Выводим ответ на экран
        //qDebug() << string;
        ///ui->textEdit->append(string);
        QFile file(filename);
        file.open(QIODevice::WriteOnly);
        file.write(*bytes);
        file.close();

        break;
        }
    case QNetworkReply::ContentNotFoundError:
    case QNetworkReply::ContentAccessDenied:
       {
       int http_status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
       qDebug() << "ошибка страницы - " << http_status_code << " - " << reply->url().url();
       // handle HTTP status.
       break;
       }
    default:
       // handle network error.
        {
        // обрабатываем ошибку
        //qDebug() << reply->errorString();
        // ui->textEdit->append(reply->errorString());
        QMessageBox::critical(QApplication::activeWindow(),tr("Ошибка!!!"),tr("Ошибка получения данных сайта EPG."));  // ::about(
        }
    }
    reply->deleteLater(); // в слоте только так
    //nam->deleteLater();
    delete bytes;
    //mutex.unlock();
}

/*void Channels::dragEnabled(int r, int c)
{
    if (ui->tableWidget->item(r,c)->isSelected()) {
        ui->tableWidget->setDragDropMode(QAbstractItemView::DragDrop);
    }
    test++;
}//*/

void Channels::readChannelInfo()
{

}

void Channels::writeChannelInfo()
{

}

ChannelsModel::ChannelsModel(Channels *dstore, QObject *parent)
{
    qDebug() << parent; /// TODO заглушка от ворнингов
    datastore=dstore;
}

void ChannelsModel::startreset()
{
    beginResetModel();
}

void ChannelsModel::endreset()
{
    endResetModel();
}

int ChannelsModel::rowCount(const QModelIndex &parent) const
{
    qDebug() << parent; /// TODO заглушка от ворнингов
    return datastore->chanlist.size(); // если // || datastore->curEPGs==nullptr) надо увеличить на epgl.size каждой строки
}

int ChannelsModel::columnCount(const QModelIndex &parent) const
{
    qDebug() << parent; /// TODO заглушка от ворнингов
    return colcount;
}

QVariant ChannelsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= datastore->chanlist.size() || index.column() >= colcount)
        return QVariant();

    if (role == Qt::DisplayRole) {
        if (index.column()==PNUM) {
            return index.row()+1;
        }
        if (index.column()==GENNAME) {
            return datastore->chanlist.at(index.row())->cn;
        }
        /// колонки требующие обработки EPG и другие тяжёлые обрабатываются последними
        if (index.column()==ICON) {
            return datastore->chanlist.at(index.row())->icon;
        }
        if (index.column()==GROUPS) {
            return datastore->chanlist.at(index.row())->groups.join("; ");
        }
        Channels::EpgInfo *epginfo=nullptr;
        int size=datastore->chanlist.at(index.row())->epgl.size();
        for (int i = 0; i < size; ++i) {
            if (datastore->chanlist.at(index.row())->epgl.at(i)->epgsource == datastore->curEPGs) { // || datastore->curEPGs==nullptr) { не покажет, нужно делать новые строки
                epginfo=datastore->chanlist.at(index.row())->epgl.at(i);
            }
        }
        if (epginfo==nullptr) {
            //QVariant ret=QVariant("нет данных источника");
            QVariant ret=QVariant();
            return ret;
        }
        if (index.column()==ID)
            return epginfo->id;
        if (index.column()==EPGNAME)
            return epginfo->cn;
        if (index.column()==DES) {
            if (datastore->chanlist.at(index.row())->cndescription.isEmpty())
                return epginfo->cndescription;
            else
                return datastore->chanlist.at(index.row())->cndescription;
        }
    }
    else {
        return QVariant();
    }

    return QVariant();
}

bool ChannelsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {  // для булевых значений Qt::CheckStateRole
        if (index.column()==GENNAME) {
            //QList<Channels::channelinfo> *test2=&datastore->chanlist;
            //Channels::channelinfo *test3=&test2->at(index.row());
            Channels::channelinfo *ci=datastore->chanlist.at(index.row());
            //Channels::channelinfo *ci=const_cast<Channels::channelinfo *> (&datastore->chanlist.at(index.row()));
            //quint64 *test=&datastore->maxvalue;
            ci->cn=value.toString();
            return true;
        }
    }
    return false;
}

QVariant ChannelsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
             return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case PNUM:
            return QString(tr("№"));
            break;
        case GENNAME:
            return QString(tr("Название канала/трека"));
            break;
        case ID:
            return QString(tr("Идентификатор EPG"));
            break;
        case EPGNAME:
            return QString(tr("Название из источника EPG"));
            break;
        case DES:
            return QString(tr("Описание канала/трека"));
            break;
        case ICON:
            return QString(tr("Значёк канала/трека"));
            break;
        case GROUPS:
            return QString(tr("Категории"));
            break;
        }
    }
    else {
        int sect=section+1;
        return QString("%1").arg(sect);
    }

    return QVariant();
}

Qt::ItemFlags ChannelsModel::flags(const QModelIndex &index) const
{
    //
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;  // для булевых значений Qt::ItemIsUserCheckable;
}

QModelIndex ChannelsModel::index(int row, int column, const QModelIndex &parent) const
{
    qDebug() << parent; /// TODO заглушка от ворнингов
    //if (!parent.isValid() || row==-1 || column == -1)
    ///qDebug() << row;
    if (row==-1 || row>=datastore->chanlist.size() || column == -1 || column >= colcount)
        return QModelIndex();

    Channels::channelinfo *temp=datastore->chanlist.at(row);
    //return hasIndex(row, column, parent) ? createIndex(row, column, (void*)&temp->displayName) : QModelIndex();
    return createIndex(row, column, (void*)temp);
}

QModelIndex ChannelsModel::parent(const QModelIndex &index) const
{
    qDebug() << index; /// TODO заглушка от ворнингов
    //return createIndex(row, column, (void*)&temp->displayName)
    return QModelIndex();
}

bool ChannelsModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    beginInsertRows(parent, position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        datastore->chanlist.insert(position, new Channels::channelinfo); // добавляем пустую строку
    }

    endInsertRows();
    return true;
}

bool ChannelsModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    beginRemoveRows(parent,position,position+rows-1);
    for (int row = 0; row < rows; ++row) {
        datastore->chanlist.removeAt(position);
    }
    endRemoveRows();
    return true;
}

bool ChannelsModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    beginMoveRows(sourceParent,sourceRow,sourceRow+count-1,destinationParent,destinationChild);
    if (sourceRow>=datastore->chanlist.size()
            || destinationChild>datastore->chanlist.size()
            || sourceRow==destinationChild
            || (sourceRow+1)==destinationChild) {
        return false;
    }
    if (sourceRow<destinationChild) {
        if ((sourceRow+count)>=destinationChild) { // перемещение внутрь области
            return false;
        }
    }
    int from,to;
    for (int i=0; i<count;i++) {
        if (from<to) {
            from=sourceRow; //+i; источник каждый раз подтягивается новый снизу т.к. предыдущий перемещён вниз
            to=destinationChild+i;
            datastore->chanlist.move(from,to-1);
        } else {
            from=sourceRow+i;
            to=destinationChild+i;
            datastore->chanlist.move(from,to);
        }
    }
    endMoveRows();
    return true;
}
