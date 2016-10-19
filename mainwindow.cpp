#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"

int tablemode;

//global constants
QString menustrepgmode;
int global_y_colpanel, global_mh_colpanel;
QString epgprefix="";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //mui=ui;
    /// переменные которые должны идти раньше создания форм
    //epgcb=ui->comboBox; // передача объекта для формы channels

    // формы
    ///CHANNELS
    channels=new Channels(0,this);
    ui->comboBox->setModel(channels->cb->model());
    connect(ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(comBox_currentIndexChanged(int)));
    //ui->comboBox_4->setModel(ui->comboBox->model());

    ///GROUPS
    groups=new Groups(0);
    if (!testGroups()) {
        QMessageBox::critical(QApplication::activeWindow(),tr("Ошибка!!!"),tr("Неустранимая проблема с группами!"));//
        /// TODO заставить приложение закрыться или спросить?
    }//*/

    tablemode=NORMALMODE;
    global_y_colpanel=ui->label->geometry().y();
    global_mh_colpanel=ui->frame->minimumHeight();
    menustrepgmode=QString(tr("Установка соответствий EPG ID"));
    // воссоздаём заголовки после загрузки программы
    setTableM3U(ui->tableWidget);
    table2.verticalHeader()->setMinimumSectionSize(23);
    table2.verticalHeader()->setDefaultSectionSize(23);
    setTableM3U(&table2);
    // настроим перенос элементов
    ui->tableWidget->verticalHeader()->setSectionsMovable(true);
    // прописываем известные атрибуты, которые особо обрабатываются в программе
    // тип обработки определяется по идентификатору переменной (номеру) колонки
    // имена для одного типа должны быть одинаковы из принципа - один тип - одна колонка - один чекбокс
    // по одному имени нельзя будет определить атрибут, используется комбинация: имя атрибута + exist
    // соответственно в одном листе не должно быть двух разных атрибутов одного типа
    // можно только несколько значений одного атрибута, например group_id
    attributes.prepend(* new attrItem);
    attributes[0].key="group_id";
    attributes[0].name="GroupID";
    attributes[0].ncol=colGroup;
    attributes[0].chb=new QCheckBox("GroupID,",ui->frame);
    attributes[0].chb->hide();
    attributes.prepend(* new attrItem);
    attributes[0].key="group_title";
    attributes[0].name="GroupID";
    attributes[0].ncol=colGroup;
    attributes[0].chb=new QCheckBox("GroupID,",ui->frame);
    attributes[0].chb->hide();
    attributes.prepend(* new attrItem);
    attributes[0].key="epg-id";
    attributes[0].name="EPGID";
    attributes[0].ncol=colEPG;
    attributes[0].chb=new QCheckBox("EPGID,",ui->frame);
    attributes[0].chb->hide();
    attributes.prepend(* new attrItem);
    attributes[0].key="id";
    attributes[0].name="EPGID";
    attributes[0].ncol=colEPG;
    attributes[0].chb=new QCheckBox("EPGID,",ui->frame);
    attributes[0].chb->hide();
    attributes.prepend(* new attrItem);
    attributes[0].key="tvg-logo";
    attributes[0].name="LogoID";
    attributes[0].ncol=colLogo;
    attributes[0].chb=new QCheckBox("LogoID,",ui->frame);
    attributes[0].chb->hide();
    attributes.prepend(* new attrItem);
    attributes[0].key="logo";
    attributes[0].name="LogoID";
    attributes[0].ncol=colLogo;
    attributes[0].chb=new QCheckBox("LogoID,",ui->frame);
    attributes[0].chb->hide();

openFile = new QAction(tr("Открыть *.M3U ..."),this);
adelrow = new QAction(this);
saveM3U = new QAction(tr("Сохранить плэйлист"),this);
asaveM3Uas = new QAction(tr("Сохранить плэйлист как..."),this);
acompareFile = new QAction(tr("Сравнить с другим файлом..."),this);
actEpgMode = new QAction(menustrepgmode,this);
actEpgMode->setCheckable(true);
//** for all space table
///ui->tableWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
///ui->tableWidget->addAction(action1);
///ui->tableWidget->addAction(action2);
///ui->tableWidget->connect(action1, SIGNAL(hovered()), this, SLOT(actionSlot1()));
///ui->tableWidget->connect(action2, SIGNAL(triggered()), this, SLOT(actionSlot2()));
//** for menu on items, headers, e.t.c
ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
contextTableMenu=new QMenu();
contextTableMenu->addAction(openFile);
contextTableMenu->addAction(acompareFile);
contextTableMenu->addAction(saveM3U);
contextTableMenu->addAction(asaveM3Uas);
contextTableMenu->addSection(QString("--Режимы--")); /// TODO
contextTableMenu->addAction(actEpgMode);
connect(openFile, SIGNAL(triggered()), this, SLOT(openFileSlot()));
connect(saveM3U, SIGNAL(triggered()), this, SLOT(savem3uslot()));
connect(asaveM3Uas, SIGNAL(triggered()), this, SLOT(savem3uas()));
connect(acompareFile, SIGNAL(triggered()), this, SLOT(compareFile()));
connect(actEpgMode, SIGNAL(triggered()), this, SLOT(setEpgMode()));
connect(adelrow, SIGNAL(triggered(bool)),this,SLOT(delrows()));

ui->tableWidget->verticalHeader()->setContextMenuPolicy(Qt::ActionsContextMenu);
ui->tableWidget->verticalHeader()->addAction(openFile);
ui->tableWidget->verticalHeader()->addAction(acompareFile);

//менюшка для EPGMODE
aSetChName=new QAction(tr("Установить это название канала"),0);
connect(aSetChName,SIGNAL(triggered(bool)),SLOT(setChannelName()));

// меню для основной панели
///generalMenu=new QMenu;
aChannels=new QAction(tr("EPG Каналы ..."),this);
ui->frame->connect(aChannels,SIGNAL(triggered(bool)),channels,SLOT(show()));
//generalMenu->addAction(aChannels);
ui->frame->addAction(aChannels);

// меню для второй таблицы
//QAction *asortS=new QAction("Распределить по источникам",this);
//connect(asortS,SIGNAL(triggered(bool)),this,SLOT(sortBySources()));
QAction *ashowS=new QAction("Показывать соответствие по источнику",this);
connect(ashowS,SIGNAL(triggered(bool)),this,SLOT(showBySources()));
ashowS->setCheckable(true);
QAction *afind1=new QAction("Найти в первой",this);
connect(afind1,SIGNAL(triggered(bool)),this,SLOT(findInTable1()));
QAction *aremoveT=new QAction("Убрать таблицу",this);
connect(aremoveT,SIGNAL(triggered(bool)),this,SLOT(removeTable2()));
table2.setContextMenuPolicy(Qt::ActionsContextMenu);
//table2.addAction(asortS);
table2.addAction(ashowS);
table2.addAction(afind1);
table2.addAction(aremoveT);
table2.setDragEnabled(true); //

