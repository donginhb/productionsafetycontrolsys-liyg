#include <QMessageBox>
#include <QString>
#include "video_control.h"

long lPort=-1;
HWND hWnd=NULL;
extern QLabel *Taget_label;
Video_Control::Video_Control()
{
  Video_ID=-1;
//  NET_DVR_Init();
//  NET_DVR_SetConnectTime(2000, 1);
//  NET_DVR_SetReconnect(10000, true);

}

int Video_Control::Start_Video(Video_Info Video_info)
{

  NET_DVR_Init();
  NET_DVR_SetConnectTime(2000, 1);  //设置连接时间
  NET_DVR_SetReconnect(10000, true);
  hWnd=(HWND)(Taget_label->winId());    //获取窗口句柄
  NET_DVR_DEVICEINFO_V30 struDeviceInfo;
  //设备注册
  WORD Port=Video_info.video_port.toInt();
  QByteArray Temp = Video_info.video_ip.toLatin1();
  char *IP_Host=Temp.data();
  QByteArray Temp2=Video_info.user_name.toLatin1();
  char *User_Name=Temp2.data();
  QByteArray Temp3=Video_info.pwd.toLatin1();
  char *Pwd=Temp3.data();
 // qDebug()<<"pwd="<<*Pwd;
  lUserID = NET_DVR_Login_V30(IP_Host, Port,User_Name,Pwd, &struDeviceInfo);
  if (lUserID < 0)
  {
      NET_DVR_Cleanup();
      return -1;
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
    return -1;
  }
  else
  {
    //qDebug()<<"连接成功";
  }
  return Video_ID;
}


void Video_Control::Stop_Video(long VideoId)
{
  NET_DVR_StopRealPlay(VideoId);
}

void Video_Control::Video_Logout(long VideoId)
{//注销用户
    NET_DVR_Logout(VideoId);
}

void Video_Control::Video_Clean()
{
  NET_DVR_Cleanup();
}

void __stdcall fRealDataCallBack_V30(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void* pUser)
{
    lPlayHandle=lPlayHandle;
    pUser=pUser;
    switch (dwDataType)
    {
      case NET_DVR_SYSHEAD:      //系统头
      if (!PlayM4_GetPort(&lPort))  //获取播放库未使用的通道号
      {
         break;
      }
      if (dwBufSize > 0)
      {
          if (!PlayM4_SetStreamOpenMode(lPort, STREAME_REALTIME))  //设置实时流播放模式
          {
                  break;
          }

          if (!PlayM4_OpenStream(lPort, pBuffer, dwBufSize, 1024*1024)) //打开流接口
          {
                  break;
          }

          if (!PlayM4_Play(lPort, hWnd)) //播放开始
          {
                  break;
          }
      }
      break;
      case NET_DVR_STREAMDATA:   //码流数据
      {
          if (dwBufSize > 0 && lPort != -1)
          {
            if (!PlayM4_InputData(lPort, pBuffer, dwBufSize))
            {
              break;
            }
          }
      }
      break;
    }
}
