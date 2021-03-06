﻿#include "productionsafetycontrolsys.h"
#include "ui_productionsafetycontrolsys.h"
#include <windows.h>
#include <QMessageBox>
#include <QUuid>
#include <QDebug>
#include "Snapshop/snapshop_dialog.h"
#include "choosedoorstate/choose_door_state.h"
#include <QDesktopWidget>
#include <QRect>
#include <QImage>

QT_CHARTS_USE_NAMESPACE
extern HWND hWnd;

ProductionSafetyControlSys::ProductionSafetyControlSys(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProductionSafetyControlSys)
{
    ui->setupUi(this);

    setWindowTitle(QStringLiteral("中国大唐集团公司电厂安全生产管控系统"));
    this->showFullScreen();
    this->setStyleSheet("ProductionSafetyControlSys{background-color: rgb(111, 170, 221);color:#ffffff;}");

    //两票执行监控初始化
    TwoTicketMoni = new TwoTicketExeMonitor(ui->page_5);

    //安全趋势分析初始化
    SafeTrendWidget = new SafeTrendAnalysis(ui->page_6);
    Alarmrecord = new Alarm_record();

    //重点区人员查询初始化
    Importarea = new ImportantArea(ui->page_7);
    Camerawidget = new CameraWidget(ui->page_3);
    ticketMtepMonitor = new TicketStepMonitor(ui->page_2);

    //区域监控初始化
    Areaemploy = new AreaEmployee(ui->page_8);

    //数据区域平面图
    RegionWidget = new RegionName(ui->bottomWiget);

    //视频界面初始化
    Hk_view = new HK_Video(this); //ui->page_4
    InitWidget();

    //重绘首页表格数据
    repaintTable();

    //线程
    //Mycurve=new MyCurve;
    //告警记录按钮界面
    Alarmwidget = new AlarmWidget(ui->page_9); //ui->page_9

    //
    worker = new WorkThreader;
    worker->moveToThread(&workThread);
    connect(this,SIGNAL(postImportArea()),worker,SLOT(UpdataTablewidgetImportarea()));
    connect(worker,SIGNAL(postImportAreaList(WorkThreader*)),this,SLOT(reciverImportAreaList(WorkThreader*)));
    workThread.start();

    //设置事件过滤器
    ui->frame_4->installEventFilter(this);                  //作业数
    ui->TodayAlarmTotalFrame->installEventFilter(this);     //作业人数
    ui->frame_5->installEventFilter(this);                  //作业区域数
    this->setMouseTracking(true);

	//初始化菜单栏
	InitPopuMenu();
    connect(ui->HomepageTable,SIGNAL(itemClicked(QTableWidgetItem*)),this,SLOT(tableWidgetClick(QTableWidgetItem*)));
    connect(Camerawidget,SIGNAL(CameraClicked()),this,SLOT(Video_Button_clicked()));
    connect(Importarea,SIGNAL(View_Id(QString)),this,SLOT(ViewShow(QString)));                      //kfss add

    //AlarmWidget界面连接走错间隔按钮响应事件
    connect(Alarmwidget,SIGNAL(SignalGoingIntervalBtn_clicked(QString)),this,SLOT(AcceptSignalGoingInterval(QString)));
    //AlarmWidget界面设备确认错误按钮响应事件
    connect(Alarmwidget,SIGNAL(DevWrongClicked(QString)),this,SLOT(Signal_DevWrongClicked(QString)));
    //AlarmWidget界面作业顺序错误按钮响应事件
    connect(Alarmwidget,SIGNAL(LeaveOutClicked(QString)),this,SLOT(Signal_LeaveOutClicked(QString)));
    //AlarmWidget界面人员闯禁按钮响应事件
    connect(Alarmwidget,SIGNAL(PersonForbidClicked(QString)),this,SLOT(Signal_PersonForbidClicked(QString)));
    //AlarmWidget界面人员求助按钮响应事件
    connect(Alarmwidget,SIGNAL(PersonHelpClicked(QString)),this,SLOT(Signal_PersonHelpClicked(QString)));
    //AlarmWidget界面人员代办按钮响应事件
    connect(Alarmwidget,SIGNAL(CommissionClicked(QString)),this,SLOT(Signal_CommissionClicked(QString)));
    //AlarmWidget界面非法门禁按钮响应事件
    connect(Alarmwidget,SIGNAL(DoorwrongClicked(QString)),this,SLOT(Signal_DoorwrongonClicked(QString)));
    //AlarmWidget界面无卡作业按钮响应事件
    connect(Alarmwidget,SIGNAL(NoCardClicked(QString)),this,SLOT(Signal_NoCardclicked(QString)));
    //AlarmWidget界面无票作业按钮响应事件
    connect(Alarmwidget,SIGNAL(NoTicketClicked(QString)),this,SLOT(Signal_NoTicketClicked(QString)));


    //ticketMtepMonitor界面工作票表格单击事件响应
    connect(TwoTicketMoni,SIGNAL(Signal_ticketMtepMonitor(QString)),this,SLOT(Signal_ticketMtepMonitorClicked(QString)));
    //ticketMtepMonitor界面操作表格单击事件响应
    connect(TwoTicketMoni,SIGNAL(Signal_OperationWidget(QString)),this,SLOT(Signal_OperationWidgetClicked(QString)));
    //ticketMtepMonitor界面返回按钮的响应事件
    connect(ticketMtepMonitor,&TicketStepMonitor::ReturnLastPage,this,&ProductionSafetyControlSys::TwoTicketStepPageReturn);

    //两票步骤监控界面返回按钮
    connect(ticketMtepMonitor, SIGNAL(ReturnLastPage()), this, SLOT(TwoTicketStepPageReturn()));
    //安全监控按钮事件响应
	connect(ui->SafetyMonitoringpushButton, &QPushButton::clicked, this, &ProductionSafetyControlSys::on_SafetyMonitoringpushButton_clicked);
	//告警查询按钮事件响应
	connect(ui->AlarmAnalyzepushButton, &QPushButton::clicked, this, &ProductionSafetyControlSys::on_AlarmAnalyzepushButton_clicked);
	//全厂总揽按钮事件响应
	connect(ui->HomePagepushButton, &QPushButton::clicked, this, &ProductionSafetyControlSys::on_HomePagepushButton_clicked);
}

