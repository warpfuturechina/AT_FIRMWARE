/**
@author kj021320
@version 3.2
**/
#include <stdio.h>
int RevDataProcess(unsigned short tag,unsigned short length,unsigned char* value);
void rxSendData(unsigned char* value,unsigned char len);
int Num2Char(unsigned char somechar){
	 return (somechar >= '0' && somechar <= '9') ? (somechar - '0') : -1;
}
unsigned char Char2Num(int n){
	return ( n<0 || n>9 )? 0 : n+'0';
}
//只支持2个字节，把字符串数字转换int数值
int GetLen(unsigned char h,unsigned char l){
	int hn = Num2Char(h); //高位
	int ln = Num2Char(l); //低位
	if(hn == -1 && ln == -1 ){
		return -1;
	}else if(hn == -1){
		return ln;
	}else if(ln == -1){
		return hn;
	}else{
		return hn*10 + ln;
	}
}
//只支持2个字节，把int数值转换为字符串数字
void ConventLen(int len,unsigned char *data){
	if(len<10){
		data[0] = '0'; //低位
		data[1] = Char2Num(len); //高位
	}else{
		data[0] = Char2Num(len / 10 % 10); //低位
		data[1] = Char2Num(len / 1 % 10); //高位
	}
	return ;
}
// hexchar to hexnum
unsigned char hc2hn(unsigned char hc){
	if(hc<='9' ){
		return hc-'0';
	}else if(hc <='Z'){
		return hc+10-'A';
	}else{
		return hc+10-'a';
	}
}
// hexnum to hexchar
unsigned char hn2hc(unsigned char hn){
	if(hn<10 ){
		return hn+'0';
	}else{
		return hn-10+'A';
	}
}
//hexstring 转换为 hexnum
unsigned char HexStr2HexNum(unsigned char left,unsigned char right){
	return ((((hc2hn(left) & 0x0F) << 4) | ( hc2hn(right) & 0x0F)) & 0xFF  );
}
//hexnum 转换为 hexstring
void HexNum2HexStr(unsigned char hexnum,unsigned char* hexstr){
	hexstr[0] = hn2hc( hexnum >> 4 ); //低位
	hexstr[1] = hn2hc( hexnum & 0x0F ); //高位
	return ;
}
//匹配帧 数据
int MatchCommand(unsigned char* sc,unsigned char* tc,int len){
	for(int l=0; l<len ; l++ ){
		if(sc[l] != tc[l]){
			return 0;
		}
	}
	return 1;
}

enum ModuleStateEnum{
	PowerOnS,//上电状态
	PidS,//pid 状态
	PkS, //pk 状态
	WcS,//配网 状态
	CompleteS //完成
};
enum ModuleStateEnum modulestate=PowerOnS;
//串口发送数据指令帧
//#define ATWDS 12
unsigned char tAThead []="AT+";
unsigned char tATtail []="\r";
unsigned char tATWPPID[]="WPPID=\"00000000000000000000000000000000\"";// 注：需要从对接交付文档/平台控制台中获取并”替换掉“PID
unsigned char tATWPPK []="WPPK=\"00000000-0000-0000-0000-000000000000\"";// 注：需要从对接交付文档/平台控制台中获取并”替换掉“PKEY
unsigned char tATWCSAS[]="WCS=\"AIRKISS\"";
unsigned char tATWCSAP[]="WCS=\"AP\"";
unsigned char tATWCSPA[]=",600,true";
unsigned char tATWCC  []="WCC";
unsigned char tATWDS  []="WDS=16,\"00020002000000000000000000000000\""; // 需要根据00FF决定最长数据的字节长度
unsigned char tATWSCLOUD[]="WSCLOUD";
unsigned char tATWSWIFI[]="WSWIFI";
unsigned char tATWFT  []="WFT=\"WFT\",\"12345678\""; //产测指令