/*
// инициализация переменных графического интерфейса
GForm1WidthOld=Form1->Width;
// перехват событий мыши
Application->OnMessage=hooku;
// отключаем автоматическую прорисовку таблицы
StringGrid1->DefaultDrawing=false;
StringGrid2->DefaultDrawing=false;
// включаем режим одной таблицы
//Splitter1->Align=alRight;
StringGrid1->Align=alClient;
///*/
findDialog=new FindDialog(this,"Поиск в первой таблице");
QAction *showDialog = new QAction(tr("showDialog "), this);
showDialog->setShortcut( QKeySequence(Qt::CTRL + Qt::Key_F));
showDialog->setStatusTip(tr("Find dialog"));
connect(showDialog , SIGNAL(triggered()), findDialog, SLOT(show()));
ui->tableWidget->addAction(showDialog);
///второй способ хоткея
QShortcut *keyCtrlS = new QShortcut(this);
keyCtrlS->setKey(Qt::CTRL + Qt::Key_S); // или пишем ("CTRL+S")
connect(keyCtrlS, SIGNAL(activated()), this, SLOT(savem3uslot()));//*/
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::testGroups()
{
    groups->openbase();
    // проверяем наличие нужных классов или создаём при необходимости
    /// TODO надо чтобы это делалось из программы и распространялось с базой
    klsChannels=groups->getklass("Каналы");
    klsGroups=groups->getklass("Группы");
    // получаем списки групп
    lstKat=groups->setlist("Категории",klsGroups);
    lstKatm=groups->setlist("Категории MyIPTV",klsGroups);
    lstChan=groups->setlist("Каналы",klsChannels);
    // получаем предопределённые группы
    groups->addgroup(lstKat,"UHD","UHD");
    groups->addgroup(lstKat,"HD","HD");
    groups->addgroup(lstKat,"SD","SD");
    groups->addgroup(lstKatm,"Общие","0");
    groups->addgroup(lstKatm,"Познавательные","1");
    groups->addgroup(lstKatm,"Новости","2");
    groups->addgroup(lstKatm,"Развлекательные","3");
    groups->addgroup(lstKatm,"Детские","4");
    groups->addgroup(lstKatm,"Музыкальные","5");
    groups->addgroup(lstKatm,"Комедийные","6");
    groups->addgroup(lstKatm,"Спортивные","7");
    groups->addgroup(lstKatm,"Интернациональные","8");
    groups->addgroup(lstKatm,"Фильмы/Сериалы","9");
    groups->addgroup(lstKatm,"Эротические","10");
    groups->addgroup(lstKatm,"Радио","11");
    groups->addgroup(lstKatm,"Немецкие","12");
    groups->addgroup(lstKat,"3D","3D");

    return true;
}

//** for menu on items, headers, e.t.c
// ПРОВЕРКА НА ТАБЛИЦЕ, ПО ЯЧЕЙКАМ, КАКОЕ КОНТЕКСТНОЕ МЕНЮ У НАС БУДЕТ
void MainWindow::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    QTableWidgetItem *item;
    item = ui->tableWidget->itemAt(pos);
    if (tablemode==NORMALMODE) {

        if(item){
            adelrow->setText(tr("Удалить строки"));
            contextTableMenu->insertAction(saveM3U, adelrow);
        }
            // else return;

        // тест прикрепляем нужные нам левые данные к пункту меню
        for (int i = 0; i < contextTableMenu->actions().size(); i++)
            contextTableMenu->actions().at(i)->setData(QVariant(ui->tableWidget->column(item)));

        contextTableMenu->exec(QCursor::pos()); // обязательно в конце функции
    }
    if (tablemode==EPGMODE) {
        if (ui->tableWidget->column(item)==colCCN) {
            // если мы на колонке каналов с EPG добавляем пункт, на других иначе удаляем
            if (menuEPG.actions().indexOf(aSetChName) == -1)
                menuEPG.insertAction(saveM3U, aSetChName);
        } else {
            if (menuEPG.actions().indexOf(aSetChName) != -1)
                menuEPG.removeAction(aSetChName);
        }
        menuEPG.exec(QCursor::pos());
    }
}

// открытие файла для таблицы 1
void MainWindow::openFileSlot()
{
    openM3U(ui->tableWidget,nullptr);
}

// открытие и парсинг M3U
void MainWindow::openM3U(QTableWidget *TW, QTextStream *stream)
{
    // определяем --- вставка из буфера обмена или из файла
    QString str;    // сюда будем читать файл и обрабатывать
    if (stream==nullptr) {
        // из файла
        stream=new QTextStream;
        filename=QFileDialog::getOpenFileName( 0, QString("Open File"));
        if (filename.isEmpty())
            return;

        bool fopen=false;   // признак открывался ли файл (не признак использования буфера обмена)

        // открываем файл
        QFile file (filename) ;
        fopen=file.open(QIODevice::ReadOnly);
        if (fopen) {
            /// ********
            /// TODO подсчитываем хэшфункцию файла, если такая в базе есть, открываем плэйлист из базы
            /// !!!!1******
            // читаем файл в буфер как текстовый поток
            /// TODO
            /// /// здесь если надо вставляем функцию определения кодировки
            /// // если необходимо вставляем параметры перекодировки (можно стырить из примера codec с превью)
            QTextStream fstream(&file);      // Внутреннее представление текста в буферах QTextStream в кодировке Unicode
            str = fstream.readAll(); // QString в Unicode т.к. взаимодействует с QTextStream без перекодировок, перевод строки переменной чтения из потока всегда \n ?
            // закрываем файл
            file.close();
            // привязываем поток к прочитанному буфферу
            stream->setString(&str);
            // открываем файл для бэкапа
            QString backname=filename + QString(".bak");
            file.setFileName(backname);
            file.remove();
            file.setFileName(filename);
            //QFile backup (backname);
            //wopen=backup.open(QIODevice::WriteOnly);//*/
            if (!file.copy(backname)) {
                // бэкап не удался
                // кидаем просто предупреждение и очищаем filename чтобы пользователь выбрал имя при сохранении
                QMessageBox::critical(QApplication::activeWindow(),tr("Ошибка!!!"),tr("Файл ") + backname + tr(" не открывается."));  // ::about(
                filename="";
            }
            // вписываем имя файла
            if (TW==ui->tableWidget) {
                QFontMetrics fm(font());
                ui->label->setGeometry(ui->label->x(),ui->label->y(),fm.width(filename),ui->label->height());
                ui->label->setText(filename);
            } else {
                /// TODO для заголовка второй таблицы
            }
            // закрываем файлы
            file.close();
        } else {
            QMessageBox::critical(QApplication::activeWindow(),tr("Ошибка!!!"),tr("Файл не открывается."));  // ::about(
            return;
        }
    } else {
        // пишем что вcтавлено из буффера обмена
        /// TODO
    }

    /// TODO использовать сценарий, если вторая таблица не открыта, очищаем панель атрибутов
    /// если вторая таблица есть, то после открытия инициализируем сравнивание с ней
    /// СЕЙЧАС ОЧИЩАЕМ ПАНЕЛЬ АТРИБУТОВ И ЗАКРЫВАЕМ ВТОРУЮ ТАБЛИЦУ при открытии главной таблицы
    if (TW==ui->tableWidget) {
        // обнуляем шаблон
        int cnt=attributes.size();
        for (int i=0;i<cnt;i++) {
            // признак отсутствия в шаблоне
            attributes[i].exist=false;
        }
        adjustChkb(-1); // делается очистка панели ??? а если пользователь нажмёт отмену?
        table2.hide(); /// TODO??? делать сразу сравнение
    }

    QString extcode=""; // флаг текущего состояния поиска, собственно что обрабатываем
    // экст.нач - начало расширения плэйлиста
    // экст.заг - заголовок плэйлиста
    // EXTINF - начало меток расширений
    // url - конкретно адрес потока (который вычищаем от пробелов)
    QString buffstr="";// буфферная строка
    int trow=0; // начальное значение заполняемой строки таблицы - первая строка
    ///TODO SG->Visible=false; // блокируем прорисовку при загрузке
    // обнуляем таблицу
    TW->setRowCount(0);
    bool dellaststr;
    // Read stream into buffer
    /// TODO читаем пару строк и принимаем решение как обрабатывать #EXT по признаку есть url или нет
    while (!stream->atEnd()) {       // читаем поток построчно
        // читаем строку
        buffstr = stream->readLine();    ///
        // убираем конечные пробелы
        buffstr=buffstr.trimmed();
        // пропускаем пустые строки
        if (buffstr.isEmpty()) continue;
        /// готовим новую строку в таблице
        dellaststr=false;
        if (TW->rowCount()<trow+1){// если не хватает строк - добавляем
            TW->setRowCount(trow+1);
            // заполняем строку объектами с пустыми значениями
            for(int column = 0; column < TW->columnCount(); column++) {
                TW->setItem(trow, column, new QTableWidgetItem);
                TW->item(trow,column)->setText("");
            }
        }
        /// определяем что за строка в буфере
        // ? это EXTINF
        if (buffstr.indexOf("#EXTINF") != -1) {
           // тут прервёмся и сделаем проверку на логику плэйлиста
           // если предыдущая обработка by extcode тоже было EXTINF
           // то либо есть url "потеряшка" либо была служебная строка.
           if (extcode=="EXTINF") { //предыдущая строка была тоже описание канала
               // в будущем сделаем поиск адреса потеряшки
               // **....................
               // ......................
               /// TODO красим красным field URL предыдущей строки
           }
           extcode="EXTINF"; // отмечаем что начали обработку EXTINF
           // парсим строку и заполняем данными таблицу(///TODO класс)
           prepareExtinf(TW, buffstr, trow);
           // новое значение строки
           trow++;
        }
        // если не EXTINF, значит либо url (в строке "://"), либо коментарий (начало с "#"), либо нечто не имеющее отношение к теме
        else {
            // ? это комментарий либо директива EXTM3U
             if (buffstr.at(0)=='#') {
                continue; // пока не отрабатываем
            }
            // ? это url
            else {
                if (buffstr.indexOf("://")  != -1) {
                    // строка с url
                    /// если предыдущий extcode="EXTINF" то возвращаемся на строку
                    if (extcode=="EXTINF") {
                        // значение следующей строки
                        trow--;
                        dellaststr=true;
                    }
                    /// если предыдущий extcode="url" то остаёмся на строке
                    if (extcode=="url") {
                    }
                    prepareUrl(TW, buffstr, trow);
                    // значение следующей строки
                    trow++;
                    extcode="url"; // метка что была обработана строка url
                } else {
                    // иначе незначащая строка
                    continue; // не обрабатываем
                }
            }
        }
    }   // конец цикла чтения потока
    // удаляем пустую строку
    if (dellaststr)
        TW->setRowCount(TW->rowCount()-1);

    /// работа над ошибками
    if (stream->status() != QTextStream::Ok) {
        qDebug () << "Ошибка чтения файла";
        QMessageBox::critical(QApplication::activeWindow(),tr("Ошибка!!!"),tr("Ошибка разбора данных."));  // ::about(
        return;
    }

    /// EPGID
    /// получаем EPG
    comBox_currentIndexChanged(ui->comboBox->currentIndex());

    /// обрабатываем получившуюся таблицу
    //пополняем список групп каналов в combobox
    ui->comboBox_2->clear();
    QString ttmp="";
    int ttmax=0;
    bool grpttl=false;
    int rc=TW->rowCount();
    for(int i = 0; i < rc; i++) {
        ttmp=TW->item(i,colGroup)->text();
        if (ui->comboBox_2->findText(ttmp)==-1) {
            ui->comboBox_2->addItem(ttmp);
            }
        // пытаемся сразу определить к какому стандарту относится список групп
        bool ok;
        if (!grpttl&&!ttmp.isEmpty()) {
            int dec=ttmp.toInt(&ok);
            if (ok) {
                if (ttmax<dec)
                    ttmax=dec;
            } else  {
                    // значит это стандарт - group-title
                    grpttl=true;
              }
        }
    }
    if (!grpttl&&ttmax<13) {
        ui->comboBox_2->clear();
        ui->comboBox_2->addItem("0 - Общие");
        ui->comboBox_2->addItem("1 - Познавательные");
        ui->comboBox_2->addItem("2 - Новости");
        ui->comboBox_2->addItem("3 - Развлекательные");
        ui->comboBox_2->addItem("4 - Детские");
        ui->comboBox_2->addItem("5 - Музыкальные");
        ui->comboBox_2->addItem("6 - Комедийные");
        ui->comboBox_2->addItem("7 - Спортивные");
        ui->comboBox_2->addItem("8 - Интернациональные");
        ui->comboBox_2->addItem("9 - Фильмы/Сериалы");
        ui->comboBox_2->addItem("10 - Эротические");
        ui->comboBox_2->addItem("11 - Радио");
        ui->comboBox_2->addItem("12 - Немецкие");
        for(int i = 0; i < rc; i++)
            {
            int test=TW->item(i,colGroup)->text().toInt();
            TW->item(i,colGroup)->setText(ui->comboBox_2->itemText(test));
            }
        ///ui->comboBox_3->setCurrentIndex(0);
    } else {
        ///ui->comboBox_3->setCurrentIndex(1);
    }

    // проверка на дубли по названию и источнику
    checkDbl(TW);
    ///SG->Visible=true; // разблокируем прорисовку при загрузке
    TW->show();
}

