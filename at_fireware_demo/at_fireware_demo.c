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
	int hn = Num2Char(h);
	int ln = Num2Char(l);
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
		data[0] = '0';
		data[1] = Char2Num(len);
	}else{
		data[0] = Char2Num(len / 10 % 10);
		data[1] = Char2Num(len / 1 % 10);
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
	hexstr[0] = hn2hc( hexnum >> 4 );
	hexstr[1] = hn2hc( hexnum & 0x0F );
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
unsigned char tATWPPID[]="AT+WPPID=\"<PID>\"\r";
unsigned char tATWPPK[]="AT+WPPK=\"<PKEY>\"\r";
unsigned char tATWCSAS[]="AT+WCS=\"AIRKISS\"\r";
unsigned char tATWCSAP[]="AT+WCS=\"AP\"\r";
unsigned char tATWCC[]="AT+WCC\r";
unsigned char tATWDS[]="AT+WDS=00,\"000200020000000000000000\"\r";
unsigned char tATWSCLOUD[]="AT+WSCLOUD\r";
unsigned char tATWSWIFI[]="AT+WSWIFI\r";

//串口接收数据指令帧
unsigned char rATREADY[]="\r\nready\r\n";
unsigned char rATOK[]="\r\nOK\r\n";
unsigned char rATERROR[]="\r\nERROR\r\n";
unsigned char rATWIFICONN[]="\r\n+WSWIFI=CONNECTED\r\n";
unsigned char rATWIFIDISCONN[]="\r\n+WSWIFI=DISCONNECTED\r\n";
unsigned char rATCLOUDCONN[]="\r\n+WSCLOUD=CONNECTED\r\n";
unsigned char rATCLOUDDISCONN[]="\r\n+WSCLOUD=DISCONNECTED\r\n";
unsigned char rATWDR[]="\r\n+WDR=";

//串口读取的数据buffer
#define RXLEN 50
unsigned char rxData[RXLEN]="\r\n+WDR=05,\"0001000200\"\r\n"	;

// TLV的V数据
unsigned char tlvValue[8];//暂定长度，有需要自己调整

int RevData(){
	unsigned short baseline =   (rxData[8] == ',' ? 0 : 1)  ;
	unsigned short tag = (HexStr2HexNum(rxData[10+baseline] , rxData[11+baseline])<<8)|
											(HexStr2HexNum(rxData[12+baseline] , rxData[13+baseline])&0xff);
	unsigned short len = (HexStr2HexNum(rxData[14+baseline] , rxData[15+baseline])<<8)|
											(HexStr2HexNum(rxData[16+baseline] , rxData[17+baseline])&0xff);
	int i=0;
	for(;i<len;i++){
		tlvValue[i] = HexStr2HexNum(rxData[i*2+18+baseline] , rxData[i*2+19+baseline]);
	}
	i = RevDataProcess( tag, len, tlvValue );
	return i;
}

//设置发送云端的TLV数据
int SetSendData(unsigned short tag,unsigned short length,unsigned char* value){
	unsigned short atlen = 4 + length;
	unsigned char atData[2];//复用数据
	int loop=0;
	ConventLen(atlen,atData);
	tATWDS[7] = atData[0];
	tATWDS[8] = atData[1];

	ConventLen(tag >> 8 ,atData);
	tATWDS[11] = atData[0];
	tATWDS[12] = atData[1];

	ConventLen(tag & 0xff ,atData);
	tATWDS[13] = atData[0];
	tATWDS[14] = atData[1];

	ConventLen(length >> 8 ,atData);
	tATWDS[15] = atData[0];
	tATWDS[16] = atData[1];

	ConventLen(length & 0xff ,atData);
	tATWDS[17] = atData[0];
	tATWDS[18] = atData[1];

	for(;loop<length;loop++){
		HexNum2HexStr(value[loop],atData);
		tATWDS[19+loop*2]=atData[0];
		tATWDS[20+loop*2]=atData[1];
	}
	return 1;
}

void ModuleCompleteDataProcess(){
	if(modulestate == CompleteS){
		if(MatchCommand( rATWDR,rxData,7)){
			RevData();
		}
		return;
	}
	if(MatchCommand( rATWIFICONN,rxData,21)){
		//Wi-Fi连接完成
		return;
	}
	if(MatchCommand( rATWIFIDISCONN,rxData,24)){
		//Wi-Fi连接失败
		return;
	}
	if(MatchCommand( rATCLOUDCONN,rxData,22)){
		//云端连接完成
		return;
	}
	if(MatchCommand( rATCLOUDDISCONN,rxData,25)){
		//云端连接失败
		return;
	}
}
void ModuleProcess(){
	//if(modulestate == PowerOnS ){
	//考虑模块有可能自动OTA 或者自动复位
	if(MatchCommand( rATREADY,rxData,9)){
		rxSendData(tATWPPID,44);
		modulestate=PidS;
		return;
	}
	//}
	if(modulestate == PidS){
		if(MatchCommand( rATOK,rxData,6)){
			rxSendData(tATWPPK,46);
			modulestate=PkS;
		}else{
			rxSendData(tATWPPID,44);
		}
		return;
	}
	if(modulestate == PkS){
		if(MatchCommand( rATOK,rxData,6)){
			modulestate=CompleteS;
		}else{
			rxSendData(tATWPPK,46);
		}
		return;
	}

	ModuleCompleteDataProcess();

}

//客户实现该方法，串口发送数据给模块
void rxSendData(unsigned char* value,unsigned char len){
	//客户实现该方法，串口发送数据给模块
	printf("%s\n",value);
}

//客户实现该方法，接收模块的数据
int RevDataProcess(unsigned short tag,unsigned short length,unsigned char* value){
	//todo
	printf("%d %d %x\n", tag, length , value[0]);
	if(tag == 0x01){
		SetSendData(3,5,"4FFFF");
		rxSendData(tATWDS, 38 ); //发送TLV上报数据
	}
	if(tag == 0x02){

	}
	return 0;
}

int main(){
	modulestate = CompleteS;
	printf("%s\n",rxData);

	ModuleProcess();
	return 0;
}