//串口接收数据指令帧
unsigned char rATREADY[]=			"r";//ready
unsigned char rATOK[]=				"O";//OK
//unsigned char rATERROR[]=			"E";//ERROR
unsigned char rATWIFICONN[]=	"+WSWIFI=C";//+WSWIFI=CONNECTED
unsigned char rATWIFIDISCONN[]="+WSWIFI=D";//+WSWIFI=DISCONNECTED
unsigned char rATCLOUDCONN[]=	"+WSCLOUD=C";//+WSCLOUD=CONNECTED
unsigned char rATCLOUDDISCONN[]="+WSCLOUD=D";//+WSCLOUD=DISCONNECTED
unsigned char rATWCSTIMEOUT[]="+WCS=T"; //配网TIMEOUT +WCS=TIMEOUT
unsigned char rATWFTTIMEOUT[]="+WFT=T"; //匹配到产测超时 +WFT=TIMEOUT
unsigned char rATWFTPASS[]=		"+WFT=P"; //匹配到产测完成 +WFT=PASS
unsigned char rATWDR[]=				"+WDR="; // 云端数据

//串口读取的数据buffer
#define RXLEN 50
unsigned char rxData[RXLEN]="\r\n+WDR=05,\"0001000200\"\r\n";
unsigned char* rxDataFramePayload = rxData+2;
// TLV的V数据
unsigned char tlvValue[8];//暂定长度，有需要自己调整