// проверка на дубли по названию и источнику
/// TODO меню проверки на дубли
void MainWindow::checkDbl(QTableWidget *TW)
{
    //
    int count=TW->rowCount();
    int cdbl;
    QString name; // название
    QString addr; // адрес источника
    QString port; // порт источника
    for (int i=0;i<count;i++) {
        name=TW->item(i,colCN)->text().trimmed();
        addr=TW->item(i,colSTRMa)->text();
        port=TW->item(i,colSTRMp)->text();
        cdbl=0;
        for (int n=i+1;n<count;n++) {
            // проверяем имя
            if (name==TW->item(n,colCN)->text().trimmed()) {
                // rename
                name=name + QString(" (DBL ") + QString::number(++cdbl) + QString(")");
                TW->item(n,colCN)->setText(name);
                // set color
                TW->item(i,colCN)->setBackground(Qt::magenta);
                TW->item(n,colCN)->setBackground(Qt::magenta);
            }
            // проверяем источник
            if (addr==TW->item(n,colSTRMa)->text() && port==TW->item(n,colSTRMp)->text()) {
                // rename
                if (!TW->item(n,colCN)->text().contains(" (DBL source ")) {
                    TW->item(n,colCN)->setText(TW->item(n,colCN)->text() + QString(" (DBL source ") + QString::number(++cdbl) + QString(")") );
                }
                // set color
                TW->item(n,colCN)->setBackground(Qt::magenta);
                TW->item(n,colSTRMa)->setBackground(Qt::magenta);
                TW->item(n,colSTRMp)->setBackground(Qt::magenta);
                TW->item(i,colSTRMa)->setBackground(Qt::magenta);
                TW->item(i,colSTRMp)->setBackground(Qt::magenta);
            }
        }
    }
}