void ProductionSafetyControlSys::InitWidget()
{
    //datalist=ReadDataFromMysql::instance()->Updata_tableWidget_ImportArea();
    lUserID = -1;
    ui->stackedWidget->setCurrentIndex(0);
    ui->hk_VideoLabel->installEventFilter(this);
    ui->LogoLabel->setStyleSheet("QLabel{border-image:url(:/image/logo.png);}");
    ui->HomePagepushButton->setStyleSheet("QPushButton{background-color:transparent;\
                                          font-family:'Microsoft YaHei';font-size:15px;color:#ffffff;}"
                                            "QPushButton::menu-indicator{image:none;}");
    ui->SafetyMonitoringpushButton->setStyleSheet("QPushButton{background-color:transparent;color:#ffffff;\
                                              font-family:'Microsoft YaHei';font-size:15px;}"
                                            "QPushButton::menu-indicator{image:none;}");
    ui->AlarmAnalyzepushButton->setStyleSheet("QPushButton{background-color:transparent;color:#ffffff;\
                                          font-family:'Microsoft YaHei';font-size:15px;}"
                                            "QPushButton::menu-indicator{image:none;}");
    ui->BaseConfpushButton->setStyleSheet("QPushButton{background-color:transparent;color:#ffffff;\
                                      font-family:'Microsoft YaHei';font-size:15px;}"
                                        "QPushButton::menu-indicator{image:none;}");


    /***************************************************************************************************
     *
     * 工作表格内容初始化
     * *************************************************************************************************/
    ui->tabWidget->setCurrentIndex(1);//设置首页tabwidget当前活动标签页
    //ui->jobtableWidget->setHorizontalScrollBar(Qt::ScrollBarAlwaysOff);
    ui->jobtableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  //隐藏水平滚动条
    QStringList header;
    ui->jobtableWidget->setRowCount(7);
    ui->jobtableWidget->setColumnCount(2);
    header <<QStringLiteral("作业名称")<<QStringLiteral("作业时间");
    ui->jobtableWidget->setHorizontalHeaderLabels(header);
    ui->jobtableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置表格不可编辑
    ui->jobtableWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->jobtableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置表格点击为选中一行
    ui->jobtableWidget->setFocusPolicy(Qt::NoFocus);  //设置表格点击虚边框为元
    ui->jobtableWidget->setShowGrid(false);           //设置无表格
    ui->jobtableWidget->setAlternatingRowColors(true);
    ui->jobtableWidget->setColumnWidth(0,400);
    ui->jobtableWidget->setColumnWidth(1,100);

    //首页数据定时更新显示
    Timer = new QTimer(this);     //设置定时器
    connect(Timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    Timer->start(5000);

    //首页时钟显示
    HomepageTimer=new QTimer(this);
    connect(HomepageTimer,&QTimer::timeout,this,&ProductionSafetyControlSys::HomePageTimeUpdate);
    HomepageTimer->start(1000);

    QPalette Color;
    Color.setColor(QPalette::WindowText,Qt::white); //设置字体颜色
    //ui->CurAlarmLabel->setPalette(Color);
    ui->CurAlarmNumLabel->setPalette(Color);
    //ui->TodayAlarmTotalLabel->setPalette(Color);
    ui->personNumLabel->setPalette(Color);
    //ui->CurExecWorkLabel->setPalette(Color);
    ui->workNumLabel->setPalette(Color);
    //ui->SysRunLabel->setPalette(Color);
    ui->key_areaLabel->setPalette(Color);
    ui->label_10->setPalette(Color);
    ui->label_11->setPalette(Color);
}


//告警分析tablewidget初始化
ProductionSafetyControlSys::~ProductionSafetyControlSys()
{
    if(NULL !=worker)
    {
         delete worker;
    }

    if(NULL !=Hk_view)
    {
        Hk_view->deleteLater();
    }

    if(Run_Flag==1)
    {
        Run_Flag=0;
        Target_Process.execute(QString("taskkill /im Webbrowser.exe /f"));
    }
    delete ui;
}


/************************************************************************
  Description：事件过滤器函数实现
  Input：
  Output:
  Return:返回事件过滤的对象
  Others:
************************************************************************/
bool ProductionSafetyControlSys::eventFilter(QObject *obj, QEvent *event)
{
    if(obj==ui->hk_VideoLabel)
    {
        if(event->type()==QEvent::MouseButtonDblClick)
        {
            if(click)
            {
                click=0;
//              ui->Webbrowerframe->setVisible(false);
                ui->tabWidget->setVisible(false);
                ui->frame_4->setVisible(false);
                ui->frame_5->setVisible(false);
                ui->frame_6->setVisible(false);
                ui->frame_7->setVisible(false);
                ui->frame_8->setVisible(false);
                ui->dataViewBtn->setVisible(false);
                ui->pushButton->setVisible(false);
                ui->frame_2->setVisible(false);
                ui->TodayAlarmTotalFrame->setVisible(false);
                QDesktopWidget *Widget=QApplication::desktop();
                QRect Screensize=Widget->geometry();
                int width=Screensize.width();
                int height=Screensize.height();
                ui->SecurityTrendFrame->resize(width,height);
            }
            else
            {
                click=1;
//				ui->Webbrowerframe->setVisible(true);
                ui->tabWidget->setVisible(true);
                ui->frame_4->setVisible(true);
                ui->frame_5->setVisible(true);
                ui->frame_6->setVisible(true);
                ui->frame_7->setVisible(true);
                ui->frame_2->setVisible(true);
                ui->frame_8->setVisible(true);
                ui->TodayAlarmTotalFrame->setVisible(true);
                ui->dataViewBtn->setVisible(true);
                ui->pushButton->setVisible(true);
                ui->SecurityTrendFrame->resize(640,296);
            }
        }
    }

    if(obj == ui->frame_4)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            TwoTicketMoniorClicked();
        }
    }

    if(obj == ui->TodayAlarmTotalFrame)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            AreaMonitoringActionClicked();
        }
    }

    if(obj == ui->frame_5)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            PersionQueryActionClicked();
        }
    }
    return QObject::eventFilter(obj,event);
}