int RevData(){
	//计算  \r\n+WDR=05,到底是05还是5，通过判断 rxData[8] ',' 判断
	unsigned short baseline =   (rxData[8] == ',' ? 0 : 1)  ;
	//计算 TLV 里面的tag 4个uchar，实际为2个字节给用户使用
	unsigned short tag = (HexStr2HexNum(rxData[10+baseline] , rxData[11+baseline])<<8)|
											(HexStr2HexNum(rxData[12+baseline] , rxData[13+baseline])&0xff);
	//计算 TLV 里面的len  4个uchar，实际为2个字节给用户使用
	unsigned short len = (HexStr2HexNum(rxData[14+baseline] , rxData[15+baseline])<<8)|
											(HexStr2HexNum(rxData[16+baseline] , rxData[17+baseline])&0xff);
  //计算 TLV 里面的value，value长度是TLV 的L决定的
	int i=0;
	for(;i<len;i++){
		tlvValue[i] = HexStr2HexNum(rxData[i*2+18+baseline] , rxData[i*2+19+baseline]);
	}
	//调用 RevDataProcess提供用户使用
	i = RevDataProcess( tag, len, tlvValue );
	return i;
}
//设置发送云端的TLV数据
int SetSendData(unsigned short tag,unsigned short length,unsigned char* value){
	// atlen 是计算 AT+WDS=<len> 指令的len长度，TLV的TL为4个字节，另外加value的长度
	unsigned short atlen = 4 + length;
	unsigned char atData[2];//复用数据
	int loop=0;
	//ConventLen(atlen,atData);
	//tATWDS[7] = atData[0];
	//tATWDS[8] = atData[1];
	//计算TAG数值
	HexNum2HexStr(tag >> 8 ,atData);
	tATWDS[11] = atData[0];
	tATWDS[12] = atData[1];
	HexNum2HexStr(tag & 0xff ,atData);
	tATWDS[13] = atData[0];
	tATWDS[14] = atData[1];
	//计算Length数值
	HexNum2HexStr(length >> 8 ,atData);
	tATWDS[15] = atData[0];
	tATWDS[16] = atData[1];
	HexNum2HexStr(length & 0xff ,atData);
	tATWDS[17] = atData[0];
	tATWDS[18] = atData[1];
	//计算Value数值
	for(;loop<length;loop++){
		HexNum2HexStr(value[loop],atData);
		tATWDS[19+loop*2]=atData[0];
		tATWDS[20+loop*2]=atData[1];
	}
	return 1;
}
void ModuleCompleteDataProcess(){
	if(modulestate == CompleteS){
		if(MatchCommand( rATWDR,rxDataFramePayload,5)){
			RevData();
		}
		return;
	}
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
	if(MatchCommand( rATWCSTIMEOUT,rxDataFramePayload,6)){
		//配网失败，客户可填充自己需要的代码，例如指示灯
		return;
	}
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
unsigned short serialSkip = 0;
//指令帧状态匹配
enum CFMStateEnum{
	FRAME_OVER,
	FRAME_HEADER_MATCHED,//帧头1match完成
	FRAME_BODY,//帧匹配开始
	FRAME_TAIL1_MATCHED//帧尾部
};
enum CFMStateEnum cfm=FRAME_OVER;
/**
	返回 >0 读取成功，发现帧，返回帧长度
	返回 0 读取失败
**/
int CommandFrameProcess(){
	int frameSize = 0;
	rxData[serialSkip] = readSerialByte();
	//当前帧是结束状态，则开启下一轮帧头匹配
	if(cfm==FRAME_OVER ){
		//匹配到 帧头第一个字节
		if(rxData[serialSkip] == 0x0d){
			cfm = FRAME_HEADER_MATCHED;
		}else{
			serialSkip = 0;
			return 0;
		}
	}else if(cfm == FRAME_HEADER_MATCHED ){
		//已经匹配到帧头第一个字节，需要判断第二个字节，如果成功则进入帧payload阶段
		if(rxData[serialSkip] == 0x0a){
			cfm = FRAME_BODY;
		}else if(rxData[serialSkip] == 0x0d){
			//发现第二个字节仍然是 0x0d，回退处理，需要若错处理 \r\r\n data \r\n 此类帧数据
			rxData[serialSkip-1] = 0x0d;
			return 0;
		}else{
			//第二个字节匹配不到 0x0a，因此状态结束退出从头再匹配
			cfm = FRAME_OVER;
			serialSkip = 0;
			return 0;
		}
	}else if(cfm == FRAME_BODY ){
		// 当前状态是在payload 处理阶段，发现  0x0d 可能是帧尾开始，进入FRAME_TAIL1_MATCHED状态
		if(rxData[serialSkip] == 0x0d){
			cfm = FRAME_TAIL1_MATCHED;
		}
	}else if(cfm == FRAME_TAIL1_MATCHED){
		//判断0x0d 之后是否 0x0a，如果是，则进入帧尾结束
		if(rxData[serialSkip] == 0x0a){
			frameSize = serialSkip+1;
			//发现帧数据小于6做容错处理
			if(frameSize<6){
				serialSkip = 2;
				rxData[0]= 0x0d;
				rxData[1]= 0x0a;
				cfm = FRAME_BODY;
				return 0;
			}
			serialSkip = 0;
			cfm = FRAME_OVER;
			return frameSize;
		}else if(rxData[serialSkip] != 0x0d){
			//发现数据并非 0x0d 则回到 payload处理状态，容错处理 \r\n data \r\r\n
			cfm = FRAME_BODY;
		}
	}
	serialSkip ++ ;
	return 0;
}
//客户实现该方法，串口发送数据给模块
void rxSendData(unsigned char* value,unsigned char len){
	//客户实现该方法，串口发送数据给模块
	printf("%s%s%s\n",tAThead,value,tATtail);
	//printf("%s",value);
}
//客户实现该方法，接收模块的数据
int RevDataProcess(unsigned short tag,unsigned short length,unsigned char* value){
	//todo
	printf("%d %d %x\n", tag, length , value[0]);
	unsigned char v[] = {0x05,0x04,0x03,0x02,0x01};
	if(tag == 0x01){
		SetSendData(0xff01,5,v);
		rxSendData(tATWDS, 41 ); // 需要根据发送TLV上报数据
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

	/**
	int frameSize = 0;
	int interruptSignal = 0;
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