// парсинг #EXT
void MainWindow::prepareExtinf(QTableWidget *TW, QString &buffstr, int trow)
{
    // #EXTINF:(длительность) (атрибуты: atr="value"), (название канала).
    //•tvg-shift - смещение во времени телепрограммы
    //•tvg-name - идентификатор телепрограммы канала
    //•tvg-logo - отображаемое название или логотип канала
    //•group-title - группа в которую канал входит
    //•audio-track - аудио дорожка канала (например: en, ru,...)

    int pos=0;
    ///таблица пока знает только длинну трека, название, и адрес источника

    /// СЛЕДУЮЩИЙ КОД ИЩЕТ ТОЛЬКО ИЗВЕСТНЫЕ АТТРИБУТЫ
    // :-1 :(длительность)
    //ищем сколько совпадающих под шаблон символов
    QRegExp exp("#EXTINF:([-0-9?]*)");
    pos=exp.indexIn(buffstr);
    // вписываем длительность трека(номер сортировки / сдвиг)
    TW->item(trow,colLen)->setText(QString("#EXTINF:")+exp.cap(1));
    // определяем следующую точку начала поиска offset
    pos=pos+exp.matchedLength();

    // Chanell Name
    exp.setPattern("([^,]*)$");
    if (exp.indexIn(buffstr) != -1)
    // вписываем группу
    TW->item(trow,colCN)->setText(exp.cap(1).trimmed());

    /// СЛЕДУЮЩИЙ КОД ИЩЕТ ВСЕ ЛЮБЫЕ АТТРИБУТЫ
    /// удовлетворяющие шаблону attribute="value(s)" (валуе может быть в кавычках либо без них)
    exp.setPattern("\\b([^= ]*)=(\"([^\"]*)|([^, ]*))\\b");
    pos=0;
    QString key, val;
    while ((pos=exp.indexIn(buffstr,pos)) != -1) {
        pos += exp.matchedLength();
        key=exp.cap(1).trimmed().toLower(); //обязательно приводим к нижнему регистру (вдруг какой умник изменит)
        val=exp.cap(3).trimmed()+exp.cap(4).trimmed();
        // проверяем наличие аттрибута в шаблоне
        bool find=false;
        int cnt=attributes.size();
        int index=-1;
        for (int i=0; i<cnt; i++){
            if (attributes.at(i).key==key){
                index=i;    // индекс в списке, атрибут есть
                // проверим наличие в шаблоне
                if (attributes.at(i).exist){
                    find=true;  // в шаблоне атрибут определён
                }
            break;
            }
        }
        // (известные атрибуты тоже должны попасть в шаблон)
        if (!find){ // в шаблоне нет атрибута
            /// ЭТО НОВЫЙ атрибут для текущего шаблона, создаём элементы
            // новый чекбокс
            QCheckBox *chb;
            // проверяем известный это атрибут или новый
            if (index!=-1) { //
                // известный аттрибут
                // транслируем известные данные
                chb=new QCheckBox(attributes.at(index).name+QString(","),ui->frame);
            } else {
                // неизвестный атрибут
                attributes.prepend(* new attrItem);
                index=0;
                chb=new QCheckBox(key+QString(","),ui->frame);
                attributes[index].key=key;
                attributes[index].name=key;
            }
            // новая колонка или старая определяется в columnCheck(int) (?? названия не соответствуют)
            chb->hide();
            connect(chb,SIGNAL(stateChanged(int)),this,SLOT(columnCheck(int)));
            // проверяем новый столбец и отображаем
            chb->setChecked(true); // по умолчанию отображаем все новые столбцы
            /// TODO сигнал может вовремя и не дойти а номер колонки уже понадобится
            /// сделать вызов columnCheck(int) напрямую, только както надо передать указатель на chb
            // заносим в список шаблона
            attributes[index].exist=true;
            attributes[index].chb=chb;
            ///attributes.at(index).ncol=; // это проверяется в функции определения столбцов
            // оформляем чекбокс на панели
            adjustChkb(index);
        }
        /// TODO както сделать обработку множественных атрибутов, например несколько group-title в одной строке

        /// далее отрабатываются известные атрибуты
        //group_id=
        //exp.setPattern("group_id=([0-9]*)");
        if (key=="group_id" || key=="group_title") { //(exp.indexIn(buffstr) != -1)
            // вписываем группу
            TW->item(trow,colGroup)->setText(val);
            continue;
        }
        if (key=="epg-id" || key=="id" ){
            // отработаем особый случай vsetv_
            QRegExp expr("vsetv_([0-9]*)\\b");
            if (expr.indexIn(val) != -1) { /// TODO ввести префикс постфиксную систему для атрибутов
                val=expr.cap(1);
                epgprefix="vsetv_"; // особый случай (Единственный допущеный без префиксной системы)
            }
            TW->item(trow,colEPG)->setText(val);
            continue;
        }
        if (key=="tvg-logo" || key=="logo" ) {
            TW->item(trow,colLogo)->setText(val);
            continue;
        }
        /// далее (новый) атрибут обрабатывается почти как "известный", вносится в свою колонку
        TW->item(trow,attributes.at(index).ncol)->setText(val);
    }//*/
}

/// TODO должна поддерживаться файловая система, например D:\music.mp3
// парсинг URL
void MainWindow::prepareUrl(QTableWidget *TW, QString &buffstr, int trow)
{
    /// раскладываем строку url
    // сначала лишаем пробелов строку (их в url не должно быть)
    QRegExp reg("[\\s]");
    buffstr.remove(reg);
    // определяем до каких пор будет идти перебор
    ///int n=buffstr.Length();

    // раскладываем на составляющие (прокси потоки и т.п.)
    int tmp;
    // пишем полностью строку URL
    TW->item(trow,colURL)->setText(buffstr);

    /// первый IP если он не единственный как правило прокси, если единственный - то адрес потока +:порт
    /// второй IP если есть, это адрес потока + :порт
    /// всё что до первого адреса - префикс протоколов
    /// всё что после первого адреса до второго - постфикс
    /// всё что после второго адреса - ending

    QString proxy(""), stream(""), pport, sport;
    QString prefix, postfix, ending;
    // ищем первый адрес
    int pos=0;
    reg.setPattern("([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}):?([0-9]*)");
    if ((pos=reg.indexIn(buffstr,0)) != -1) {
        // отлично, есть первый IP определяем пока как адрес потока
        stream=reg.cap(1);
        sport=reg.cap(2);
        prefix=buffstr.left(pos);
        pos += reg.matchedLength();
        tmp=pos;
        // ищем второй адрес
        if ((pos=reg.indexIn(buffstr,pos)) != -1) {
            // отлично, у нас есть второй IP
            proxy=stream; pport=sport;
            stream=reg.cap(1);
            sport=reg.cap(2);
            postfix=buffstr.mid(tmp,pos-tmp);
            ending=buffstr.mid(pos+reg.matchedLength());
        } else {
            // имеем прямой поток
            ending=buffstr.mid(pos);
        }
    } else {
        // иначе это что-то другое, может указан доменный адрес??
        ;
    }
    TW->item(trow,colPrefix)->setText(prefix);
    TW->item(trow,colHTPXa)->setText(proxy);
    TW->item(trow,colHTPXp)->setText(pport);
    TW->item(trow,colPostfix)->setText(postfix);
    TW->item(trow,colSTRMa)->setText(stream);
    TW->item(trow,colSTRMp)->setText(sport);
    //TW->item(trow,col)->setText(ending);
 // конец препарации url
}

// получение и парсинг из EPG источников
void MainWindow::getEPGID(QString source)
{
    QUrl url(QString ("http://www.") + source);
    QNetworkAccessManager nam;
    QNetworkReply *reply = nam.get(QNetworkRequest(url));
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()),&loop, SLOT(quit()));
    loop.exec();
    QByteArray bytes;
    // Не произошло-ли ошибки?
    if (reply->error() == QNetworkReply::NoError)
    {
        // Читаем ответ от сервера
        bytes = reply->readAll();
        ///QString string(bytes);

        // Выводим ответ на экран
        //qDebug() << string;
        ///ui->textEdit->append(string);
    }
    // Произошла какая-то ошибка
    else
    {
        // обрабатываем ошибку
        //qDebug() << reply->errorString();
        // ui->textEdit->append(reply->errorString());
        QMessageBox::critical(QApplication::activeWindow(),tr("Ошибка!!!"),tr("Оширка получения данных сайта EPG."));  // ::about(
    }
    delete reply;

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
    exp.setPattern("value=channel_([0-9]+)>([^<]+)");
    int pos=0;
    while ((pos=exp.indexIn(string,pos)) != -1) {
        epgid[exp.cap(1)]=exp.cap(2);
        pos += exp.matchedLength();
    }
}