//更新首面信息
void ProductionSafetyControlSys::updateHomepageInfo()
{
    WriDataToTable(ReadDataFromMysql::instance()->UpdateAlarm_FirstPage());
}

//首面表格数据
void ProductionSafetyControlSys::WriDataToTable(QList<AlarmFirstPage> strList)
{
    QStringList header;
    AlarmFirstPage tmpAlarmInfo;

    //设置显示表头的内容
	header << QStringLiteral("人员") << QStringLiteral("作业区域") << QStringLiteral("告警事件") << QStringLiteral("告警时间") << QStringLiteral("");
    ui->HomepageTable->setHorizontalHeaderLabels(header);

    ui->HomepageTable->setRowCount(strList.size());
    ui->alarm_label->setText(QString::number(strList.size()));  //本日告警总数
    for(int i = 0; i < strList.count(); i++)
    {
        tmpAlarmInfo = strList.at(i);
        ui->HomepageTable->setItem(i,0,new QTableWidgetItem(tmpAlarmInfo.Employy_Name));
        ui->HomepageTable->setItem(i,1,new QTableWidgetItem(tmpAlarmInfo.Area_Name));
        ui->HomepageTable->setItem(i,2,new QTableWidgetItem(tmpAlarmInfo.Alarm_Content));
        ui->HomepageTable->setItem(i,3,new QTableWidgetItem(tmpAlarmInfo.Alarm_Time));
        ui->HomepageTable->setItem(i,4, new QTableWidgetItem(QStringLiteral("确认")));

        //每行的字体显示居中
        ui->HomepageTable->item(i,0)->setTextAlignment(Qt::AlignCenter);
        ui->HomepageTable->item(i,1)->setTextAlignment(Qt::AlignCenter);
        ui->HomepageTable->item(i,2)->setTextAlignment(Qt::AlignCenter);
        ui->HomepageTable->item(i,3)->setTextAlignment(Qt::AlignCenter);
        ui->HomepageTable->item(i,4)->setTextAlignment(Qt::AlignCenter);

        if(tmpAlarmInfo.Alarm_Level == "5" || tmpAlarmInfo.Alarm_Level=="4")
        {
            ui->HomepageTable->item(i, 4)->setTextColor(QColor(255, 0, 0));
        }
        else
        {
            ui->HomepageTable->item(i, 4)->setTextColor(QColor(255, 170, 0));
        }
    }
}

/************************************************************************
  Description：定时器槽函数响应事件
  Input：
  Output:
  Return:
  Others:
************************************************************************/
void ProductionSafetyControlSys::timerUpdate()
{
    emit postImportArea();
    //QDateTime time=QDateTime::currentDateTime();
    //QString timestr=time.toString("yyyy/MM/dd hh:mm:ss");
    //QString weektime=time.toString("dddd");

    //ui->label_10->setText(timestr);
    //ui->label_11->setText(weektime);
    bool isWork=ReadDataFromMysql::instance()->IsAreaAlarm();
    //qDebug()<<"iswork111="<<isWork;
    //bool isWork=Mycurve->Getiswork();
    if(isWork)
    {
        //qDebug()<<"iswork";
        ui->homepageBtnB->setStyleSheet("QToolButton{background-color:#ff0000;border: none solid ;"
                                        "border-style: solid;border-radius:6px;width: 40px;"
                                        "height:20px; padding:0 0px;margin:0 0px;}");
    }
    else
    {
        QPixmap pixmap_1(":/image/1.png");
        ui->homepageBtnB->setMask(pixmap_1.mask());
        ui->homepageBtnB->setStyleSheet("QToolButton{background-color:transparent;border: none solid ;"
                                        "border-style: solid;border-radius:6px;width: 40px;"
                                        "height:20px; padding:0 0px;margin:0 0px;}");
    }

    RIS_Message msg(SM_ARM_TABLE);
    DB_Thread::instance()->onEvent(msg);
    updateHomepageInfo();
}

void ProductionSafetyControlSys::Updata_Work_Alarm_Table()
{
    //更新告警表
    //QString Text=ui->listWidget->item(Current_Row)->text();
    QString Text=Area_Name;
    QString Area_Id=ReadDataFromMysql::instance()->Get_Area_ID(Text);
    Current_Area_Id=Area_Id;
    Alarm_Table_Data=ReadDataFromMysql::instance()->Updata_tableWidget_Alarm(Area_Id);
    //Updata_Alarm_tableWidget(Alarm_Table_Data);
    //更新工作表
    QList<Work_Table> Work_Table;
    Work_Table=ReadDataFromMysql::instance()->Updata_tableWidget_Work(Text);
    //Updata_Work_tableWidget(Work_Table);
    //Area_Pic(Area_Id);
}

