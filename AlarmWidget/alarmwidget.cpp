﻿#include "alarmwidget.h"
#include "ui_alarmwidget.h"

#define GOINGINTERVAL   "走错间隔"

AlarmWidget::AlarmWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AlarmWidget)
{
    ui->setupUi(this);
    //告警记录查询界面
    dev=0;
    going_wrong=0;
    work_wrong=0;
    Person=0;
    person_help=0;
    fatigued_operations=0;
    commission=0;
    outofjob=0;
    numberofpeople=0;
    NoWrok=0;
    doorwrong=0;
    personcheckout=0;
    scanwronglabel=0;
    InitWidget();
}

AlarmWidget::~AlarmWidget()
{
    delete ui;
}

void AlarmWidget::InitWidget()
{
    Alarm_His alarmhis;
    dev=0;
    going_wrong=0;
    work_wrong=0;
    Person=0;
    person_help=0;
    fatigued_operations=0;
    commission=0;
    outofjob=0;
    numberofpeople=0;
    NoWrok=0;
    doorwrong=0;
    personcheckout=0;
    scanwronglabel=0;

    for(int i=0;i<list.count();i++)
    {
        alarmhis=list.at(i);
        //统计各种区域告警的个数
        if (alarmhis.Alarm_type.compare(QStringLiteral("走错间隔")) == 0)
        {
            going_wrong++;
        }
        if (alarmhis.Alarm_type.compare(QStringLiteral("作业顺序错误")) == 0)
        {
            ++work_wrong;
        }
        if (alarmhis.Alarm_type.compare(QStringLiteral("人员闯入禁止区域")) == 0)
        {
            ++Person;
        }
        if (alarmhis.Alarm_type.compare(QStringLiteral("作业空间人员求助")) == 0)
        {
            ++person_help;
        }
        if (alarmhis.Alarm_type.compare(QStringLiteral("疲劳作业")) == 0)
        {
            ++fatigued_operations;
        }

        if (alarmhis.Alarm_type.compare(QStringLiteral("人员代办")) == 0)
        {
            commission++;
        }

        if (alarmhis.Alarm_type.compare(QStringLiteral("作业期间脱离工作岗位")) == 0)
        {
            outofjob++;
        }
        if (alarmhis.Alarm_type.compare(QStringLiteral("身份校验错误")) == 0)
        {
            numberofpeople++;
        }
        if (alarmhis.Alarm_type.compare(QStringLiteral("无票作业")) == 0)
        {
            NoWrok++;
        }
        if (alarmhis.Alarm_type.compare(QStringLiteral("智能门禁告警")) == 0)
        {
            doorwrong++;
        }
        if (alarmhis.Alarm_type.compare(QStringLiteral("作业负责人未签到")) == 0)
        {
            personcheckout++;
        }
        if(alarmhis.Alarm_type.compare(QStringLiteral("无卡作业"))==0)
        {
            scanwronglabel++;
        }
    }

    qDebug()<<QString::fromLocal8Bit("门禁告警 =")<<doorwrong;
	ui->GoingIntervalLabel->setText(QStringLiteral("走错间隔(%1)").arg(going_wrong));
    ui->GoingIntervalLabel->setAlignment(Qt::AlignCenter);    //居中显示
	ui->LeaveOutLabel->setText(QStringLiteral("作业顺序错误(%1)").arg(work_wrong));
    ui->LeaveOutLabel->setAlignment(Qt::AlignCenter);
	ui->PersonForbidLabel->setText(QStringLiteral("人员闯禁(%1)").arg(Person));
    ui->PersonForbidLabel->setAlignment(Qt::AlignCenter);
	ui->PersonHelpQLabel->setText(QStringLiteral("人员求助(%1)").arg(person_help));
    ui->PersonHelpQLabel->setAlignment(Qt::AlignCenter);
	ui->FatiguedOperationsLabel->setText(QStringLiteral("疲劳作业(%1)").arg(fatigued_operations));
    ui->FatiguedOperationsLabel->setAlignment(Qt::AlignCenter);
	ui->personcommissionLabel->setText(QStringLiteral("人员代办(%1)").arg(commission));
    ui->personcommissionLabel->setAlignment(Qt::AlignCenter);
	ui->outofjobLabel->setText(QStringLiteral("作业期间脱离岗位(%1)").arg(outofjob));
    ui->outofjobLabel->setAlignment(Qt::AlignCenter);
	ui->numberofpeopleLabel->setText(QStringLiteral("身份校验错误(%1)").arg(numberofpeople));
    ui->numberofpeopleLabel->setAlignment(Qt::AlignCenter);
    ui->NoworkLabel->setText(QStringLiteral("无票作业(%1)").arg(NoWrok));
    ui->NoworkLabel->setAlignment(Qt::AlignCenter);
	ui->doorwrongLabel->setText(QStringLiteral("非法门禁(%1)").arg(doorwrong));
    ui->doorwrongLabel->setAlignment(Qt::AlignCenter);
    ui->personcheckoutLabel->setText(QStringLiteral("作业负责人未签到(%1)").arg(personcheckout));
    ui->personcheckoutLabel->setAlignment(Qt::AlignCenter);
    ui->scanwronglabel->setText(QStringLiteral("无卡作业(%1)").arg(scanwronglabel));
    ui->scanwronglabel->setAlignment(Qt::AlignCenter);
}

void AlarmWidget::SetAlarmHis(QList<Alarm_His> alarm_list)
{
    this->list = alarm_list;
}

//走错间隔按钮单击事件
void AlarmWidget::on_GoingIntervalBtn_clicked()
{
//    AlarmRecord->show();
    //给首页发送一个信号启动告警查询记录界面
    QString name=QString::number(going_wrong);
    emit SignalGoingIntervalBtn_clicked(name);
}



//作业顺序错误
void AlarmWidget::on_LeaveOutBtn_clicked()
{
    QString name=QString::number(work_wrong);
    emit LeaveOutClicked(name);
}

//人员闯禁
void AlarmWidget::on_PersonForbidBtn_clicked()
{
    QString name=QString::number(Person);
    emit PersonForbidClicked(name);
}

//人员求助
void AlarmWidget::on_PersonHelpBtn_clicked()
{
    QString name=QString::number(person_help);
    emit PersonHelpClicked(name);
}

//人员代办
void AlarmWidget::on_personcommissionBtn_clicked()
{
    QString name=QString::number(commission);
    emit CommissionClicked(name);
}

//脱离工作岗位
void AlarmWidget::on_outofjobBtn_clicked()
{


}

//作业负责人未签到
void AlarmWidget::on_personcheckoutBtn_clicked()
{

}

//无票作业
void AlarmWidget::on_NoWrokBtn_clicked()
{
    QString name = QString::number(NoWrok);
    emit NoTicketClicked(name);
}

//非法门禁
void AlarmWidget::on_doorwrongBtn_clicked()
{
    QString name=QString::number(doorwrong);
    emit DoorwrongClicked(name);
}

//身份校验错误
void AlarmWidget::on_numberofpeopleBtn_clicked()
{

}

//无卡作业
void AlarmWidget::on_scanworngBtn_clicked()
{
    QString name = QString::number(scanwronglabel);
    emit NoCardClicked(name);
}