// поиск показ и скрытие колонок по ключу из списка атрибутов
// проверка наличия и создание новых колонок для новых атрибутов
void MainWindow::columnCheck(int)
{
    // определяем от кого пришел сигнал
    QCheckBox  *chcb = qobject_cast<QCheckBox*>(sender());
    // получаем название колонки
    QString cn=chcb->text();
    // отсекаем запятую
    QRegExp exp("([^,]*)");
    exp.indexIn(cn);
    cn=exp.cap(1);
    // определяем позицию в шаблоне
    ///сюда попадают с уже определённым в шаблоне названием атрибута индекс никогда не должен быть == -1
    int cnt=attributes.size();
    int index=-1;
    for (int i=0; i<cnt; i++){
        if (attributes.at(i).name==cn) {
            index=i;
        }
    }
    // отключаем или показываем колонки
    /// TODO если в таблицах колонки по разному перемещены?
    // сначала смотрим определена ли колонка для этого атрибута
    int col=attributes.at(index).ncol;
    if (col!=-1) { // известная колонка и атрибут
       // нашли колонку
       if (!chcb->isChecked()) {
           ui->tableWidget->hideColumn(col);
           table2.hideColumn(col);
       } else {
           ui->tableWidget->showColumn(col);
           table2.showColumn(col);
       }
    } else { //col==-1
        // новая, такой нет
        colcnt++;
        col=colcnt-1;
        ui->tableWidget->setColumnCount(colcnt);
        ui->tableWidget->setHorizontalHeaderItem(col, new QTableWidgetItem(cn));
        // заполняем ячейки
        int rcnt=ui->tableWidget->rowCount();
        for (int i=0;i<rcnt;i++){
            ui->tableWidget->setItem(i,col,new QTableWidgetItem(""));
        }
        table2.setColumnCount(colcnt);
        table2.setHorizontalHeaderItem(col, new QTableWidgetItem(cn));
        // заполняем ячейки
        rcnt=table2.rowCount();
        for (int i=0;i<rcnt;i++){
            table2.setItem(i,col,new QTableWidgetItem(""));
        }
        attributes[index].ncol=col;
        // if (!chcb->isChecked())  новые всегда отображаются
        return; // table2 уже нет смысл отрабатывать
    }
    // col уже был определен, а если новая то был return
    //if (table2.model()->headerData(i,Qt::Horizontal).toString() == cn) {
}

void MainWindow::findInTable1()
{

}

void MainWindow::findInTable2()
{

}

// прорисовка чекбоксов параметров
// если передан -1, очищаем всё и перерисовываем, иначе просто добавляем чекбокс на панель.
void MainWindow::adjustChkb(int idx)
{
    // секция для определения размеров и компоновки на форме элементов
    QFontMetrics fm(font());
    int len;// = fm.width("qwerty");
    //int heispasing = fm.lineSpacing();
    // узнаём точку отсчёта, отсюда будем добавлять чекбоксы
    int Y=global_y_colpanel;
    int X=ui->label->geometry().x();

    // узнаём текущий размер формы
    int fw=ui->frame->width();
    int stdw=20; // константа для пропуска галочки + промежуток
    int cnt=attributes.size();
    int maxX=X;
    int maxY=Y;
    int tmpx, tmpy;
    int index=-1;
    int cntstr=0; // максимальное количество строк - 3
    if (idx==-1) { /// перерисовка формы
        // очищаем панель (просто скрываем чекбоксы, чтобы не удалились совсем)
        for (int i=0;i<cnt;i++) {
            attributes.at(i).chb->hide();
        }
        // возвращаем всё на место
        ui->label->move(X,global_y_colpanel);
        ui->frame->setMinimumHeight(global_mh_colpanel);
    } else {
        // определяем, отображен ли чекбокс  отсекаем отображенные
        if (!attributes.at(idx).chb->isHidden())
            return;
        //else
        //перебираем чекбоксы, узнаём где отображён последний
        for (int i=0;i<cnt;i++) {
            if (!attributes.at(i).exist || attributes.at(i).chb->isHidden()) // отсекаем отсутствующие и скрытые (новый пока скрыт)
                continue;
            /// отсекаем также отображённые одного типа из принципа один тип, одна колонка, одно название (делается в columnCheck )
            /// но мы должны отсеч те атрибуты которые имеют одинаковые имена и exists==false
            ///  TODO если в плэйлисте есть такие два разных атрибута одного типа делать предупреждение
            /// TODO или в конфиге шаблона указывать принадлежность типа к имени атрибута
            if (attributes.at(i).ncol==attributes.at(idx).ncol && i!=idx) // идёт сравнение с отображёнными атрибутами
                return;
            tmpx=attributes.at(i).chb->geometry().x();
            tmpy=attributes.at(i).chb->geometry().y();
            if (tmpy<maxY) // можно сразу откидывать чекбоксы в меньшей строчке
                continue;
            if (tmpy>maxY || cntstr==0){ // || cntstr=0 обеспечиваем фиксацию первого чекбокса
                maxY=tmpy;
                // фиксируем
                index=i;
                cntstr++; //считаем строки
            }
            if (tmpx>maxX){
                maxX=tmpx;
                index=i;
            }
        }
        // вычисляем место текущей новой позиции
        if (index!=-1) { // или тоже самое (cntstr!=0)
            X=maxX+stdw+fm.width(attributes.at(index).name)+7;  /// + отступ
            Y=maxY;
        }
    }

    QCheckBox* ch; // для перебора
    for (int i=0;i<cnt;i++){
        if (idx!=-1) {
            i=cnt;// break; // если передан это не перебор
            ch=attributes.at(idx).chb;
        } else {
            if (!attributes.at(i).exist) // отсекаем отсутствующие
                continue;
            ch=attributes.at(i).chb;
        }
        // определяем длину нового чекбокса
        len=stdw+fm.width(ch->text());
        if ((X+len)>fw||cntstr==0) {
            // оформляем новую строку (превышен размер или первая строка)
            Y=ui->label->geometry().y();
            X=ui->label->geometry().x();
            // увеличиваем высоту фрейма
            ui->frame->setMinimumHeight(ui->frame->minimumHeight()+17);//+ch->geometry().height());// >height());//
            // сдвигаем лабел
            ui->label->move(X,Y+17);//+ch->geometry().height());
            cntstr++;
            if (cntstr>3) {
                // строки заполнены
                break;
            }
        }
        // строчка готова располагаем чекбокс
        ch->setGeometry(X,Y,len,17);//ch->geometry().height());
        // делаем видимым
        ch->show();
        X=X+len+7;  /// + отступ
    }
}

// сохранение файла ...
void MainWindow::savem3uas()
{
    savem3ufile(true);
}
// сохранение файла
void MainWindow::savem3uslot()
{
    savem3ufile(false);
}
void MainWindow::savem3ufile(bool as)
{
    //проверка на режим
    if (tablemode!=NORMALMODE) {
        QMessageBox::critical(QApplication::activeWindow(),tr("Внимание!"),tr("Сохранение Файла пока не работает в этом режиме."));
        return;
    }//*/
    // TODO смотрим группировку каналов, как будем оформлять группы
    ;
    // сохраняем в "тот же" файл
    if (filename.isEmpty()||as)
        filename=QFileDialog::getSaveFileName( 0, tr("Save File"));
    if (filename.isEmpty())
        return;
    /// TODO Сохранение должно быть по составленному шаблону

    ///****
    ///
    // пока тупо сохраняем для novosibIPTV2
    // открываем файл на запись
    QFile fout(filename);
    bool fopen=fout.open(QIODevice::WriteOnly);
    if (!fopen) {
        QMessageBox::critical(QApplication::activeWindow(),tr("Ошибка!!!"),tr("Файл не открывается.") + fout.errorString());  // ::about(
        return;
    }
    QTextStream out(&fout);
    /// TODO установка кодека из диалога (если пишем в тотже файл то в определенной при открытии кодировке)
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);
    // заголовок и служебные строки
    out << "#EXTM3U" << endl;
    // перебираем таблицу
    int countrow=ui->tableWidget->rowCount();
    for (int i=0;i<countrow;i++) {
        /// здесь должна вестись запись по шаблону
        // пропуск пустых
        if (ui->tableWidget->item(i,colCN)->text().isEmpty()) continue;
        // перебор и запись по шаблону
        // EXTINF
        out << ui->tableWidget->item(i,colLen)->text() << " ";
        // group id
        int grp=ui->comboBox_2->findText(ui->tableWidget->item(i,colGroup)->text());
        out << "group_id=" << grp << " ";
        // id EPG
        out << "id=" << ui->tableWidget->item(i,colEPG)->text() << " ";
        // logo
        out << "logo=" << ui->tableWidget->item(i,colLogo)->text() << " ";
        // channel name
        out << "," << ui->tableWidget->item(i,colCN)->text() << "\r\n";
        /// запись по шаблону строки url
        /// TODO url формировать автоматом в таблице по шаблонам
        out << ui->tableWidget->item(i,colURL)->text() << "\r\n";
    }
}

