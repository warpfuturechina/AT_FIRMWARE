/**
@author kj021320
@version 3.4
**/
#include <stdio.h>
#include "sdkapi.h"

void ModuleCompleteDataProcess(){
	if(modulestate == CompleteS){
		if(MatchCommand( rATWDR,rxDataFramePayload,5)){
			RevData();
		}
		return;
	}
	/*
	if(MatchCommand( rATWIFICONN,rxDataFramePayload,9)){
		//Wi-Fi连接完成，客户可填充自己需要的代码，例如指示灯
		return;
	}
	if(MatchCommand( rATWIFIDISCONN,rxDataFramePayload,9)){
		//Wi-Fi连接失败，客户可填充自己需要的代码，例如指示灯
		return;
	}
	if(MatchCommand( rATCLOUDCONN,rxDataFramePayload,10)){
		//云端连接完成，客户可填充自己需要的代码，例如指示灯
		return;
	}
	if(MatchCommand( rATCLOUDDISCONN,rxDataFramePayload,10)){
		//云端连接失败，客户可填充自己需要的代码，例如指示灯
		return;
	}
	if(MatchCommand( rATWCSTIMEOUT,rxDataFramePayload,3)){
		//配网失败，客户可填充自己需要的代码，例如指示灯
		return;
	}*/
	/*
	if(MatchCommand( rATWFTTIMEOUT,rxDataFramePayload,12)){
		//匹配到产测失败
		return;
	}
	if(MatchCommand( rATWFTPASS,rxDataFramePayload,9)){
		//匹配到产测成功
		return;
	}
	*/
}
void ModuleProcess(){
	//if(modulestate == PowerOnS ){
	//考虑模块有可能自动OTA 或者自动复位，匹配到ready指令
	if(MatchCommand( rATREADY,rxDataFramePayload,1)){
		rxSendData(tATWPPID,40);
		modulestate=PidS;
		return;
	}
	//}
	//模块当下状态为发送完成PID
	if(modulestate == PidS){
		//等待反馈OK指令，如果匹配成功OK指令，则发送PK，否则重发PID
		if(MatchCommand( rATOK,rxDataFramePayload,1)){
			rxSendData(tATWPPK,43);
			modulestate=PkS;
		}else{
			rxSendData(tATWPPID,40);
		}
		return;
	}
	//模块当下状态为发送完成PK
	if(modulestate == PkS){
		//等待反馈OK指令，如果匹配成功OK指令，模块设置完成，否则重发PK
		if(MatchCommand( rATOK,rxDataFramePayload,1)){
			modulestate=CompleteS;
		}else{
			rxSendData(tATWPPK,43);
		}
		return;
	}
	ModuleCompleteDataProcess();
}

//int serindex = 0;
//unsigned char testsdata[] = "aaa\r\n\r\r\nOK\r\naa";
unsigned char readSerialByte(){
	//客户实现该方法，判断串口中断读取一个字节数
	//return testsdata[serindex++];
	return 0x00;
}
//客户实现该方法，串口发送数据给模块
void serialSend(unsigned char data){
	printf("%c",data);
	//客户实现该方法，串口发送数据给模块
}
void rxSendData(const unsigned char* value,unsigned char len){
	//printf("%s%s%s\n",tAThead,value,tATtail);
	//tAThead
	serialSend('A');
	serialSend('T');
	serialSend('+');
	for(unsigned char i=0;i<len;i++){
		serialSend(value[i]);
	}
	//tATtail
	serialSend('\r');
}
//客户实现该方法，接收模块的数据
int RevDataProcess(unsigned short tag,unsigned short length,unsigned char* value){
	//todo
	printf("TLV:%d %d %x\n", tag, length , value[0]);
	unsigned char v[] = {0x05,0x04,0x03,0x02,0x01};
	if(tag == 0x01){
		AtSendTLV(0xff,5,v); //需要根据发送TLV上报数据
		printf("\n");
	}
	if(tag == 0x02){
	}
	return 0;
}

int main(){
	//readSerialByte 客户实现该方法读取串口数据
	//rxSendData	客户实现该方法往串口写数据
	//RevDataProcess 客户实现该方法 处理TLV 数据
	//ModuleCompleteDataProcess 客户可补全里面 Wi-Fi模块和网络状态解析处理
	//RXLEN 客户可修改 该宏，调整串口帧数据buffer size
	//tlvValue 客户可修改，此值的数据buffer size
	/*
	int frameSize = 0;
	int interruptSignal = 1;
	if(interruptSignal){
		//接收到串口中断，调用CommandFrameProcess，处理帧数据，解析帧头帧尾，完成解析之后会把数据放到rxData中
		int frameSize = CommandFrameProcess();
		if(frameSize > 0){
			printf("%s",rxData);
		}
	}
	//获取完成之后，调用解析帧内容，代码会自动处理TLV解析和AT指令帧解析

	modulestate = CompleteS;
	ModuleProcess();
	*/

	return 0;
}