/************************************************************************/
/* 初始化标题栏的按钮菜单栏                                                                     */
/************************************************************************/
void ProductionSafetyControlSys::InitPopuMenu()
{
	safeMenu = new QMenu(this);
	TwoTicketExecAction = safeMenu->addAction(QStringLiteral("两票执行监控"));
	PersionQueryAction = safeMenu->addAction(QStringLiteral("重点区域监控"));
	AreaMonitoringAction = safeMenu->addAction(QStringLiteral("区域人员查询"));


	AlarmAnalysisMenu = new QMenu(this);
	AlarmSelectAction = AlarmAnalysisMenu->addAction(QStringLiteral("告警查询"));
	AlarmAnalysisAction = AlarmAnalysisMenu->addAction(QStringLiteral("安全趋势分析"));
	ReportQueryAction = AlarmAnalysisMenu->addAction(QStringLiteral("报表查询"));


	connect(TwoTicketExecAction, &QAction::triggered, this, &ProductionSafetyControlSys::TwoTicketMoniorClicked);
	connect(PersionQueryAction, SIGNAL(triggered(bool)), this, SLOT(PersionQueryActionClicked()));
	connect(AreaMonitoringAction, SIGNAL(triggered(bool)), this, SLOT(AreaMonitoringActionClicked()));

	connect(AlarmSelectAction, SIGNAL(triggered(bool)), this, SLOT(AlarmSelectActionClicked()));
    connect(AlarmAnalysisAction, SIGNAL(triggered(bool)), this, SLOT(AlarmAnalysisActionClicked()));
}

void ProductionSafetyControlSys::reciverImportAreaList(WorkThreader* work)
{
    if(work->importAreaList.size() != 0)
    {
        this->datalist = work->importAreaList;
        int personNum = 0;
        int AlarmNum = 0;
        int workNum = 0;
        for(int i = 0; i < datalist.size(); ++i)
        {
            Import_Area  import_Area= datalist.at(i);
            personNum += import_Area.PersonNum.toInt();
            AlarmNum += import_Area.AlarmNum.toInt();
            workNum += import_Area.WorkNum.toInt();

        }
        ui->personNumLabel->setText(QString::number(personNum));  //作业人数
        ui->personNumLabel->setAlignment(Qt::AlignCenter);

        ui->CurAlarmNumLabel->setText(QString::number(workNum));  //作业数
        ui->CurAlarmNumLabel->setAlignment(Qt::AlignCenter);

        //ui->alarm_label->setText(QString::number(AlarmNum));  //本日告警总数
        ui->alarm_label->setAlignment(Qt::AlignHCenter);
        ui->key_areaLabel->setText("0");  //重点区域人数
        ui->workNumLabel->setText(QString::number(datalist.size()));  //作业区域数
    }
}


void ProductionSafetyControlSys::Video_Button_clicked()
{//点了摄像头
    //获取摄像头名称  //kfss add 6

    QString Name=ReadDataFromMysql::instance()->Get_Cameralname(Current_Area_Id);
    qDebug()<<"name="<<Name;
    Hk_view->Appoint_Camera(Name);
    Hk_view->Timer->start(1000);
    Hk_view->timerUpdate();


     ui->stackedWidget->addWidget(Hk_view);

     ui->stackedWidget->setCurrentIndex(9);
//     ui->stackedWidget->setCurrentWidget(ui->page_4);
 //    Hk_view->show();
}



//首页表格单元格点击事件实现
void ProductionSafetyControlSys::tableWidgetClick(QTableWidgetItem *item)
{
    QString Alarm_id=ReadDataFromMysql::instance()->UpdateAlarm_FirstPage().at(item->row()).Alarm_ID;
    int row=ui->HomepageTable->currentRow();
    if(item->column() == 4)   //如果点击的是表格的第四列
    {
        if(row !=-1)            //如果所点击的行不为空，则删除当前行同时删除数据库里面当前显示的内容
        {
             ReadDataFromMysql::instance()->deleteAlarmInfo(Alarm_id);

             WriDataToTable(ReadDataFromMysql::instance()->UpdateAlarm_FirstPage());

              ui->HomepageTable->removeRow(row);
        }
      //  QString Alarm_id=ReadDataFromMysql::instance()->HomePageAlarmInfo().at(item->row() * 5 + item->column());

    }

}

//得绘首面表格的实现
void ProductionSafetyControlSys::repaintTable()
{
    QStringList header;
	header << QStringLiteral("人员") << QStringLiteral("作业区域") << QStringLiteral("告警事件") << QStringLiteral("告警时间") << QStringLiteral("");
    //设置表头字体加粗
    QFont font=ui->HomepageTable->horizontalHeader()->font();
    font.setBold(true);
    ui->HomepageTable->horizontalHeader()->setFont(font);
    ui->HomepageTable->setHorizontalHeaderLabels(header);
    ui->HomepageTable->setRowCount(7);     //设置行数
    ui->HomepageTable->setColumnCount(5);  //设置列数
    //设置无边框
    ui->HomepageTable->setFrameShape(QFrame::NoFrame);
    ui->HomepageTable->setObjectName(tr("Hometablewidget"));
    ui->HomepageTable->verticalHeader()->setVisible(false);       //设置垂直头不可见
    ui->HomepageTable->horizontalHeader()->setFixedHeight(35);
    ui->HomepageTable->setShowGrid(false);           //设置无表格
    ui->HomepageTable->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置表格不可编辑
    ui->HomepageTable->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->HomepageTable->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置表格点击为选中一行
    ui->HomepageTable->setFocusPolicy(Qt::NoFocus);  //设置表格点击虚边框为元
    ui->HomepageTable->setColumnWidth(0,50);
    ui->HomepageTable->setColumnWidth(1,130);
    ui->HomepageTable->setColumnWidth(2,180);
    ui->HomepageTable->setColumnWidth(3,90);
    ui->HomepageTable->setColumnWidth(4,60);
    ui->HomepageTable->setAlternatingRowColors(true);  //设置tablewidget隔行交替显示颜色
    ui->HomepageTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  //隐藏水平滚动条
    ui->HomepageTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);   //隐藏垂直滚动条
    ui->HomepageTable->setFrameShape(QFrame::NoFrame);  //设置无边框
}