void MainWindow::findInTable(QTableWidget *TW, int col, QString text)
{
    if (TW==TW || col==col || text==text) /// TODO заглушка варнингов
        col=col;
}

// проверка наличия и добавление пунктов меню в EPGMODE
void MainWindow::contextMenuComboBoxEPG()
{
    // in EPGMODE
    if (menuEPG.actions().indexOf(aSetChName) == -1)
        menuEPG.insertAction(saveM3U, aSetChName);
    menuEPG.exec(QCursor::pos());
}

// Установить это название канала
void MainWindow::setChannelName()
{
    // in EPGMODE
    ui->tableWidget->item(ui->tableWidget->currentRow(),colCN)->setText(ui->tableWidget->currentItem()->text());
    /// TODO проверка на дубли
}

// удаление строк
void MainWindow::delrows()
{
    // проверяем сколько строк выбрано
    //QList selrows(ui->tableWidget->selectedRanges());
    QList<QTableWidgetSelectionRange> selran=ui->tableWidget->selectedRanges();
    //ui->tableWidget->verticalHeader()->selectionModel()->isRowSelected(>selectedRows();
    //ui->tableWidget->model()->beginRemoveRows();
    //for (int i=0; i< selran.size(); i++) {
    while (!selran.isEmpty()) {
        //QTableWidgetSelectionRange wsr=selrows.at(i);
        //wsr.topRow()
        //selran.at(i).
        ui->tableWidget->model()->removeRows(selran.at(0).topRow(),selran.at(0).rowCount());
        selran=ui->tableWidget->selectedRanges();
    }
    contextTableMenu->removeAction(adelrow);
}

// сравнение двух списков по источникам
void MainWindow::sortBySources() /// TODO delete function
{
    /// TODO +прогресбар
        // отсутствующие в первой таблице источники красим бледно-красным цветом, всю строку  (State == "")
        // те у которых совпадают источники но отличаются названия каналов красим жёлтым цветом  (== "N")
        // совпадающие (== "I")
        // TODO в первой тоже красим, чтобы обнаружить "лишние" каналы, лучше их потом определять/проверять тестом
        int count1=ui->tableWidget->rowCount();
        int count2=table2.rowCount();
        QString tmp;
        int identical;
        for (int i=0;i<count1;i++) {
            identical=-1;
            for (int n=0;n<count2;n++) {
                // проверяем совпадение на адрес источника
                // --- пока не проверяем и не сверяем и не отмечаем прокси---- считаем UDP и UDPX одинаковым типом (наличие прокси зависит от прова,
                // TODO если провайдер разный тогда каналы разные)
                // !!!!!!!!!!!!!! Пока делаем на одного провайдера !!!!!!!!!!!!!!

                if (ui->tableWidget->item(i,colSTRMa)->text() == table2.item(n,colSTRMa)->text()) {
                    // адреса совпали, проверяем порты
                    if (ui->tableWidget->item(i,colSTRMp)->text() == table2.item(n,colSTRMp)->text()) {
                        // порты совпали,
                        identical=n; /// поднимется последняя совпавшая строка
                        // проверяем имена каналов
                        tmp=ui->tableWidget->item(i,colCN)->text();
                        if (tmp == table2.item(n,colCN)->text()) {
                            // имена каналов совпали
                            ui->tableWidget->item(i,colState)->setText("I");
                            table2.item(i,colState)->setText("I");
                        } else {
                            ui->tableWidget->item(i,colState)->setText("N");
                            table2.item(i,colState)->setText("N");
                        }
                    }
                }
            }
            // расстановка строк
            if (identical!=-1) {
                // поднимаем строку
            } else {
                // совпавших нет, ставим пустую строку
                // (а надо ли, если первый список 100 строк а второй всего две, где они будут?)
            }
        }
        // теперь краски-раскраски :)
        int colcount=ui->tableWidget->columnCount();
        //QBrush
        for (int i=0;i<count1;i++) {
            if (ui->tableWidget->item(i,colState)->text() == QString("")) {
                for (int n=0; n < colcount; n++) {
                    ui->tableWidget->item(i,n)->setBackground(Qt::red);
                }
            }
            if (ui->tableWidget->item(i,colState)->text() == QString("N")) {
                ui->tableWidget->item(i,colCN)->setBackground(Qt::yellow);
            }
        }
        colcount=table2.columnCount();
        // вторая табличка
        for (int i=0;i<count2;i++) {
            if (table2.item(i,colState)->text() == QString("")) {
                for (int n=0; n < colcount; n++) {
                    table2.item(i,n)->setBackground(Qt::red);
                }
            }
            if (table2.item(i,colState)->text() == QString("N")) {
                table2.item(i,colCN)->setBackground(Qt::yellow);
            }
        }
}

//установка и снятие режима отображения совпадающих строк для table2 SHOWBYSOURCES
void MainWindow::showBySources()
{
    // устанавливаем режим отображения совпадающих строк для table2
    QAction* act = qobject_cast<QAction*>(sender());
    if (act->isChecked()){
        // установка режима
        tablemode=SHOWBYSOURCES;
        on_tableWidget_currentCellChanged(ui->tableWidget->currentRow(),ui->tableWidget->currentColumn(),ui->tableWidget->currentRow(),ui->tableWidget->currentColumn());
    } else {
        // снятие режима
        //for (int i=0) todo setnormal()
        tablemode=NORMALMODE;
    }
}

void MainWindow::removeTable2()
{
    table2.hide();
}

// открытие файла для таблицы 2 и сравнение
void MainWindow::compareFile()
{
    /// подключаем вторую таблицу
    // динамическое формирование таблицы
    ui->splitter->addWidget(&table2);
    // открытие файла
    openM3U(&table2,nullptr);
    //table2.show(); сделано в openM3U
    if (!table2.isHidden()) // значит таблица для сравнения открыта
        compareM3U();
}

// сравнение двух списков M3U
void MainWindow::compareM3U()
{
/// TODO +прогресбар
    // отсутствующие в первой таблице источники красим бледно-красным цветом, всю строку  (State == "")
    // те у которых совпадают источники но отличаются названия каналов красим жёлтым цветом  (== "N")
    // совпадающие (== "I")
    // TODO в первой тоже красим, чтобы обнаружить "лишние" каналы, лучше их потом определять/проверять тестом
    int count1=ui->tableWidget->rowCount();
    int count2=table2.rowCount();
    QString tmp;
    for (int i=0;i<count1;i++) {
        for (int n=0;n<count2;n++) {
            // проверяем совпадение на адрес источника
            // --- пока не проверяем и не сверяем и не отмечаем прокси---- считаем UDP и UDPX одинаковым типом (наличие прокси зависит от прова,
            // TODO если провайдер разный тогда каналы разные)
            // !!!!!!!!!!!!!! Пока делаем на одного провайдера !!!!!!!!!!!!!!
            if (ui->tableWidget->item(i,colSTRMa)->text() == table2.item(n,colSTRMa)->text()) {
                // адреса совпали, проверяем порты
                if (ui->tableWidget->item(i,colSTRMp)->text() == table2.item(n,colSTRMp)->text()) {
                    // порты совпали, проверяем имена каналов
                    tmp=ui->tableWidget->item(i,colCN)->text();
                    if (tmp == table2.item(n,colCN)->text()) {
                        // имена каналов совпали
                        ui->tableWidget->item(i,colState)->setText("I");
                        table2.item(n,colState)->setText("I");
                    } else {
                        ui->tableWidget->item(i,colState)->setText("N");
                        table2.item(n,colState)->setText("N");
                    }
                }
            }
        }
    }
    // теперь краски-раскраски :)
    /// TODO убрать покраску дублей перед этим (и активировать меню поиска дублей)
    int colcount=ui->tableWidget->columnCount();
    //QBrush
    for (int i=0;i<count1;i++) {
        if (ui->tableWidget->item(i,colState)->text() == QString("")) {
            for (int n=0; n < colcount; n++) {
                ui->tableWidget->item(i,n)->setBackground(Qt::red);
            }
        }
        if (ui->tableWidget->item(i,colState)->text() == QString("N")) {
            ui->tableWidget->item(i,colCN)->setBackground(Qt::yellow);
        }
    }
    colcount=table2.columnCount();
    // вторая табличка
    for (int i=0;i<count2;i++) {
        if (table2.item(i,colState)->text() == QString("")) {
            for (int n=0; n < colcount; n++) {
                table2.item(i,n)->setBackground(Qt::red);
            }
        }
        if (table2.item(i,colState)->text() == QString("N")) {
            table2.item(i,colCN)->setBackground(Qt::yellow);
        }
    }
}

