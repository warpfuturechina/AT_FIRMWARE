// 8000 %82
// 768 %75
int RevDataProcess(unsigned short tag,unsigned short length,unsigned char* value);
void rxSendData(const unsigned char* value,unsigned char len);
unsigned char readSerialByte();
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
	return hn<10 ?  hn+'0' : hn-10+'A';
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
int MatchCommand(const unsigned char* sc,unsigned char* tc,int len){
	int l=0;
	for(; l<len ; l++ ){
		if(sc[l] != tc[l]){
			return 0;//没有匹配命中
		}
	}
	return 1;//匹配命中
}

enum ModuleStateEnum{
	PowerOnS,//上电状态
	PidS,//pid 状态
	PkS, //pk 状态
	WcS,//配网 状态
	CompleteS //完成
};
enum ModuleStateEnum modulestate=PowerOnS;

//const unsigned char tAThead []={'A','T','+'};
//const unsigned char tATtail []={'\r'};
unsigned char tATWDS  []="WDS=16,\"00020002000000000000000000000000\""; // 需要根据00FF决定最长数据的字节长度
const unsigned char tATWPPID[]="WPPID=\"00000000000000000000000000000000\"";// 注：需要从对接交付文档/平台控制台中获取并”替换掉“PID
const unsigned char tATWPPK []="WPPK=\"00000000-0000-0000-0000-000000000000\"";// 注：需要从对接交付文档/平台控制台中获取并”替换掉“PKEY
const unsigned char tATWCSAS[]={'W','C','S','=','"','A','I','R','K','I','S','S','"'};//WCS="AIRKISS"
const unsigned char tATWCSAP[]={'W','C','S','=','"','A','P','"'};//WCS="AP"
const unsigned char tATWCSPA[]={',','6','0','0',',','t','r','u','e'};//,600,true
const unsigned char tATWSCLOUD[]={'W','S','C','L','O','U','D'};
const unsigned char tATWFT  []={'W','F','T','=','"','W','F','T','"',',','"','1','2','3','4','5','6','7','8','"'}; //产测指令 WFT="WFT","12345678"
//const unsigned char tATWSWIFI[]={'W','S','W','I','F','I'};
//const unsigned char tATWCC  []={'W','C','C'}; // wcc

//串口接收数据指令帧
const unsigned char rATREADY[]={'r'};//ready
const unsigned char rATOK[]={'O'};//OK
//unsigned char rATERROR[]=			"E";//ERROR
//unsigned char rATWIFICONN[]=	"+WSWIFI=C";//+WSWIFI=CONNECTED
//unsigned char rATWIFIDISCONN[]="+WSWIFI=D";//+WSWIFI=DISCONNECTED
//unsigned char rATCLOUDCONN[]=	"+WSCLOUD=C";//+WSCLOUD=CONNECTED
//unsigned char rATCLOUDDISCONN[]="+WSCLOUD=D";//+WSCLOUD=DISCONNECTED
const unsigned char rATWCSTIMEOUT[]={'+','W','C'}; //配网TIMEOUT +WCS=TIMEOUT
const unsigned char rATWFTTIMEOUT[]={'+','W','F','T','=','T'}; //匹配到产测超时 +WFT=TIMEOUT
const unsigned char rATWFTPASS[]=		{'+','W','F','T','=','P'}; //匹配到产测完成 +WFT=PASS
const unsigned char rATWDR[]=				{'+','W','D','R','='}; // 云端数据
//串口读取的数据buffer
#define RXLEN 50
unsigned char rxData[RXLEN]="\r\n+WDR=05,\"0001000100\"\r\n";
unsigned char* rxDataFramePayload = rxData+2;
// TLV的V数据
unsigned char tlvValue[6];//暂定长度，有需要自己调整

//设置发送云端的TLV数据
int SetSendData(unsigned short tag,unsigned short length,unsigned char* value){
	// atlen 是计算 AT+WDS=<len> 指令的len长度，TLV的TL为4个字节，另外加value的长度
	//unsigned short atlen = 4 + length;
	//unsigned char atData[2];//复用数据
	int loop=0;
	//ConventLen(atlen,atData);
	//tATWDS[7] = atData[0];
	//tATWDS[8] = atData[1];

	//计算TAG数值
	HexNum2HexStr(tag >> 8 ,tATWDS+8);
	HexNum2HexStr(tag & 0xff ,tATWDS+10);
	//计算Length数值
	HexNum2HexStr(length >> 8 ,tATWDS+12);
	HexNum2HexStr(length & 0xff ,tATWDS+14);

	//计算Value数值
	for(;loop<length;loop++){
		HexNum2HexStr(value[loop], tATWDS +16+loop*2 );
	}
	return 1;
}

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
void AtSendTLV(unsigned short tag,unsigned short length,unsigned char* value){
	SetSendData(tag,length,value);
	rxSendData(tATWDS, 41 ); // 需要根据发送TLV上报数据
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