void ProductionSafetyControlSys::AlarmAnalysisActionClicked()
{

    ReadDataFromMysql::instance()->Updata_AreaLayoutChart(Arealist);

    if(SafeTrendWidget !=NULL)
    {
        delete SafeTrendWidget;
        SafeTrendWidget=new SafeTrendAnalysis(ui->page_6);
    }
    ui->stackedWidget->setCurrentWidget(ui->page_6);
    SafeTrendWidget->setParent(ui->page_6);
    SafeTrendWidget->show();
    qDebug()<<"区域告警分析曲线图";

    //    qDebug()<<"area="<<chats.Alarm_Num<<"="<<chats.Alarm_Type;
    qDebug()<<Arealist.count();
    SafeTrendWidget->InitAniLayoutChart(ReadDataFromMysql::instance()->Updata_AniLayoutChart()); //安全趋势分析图
    SafeTrendWidget->InitAreaLayoutChart(Arealist); //区域堆叠图

}


//启动两票执行监控界面
void ProductionSafetyControlSys::TwoTicketMoniorClicked()
{   
//    QList<TwoTicketWorkstation> list;
    if(!TwoTicketMoni)
    {
        TwoTicketMoni = new TwoTicketExeMonitor(ui->page_5);
    }
    //TwoTicketMoni->InitTwoTicketWidget();
    TwoTicketMoni->InitTwoTicketData();
    ui->stackedWidget->setCurrentWidget(ui->page_5);
    TwoTicketMoni->setGeometry(this->geometry());

//    ReadDataFromMysql::instance()->QueryWorkStatisticsTable(list);
//    qDebug()<<"list="<<list.size();
//    TwoTicketMoni->InsertDataToTableWidget(list);


}

void ProductionSafetyControlSys::on_homepageBtnB_clicked()
{

    QString Name=ReadDataFromMysql::instance()->Get_Cameralname(Current_Area_Id);
 //   qDebug()<<"name="<<Name;
    Hk_view->Appoint_Camera(Name);
    Hk_view->Timer->start(1000);
    Hk_view->timerUpdate();


     ui->stackedWidget->addWidget(Hk_view);

     ui->stackedWidget->setCurrentIndex(9);
}

void ProductionSafetyControlSys::AlarmSelectActionClicked()
{
    Hislist = ReadDataFromMysql::instance()->SelectTableToAlarmTableWidget();
    Alarmwidget->SetAlarmHis(Hislist);
    Alarmwidget->InitWidget();

    ui->stackedWidget->setCurrentWidget(ui->page_9);
    Alarmwidget->setGeometry(ui->page_9->geometry());
    Alarmwidget->show();
//    Alarmrecord->InsertTableWidget(ReadDataFromMysql::instance()->SelectTableToAlarmTableWidget());
}

//重点区域人员查询按钮实现
void ProductionSafetyControlSys::PersionQueryActionClicked()
{
    if(NULL !=Importarea)
    {
        delete Importarea;
        Importarea=new ImportantArea(ui->page_7);
    }

    ui->stackedWidget->setCurrentWidget(ui->page_7);
    Importarea->show();
    Importarea->UpdataTableInfo(datalist);
}

//区域人员查询按钮事件实现
void ProductionSafetyControlSys::AreaMonitoringActionClicked()
{
    QList<Area_person> area_list;

    if(!Areaemploy)
    {
        Areaemploy=new AreaEmployee(ui->page_8);
    }
    else
    {
        Areaemploy->InitWidget();
    }
    ui->stackedWidget->setCurrentWidget(ui->page_8);
    bool value=ReadDataFromMysql::instance()->Updata_tableWidget_AreaEmployee(area_list);

    if(value)
    {
        Areaemploy->UpdateTableInfo(area_list);//往tablewidget插入数据
    }

}

//void ProductionSafetyControlSys::on_deleteAlarmInfo(QString str)
//{
//    ReadDataFromMysql::instance()->deleteAlarmInfo(str);
//    emit postDeleteInfo();
//}

//void ProductionSafetyControlSys::ReceiveDeleteInfo()
//{
//    WriDataToTable(ReadDataFromMysql::instance()->HomePageAlarmInfo());
//    //CameraTableDisp(ReadDataFromMysql::instance()->CameraAlarmInfo());
//}


void ProductionSafetyControlSys::tool_tip(bool status, int index)
{
    status=status;
    index=index;
    QObject *obj = sender();
    QBarSet *button_tmp = qobject_cast<QBarSet *>(obj);
    qDebug()<<"obj="<<button_tmp;
    if(button_tmp)
    {
        chart->setToolTip(button_tmp->label());
    }
}

void ProductionSafetyControlSys::ViewShow(QString)
{
    ui->stackedWidget->setCurrentIndex(5);
    Target_Name=Area_Name;
    QPushButton* Button_Temp;
    for(int i=0;i<Button_List.count();i++)
    {
        Button_Temp=Button_List.at(i);
        if(Button_Temp->text()==Target_Name)
        {
            Button_Temp->click();
            break;
        }
    }
}

void ProductionSafetyControlSys::AcceptSignalGoingInterval(QString name)
{
    qDebug()<<"走错间隔"<<Hislist.size()<<name;
    if(name == "0")
    {
        return;
    }
    //定义一个结构体存放链表中的数据
    Alarm_His alarm_his;
    //定义一个结果构体链表存放数据
    QList<Alarm_His>  list;
    //遍历链表获取数据
    for(int i=0;i<Hislist.count();i++)
    {
        alarm_his=Hislist.at(i);
//        qDebug()<<"alarm="<<alarm_his.Alarm_type;
        if(alarm_his.Alarm_type.compare(QStringLiteral("走错间隔"))==0)
        {
            qDebug()<<"alarm1="<<alarm_his.Alarm_type;
            qDebug()<<"走错间隔";
            list.append(alarm_his);
        }
    }
    Alarmrecord->InsertTableWidget(list);
	Alarmrecord->SetLabelName(QStringLiteral("走错间隔"));
    //Alarmrecord->setWindowFlags(Qt::FramelessWindowHint |Qt::WindowStaysOnTopHint);  //窗口永久置顶
    Alarmrecord->move(15,100);     //移动界面位置
    Alarmrecord->show();
}