// формирование таблицы (константная форма, вызывается один раз в кострукторе окна)
void MainWindow::setTableM3U(QTableWidget *TW)
{
    // рисуем столбцы
    /// TODO изменение названия колонок влечёт рассинхронизацию в работе шаблонов
    /// надо оформить автоматическую обработку либо отдельным обработчиком либо классом
    /// триада шаблон - таблица - параметры
    // при изменении состава меняем TForm1::StringGrid1ColumnMoved
    TW->setColumnCount(colcnt);
    TW->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ///TW->setHorizontalHeaderItem(colNst, new QTableWidgetItem("Nst")); // ->horizontalHeaderItem(colNst)->setText("Nst"); //
    ///TW->setColumnWidth(colNst,wthNst);
    ///TW->horizontalHeaderItem(colNst)->setTextAlignment(Qt::AlignLeft);
    TW->setHorizontalHeaderItem(colCN, new QTableWidgetItem("Track/Channel Name"));
    TW->setColumnWidth(colCN,wthCN);
    TW->setHorizontalHeaderItem(colLen, new QTableWidgetItem("Duration")); // Length
    TW->setColumnWidth(colLen,wthLen);
    TW->setHorizontalHeaderItem(colGroup, new QTableWidgetItem("GroupID"));
    TW->setColumnWidth(colGroup,wthGroup);
    TW->hideColumn(colGroup);
    TW->setHorizontalHeaderItem(colEPG, new QTableWidgetItem("EPGID"));
    TW->setColumnWidth(colEPG,wthEPG);
    TW->hideColumn(colEPG);
    TW->setHorizontalHeaderItem(colLogo, new QTableWidgetItem("LogoID"));
    TW->setColumnWidth(colLogo,wthLogo);
    TW->hideColumn(colLogo);
    TW->setHorizontalHeaderItem(colURL, new QTableWidgetItem("URL"));
    TW->setColumnWidth(colURL,wthURL);
    //TW->hideColumn(colURL);
    TW->setHorizontalHeaderItem(colPrefix, new QTableWidgetItem("Prefix"));
    TW->setColumnWidth(colPrefix,wthPrefix);
    TW->setHorizontalHeaderItem(colSTRMa, new QTableWidgetItem("Addr.Stream"));
    TW->setColumnWidth(colSTRMa,wthSTRMa);
    TW->setHorizontalHeaderItem(colSTRMp, new QTableWidgetItem("Port.Stream"));
    TW->setColumnWidth(colSTRMp,wthSTRMp);
    TW->setHorizontalHeaderItem(colPostfix, new QTableWidgetItem("Postfix"));
    TW->setColumnWidth(colPostfix,wthPostfix);
    TW->setHorizontalHeaderItem(colHTPXa, new QTableWidgetItem("Addr.Proxy"));
    TW->setColumnWidth(colHTPXa,wthHTPXa);
    TW->setHorizontalHeaderItem(colHTPXp, new QTableWidgetItem("Port.Proxy"));
    TW->setColumnWidth(colHTPXp,wthHTPXp);
    TW->hideColumn(colProv); //----- провайдер
    TW->setHorizontalHeaderItem(colState, new QTableWidgetItem("State"));
    TW->setColumnWidth(colState,wthState);
    TW->hideColumn(colState);
    TW->setHorizontalHeaderItem(colCCN, new QTableWidgetItem("Canonical Channel Name"));
    TW->setColumnWidth(colCCN,wthState);
    TW->hideColumn(colCCN);
    //TW->setHorizontalHeaderItem(colAudio, new QTableWidgetItem("AudioTrk"));
    //TW->setColumnWidth(colAudio,wthAudio);
    //StringGrid1->ColWidths[colSTRM]=-1; /// скрываем столбец
    //*/
}

// установка и снятие режима редактирования EPG ID
void MainWindow::setEpgMode()
{
    tablemode=EPGMODE;
    //ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    QAction* act = qobject_cast<QAction*>(sender());
    if (act->isChecked()) {
        // так как у нас всё будет в динамике, скрываем колонки в цикле кроме нужных
        int colcount=ui->tableWidget->columnCount();
        for (int i=0; i<colcount; i++) {
            if (i!=colCN && i != colEPG )
                ui->tableWidget->hideColumn(i);
        }
        /*/ добавляем колонку наименований каналов с сайтов источников EPG
        ui->tableWidget->insertColumn(colEPG+1);
        ui->tableWidget->setHorizontalHeaderItem(colEPG+1, new QTableWidgetItem("EPG Channel Name"));
        ui->tableWidget->setColumnWidth(colEPG+1,150);//*/
        ui->tableWidget->showColumn(colCCN);
        // заполняем колонку соответствия EPG по индексам, если есть
        int rowcount=ui->tableWidget->rowCount();
        QString key, val;
        int cnt;
        for (int i=0; i< rowcount; i++) {
            QTableWidgetItem *item=new QTableWidgetItem;
            // берём значение из ячейки EPGID
            key=ui->tableWidget->item(i,colEPG)->text();
            // находим соответствие с названием канала
            //val=epgid.value(key);
            //val=epgchModel.findItems(key,,1).at(0)->text(); // column 1 - id
            cnt=epgchModel.rowCount();
            for (int i=0; i<cnt; i++) {
                if (epgchModel.item(i,1)->text()==key) {        // column 1 - id
                    val=epgchModel.item(i,0)->text();   // column 0 - cn
                }
            }
            item->setText(val);
            // устанавливаем ячейку в таблицу
            ui->tableWidget->setItem(i,colCCN,item);
        }
        // делаем менюшку для EPGMODE
        menuEPG.addAction(aSetChName);
        menuEPG.addAction(saveM3U);
        menuEPG.addAction(asaveM3Uas);
        menuEPG.addSection("-Режимы редактирования-"); // TODO что-то не работает
        menuEPG.addAction(actEpgMode);
    } else { // вызвана эта же менюха, возвращаем NORMALMODE
        // unchecked
        // удаляем EPGCH колонку
        ui->tableWidget->hideColumn(colCCN);
        setNormalModeTable();
        tablemode=NORMALMODE;
    }
}

void MainWindow::setNormalModeTable()
{
    // прорисовываем константные колонки
    ui->tableWidget->showColumn(colCN);
    ui->tableWidget->showColumn(colLen);
    ui->tableWidget->showColumn(colSTRMa);
    ui->tableWidget->showColumn(colSTRMp);
    /// TODO надо ли остальные в константные?
    ui->tableWidget->showColumn(colHTPXa);
    ui->tableWidget->showColumn(colHTPXp);
    ui->tableWidget->showColumn(colURL);
    ui->tableWidget->showColumn(colPrefix);
    ui->tableWidget->showColumn(colPostfix);
    // прорисовываем атрибуты
    int cnt = attributes.size();
    for (int i=0;i<cnt;i++) {
        if (attributes.at(i).chb->isChecked())
            ui->tableWidget->showColumn(attributes.at(i).ncol);
    }
}