//平面区域图
void ProductionSafetyControlSys::on_pushButton_clicked()
{
    RegionWidget->close();
}


//设备确认错误按钮响应事件
void ProductionSafetyControlSys::Signal_DevWrongClicked(QString str)
{
    qDebug()<<"设备确认错误"<<Hislist.size();
    if(str == "0")
    {
        return;
    }
    //定义一个结构体存放链表中的数据
    Alarm_His alarm_his;
    //定义一个结果构体链表存放数据
    QList<Alarm_His>  list;
    //遍历链表获取数据
    for(int i=0;i<Hislist.count();i++)
    {
        alarm_his=Hislist.at(i);
        qDebug()<<"alarm="<<alarm_his.Alarm_type;
        if(alarm_his.Alarm_type.compare(QStringLiteral("设备确认错误"))==0)
        {
            qDebug()<<"alarm1="<<alarm_his.Alarm_type;
            qDebug()<<"设备确认错误";
            list.append(alarm_his);
        }
    }

    Alarmrecord->InsertTableWidget(list);
    Alarmrecord->SetLabelName(QStringLiteral("设备确认错误"));
    Alarmrecord->move(15,100);     //移动界面位置
    Alarmrecord->show();
}

//作业顺序错误按钮响应事件
void ProductionSafetyControlSys::Signal_LeaveOutClicked(QString str)
{
    qDebug()<<"作业顺序错误"<<Hislist.size();
    if(str == "0")
    {
        return;
    }
    //定义一个结构体存放链表中的数据
    Alarm_His alarm_his;
    //定义一个结果构体链表存放数据
    QList<Alarm_His>  list;
    //遍历链表获取数据
    for(int i=0;i<Hislist.count();i++)
    {
        alarm_his=Hislist.at(i);
//        qDebug()<<"alarm="<<alarm_his.Alarm_type;
        if(alarm_his.Alarm_type.compare(QStringLiteral("作业顺序错误"))==0)
        {
//            qDebug()<<"alarm1="<<alarm_his.Alarm_type;
//            qDebug()<<"作业顺序错误";
            list.append(alarm_his);
        }
    }

    Alarmrecord->InsertTableWidget(list);
    Alarmrecord->SetLabelName(QStringLiteral("作业顺序错误"));
    Alarmrecord->move(15,100);     //移动界面位置
    Alarmrecord->show();
}

/************************************************************************
  Description：人员闯禁按钮响应事件实现
  Input：
  Output: 输入按钮的名字
  Return:
  Others:
************************************************************************/
void ProductionSafetyControlSys::Signal_PersonForbidClicked(QString str)
{
    qDebug()<<"人员闯禁="<<Hislist.size();
    //定义一个结构体存放链表中的数据
    Alarm_His alarm_his;
    //定义一个结果构体链表存放数据
    QList<Alarm_His>  list;
    //遍历链表获取数据
    for(int i=0;i<Hislist.count();i++)
    {
        alarm_his=Hislist.at(i);
//        qDebug()<<"alarm="<<alarm_his.Alarm_type;
        if (alarm_his.Alarm_type.compare(QStringLiteral("人员闯禁")) == 0)
        {
//            qDebug()<<"alarm1="<<alarm_his.Alarm_type;
//            qDebug()<<"人员闯禁";
            list.append(alarm_his);
        }
    }
    Alarmrecord->InsertTableWidget(list);
    Alarmrecord->SetLabelName(QStringLiteral("人员闯禁"));
    Alarmrecord->move(15,100);     //移动界面位置
    Alarmrecord->setWindowFlags(Qt::FramelessWindowHint |Qt::WindowStaysOnTopHint);
    Alarmrecord->show();
}

void ProductionSafetyControlSys::Signal_PersonHelpClicked(QString str)
{
    qDebug()<<"作业空间人员求助"<<Hislist.size();
    if(str == "0")
    {
        return;
    }
    //定义一个结构体存放链表中的数据
    Alarm_His alarm_his;
    //定义一个结果构体链表存放数据
    QList<Alarm_His>  list;
    //遍历链表获取数据
    for(int i=0;i<Hislist.count();i++)
    {
        alarm_his=Hislist.at(i);
    //    qDebug()<<"alarm="<<alarm_his.Alarm_type;
        if(alarm_his.Alarm_type.compare(QStringLiteral("人员求助"))==0)
        {
//            qDebug()<<"alarm1="<<alarm_his.Alarm_type;
//            qDebug()<<"作业空间人员求助";
            list.append(alarm_his);
        }
    }

    Alarmrecord->InsertTableWidget(list);
    Alarmrecord->SetLabelName(QStringLiteral("人员求助"));
    Alarmrecord->move(15,100);     //移动界面位置
    Alarmrecord->show();
}

//人员代办
void ProductionSafetyControlSys::Signal_CommissionClicked(QString str)
{
    qDebug()<<"人员代办"<<Hislist.size();
    if(str == "0")
    {
        return;
    }
    //定义一个结构体存放链表中的数据
    Alarm_His alarm_his;
    //定义一个结果构体链表存放数据
    QList<Alarm_His>  list;
    //遍历链表获取数据
    for(int i=0;i<Hislist.count();i++)
    {
        alarm_his=Hislist.at(i);
 //       qDebug()<<"alarm="<<alarm_his.Alarm_type;
        if(alarm_his.Alarm_type.compare(QStringLiteral("人员代办"))==0)
        {
//            qDebug()<<"alarm1="<<alarm_his.Alarm_type;
//            qDebug()<<"人员代办";
            list.append(alarm_his);
        }
    }

    Alarmrecord->InsertTableWidget(list);
	Alarmrecord->SetLabelName(QStringLiteral("人员代办"));
    Alarmrecord->move(15,100);     //移动界面位置
    Alarmrecord->show();
}

//智能门禁非法错误
void ProductionSafetyControlSys::Signal_DoorwrongonClicked(QString str)
{
//    for(int i=0;i<str.size();i++)
//    {
//        QString strdata=str.at(i);
//        if(strdata.compare("0")== 0)
//        {
//            return;
//        }
//    }
    if(str == "0")
    {
        return;
    }
    //定义一个结构体存放链表中的数据
    Alarm_His alarm_his;
    //定义一个结果构体链表存放数据
    QList<Alarm_His>  list;
    //遍历链表获取数据
    for(int i=0;i<Hislist.count();i++)
    {
        alarm_his=Hislist.at(i);
//        qDebug()<<"alarm="<<alarm_his.Alarm_type;
        if (alarm_his.Alarm_type.compare(QStringLiteral("非法门禁")) == 0)
        {
//            qDebug()<<"alarm1="<<alarm_his.Alarm_type;
//            qDebug()<<"警告：人员代办";
            list.append(alarm_his);
        }
    }

    Alarmrecord->InsertTableWidget(list);
	Alarmrecord->SetLabelName(QStringLiteral("非法门禁"));
    Alarmrecord->move(10,150);     //移动界面位置
    Alarmrecord->show();
}

void ProductionSafetyControlSys::Signal_ticketMtepMonitorClicked(QString str)
{
    ui->stackedWidget->setCurrentWidget(ui->page_2);
   // ticketMtepMonitor->setGeometry(ui->stackedWidget->geometry());
    ticketMtepMonitor->updateStepTableData(ReadDataFromMysql::instance()->TwoTicketExecMonitor(str));
    ticketMtepMonitor->show();
}

 //ticketMtepMonitor界面操作表格单击事件响应
void ProductionSafetyControlSys::Signal_OperationWidgetClicked(QString str)
{

}

void ProductionSafetyControlSys::Signal_NoCardclicked(QString str)
{
    qDebug()<<"无卡作业"<<Hislist.size();
    if(str == "0")
    {
        return;
    }
    //定义一个结构体存放链表中的数据
    Alarm_His alarm_his;
    //定义一个结果构体链表存放数据
    QList<Alarm_His>  list;
    //遍历链表获取数据
    for(int i=0;i<Hislist.count();i++)
    {
        alarm_his=Hislist.at(i);
        qDebug()<<"alarm="<<alarm_his.Alarm_type;
        if(alarm_his.Alarm_type.compare(QStringLiteral("无卡作业"))==0)
        {
            qDebug()<<"alarm1="<<alarm_his.Alarm_type;
            qDebug()<<"无卡作业";
            list.append(alarm_his);
        }
    }

    Alarmrecord->InsertTableWidget(list);
    Alarmrecord->SetLabelName(QStringLiteral("无卡作业"));
    Alarmrecord->move(15,100);     //移动界面位置
    Alarmrecord->show();
}

void ProductionSafetyControlSys::Signal_NoTicketClicked(QString str)
{
    qDebug()<<"无票作业"<<Hislist.size();
    if(str == "0")
    {
        return;
    }
    //定义一个结构体存放链表中的数据
    Alarm_His alarm_his;
    //定义一个结果构体链表存放数据
    QList<Alarm_His>  list;
    //遍历链表获取数据
    for(int i=0;i<Hislist.count();i++)
    {
        alarm_his=Hislist.at(i);
        qDebug()<<"alarm="<<alarm_his.Alarm_type;
        if(alarm_his.Alarm_type.compare(QStringLiteral("无票作业"))==0)
        {
            qDebug()<<"alarm1="<<alarm_his.Alarm_type;
            qDebug()<<"无票作业";
            list.append(alarm_his);
        }
    }

    Alarmrecord->InsertTableWidget(list);
    Alarmrecord->SetLabelName(QStringLiteral("无票作业"));
    Alarmrecord->move(15,100);     //移动界面位置
    Alarmrecord->show();
}



//首页tablewidget的单击事件响应
void ProductionSafetyControlSys::on_HomepageTable_cellClicked(int row, int column)
{
//    qDebug()<<"1234";
    Video_Info Video_info;
//    qDebug()<<row<<column;
    if(ui->HomepageTable->item(row,column)==NULL)  //如果单元格为空则返回操作
    {
        return;
    }

    //获取所点击的单元格的内容
    QString name= ui->HomepageTable->item(row,column)->text();
    if(lUserID !=-1)
    {
        NET_DVR_StopRealPlay(lUserID);
        lUserID=-1;
    }

    //如果所点击的单元格第一列的话，那就启动摄像头
    if(column == 1)
    {

        Video_info=ReadDataFromMysql::instance()->Get_Video_InfoOne(name);
   //     qDebug()<<"name="<<Video_info.user_name<<Video_info.video_port<<name;

        NET_DVR_Init();
        NET_DVR_SetConnectTime(2000, 1);  //设置连接时间
        NET_DVR_SetReconnect(10000, true);

        hWnd=(HWND)(ui->hk_VideoLabel->winId());
        //获取窗口句柄
        NET_DVR_DEVICEINFO_V30 struDeviceInfo;

        //设备注册
        WORD Port=Video_info.video_port.toInt();
        QByteArray Temp = Video_info.video_ip.toLatin1();
        char *IP_Host=Temp.data();
        QByteArray Temp2=Video_info.user_name.toLatin1();
        char *User_Name=Temp2.data();
        QByteArray Temp3=Video_info.pwd.toLatin1();
        char *Pwd=Temp3.data();
        qDebug()<<"pwd="<<*Pwd;
        lUserID = NET_DVR_Login_V30(IP_Host, Port,User_Name,Pwd, &struDeviceInfo);
        if (lUserID < 0)
        {
            NET_DVR_Cleanup();
            return ;
        }
        else
        {
            //qDebug()<<"连接成功";
        }
        NET_DVR_PREVIEWINFO lpPreviewInfo={0};
        lpPreviewInfo.lChannel=Video_info.Channel_No.toInt();//  3;   //数据库查

        lpPreviewInfo.hPlayWnd = NULL;         //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
        lpPreviewInfo.lChannel     = lpPreviewInfo.lChannel;       //预览通道
        lpPreviewInfo.dwStreamType = 0;       //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
        lpPreviewInfo.dwLinkMode   = 0;       //0- TCP方式

        //启动预览
        Video_ID = NET_DVR_RealPlay_V40( lUserID ,&lpPreviewInfo,
                                         &fRealDataCallBack_V30,
                                         NULL);//&fRealDataCallBack_V30

        if(-1==Video_ID)
        {
            //qDebug()<<"连接失败";
            //QMessageBox::critical(NULL, QObject::tr("提示"), QObject::tr("启动预览失败"));
            return ;
        }
        else
        {
            //qDebug()<<"连接成功";
        }
        //           return Video_ID;
    }
}