// вписываем в таблицу выбранное значение ID канала EPG
void MainWindow::cb_currentIndexChanged(int idx)
{
    // у нас оно работает в режиме epgmode
    if (tablemode==EPGMODE) {
        // получаем адрес объекта пославшего сигнал
        QComboBox *cb=qobject_cast<QComboBox *>(sender());
        // вписываем в таблицу выбранное значение
        if (epgchModel.item(idx,1)!=0){
            //ui->tableWidget->item(ui->tableWidget->currentRow(),colEPG)->setText(epgid.key(cb->currentText()));
            ui->tableWidget->item(ui->tableWidget->currentRow(),colEPG)->setText(epgchModel.item(idx,1)->text());
            ui->tableWidget->currentItem()->setText(cb->currentText());
        }
    }
}

void MainWindow::on_comboBox_4_currentIndexChanged(int index)
{
    index=index; /// TODO заглушка варнингов
}

// выбор источника EPG
void MainWindow::comBox_currentIndexChanged(int index)
{
    // получаем данные с Channels
    Channels::EpgSources *source=static_cast<Channels::EpgSources *>(channels->cb->itemData(index).value<void *>());
    channels->readEPGs(source);
    // перед удалением модели удаляем виджет с ней
    //ui->tableWidget->setCellWidget(cellwidgetcoord.row, cellwidgetcoord.column, nullptr);
    // но перед этим отключаем сигналы, виджет сразу не удаляется а сигналы летят когда ещё модель не готова
    if (cb!=nullptr) {
        disconnect(cb,SIGNAL(currentIndexChanged(int)));
        disconnect(cb,SIGNAL(customContextMenuRequested(QPoint)));
    }
    ui->tableWidget->removeCellWidget(cellwidgetcoord.row,cellwidgetcoord.column); //*/ //будет удалён также QStackedWidget
    // очищаем от старого или другого EPG
    epgchModel.clear();
    // получаем доступ к хранилищу
    QListIterator<Channels::EpgInfo *> i(channels->listepgs.at(index)->epgchannels);
    // заполняем модель
    epgchModel.setColumnCount(3); // cn, id, icon,
    //epgchModel.setRowCount(channels->listepgs.at(index)->epgchannels.size());
    //epgchModel.setRowCount(10); само установится в setitem
    Channels::EpgInfo tmp;
    int cnt=0;
    while(i.hasNext()) {
        tmp=*i.next();
        //epgchModel.appendRow(new QStandardItem(i.next().cn));
        epgchModel.setItem(cnt,0,new QStandardItem(tmp.cn));
        epgchModel.setItem(cnt,1,new QStandardItem(tmp.id));
        epgchModel.setItem(cnt,2,new QStandardItem(tmp.urlicon));
        cnt++;
    }
    epgchModel.sort(0);//*/
    /*/ проверяем читали мы уже epgid или нет
    if (!ui->comboBox->currentData().isValid()) {
        // получаем текущий список каналов и epgID c указанного сайта
        getEPGID(ui->comboBox->currentText());
        QVariant *var=new QVariant;
        var->setValue(epgid);
        ui->comboBox->setItemData(index,*var);
    }///
    // очищаем от старого или другого EPG
    epgchModel.clear();
    // запихиваем список в combobox 4
    QVariant test=ui->comboBox->currentData();
    // if (test.isValid()) на валидность уже проверили перед этим
    // получаем хранилище списка
    QHash<QString, QString> *tst2=(QHash<QString, QString> *)test.data();
    QHashIterator<QString, QString> i(*tst2);
    while(i.hasNext()) {
        i.next();
        epgchModel.appendRow(new QStandardItem(i.value()));
    }
    epgchModel.sort(0);//*/
}

// действие при проводе мышкой над ячейкой таблицы
void MainWindow::on_tableWidget_cellEntered(int row, int column)
{
    if (actEpgMode->isChecked() && column==(colCCN)) {
        // то что надо,
        // закрепляем текущую ячейку
        ui->tableWidget->setCurrentCell(row,column);
        // читаем координаты старой ячейки
        int r=cellwidgetcoord.row; int c=cellwidgetcoord.column;
        // сохраняем координаты текущей ячейки (для последующего её восстановления)
        cellwidgetcoord.row=row; cellwidgetcoord.column=column;
        // удаляем виджет из старой ячейки
        ui->tableWidget->removeCellWidget(r,c); // будет удалён также QStackedWidget
        // устранавливаем в ячейку виджет
        cb=new QComboBox;
        cb->setModel(&epgchModel);
        cb->setMaxVisibleItems(30);
        cb->setCurrentIndex(cb->findText(ui->tableWidget->item(row,column)->text()));
        connect(cb,SIGNAL(currentIndexChanged(int)),SLOT(cb_currentIndexChanged(int)));
        cb->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(cb,SIGNAL(customContextMenuRequested(QPoint)),SLOT(contextMenuComboBoxEPG()));
        ui->tableWidget->setCellWidget(row, column, cb);
        // TODO ставим блокировку срабатывания события на изменение текущего индекса в on_comboBox_4_currentIndexChanged
    }
}

// разбор строки шаблона
void prepStrTemplate(QString *str)
{
    int pos=0;
    /// line dfdfdf$$34234234$$xxxxxxxxxx$$2342434234
    // константа
    QRegExp *exp=new QRegExp("([^\\$\\$]*)");   /// match-dfdfdf cap1-dfdfdf
    // подстановка
    QRegExp *parx=new QRegExp("\\$\\$([^\\$\\$]*)\\$\\$"); /// match-$$34234234$$ cap1-34234234

    if (str==str || pos==pos || exp == exp || parx==parx) /// TODO заглушка варнингов
        pos=pos;
}

void MainWindow::on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    // смотрим режим
    if (tablemode==SHOWBYSOURCES) {
        // перебираем вторую таблицу и скрываем ненужные ячейки
        int count=table2.rowCount();
        QString addr=ui->tableWidget->item(currentRow,colSTRMa)->text();
        QString port=ui->tableWidget->item(currentRow,colSTRMp)->text();
        for (int i=0; i<count; i++) {
            if (table2.item(i,colSTRMa)->text()==addr && table2.item(i,colSTRMp)->text()==port) {
                table2.showRow(i);
            } else {
                table2.hideRow(i);
            }
        }
    }
    if (currentColumn || previousRow || previousColumn) /// TODO заглушка варнингов
        currentColumn=currentColumn;
}

FindDialog::FindDialog(QWidget *parent, const char *name)
     : QDialog(parent)
{
    setWindowTitle(tr(name));
    label = new QLabel(tr("Что ищем?:"), this);
    lineEdit = new QLineEdit(this);
    label->setBuddy(lineEdit);
    findButton = new QPushButton(tr("Найти"), this);
    findButton->setDefault(true);
    findButton->setEnabled(false);
    connect(lineEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(enableFindButton(const QString &)));
    connect(findButton, SIGNAL(clicked()),
        this, SLOT(findClicked()));
    QHBoxLayout *topLeftLayout = new QHBoxLayout(this);
    topLeftLayout->addWidget(label);
    topLeftLayout->addWidget(lineEdit);
    topLeftLayout->addWidget(findButton);
}

void MainWindow::on_MainWindow_destroyed()
{
    // удаляем вторую форму
    channels->close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    /*if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }//*/
    groups->savebase();
    channels->savebase();
    channels->close();
    groups->close();
    event->accept();
}

///////// !!!!!!!!!!!! TODO TEST  DEL
void MainWindow::on_pushButton_pressed()
{
    ui->pushButton->setDown(true);
    QDrag *drag = new QDrag(ui->pushButton);
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(ui->pushButton->text());
    drag->setMimeData(mimeData);
    drag->exec();
    ui->pushButton->setDown(false);
}

///DEL
void MainWindow::on_pushButton_clicked()
{
    //
}

///DEL
void MainWindow::on_pushButton_2_clicked()
{
    //groups->setWindowFlags(Qt::Popup);
    groups->show();
}