//两票步骤界面返回按钮
void ProductionSafetyControlSys::TwoTicketStepPageReturn()
{
    ui->stackedWidget->setCurrentWidget(ui->page_5);
}



//数据区域图
void ProductionSafetyControlSys::on_dataViewBtn_clicked()
{
    Import_Area area;
    QStringList list= ReadDataFromMysql::instance()->Updata_Area_List();
 //   qDebug()<<"123list="<<list.count();
    QString alarm_name=NULL;
    QString work_name=NULL;
    QString person_name=NULL;

    RegionWidget->setGeometry(ui->bottomWiget->geometry());  //设置数据区域显示的大小和首面widget的大小一样
    //获取到区域的个数、根据区域个数创建数据区域表格
    for(int i=0;i<list.count();++i)
    {
        QString AreaName=list.at(i);
        //通过得到区域名进行对比，如果区域名相同就往相同的区域名里面显示数据
        for(int j=0;j<datalist.count();++j)
        {
            area=datalist.at(i);
            if(AreaName.compare(area.Area_Name)==0)
            {
              //  qDebug()<<"areaname"<<area.Area_Name;
                alarm_name=area.AlarmNum;
                work_name=area.WorkNum;
                person_name=area.PersonNum;
            }
        }

        QString rowNum=QString::number(i/4);   //行数
        QString colNum=QString::number(i%4);    //列数
        /************************************************************************
     Description：初始化数据区域的表格显示
     Input： 区域名、告警数、作业数、人员数、行数、列数
     Output:
     Return:
     Others:
   ************************************************************************/
     RegionWidget->InitWidget(AreaName,alarm_name,work_name,person_name,rowNum,colNum);
    }

    RegionWidget->move(0,0);
    RegionWidget->show();
}

/************************************************************************/
/* 安全监控按钮事件响应                                                                     */
/************************************************************************/
void ProductionSafetyControlSys::on_SafetyMonitoringpushButton_clicked()
{
	
	safeMenu->popup(QPoint(ui->SafetyMonitoringpushButton->x(), ui->SafetyMonitoringpushButton->y() + ui->SafetyMonitoringpushButton->height()));
}

/************************************************************************/
/* 告警查询按钮事件响应                                                                     */
/************************************************************************/
void ProductionSafetyControlSys::on_AlarmAnalyzepushButton_clicked()
{	
    AlarmAnalysisMenu->popup(QPoint(ui->AlarmAnalyzepushButton->x(), ui->AlarmAnalyzepushButton->y() + ui->AlarmAnalyzepushButton->height()));   
}

/************************************************************************/
/* 全厂总揽事件响应                                                                     */
/************************************************************************/
void ProductionSafetyControlSys::on_HomePagepushButton_clicked()
{
	ui->stackedWidget->setCurrentIndex(0);
}

/************************************************************************
  Description：首页作业表数据显示
  @param：
  Output:
  @Return:
  @author:
************************************************************************/
void ProductionSafetyControlSys::on_tabWidget_tabBarClicked(int index)
{

    //获得数据库接口的数据并存储到链表里面
    QList<opertion_station> opertionlist;
    opertion_station opertion;
    ReadDataFromMysql::instance()->homepage_operationticket(opertionlist);
    //如果点击的是tabwidget索引为1就往作业表里面插入数据
    if(0==index)
    {
        for(int i=0;i<opertionlist.size();++i)
        {
            opertion=opertionlist.at(i);
            QTableWidgetItem *workitem=new QTableWidgetItem(opertion.work_name);
            workitem->setTextAlignment(Qt::AlignCenter);

            QTableWidgetItem* timeitem=new QTableWidgetItem(opertion.work_time);
            timeitem->setTextAlignment(Qt::AlignCenter);

            ui->jobtableWidget->setItem(i,0,workitem);
            ui->jobtableWidget->setItem(i,1,timeitem);

            //表格中显示对齐方式为居中
//            ui->jobtableWidget->item(i,0)->setTextAlignment(Qt::AlignCenter);
//            ui->jobtableWidget->item(i,1)->setTextAlignment(Qt::AlignCenter);
        }
    }
    if(1==index)
    {
      //  qDebug()<<"2222";
    }
}

/************************************************************************
  Description：首页时间更新定时器响应事件
  @param：
  Output:
  @Return:
  @author:
************************************************************************/
void ProductionSafetyControlSys::HomePageTimeUpdate()
{
    QDateTime time = QDateTime::currentDateTime();
    QString timestr = time.toString("yyyy/MM/dd hh:mm:ss");
    QString weektime = time.toString("dddd");

    ui->label_10->setText(timestr);
    ui->label_11->setText(weektime);
}
