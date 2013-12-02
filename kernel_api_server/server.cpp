/*
 * =====================================================================================
 *
 *       Filename:  server.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2011年06月17日 11时00分03秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  GarfieldLinux (zjf), garfieldlinux@gmail.com
 *        Company:  江苏天联信息科技发展有限公司
 *
 * =====================================================================================
 */

#include "netmask.h"
#include "ipmstor.h"
#include "errors.h"
#include <mysql/mysql.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <sstream>
#include "Config.h"
#include <math.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include "IPMS_sql_so/mysqlgenius.h"
//#include <boost/bind.hpp> 
#include <sys/stat.h> 
#include <pthread.h>


using namespace std;
//using namespace boost; 

#define N 8 
int port; 
string ipaddress;  
string username;  
string password;  
string dbname;
string server_adapter;
string adapter_address;
const char ConfigFile[]= "server.conf" ;	


MYSQL  mysql;   
MYSQL_RES  *mysql_ret;   
MYSQL_ROW  mysql_row;   


int initsql(const string&  mes )
{
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
	printf("连接成功! \t"); 
	}   
	else   
	{   
		printf("连接失败\n"); 
        return -1;	
	}    
	return  0 ; 
}



string   cstr_to_string(char *p_str)
{
	//该函数的功能 将char* 类型的转换为string 类的值
	return p_str;
}

unsigned long iptolong(string ip)
{
//该函数的功能是将IP地址转换为无符号的长整数
	return  htonl(inet_addr(ip.c_str()));
}

string longtostring(unsigned long lon)
{
//将无符号的长整型转换为string类型
	stringstream s;
	string lts="";
	s<<lon;
	s>>lts;
	return lts;
}

string longtoip(long iplong)
{
//将IP地址的长整型转换为string类型的表示
	string ipstring="";
	for(int i=0;i<3;i++)
	{
		stringstream s1;
		string ss;
		s1<<((iplong>>(24-i*8))&0x000000ff);
		s1>>ss;
		ss=ss+".";
		ipstring=ipstring+ss;
	}
	stringstream s4;
	s4<<(iplong&0x000000ff);
	string sf;
	s4>>sf;
	ipstring=ipstring+sf;
	return ipstring ;
}

int   dir_exists(char   *filename)
{
//检查文件是否有效
	return (access(filename,0)==0);
}

//生成省市级别的文件夹以及文件
bool viewfile(string country,string provpy,string citypy,string isp,string filebuf)
{
	cout<<"生成数据文件在服务端"<<endl;
	int statExist;
	char DIRNAME[]="IPMS_file";
	statExist=dir_exists( DIRNAME );
	if( statExist )
	{
		//printf("dir %s   exist,begin download ......\n",DIRNAME);
		int statprov;
		statprov=dir_exists(strdup(provpy.c_str()));	
		if(statprov){
		}else{
				int statprov ;
				string lujin="IPMS_file/"+provpy+"/";
				statprov = mkdir(lujin.c_str(),0755);
				if(!statprov)
				{
					//printf("省级文件夹创建成功\n");
				}else{
					//printf("省级文件夹创建失败\n");
				}
		}
	}else
	{
		int statMd;
		statMd = mkdir(DIRNAME,0755);
		if ( !statMd )
		{
			//printf("Directory created,begin download ...... \n");
			int statprov ;
			statprov = mkdir(("IPMS_file/"+provpy+"/").c_str(),0755);
			if(!statprov)
			{
				//printf("省级文件夹创建成功\n");
			}else{
				//printf("省级文件夹创建失败\n");
			}
		}
		else
		{
			printf("Unable to create directory\n");
			return -1;
		}
	}	
	string filena="";
	if(citypy!=""){
		filena="IPMS_file/"+provpy+"/"+provpy+"-"+citypy+"-"+isp ;
	}else{
		filena="IPMS_file/"+provpy+"/"+provpy+"-"+isp;
	}
	const char* fn = filena.c_str();
	ofstream o_file;
	o_file.open(fn,ios_base::app);
	o_file<<filebuf<<endl; //将内容写入到文本文件中
	o_file.close();     

 


}

IPinfoList read_ipdatautf8();
bool xadd(const IPinfo& ipinfo);
bool update_ipdatautf8();

int btt(int *array)
{
//将二进制数字转换为十进制的表示方式?
	int maski[]={256,128,64,32,16,8,4,2,1};
	int last_iterator=sizeof(maski)/4-1;
	int temp=0;
	for(int i=7;i>=0;i--)
		temp+=(1&(array[i]-'0'))*maski[last_iterator--];
	return temp;
}

string  binto(int x)
{
//将十进制的数字表示成二进制的字符串形式
	string s="";
	for(int i=1;i<=N ; i++){
		if((1<<N-i)&x)
		{
			s=s+"1.";
		}
		else
			s=s+"0.";
	}
	return s ;
}

void LoadConfig(int iSize);//加载程序配置文件
IPpart get_network_submask(char* argv);//获得子网掩码的相关信息
string get_network_ippart(string startip,string endip);//合并IP段的相关函数


unsigned long get_network_ipaddr(char *ipaddr)//获取网络IP地址
{
//这个函数的怎么木有返回值？
	int ip[32]={-1} ;
	if(ipaddr==NULL)
		return 0 ;
	int first1,first2,first3,first4 ;
	first1=first2=first3=first4=-1 ;
	sscanf(ipaddr,"%d.%d.%d.%d",&first1,&first2,&first3,&first4);
	printf("处理IP地址之前 \n");
	printf("%d.%d.%d.%d \n",first1,first2,first3,first4);
	printf("将十进制IP地址转换为二进制的IP地址:  ");

	const string s1=binto(first1);
	sscanf(s1.c_str(),"%d.%d.%d.%d.%d.%d.%d.%d.",&ip[0],&ip[1],&ip[2],&ip[3],&ip[4],&ip[5],&ip[6],&ip[7]);
	const string s2=binto(first2);
	sscanf(s2.c_str(),"%d.%d.%d.%d.%d.%d.%d.%d.",&ip[8],&ip[9],&ip[10],&ip[11],&ip[12],&ip[13],&ip[14],&ip[15]);
	const string s3=binto(first3);
	sscanf(s3.c_str(),"%d.%d.%d.%d.%d.%d.%d.%d.",&ip[16],&ip[17],&ip[18],&ip[19],&ip[20],&ip[21],&ip[22],&ip[23]);
	const string s4=binto(first4);
	sscanf(s4.c_str(),"%d.%d.%d.%d.%d.%d.%d.%d.",&ip[24],&ip[25],&ip[26],&ip[27],&ip[28],&ip[29],&ip[30],&ip[31]);

	cout<<"返回字符串:"<<s1<<"-"<<s2<<"-"<<s3<<"-"<<s4<<endl;
	for(int i=0 ;i<32 ;i++)
	{
		printf("%d",ip[i]);
	}
	printf("\n");
}


string ip_mssea(const string& start )
{
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	unsigned  long  num_rows;   
	int  ret; 
	string result="";	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
	printf("连接成功! \t"); 
	string mys="";
//	string sstart=longtostring(iptolong(start));
	mys="select isp_id from Dick   where "+start+" >= start  and "+start+" <= end;";
	cout<<mys<<endl;
	mysql_query(&mysql,"set names utf8 ;");	
	ret  =  mysql_query(&mysql,mys.c_str());  
	if(!ret)   
	{   
		//	printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{   
		//		printf("存储结果成功!\t");   
				num_rows  =  mysql_num_rows(mysql_ret);   
				if(num_rows  !=  0)   
				{   
					printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{
		  				result=mysql_row[0];	
						cout<<"待返回的结果"<<result<<endl; 
					}   
				}   
				else   
				{   
				}   
				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{   
		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	//cout << num_rows << endl;
	mysql_close(&mysql);
	return result ; 
}


class IpmstorI : public Ipmstor {
	public:
		virtual void printString(const string& s);
		virtual string ipmssea(const string& start,const string& end);
		virtual int ipmsdel(const string& start ,const string& end );
		virtual int ipmsadd(const string& startip,const string& endip,const string& location,const string& isp );
		virtual IPpart ipsplit(const string& ipinfo);
		virtual int ipmerge(const string& startip,const string& endip);
		virtual int ipvalid(const string& ipinfo);
		virtual string getResult(const string& startip,const string& endip,int& resuetsource);
		virtual  IPinfoList dnipinfoadd(const  IPinfo& ipinfo   );
		virtual bool x();
		virtual bool updatednipdata(const IPinfo& ipinfo  );
		virtual string updatewhois(const string& ip );
		virtual string feedbackview(const string& prov,const string& city, const string& isp,const string& opflag );
	        virtual string feedbackviewlargearea(const string& largearea , const string& isppy );
		virtual string feedbackviewarea(const string& area );
		virtual string feedbackipinfo(const string& ip );
virtual string feedbackviewsheng(const string& sheng,const string& isppy );
virtual string feedbackqitalianxu(const string& prov,const string& city );
		virtual string ipanalysis(const string& ip  );
		virtual string feedbackareaview(const string& areapy  );
		virtual SSinfoList feedbackss();
		virtual SSIspList feedbackoldss();
		virtual SSinfoList feedbackhaiwai();
		virtual SSinfoList feedbacks();	
		virtual SSinfoList feedbackpushengpy();	
		virtual ISPinfoList feedbackisp();
		virtual PRinfoList feedbacksp();
		virtual AREAinfoList feedbackarea();	
		virtual SSIspList feedbackssisp();
		virtual string  tosmallcity(const string& ippart );
		virtual bool updateprovpy(const string& provname,const string& provpy );
		virtual bool updatecitypy(const string& provname,const string& provpy,const string& cityname,const string& citypy );	
		virtual bool updatehaiwaipy(const string& provname,const string& provpy,const string& cityname,const string& citypy );	
		virtual bool updateisppy(const string& ispname,const string& isppy );
		virtual bool updateviewinfo( );
		virtual bool updateOdie();

};

IPinfo dnipinfodata(string startip,string endip,string location)
{
//靠，这个不会就是组装完就完事了吧……
	IPinfo ii ;
	ii.startip="9.9.9.9";
	ii.endip="9.9.9.255";
	ii.location="广东省汕头市电信";
	return ii ;
}


bool updateipfenxiisppy(const string& ipstart , const string& ipend,  const string& isp_py )
{
cout<<"更新ISPPY操作开始"<<endl;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
   int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		//string mys="update  log_analysis  set isp_id='"+isp_py+"' ,location='"+location+"' ,isp_name='"+isp_name+"'   , opflag='1'  where  ip_num >="+ipstart+"  and ip_num <= "+ipend+" ;";
 string mys="update  bhaiwai  set haiwai='"+isp_py+"'  where  bnum >="+ipstart+"  and bnum <= "+ipend+" ;";

                
cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
//	cout << num_rows << endl;
	mysql_close(&mysql);

}



string findbeforeqitayouxu(const char* ips,string prov,string city)
{
       unsigned  long  num_rows;   
	int  ret; 
	string result="";	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
	//printf("连接成功! \t"); 
	string mys="";
	 string sstart=longtostring(iptolong(ips)-1);
	mys="select isp_id from Odie_OK  where end = '"+sstart+"' and isp_id!='qita' and prov_id='"+prov+"'  and city_id='"+city+"' ;";
	//cout<<mys<<endl;
	mysql_query(&mysql,"set names utf8 ;");	
	ret  =  mysql_query(&mysql,mys.c_str());  
	if(!ret)   
	{   
			//printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{   
			//	printf("存储结果成功!\t");   
				num_rows  =  mysql_num_rows(mysql_ret);   
				if(num_rows  !=  0)   
				{   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{  
						result=mysql_row[0] ;
					}   
				}else{
					mysql_free_result(mysql_ret);
				}	


			}   
			else   
			{
					mysql_free_result(mysql_ret);
   
			}   
		}   
		else   
		{   
		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	//cout << num_rows << endl;
	mysql_close(&mysql);
	if(num_rows==0)
	{
		result="qita";
	}
	return result ; 


}


string findnextqitayouxu(const char* ipe,string prov,string city)
{
	unsigned  long  num_rows;   
	int  ret; 
	string result="";	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
	//printf("连接成功! \t"); 
	string mys="";
	 string sstart=longtostring(iptolong(ipe)+1);
	mys="select isp_id from Odie_OK  where start = '"+sstart+"'   and isp_id !='qita'   and prov_id='"+prov+"'  and city_id='"+city+"'   order by start  ;";
	//cout<<mys<<endl;
	mysql_query(&mysql,"set names utf8 ;");	
	ret  =  mysql_query(&mysql,mys.c_str());  
	if(!ret)   
	{   
			//printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{   
			//	printf("存储结果成功!\t");   
				num_rows  =  mysql_num_rows(mysql_ret);   
				if(num_rows  !=  0)   
				{   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{  
						result=mysql_row[0] ;
						mysql_free_result(mysql_ret);
					}   
				}else{
					mysql_free_result(mysql_ret);
				}	


			}   
			else   
			{
					mysql_free_result(mysql_ret);
   
			}   
		}   
		else   
		{   
		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
//	cout << num_rows << endl;
if(num_rows==0)
{
result="qita";
}

	mysql_close(&mysql);
	return result ; 


}



string findbefore(const char* ips,string prov,string city)
{
       unsigned  long  num_rows;   
	int  ret; 
	string result="";	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
	//printf("连接成功! \t"); 
	string mys="";
	 string sstart=longtostring(iptolong(ips)-1);
	mys="select isp_id from Odie_OK  where end = '"+sstart+"' and isp_id!='qita' and prov_id='"+prov+"' ;";
	//cout<<mys<<endl;
	mysql_query(&mysql,"set names utf8 ;");	
	ret  =  mysql_query(&mysql,mys.c_str());  
	if(!ret)   
	{   
			//printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{   
			//	printf("存储结果成功!\t");   
				num_rows  =  mysql_num_rows(mysql_ret);   
				if(num_rows  !=  0)   
				{   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{  
						result=mysql_row[0] ;
					}   
				}else{
					mysql_free_result(mysql_ret);
				}	


			}   
			else   
			{
					mysql_free_result(mysql_ret);
   
			}   
		}   
		else   
		{   
		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	//cout << num_rows << endl;
	mysql_close(&mysql);
	if(num_rows==0)
	{
		result="qita";
	}
	return result ; 


}


string findnext(const char* ipe,string prov,string city)
{
	unsigned  long  num_rows;   
	int  ret; 
	string result="";	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
	//printf("连接成功! \t"); 
	string mys="";
	 string sstart=longtostring(iptolong(ipe)+1);
	mys="select isp_id from Odie_OK  where start = '"+sstart+"'   and isp_id !='qita'   and prov_id='"+prov+"'  order by start  ;";
	//cout<<mys<<endl;
	mysql_query(&mysql,"set names utf8 ;");	
	ret  =  mysql_query(&mysql,mys.c_str());  
	if(!ret)   
	{   
			//printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{   
			//	printf("存储结果成功!\t");   
				num_rows  =  mysql_num_rows(mysql_ret);   
				if(num_rows  !=  0)   
				{   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{  
						result=mysql_row[0] ;
						mysql_free_result(mysql_ret);
					}   
				}else{
					mysql_free_result(mysql_ret);
				}	


			}   
			else   
			{
					mysql_free_result(mysql_ret);
   
			}   
		}   
		else   
		{   
		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
//	cout << num_rows << endl;
if(num_rows==0)
{
result="qita";
}

	mysql_close(&mysql);
	return result ; 


}



bool upispidqitalianxu(string start,string end,string ispid)
{
cout<<"OKOK"<<endl;		
	string isppy= ispid;
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
	string mys="update Odie_OK  set isp_id='"+ispid+"'  where end <= "+end+" and  start >="+start+" and  isp_id='qita'  ;";
        cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}   
	mysql_close(&mysql);
return true ;
}



bool upispid(string ip,string ispid)
{
	string ipend=ip;
	string isppy= ispid;
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
	string mys="update Odie_OK  set isp_id='"+ispid+"'  where endip= '"+ip+"' and isp_id='qita'  ;";
        cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}   
	mysql_close(&mysql);
return true ;
}

bool IpmstorI::updateOdie()
{
        string s="";
	string mysc ="";
	int opflag=0;

	//mysc="select start,end,location,isp_name,country_id,prov_id,city_id,isp_id  from alern  order by start; ";//测试整个数据库的合并 
mysc="select startip,endip,isp_id,prov_id,city_id  from  Odie_OK where isp_id='qita' and  country_id='china'  and prov_id!='qita'  order by start";
OdieList odielist;	
	MYSQL  mysqlc;   
	MYSQL_RES  *mysql_retc;   
	MYSQL_ROW  mysql_rowc;   
	unsigned  long  num_rowsc;   
	int  retc; 
	string result="";	
	mysql_init(&mysqlc);
	cout<<"开始连接数据库"<<endl;
	if(mysql_real_connect(&mysqlc,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t"); 
		mysql_query(&mysqlc,"set names utf8 ;");	
		retc  =  mysql_query(&mysqlc,mysc.c_str());  
		if(!retc)   
		{   
			printf("查询成功!\t");   
			mysql_retc  =  mysql_store_result(&mysqlc);   
			if(mysql_retc  !=  NULL)   
			{   
				printf("存储结果成功!\t");   
				num_rowsc  =  mysql_num_rows(mysql_retc);   
				cout<<"num: "<<num_rowsc<<endl;
				if(num_rowsc  !=  0)   
				{
					while(mysql_rowc  =  mysql_fetch_row(mysql_retc))
					{
//cout<<"0:"<<mysql_rowc[0]<<"1:"<<mysql_rowc[1]<<"7:"<<mysql_rowc[2]<<endl;
						Odie odie;
						odie.ispname=mysql_rowc[0];
						odie.location=mysql_rowc[1];
						odie.ispid=mysql_rowc[2];
						odie.provid=mysql_rowc[3];
						odie.cityid=mysql_rowc[4];
						odielist.push_back(odie);
					}}}
					cout<<"查出条数:"<<num_rowsc<<endl;
		}
	}
mysql_close(&mysqlc);

		int j=0;
	for(int i=0;i<odielist.size();i++)
	{
		//cout<<"isp确认之前"<<odielist[i].ispid<<endl;
	string	isppys=findbefore(odielist[i].ispname.c_str(),odielist[i].provid,odielist[i].cityid);
	string	isppye=findnext(odielist[i].location.c_str(),odielist[i].provid,odielist[i].cityid);
		if(isppye==isppys&&isppye!="qita"&&isppys!="qita"&&isppye!=""&&isppys!="")
		{
	cout<<"第："<<j<<"次确认==startip:"<<odielist[i].ispname<<" : "<<isppye<<"---"<<isppys<<endl;
			j++;
			upispid(odielist[i].location,isppye);

		}

}

}



string IpmstorI::ipanalysis(const string& ip  )
{
string s = "分析返回结果";
cout<<"Ip分析数据开始执行"<<endl;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t");  
		//string mys="select start,end,isp_id,location,isp_name from  Dick  ;";
		
//string mys="select start,end, prov_id from Dick where prov_id='haiwai'  ";

string mys="select ip_add , ip_num,haiwai from bhaiwai ";
cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
				num_rows  =  mysql_num_rows(mysql_ret);
				//printf("存储结果成功!\t");  
				cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
				printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{  
	cout<<mysql_row[0]<<"     "<<mysql_row[1]<<"      "<<mysql_row[2]<<endl;
						//string isp_id =ip_mssea(mysql_row[1]);
						//cout<<"返回的ISP信息"<<isp_id<<endl;
					//	updateipfenxiisppy(mysql_row[0],mysql_row[1],mysql_row[2]);	
					}



   
				}   
				else   
				{   
				}

				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	//	cout << num_rows << endl;
	mysql_close(&mysql);






return s ;
}

bool IpmstorI::updateisppy(const string& ispname,const string& isppy )
{
cout<<"更新ISPPY操作开始"<<endl;
cout<<ispname<<endl;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
   int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
//	string mys="select locationinfo,isp_id from   dn_ip_data  where location like '%美国%' ;
//string mys="update  dn_ip_data_jingjian  set country_id='Kyrgyzstan'  where location like '吉尔吉斯斯坦' ;";
// string mys="update  dn_ip_data  set country_id='US'  where location like '%美国%' ;";
//		string mys="update  Odie_OK  set isp_id='"+isppy+"'  where isp_name like '%"+ispname+"%' and isp_id = 'qita' ;";
	string mys="update ip13820121115   set isp_id='"+isppy+"'  where location like '%"+ispname+"%' and isp_id = 'qita' ;";

                 cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
//	cout << num_rows << endl;
	mysql_close(&mysql);

}



bool IpmstorI::updateprovpy(const string& provname,const string& provpy )
{
	cout<<"解析省"<<endl;
	string pname=provname;
	string ppy= provpy;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
	//string mys="update Odie_OK  set prov_id='"+ppy+"' , country_id='china'     where location like '"+pname+"%' and country_id='qita'  ;";
	string mys="update ip13820121115  set prov_id='"+ppy+"' , country_id='china'     where location like '"+pname+"%' and country_id='qita'  ;";

        cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
//	cout << num_rows << endl;
	mysql_close(&mysql);

}



bool IpmstorI::updatehaiwaipy(const string& provname,const string& provpy,const string& cityname,const string& citypy )
{
	cout<<"解析城市"<<endl;
	string pname=provname;
	string ppy= provpy;
	string cname= cityname;
	string cpy= citypy;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
//	string mys="update  Odie_OK  set city_id='"+cpy+"' , prov_id='"+ppy+"' ,country_id='"+ppy+"'  where     location like '"+cname+"%'  and country_id='qita'  ;";
string mys="update  ip13820121115  set city_id='"+cpy+"' , prov_id='"+ppy+"' ,country_id='"+ppy+"'  where     location like '"+cname+"%'  and country_id='qita'  ;";

        cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
//	cout << num_rows << endl;
	mysql_close(&mysql);
}



bool IpmstorI::updatecitypy(const string& provname,const string& provpy,const string& cityname,const string& citypy )
{
	cout<<"解析城市"<<endl;
	string pname=provname;
	string ppy= provpy;
	string cname= cityname;
	string cpy= citypy;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
//	string mys="update  Odie_OK  set city_id='"+cpy+"'  where     location like '%"+cname+"%' and prov_id='"+provpy+"'  and city_id ='qita'  ;";
	string mys="update  ip13820121115  set city_id='"+cpy+"'  where     location like '%"+cname+"%' and prov_id='"+provpy+"'  and city_id ='qita'  ;";

        cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		ret  =  mysql_query(&mysql,mys.c_str());  
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	mysql_close(&mysql);
}


string IpmstorI::tosmallcity(const string& ippart  )
{
	//啥也木有做	
	cout<<"大表合并开始"<<endl;
	string input=ippart ;
	return input ;
}


PRinfoList IpmstorI::feedbacksp()
{
	PRinfoList pril;
	cout<<"获取省的相关信息"<<endl;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		//		printf("连接成功! \t");  
		string mys="select prov_name,prov_py  from dn_prov  ;";
		cout<<"sql: "<<mys<<endl;
		//mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			//			printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
				num_rows  =  mysql_num_rows(mysql_ret);
				//printf("存储结果成功!\t");  
				//cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
				//printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
		//printf("%s\t%s\t%s\t%s\t\n",mysql_row[0],mysql_row[1],mysql_row[2],mysql_row[3]);  
						//在这里组装我们的IPinfoList
						PRinfo  pri;
						pri.provname=mysql_row[0];
						pri.provpy=mysql_row[1];
						pril.push_back(pri);

					}   
				}   
				else   
				{   
				}

				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	//	cout << num_rows << endl;
	mysql_close(&mysql);
	return pril;
}



AREAinfoList IpmstorI::feedbackarea()
{
	//cout<<"查找isp "<<endl;
	AREAinfoList alist ;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		//printf("连接成功! \t");  
		string mys="select area_id , area_name  from area  ;";
		cout<<"sql: "<<mys<<endl;
		//mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
		//printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
				num_rows  =  mysql_num_rows(mysql_ret);
				if(num_rows  !=  0)   
				{   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
						AREAinfo  ainfo;
						ainfo.areaname=mysql_row[1];
						ainfo.areapy=mysql_row[0];
						alist.push_back(ainfo);
					}   
				}   
				else   
				{   
				}
				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	mysql_close(&mysql);
	return alist;
}



ISPinfoList IpmstorI::feedbackisp()
{
	//cout<<"查找isp "<<endl;
	ISPinfoList ispil ;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		//printf("连接成功! \t");  
		string mys="select vi_ispname,vi_isppy  from viewinfo_isp  ;";
		cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			//printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
				num_rows  =  mysql_num_rows(mysql_ret);
				//printf("存储结果成功!\t");  
				//cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
					//printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
		//printf("%s\t%s\t%s\t%s\t\n",mysql_row[0],mysql_row[1],mysql_row[2],mysql_row[3]);  
						//在这里组装我们的IPinfoList
						ISPinfo  isp;
						isp.ispname=mysql_row[0];
						isp.isppy=mysql_row[1];
						ispil.push_back(isp);

					}   
				}   
				else   
				{   
				}

				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	//	cout << num_rows << endl;
	mysql_close(&mysql);
	return ispil;
}



SSinfoList IpmstorI::feedbackpushengpy()
{
//cout<<"查找省市 "<<endl;
	SSinfoList ssil ;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		//printf("连接成功! \t");  
        string mys="select distinct(vi_shengpy)  from viewinfo_sheng where vi_areaid>1 and vi_areaid<=50";
//获取全国32个省市的省的py 编码
	cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			//printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
			    num_rows  =  mysql_num_rows(mysql_ret);
			//	printf("存储结果成功!\t");  
			   cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
			//printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
						SSinfo  ssi;
						ssi.provpy=mysql_row[0];
						ssil.push_back(ssi);
					}   
				}   
				else   
				{   
				}

				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
//	cout << num_rows << endl;
	mysql_close(&mysql);
return ssil;
}


SSinfoList IpmstorI::feedbacks()
{
//cout<<"查找省市 "<<endl;
	SSinfoList ssil ;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		//printf("连接成功! \t");  
	//	string mys="select dn_prov.prov_name,dn_prov.prov_py,dn_city.dn_city_name,dn_city.dn_city_py from dn_prov INNER JOIN dn_city ON dn_prov.sort = dn_city.parentid;";
	string mys="select vi_des,vi_shengpy  from viewinfo_sheng ;";
	cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			//printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
			    num_rows  =  mysql_num_rows(mysql_ret);
			//	printf("存储结果成功!\t");  
			   cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
			//printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
		//printf("%s\t%s\t%s\t%s\t\n",mysql_row[0],mysql_row[1],mysql_row[2],mysql_row[3]);  
						//在这里组装我们的IPinfoList
						SSinfo  ssi;
						ssi.provname=mysql_row[0];
						ssi.provpy=mysql_row[1];
						ssil.push_back(ssi);
					}   
				}   
				else   
				{   
				}

				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
//	cout << num_rows << endl;
	mysql_close(&mysql);
return ssil;
}


SSinfoList IpmstorI::feedbackhaiwai()
{
//cout<<"查找省市 "<<endl;
	SSinfoList ssil ;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
        int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		//printf("连接成功! \t");  
	//	string mys="select dn_prov.prov_name,dn_prov.prov_py,dn_city.dn_city_name,dn_city.dn_city_py from dn_prov INNER JOIN dn_city ON dn_prov.sort = dn_city.parentid;";
		string mys="select vi_shiname,vi_shipy from viewinfo_shi  where vi_areaid=45 ";
	cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			//printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
			    num_rows  =  mysql_num_rows(mysql_ret);
			//printf("存储结果成功!\t");  
			   cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
			//printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
		//printf("%s\t%s\t%s\t%s\t\n",mysql_row[0],mysql_row[1],mysql_row[2],mysql_row[3]);  
						//在这里组装我们的IPinfoList
						SSinfo  ssi;
						ssi.provname="海外"+cstr_to_string(mysql_row[0]);
						ssi.provpy="haiwai";
						ssi.cityname=mysql_row[0];
						ssi.citypy=mysql_row[1];
						ssil.push_back(ssi);
					}   
				}   
				else   
				{   
				}
				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
//	cout << num_rows << endl;
	mysql_close(&mysql);
return ssil;
}



string  IpmstorI::feedbackipinfo(const string& ip )
{
//cout<<"查找省市 "<<endl;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
   int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		//printf("连接成功! \t");  
		string mys="select vi_shengpy,vi_shipy,vi_isppy from  viewinfo_view";
        	cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			//printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
			    num_rows  =  mysql_num_rows(mysql_ret);
			//printf("存储结果成功!\t");  
			   cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
			//printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
		//printf("%s\t%s\t%s\t%s\t\n",mysql_row[0],mysql_row[1],mysql_row[2],mysql_row[3]);  
						//在这里组装我们的IPinfoList
					}   
				}   
				mysql_free_result(mysql_ret);   
			}   
		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	mysql_close(&mysql);
return "soOK--:"+ip;
}



SSIspList IpmstorI::feedbackoldss()
{
//cout<<"查找省市 "<<endl;
	SSIspList ssisplist ;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
   int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		//printf("连接成功! \t");  
	//	string mys="select dn_prov.prov_name,dn_prov.prov_py,dn_city.dn_city_name,dn_city.dn_city_py from dn_prov INNER JOIN dn_city ON dn_prov.sort = dn_city.parentid;";
		string mys="select vi_shengpy,vi_shipy,vi_isppy from  viewinfo_view";
	cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			//printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
			    num_rows  =  mysql_num_rows(mysql_ret);
			//printf("存储结果成功!\t");  
			   cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
			//printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
		//printf("%s\t%s\t%s\t%s\t\n",mysql_row[0],mysql_row[1],mysql_row[2],mysql_row[3]);  
						//在这里组装我们的IPinfoList
						SSIsp  ssisp;
						ssisp.shengpy=mysql_row[0];
						ssisp.shipy=mysql_row[1];
						ssisp.isppy=mysql_row[2];
						ssisplist.push_back(ssisp);

					}   
				}   
				else   
				{   
				}

				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
//	cout << num_rows << endl;
	mysql_close(&mysql);
return ssisplist;
}



bool updatewho(string startip,string isppy)
{
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
   int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t");  
string mys="update Odie_OK set isp_id='"+isppy+"' where startip='"+startip+"'  and isp_id ='qita' ;";
        	cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		ret  =  mysql_query(&mysql,mys.c_str());  
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	mysql_close(&mysql);
}


//函数重载
bool updatewho(string ipstart,string ipend,string isppy)  
{
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
cout<<"输入参数 ipstart:"<<ipstart<<"--ipend:"<<ipend<<"--isp:"<<isppy<<endl;	
if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0)) 
	{   
		//printf("连接成功!  ");  
       string mys="update Odie_OK set isp_id='"+isppy+"' where start>="+ipstart+"  and  end<="+ipend+" ;";
        	cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		ret  =  mysql_query(&mysql,mys.c_str());  
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}   
 
	mysql_close(&mysql);
}



string IpmstorI::updatewhois(const string& ip )
{
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;
	string result="qita";	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t");  
	//string mys="select isp_id from Whois_OK where inet_aton('"+ip+"') >=start  and inet_aton('"+ip+"')<=end order by start  ;";
	string mys="select start,end,isp_id from Whois_OK order by start  ;";
	cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
				num_rows  =  mysql_num_rows(mysql_ret);
				printf("存储结果成功!\t");  
				cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
					printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
//cout<<"IPstart:"<<mysql_row[0]<<"---"<<"IPend"<<mysql_row[1]<<"---isp:"<<mysql_row[2]<<endl;
						updatewho(mysql_row[0],mysql_row[1],mysql_row[2]);
						result=mysql_row[0];
					}   
				}   
				mysql_free_result(mysql_ret);   
			}   
		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	//cout << num_rows << endl;
	mysql_close(&mysql);
	return result;
}


SSinfoList IpmstorI::feedbackss()
{
//cout<<"查找省市 "<<endl;
	SSinfoList ssil ;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
        int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		//printf("连接成功! \t");  
	       //string mys="select dn_prov.prov_name,dn_prov.prov_py,dn_city.dn_city_name,dn_city.dn_city_py from dn_prov INNER JOIN dn_city ON dn_prov.sort = dn_city.parentid;";
		string mys="select viewinfo_sheng.vi_des,viewinfo_sheng.vi_shengpy,viewinfo_shi.vi_shiname,viewinfo_shi.vi_shipy from viewinfo_sheng INNER JOIN viewinfo_shi ON  viewinfo_sheng.vi_areaid = viewinfo_shi.vi_areaid;";
	cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			//printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
			    num_rows  =  mysql_num_rows(mysql_ret);
			//printf("存储结果成功!\t");  
			   cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
			//printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
		//printf("%s\t%s\t%s\t%s\t\n",mysql_row[0],mysql_row[1],mysql_row[2],mysql_row[3]);  
						SSinfo  ssi;
						ssi.provname=mysql_row[0];
						ssi.provpy=mysql_row[1];
						ssi.cityname=mysql_row[2];
						ssi.citypy=mysql_row[3];
						ssil.push_back(ssi);
					
					}   
				}   
				else   
				{   
				}

				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
//	cout << num_rows << endl;
	mysql_close(&mysql);
return ssil;
}



string IpmstorI::feedbackareaview(const string& areapy )
{
	string s;
	string mys ="";
	mys="select ippart from dn_ip_data_area  where area='"+areapy+"' order by start ;";
	cout<<mys<<endl;
	s="view  "+areapy+" { match-clients {";
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	unsigned  long  num_rows;   
	int  ret;   
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
	//	printf("连接成功! \t");  
		mysql_query(&mysql,"set names utf8 ;");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			//printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{   
				//printf("存储结果成功!\t");   
				num_rows  =  mysql_num_rows(mysql_ret);   
				if(num_rows  !=  0)   
				{   
				//	printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
						printf("%s\n",mysql_row[0]); 
							s=s+mysql_row[0];
		//	viewfile("china",mysql_row[1],mysql_row[2],mysql_row[3],mysql_row[0]);
					//生成数据文件
					}   
				}   
				else   
				{
					mysql_close(&mysql);
					s="";
					return s ;
				}   
				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	mysql_close(&mysql);
	return s+"};};";
}


SSIspList IpmstorI::feedbackssisp()
{
	SSIspList ssisplist;
	SSIsp ssisp ;
	string s;
	string mys ="";
	mys="select vi_sheng,vi_shi,vi_isp from viewinfo3 ;";
	cout<<mys<<endl;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	unsigned  long  num_rows;   
	int  ret;   
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
	//	printf("连接成功! \t");  
		mysql_query(&mysql,"set names utf8 ;");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			//printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{   
				//printf("存储结果成功!\t");   
				num_rows  =  mysql_num_rows(mysql_ret);   
				if(num_rows  !=  0)   
				{   
					//	printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
						ssisp.shengpy=mysql_row[0];
						ssisp.shipy=mysql_row[1];
						ssisp.isppy=mysql_row[2];
						ssisplist.push_back(ssisp);
					}   
				}   
				else   
				{
					mysql_close(&mysql);

				}   
				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	mysql_close(&mysql);
return ssisplist;
}



string IpmstorI::feedbackqitalianxu(const string& prov,const string& city )
{
	string s="";
	cout<<"开始处理isp_name 为其它的数据"<<endl;
	string mysc="select start,end  from Odie_OK where country_id='china' and prov_id='"+prov+"'  and  city_id='"+city+"'   and  isp_id='qita'  order by start; ";//自定义SQL 语句执行
	IPhebingList iphebinglist;
	MYSQL  mysqlc;   
	MYSQL_RES  *mysql_retc;   
	MYSQL_ROW  mysql_rowc;   
	unsigned  long  num_rowsc;   
	int  retc; 
	string result="";	
	mysql_init(&mysqlc);
	cout<<"开始连接数据库"<<endl;
	if(mysql_real_connect(&mysqlc,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
	printf("连接成功! \t"); 
		mysql_query(&mysqlc,"set names utf8 ;");	
		retc  =  mysql_query(&mysqlc,mysc.c_str());  
		if(!retc)   
		{   
			printf("查询成功!\t");   
			mysql_retc  =  mysql_store_result(&mysqlc);   
			if(mysql_retc  !=  NULL)   
			{   
			printf("存储结果成功!\t");   
				num_rowsc  =  mysql_num_rows(mysql_retc);   
				cout<<"num: "<<num_rowsc<<endl;
				if(num_rowsc  !=  0)   
				{
					while(mysql_rowc  =  mysql_fetch_row(mysql_retc))
					{
						//cout<<mysql_rowc[0]<<"------"<<mysql_rowc[1]<<endl;

						IPhebing iphebing;
						iphebing.start=mysql_rowc[0];
						iphebing.end=mysql_rowc[1];
						iphebinglist.push_back(iphebing);
					}}}
					//cout<<"cunchu"<<endl; 
					//cout<<"查出条数:"<<num_rowsc<<endl;
					if(num_rowsc==0){
						string rs="";
						mysql_close(&mysqlc);
						return rs;
					}else{
						cout<<s;

					}
		}
	}

IPhebingList ipduanin;
IPhebing ipdd;
ipdd=iphebinglist[0];

for(int i=0;i<iphebinglist.size();i++)
{
	//cout<<"start:"<<iphebinglist[i].start<<"----"<<"end:"<<iphebinglist[i].end<<endl;
	if(i==iphebinglist.size()-1)
	{
		cout<<"可以合并:"<<ipdd.start<<"---"<<ipdd.end<<endl;
		ipduanin.push_back(ipdd);
		break;
	}
//以下两个置换的操作是为了去除潜在的段包含重复的数据项
	if(strtoul(iphebinglist[i+1].start.c_str(),NULL,0) <= strtoul(iphebinglist[i].end.c_str(),NULL,0)&&strtoul(iphebinglist[i+1].end.c_str(),NULL,0) >= strtoul(iphebinglist[i].end.c_str(),NULL,0))
	{
		ipdd.end=iphebinglist[i+1].end;
		cout<<"置换1"<<endl;
	}

	if(strtoul(iphebinglist[i+1].start.c_str(),NULL,0) <= strtoul(iphebinglist[i].end.c_str(),NULL,0)&&strtoul(iphebinglist[i+1].end.c_str(),NULL,0) < strtoul(iphebinglist[i].end.c_str(),NULL,0))
	{
		ipdd.end=iphebinglist[i].end;
		cout<<"置换2: "<<iphebinglist[i].start<<"----"<<iphebinglist[i].end<<"---"<<iphebinglist[i+1].end<<endl;
	}
  
//if((strtoul(iphebinglist[i+1].start.c_str(),NULL,0)-strtoul(iphebinglist[i].end.c_str(),NULL,0)==1)&&(iphebinglist[i+1].provid==iphebinglist[i].provid)&&(iphebinglist[i+1].cityid==iphebinglist[i].cityid))
if(strtoul(iphebinglist[i+1].start.c_str(),NULL,0)-strtoul(iphebinglist[i].end.c_str(),NULL,0)==1)
{
		cout<<"可以合并:"<<ipdd.start<<"---"<<ipdd.end<<endl;
		ipdd.end=iphebinglist[i+1].end;
	}else{
	
		cout<<"可以合并:"<<ipdd.start<<"---"<<ipdd.end<<endl;
		cout<<"找到完整段"<<endl;
		ipduanin.push_back(ipdd);
		if(i+1<iphebinglist.size()){
			ipdd=iphebinglist[i+1];
		}
	}

}
cout<<"合并前共计"<<iphebinglist.size()<<endl;
cout<<"合并后共计"<<ipduanin.size()<<endl;
int j=0;
for(int i=0;i<ipduanin.size();i++)
{
cout<<"合并后"<<ipduanin[i].start<<"---"<<ipduanin[i].end<<endl;
	const char * s1=ipduanin[i].start.c_str();
	const char * s2=ipduanin[i].end.c_str();
	string ip1 = longtoip(strtoul(s1,NULL,0));
	string ip2=longtoip(strtoul(s2,NULL,0));


	string  isppys=findbeforeqitayouxu(s1,prov,city);
	string  isppye=findnextqitayouxu(s2,prov,city);
	cout<<" 前一个isp:"<<isppys<<"后一个isp:"<<isppye<<endl;

	if(isppye==isppys&&isppye!="qita"&&isppys!="qita"&&isppye!=""&&isppys!="")
	{
	cout<<"第："<<j<<"次确认==startip:"<<ip1<<"---"<<"endip"<<ip2<<": isp_id :"<<isppys<<endl;
		j++;
	upispidqitalianxu(ipduanin[i].start,ipduanin[i].end,isppye);

	}



}




mysql_close(&mysqlc);
return s+"};};";
}



string IpmstorI::feedbackviewsheng(const string& sheng,const string& isppy  )
{
	

string s="";
	string mysc ="";
	ofstream o_file;
	const char filename[] = "";


	int statExist;
	char DIRNAME[]="/var/www/html/usvn/drupal/conf";
	statExist=dir_exists( DIRNAME );
	if( statExist )
	{
	}else
	{
		int statMd;
		statMd = mkdir(DIRNAME,0755);
		if ( !statMd )
		{
			//printf("Directory created,begin download ...... \n");
			int statprov ;
			statprov = mkdir("/var/www/html/usvn/drupal/conf/",0755);
			if(!statprov)
			{
				//printf("文件夹创建成功\n");
			}else{
				//printf("文件夹创建失败\n");
			}
		}
		else
		{
			printf("Unable to create directory\n");
		}
	}	

char fn[] = "/var/www/html/usvn/drupal/conf/201306014named.conf";
string opflag="viewinfo";
if(opflag=="viewinfo")
{
	ofstream o_file;
	o_file.open(fn,ios_base::app);
	mysc="select start,end  from Odie_OK where prov_id='"+sheng+"' and isp_id='"+isppy+"'   order by start; ";//自定义SQL 语句执行
	s="view "+sheng+"- "+isppy+"    { match-clients {";

}	
	
	IPhebingList iphebinglist;
	MYSQL  mysqlc;   
	MYSQL_RES  *mysql_retc;   
	MYSQL_ROW  mysql_rowc;   
	unsigned  long  num_rowsc;   
	int  retc; 
	string result="";	
	mysql_init(&mysqlc);
	cout<<"开始连接数据库"<<endl;
	if(mysql_real_connect(&mysqlc,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t"); 
		mysql_query(&mysqlc,"set names utf8 ;");	
		retc  =  mysql_query(&mysqlc,mysc.c_str());  
		if(!retc)   
		{   
			printf("查询成功!\t");   
			mysql_retc  =  mysql_store_result(&mysqlc);   
			if(mysql_retc  !=  NULL)   
			{   
				printf("存储结果成功!\t");   
				num_rowsc  =  mysql_num_rows(mysql_retc);   
				cout<<"num: "<<num_rowsc<<endl;
				if(num_rowsc  !=  0)   
				{
					while(mysql_rowc  =  mysql_fetch_row(mysql_retc))
					{
						//cout<<mysql_rowc[0]<<"------"<<mysql_rowc[1]<<endl;

						IPhebing iphebing;
						iphebing.start=mysql_rowc[0];
						iphebing.end=mysql_rowc[1];
						iphebinglist.push_back(iphebing);
					}}}
					//cout<<"cunchu"<<endl; 
					//cout<<"查出条数:"<<num_rowsc<<endl;
					if(num_rowsc==0){
						string rs="";
						mysql_close(&mysqlc);
						return rs;
					}else{
						cout<<s;

					}
		}
	}

IPhebingList ipduanin;
IPhebing ipdd;
ipdd.start=iphebinglist[0].start;
ipdd.end=iphebinglist[0].end;
for(int i=0;i<iphebinglist.size();i++)
{
	//cout<<"start:"<<iphebinglist[i].start<<"----"<<"end:"<<iphebinglist[i].end<<endl;
	if(i==iphebinglist.size()-1)
	{
               ipduanin.push_back(ipdd); 
		break;
	}

	if(strtoul(iphebinglist[i+1].start.c_str(),NULL,0) <= strtoul(iphebinglist[i].end.c_str(),NULL,0)&&strtoul(iphebinglist[i+1].end.c_str(),NULL,0) >= strtoul(iphebinglist[i].end.c_str(),NULL,0))
	{
		ipdd.end=iphebinglist[i+1].end;
		cout<<"置换1"<<endl;
	}

	if(strtoul(iphebinglist[i+1].start.c_str(),NULL,0) <= strtoul(iphebinglist[i].end.c_str(),NULL,0)&&strtoul(iphebinglist[i+1].end.c_str(),NULL,0) < strtoul(iphebinglist[i].end.c_str(),NULL,0))
	{
		ipdd.end=iphebinglist[i].end;
		cout<<"置换2: "<<iphebinglist[i].start<<"----"<<iphebinglist[i].end<<"---"<<iphebinglist[i+1].end<<endl;
	}

if((strtoul(iphebinglist[i+1].start.c_str(),NULL,0)-strtoul(iphebinglist[i].end.c_str(),NULL,0)==1))
	{
	//	cout<<"可以合并"<<endl;
		ipdd.end=iphebinglist[i+1].end;
	}else{
	//	cout<<"找到完整段"<<endl;
		ipduanin.push_back(ipdd);
		
		if(i+1<iphebinglist.size()){
			ipdd.start=iphebinglist[i+1].start;
			ipdd.end=iphebinglist[i+1].end;
		}
	}
}
    //cout<<"合并后共计"<<ipduanin.size()<<endl;
    //const char filename[] = "mytext.txt";
    //将结果写入文本文件
    o_file.open(fn,ios_base::app);
    o_file<<s; //将内容写入到文本文件中
    o_file.close();

for(int i=0;i<ipduanin.size();i++)
{
	const char * s1=ipduanin[i].start.c_str();
	const char * s2=ipduanin[i].end.c_str();
	string ip1 = longtoip(strtoul(s1,NULL,0));
	string ip2=longtoip(strtoul(s2,NULL,0));
//	string ipduan=get_network_ippart(ip1,ip2);// 旧的算法 结果不准确
	string caozuo="netmask "+ip1+":"+ip2+" >>"+cstr_to_string(fn)+" ;";
        system(caozuo.c_str());
  //      s=s+ipduan;	

}


mysql_close(&mysqlc);

    o_file.open(fn,ios_base::app);
o_file<<"};};"<<endl;
  o_file.close();         
 
return s+"};};";

}



string IpmstorI::feedbackviewlargearea(const string& largearea , const string& isppy )
{
	

string s="";
	string mysc ="";
	ofstream o_file;
	const char filename[] = "";


	int statExist;
	char DIRNAME[]="/var/www/html/usvn/drupal/conf";
	statExist=dir_exists( DIRNAME );
	if( statExist )
	{
	}else
	{
		int statMd;
		statMd = mkdir(DIRNAME,0755);
		if ( !statMd )
		{
			//printf("Directory created,begin download ...... \n");
			int statprov ;
			statprov = mkdir("/var/www/html/usvn/drupal/conf/",0755);
			if(!statprov)
			{
				//printf("文件夹创建成功\n");
			}else{
				//printf("文件夹创建失败\n");
			}
		}
		else
		{
			printf("Unable to create directory\n");
		}
	}	

 char fn[] = "/var/www/html/usvn/drupal/conf/view.xinanxibeiliantong";
string opflag="viewinfo";
if(opflag=="viewinfo")
{
	ofstream o_file;
	o_file.open(fn,ios_base::app);
//西南//
//mysc="select start,end  from Odie_OK where ( prov_id='sichuan' or prov_id='yunnan' or prov_id='guizhou' or prov_id='xizang' or prov_id='chongqing' )  and isp_id='liantong'  order by start; ";//自定义SQL 语句执行
// 西北i
//mysc="select start,end  from Odie_OK where ( prov_id='ningxia' or prov_id='xinjiang' or prov_id='qinghai' or prov_id='shanxi' or prov_id='gansu' )  and isp_id='liantong'  order by start; ";//自定义SQL 语句执行

	mysc="select start,end  from Odie_OK where large_area='"+largearea+"' and isp_id='"+isppy+"'  order by start; ";//自定义SQL 语句执行
	s="view "+largearea+"-"+isppy +" { match-clients {";
//s="view  xinan-liantong  { match-clients { " ;
//s="view  xibei-liantong  { match-clients { " ;
}	
	
	IPhebingList iphebinglist;
	MYSQL  mysqlc;   
	MYSQL_RES  *mysql_retc;   
	MYSQL_ROW  mysql_rowc;   
	unsigned  long  num_rowsc;   
	int  retc; 
	string result="";	
	mysql_init(&mysqlc);
	cout<<"开始连接数据库"<<endl;
	if(mysql_real_connect(&mysqlc,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t"); 
		mysql_query(&mysqlc,"set names utf8 ;");	
		retc  =  mysql_query(&mysqlc,mysc.c_str());  
		if(!retc)   
		{   
			printf("查询成功!\t");   
			mysql_retc  =  mysql_store_result(&mysqlc);   
			if(mysql_retc  !=  NULL)   
			{   
				printf("存储结果成功!\t");   
				num_rowsc  =  mysql_num_rows(mysql_retc);   
				cout<<"num: "<<num_rowsc<<endl;
				if(num_rowsc  !=  0)   
				{
					while(mysql_rowc  =  mysql_fetch_row(mysql_retc))
					{
						//cout<<mysql_rowc[0]<<"------"<<mysql_rowc[1]<<endl;

						IPhebing iphebing;
						iphebing.start=mysql_rowc[0];
						iphebing.end=mysql_rowc[1];
						iphebinglist.push_back(iphebing);
					}}}
					//cout<<"cunchu"<<endl; 
					//cout<<"查出条数:"<<num_rowsc<<endl;
					if(num_rowsc==0){
						string rs="";
						mysql_close(&mysqlc);
						return rs;
					}else{
						cout<<s;

					}
		}
	}

IPhebingList ipduanin;
IPhebing ipdd;
ipdd.start=iphebinglist[0].start;
ipdd.end=iphebinglist[0].end;
for(int i=0;i<iphebinglist.size();i++)
{
	//cout<<"start:"<<iphebinglist[i].start<<"----"<<"end:"<<iphebinglist[i].end<<endl;
	if(i==iphebinglist.size()-1)
	{
               ipduanin.push_back(ipdd); 
		break;
	}

	if(strtoul(iphebinglist[i+1].start.c_str(),NULL,0) <= strtoul(iphebinglist[i].end.c_str(),NULL,0)&&strtoul(iphebinglist[i+1].end.c_str(),NULL,0) >= strtoul(iphebinglist[i].end.c_str(),NULL,0))
	{
		ipdd.end=iphebinglist[i+1].end;
		cout<<"置换1"<<endl;
	}

	if(strtoul(iphebinglist[i+1].start.c_str(),NULL,0) <= strtoul(iphebinglist[i].end.c_str(),NULL,0)&&strtoul(iphebinglist[i+1].end.c_str(),NULL,0) < strtoul(iphebinglist[i].end.c_str(),NULL,0))
	{
		ipdd.end=iphebinglist[i].end;
		cout<<"置换2: "<<iphebinglist[i].start<<"----"<<iphebinglist[i].end<<"---"<<iphebinglist[i+1].end<<endl;
	}

if((strtoul(iphebinglist[i+1].start.c_str(),NULL,0)-strtoul(iphebinglist[i].end.c_str(),NULL,0)==1))
	{
	//	cout<<"可以合并"<<endl;
		ipdd.end=iphebinglist[i+1].end;
	}else{
	//	cout<<"找到完整段"<<endl;
		ipduanin.push_back(ipdd);
		
		if(i+1<iphebinglist.size()){
			ipdd.start=iphebinglist[i+1].start;
			ipdd.end=iphebinglist[i+1].end;
		}
	}
}
//cout<<"合并后共计"<<ipduanin.size()<<endl;


          //const char filename[] = "mytext.txt";
  
         //将结果写入文本文件
                           o_file.open(fn,ios_base::app);
          o_file<<s; //将内容写入到文本文件中
         o_file.close();

for(int i=0;i<ipduanin.size();i++)
{
	const char * s1=ipduanin[i].start.c_str();
	const char * s2=ipduanin[i].end.c_str();
	string ip1 = longtoip(strtoul(s1,NULL,0));
	string ip2=longtoip(strtoul(s2,NULL,0));
//	string ipduan=get_network_ippart(ip1,ip2);// 旧的算法 结果不准确
	string caozuo="netmask "+ip1+":"+ip2+" >>"+cstr_to_string(fn)+" ;";
        system(caozuo.c_str());
  //      s=s+ipduan;	

}


mysql_close(&mysqlc);

    o_file.open(fn,ios_base::app);
o_file<<"};};"<<endl;
  o_file.close();         
 
return s+"};};";

}



string IpmstorI::feedbackviewarea(const string& area  )
{
    string s="";
	string mysc ="";
	ofstream o_file;
	const char filename[] = "";
	int statExist;
	char DIRNAME[]="/var/www/html/usvn/drupal/conf";
	statExist=dir_exists( DIRNAME );
	if( statExist )
	{
	}else
	{
		int statMd;
		statMd = mkdir(DIRNAME,0755);
		if ( !statMd )
		{
			//printf("Directory created,begin download ...... \n");
			int statprov ;
			statprov = mkdir("/var/www/html/usvn/drupal/conf/",0755);
			if(!statprov)
			{
				//printf("文件夹创建成功\n");
			}else{
				//printf("文件夹创建失败\n");
			}
		}
		else
		{
			printf("Unable to create directory\n");
		}
	}	

 char fn[] = "/var/www/html/usvn/drupal/conf/named_conf_2013-07-09";
string opflag="viewinfo";
if(opflag=="viewinfo")
{
	ofstream o_file;
	o_file.open(fn,ios_base::app);
mysc="select start,end  from Odie_OK where area='"+area+"'      order by start; ";
	//mysc="select  start,end from  Odie_OK  where isp_name like '%歌华宽带%'  order by start; ";//自定义SQL 语句执行
cout<<"sql:"<<mysc<<endl;
	s="view "+area+" { match-clients {";

}	
	
	IPhebingList iphebinglist;
	MYSQL  mysqlc;   
	MYSQL_RES  *mysql_retc;   
	MYSQL_ROW  mysql_rowc;   
	unsigned  long  num_rowsc;   
	int  retc; 
	string result="";	
	mysql_init(&mysqlc);
	cout<<"开始连接数据库"<<endl;
	if(mysql_real_connect(&mysqlc,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t"); 
		mysql_query(&mysqlc,"set names utf8 ;");	
		retc  =  mysql_query(&mysqlc,mysc.c_str());  
		if(!retc)   
		{   
			printf("查询成功!\t");   
			mysql_retc  =  mysql_store_result(&mysqlc);   
			if(mysql_retc  !=  NULL)   
			{   
				printf("存储结果成功!\t");   
				num_rowsc  =  mysql_num_rows(mysql_retc);   
				cout<<"num: "<<num_rowsc<<endl;
				if(num_rowsc  !=  0)   
				{
					while(mysql_rowc  =  mysql_fetch_row(mysql_retc))
					{
						//cout<<mysql_rowc[0]<<"------"<<mysql_rowc[1]<<endl;

						IPhebing iphebing;
						iphebing.start=mysql_rowc[0];
						iphebing.end=mysql_rowc[1];
						iphebinglist.push_back(iphebing);
					}}}
					//cout<<"cunchu"<<endl; 
					//cout<<"查出条数:"<<num_rowsc<<endl;
					if(num_rowsc==0){
						string rs="";
						mysql_close(&mysqlc);
						return rs;
					}else{
						cout<<s;

					}
		}
	}

IPhebingList ipduanin;
IPhebing ipdd;
ipdd.start=iphebinglist[0].start;
ipdd.end=iphebinglist[0].end;
for(int i=0;i<iphebinglist.size();i++)
{
	//cout<<"start:"<<iphebinglist[i].start<<"----"<<"end:"<<iphebinglist[i].end<<endl;
	if(i==iphebinglist.size()-1)
	{
               ipduanin.push_back(ipdd); 
		break;
	}

	if(strtoul(iphebinglist[i+1].start.c_str(),NULL,0) <= strtoul(iphebinglist[i].end.c_str(),NULL,0)&&strtoul(iphebinglist[i+1].end.c_str(),NULL,0) >= strtoul(iphebinglist[i].end.c_str(),NULL,0))
	{
		ipdd.end=iphebinglist[i+1].end;
		cout<<"置换1"<<endl;
	}

	if(strtoul(iphebinglist[i+1].start.c_str(),NULL,0) <= strtoul(iphebinglist[i].end.c_str(),NULL,0)&&strtoul(iphebinglist[i+1].end.c_str(),NULL,0) < strtoul(iphebinglist[i].end.c_str(),NULL,0))
	{
		ipdd.end=iphebinglist[i].end;
cout<<"置换2: "<<iphebinglist[i].start<<"----"<<iphebinglist[i].end<<"---"<<iphebinglist[i+1].end<<endl;
	}

if((strtoul(iphebinglist[i+1].start.c_str(),NULL,0)-strtoul(iphebinglist[i].end.c_str(),NULL,0)==1))
	{
	//	cout<<"可以合并"<<endl;
		ipdd.end=iphebinglist[i+1].end;
	}else{
	//	cout<<"找到完整段"<<endl;
		ipduanin.push_back(ipdd);
		
		if(i+1<iphebinglist.size()){
			ipdd.start=iphebinglist[i+1].start;
			ipdd.end=iphebinglist[i+1].end;
		}
	}
}

for(int i=0;i<ipduanin.size();i++)
{
	const char * s1=ipduanin[i].start.c_str();
	const char * s2=ipduanin[i].end.c_str();
	string ip1 = longtoip(strtoul(s1,NULL,0));
	string ip2=longtoip(strtoul(s2,NULL,0));
//	string ipduan=get_network_ippart(ip1,ip2);// 旧的算法 结果不准确
 	 string ss=ip1+":"+ip2 ;
     output_t output = OUT_CIDR;
     char sw[50];
     strcpy(sw,ss.c_str());
     spectoaml(sw,1);
     string netmaskipduan=displayduan(output);
 //在这里计算IP 段的值
      s=s+netmaskipduan;
}
o_file.open(fn,ios_base::app);
 o_file<<s<<"};};"<<endl;
 o_file.close();
 return s+"};};";

}


string IpmstorI::feedbackview(const string& prov,const string& city, const string& isp,const string& opflag )
{
	string s="";
	string mysc ="";
	ofstream o_file;
	const char filename[] = "";
	int statExist;
	char DIRNAME[]="/var/www/html/usvn/drupal/conf";
	statExist=dir_exists( DIRNAME );
	if( statExist )
	{
	}else
	{
		int statMd;
		statMd = mkdir(DIRNAME,0755);
		if ( !statMd )
		{
			//printf("Directory created,begin download ...... \n");
			int statprov ;
			statprov = mkdir("/var/www/html/usvn/drupal/conf/",0755);
			if(!statprov)
			{
				//printf("文件夹创建成功\n");
			}else{
				//printf("文件夹创建失败\n");
			}
		}
		else
		{
			printf("Unable to create directory\n");
		}
	}	

char fn[] = "/var/www/html/usvn/drupal/conf/5min_named2013_07_09";
if(opflag=="viewinfo")
{
	ofstream o_file;
	o_file.open(fn,ios_base::app);
	if(prov==""&&city==""&&isp=="")
	{//省市ISP信息都没有确认的
		//mysc="select start,end from Dick  where prov_id='"+prov+"' order by start;";
		//mysc="select start,end from dn_ip_data_jingjian  where prov_id='"+prov+"' order by start;";	cout<<mysc<<endl;
		s="view "+prov+" { match-clients {";
	}
	if(prov!=""&&city==""&&isp=="")
	{
		//市ISP都为空的输入参数   
		mysc="select start,end from Odie_OK  where prov_id='"+prov+"' and city_id='qita' order by start;";
		//mysc="select start,end from dn_ip_data_jingjian  where prov_id='"+prov+"' order by start;";	cout<<mysc<<endl;
		s="view "+prov+" { match-clients {";
	}
	if(prov!=""&&city==""&&isp!="")
	{
	
//	cout<<mysc<<endl;
//	//城市编号为空 此查询条件查询 直辖市的相关数据 以及省的其它数据  相当于查询直辖市当成省来处理 直辖市的市级别的代码 数据库内的标记都为qita   
	//	mysc="select start,end from Odie_OK  where  city_id='qita'  and  prov_id='"+prov+"' and isp_id= '"+isp+"'    order  by start ;";
	//	cout<<mysc<<endl;
//下面为全省全部提取 
	//mysc="select start,end from Odie_OK  where  prov_id='"+prov+"'  and isp_id= '"+isp+"' order  by start ;";
//下面为5min 专用
	mysc="select start,end from Odie_OK  where  prov_id='"+prov+"' and city_id='qita' and isp_id= '"+isp+"'    order  by start ;";
//cout<<mysc<<endl;
		s="view "+prov+"-"+isp+" { match-clients {";
	}


	if(prov!=""&&city!=""&&isp!="")
	{
		mysc="select start,end from Odie_OK  where prov_id='"+prov+"' and city_id='"+city+"' and city_id != 'qita'  and isp_id='"+isp+"' order by start ;";
		cout<<mysc<<endl;
		s="view "+prov+"-"+city+"-"+isp+" { match-clients {";
		//cout<<"离开组装"<<endl;
	}
	if(prov==""&&city==""&&isp!="")
	{
		mysc="select start,end from Odie_OK  where isp_id='"+isp+"'  order by start ;";
	//	cout<<mysc<<endl;
		//s="view -"+isp+" { match-clients {";
	}
	if(prov==""&&city==""&&isp==""){
		return "输入有误^-^";
	}
//	mysc="select start,end  from Odie_OK where country_id='china'   and isp_id='"+isp+"'   order by start; ";//自定义SQL 语句执行
	
}	
	
	IPhebingList iphebinglist;
	MYSQL  mysqlc;   
	MYSQL_RES  *mysql_retc;   
	MYSQL_ROW  mysql_rowc;   
	unsigned  long  num_rowsc;   
	int  retc; 
	string result="";	
	mysql_init(&mysqlc);
	//cout<<"开始连接数据库"<<endl;
	if(mysql_real_connect(&mysqlc,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		//printf("连接成功! \t"); 
		mysql_query(&mysqlc,"set names utf8 ;");	
		retc  =  mysql_query(&mysqlc,mysc.c_str());  
		if(!retc)   
		{   
			//printf("查询成功!\t");   
			mysql_retc  =  mysql_store_result(&mysqlc);   
			if(mysql_retc  !=  NULL)   
			{   
				//printf("存储结果成功!\t");   
				num_rowsc  =  mysql_num_rows(mysql_retc);   
				//cout<<"num: "<<num_rowsc<<endl;
				if(num_rowsc  !=  0)   
				{
					while(mysql_rowc  =  mysql_fetch_row(mysql_retc))
					{
						//cout<<mysql_rowc[0]<<"------"<<mysql_rowc[1]<<endl;

						IPhebing iphebing;
						iphebing.start=mysql_rowc[0];
						iphebing.end=mysql_rowc[1];
						iphebinglist.push_back(iphebing);
					}}}
					//cout<<"cunchu"<<endl; 
					//cout<<"查出条数:"<<num_rowsc<<endl;
					if(num_rowsc==0){
						string rs="";
						mysql_close(&mysqlc);
						return rs;
					}else{
						//cout<<s<<endl;//干嘛要输出阿 
					}
		}
	}

IPhebingList ipduanin;
IPhebing ipdd;
ipdd.start=iphebinglist[0].start;
ipdd.end=iphebinglist[0].end;
for(int i=0;i<iphebinglist.size();i++)
{
	//cout<<"start:"<<iphebinglist[i].start<<"----"<<"end:"<<iphebinglist[i].end<<endl;
	if(i==iphebinglist.size()-1)
	{
               ipduanin.push_back(ipdd); 
	       break;
	}

	if(strtoul(iphebinglist[i+1].start.c_str(),NULL,0) <= strtoul(iphebinglist[i].end.c_str(),NULL,0)&&strtoul(iphebinglist[i+1].end.c_str(),NULL,0) >= strtoul(iphebinglist[i].end.c_str(),NULL,0))
	{
		ipdd.end=iphebinglist[i+1].end;
		cout<<"置换1"<<endl;
	}

	if(strtoul(iphebinglist[i+1].start.c_str(),NULL,0) <= strtoul(iphebinglist[i].end.c_str(),NULL,0)&&strtoul(iphebinglist[i+1].end.c_str(),NULL,0) < strtoul(iphebinglist[i].end.c_str(),NULL,0))
	{
		ipdd.end=iphebinglist[i].end;
		cout<<"置换2: "<<iphebinglist[i].start<<"----"<<iphebinglist[i].end<<"---"<<iphebinglist[i+1].end<<endl;
	}

if((strtoul(iphebinglist[i+1].start.c_str(),NULL,0)-strtoul(iphebinglist[i].end.c_str(),NULL,0)==1))
	{
	//	cout<<"可以合并"<<endl;
		ipdd.end=iphebinglist[i+1].end;
	}else{
	//	cout<<"找到完整段"<<endl;
		ipduanin.push_back(ipdd);
		
		if(i+1<iphebinglist.size()){
			ipdd.start=iphebinglist[i+1].start;
			ipdd.end=iphebinglist[i+1].end;
		}
	}
}
//cout<<"合并后共计"<<ipduanin.size()<<endl;
          //const char filename[] = "mytext.txt";
         //将结果写入文本文件

for(int i=0;i<ipduanin.size();i++)
{
    const char * s1=ipduanin[i].start.c_str();
    const char * s2=ipduanin[i].end.c_str();
    string ip1 = longtoip(strtoul(s1,NULL,0));
    string ip2=longtoip(strtoul(s2,NULL,0));
    string ss=ip1+":"+ip2 ;
    string netmaskipduan="";
    output_t output = OUT_CIDR;
    char sw[50];
    strcpy(sw,ss.c_str());
    spectoaml(sw,1);
    netmaskipduan=displayduan(output);
    //在这里计算IP 段的值
    s=s+netmaskipduan;   
}
mysql_close(&mysqlc);
o_file.open(fn,ios_base::app);
o_file<<s<<"};};"<<endl;
o_file.close();         
return s+"};};";
/*
	string s;
	string mys ="";
	if(prov!=""&&city==""&&isp=="")
	{
	mys="select ippart,prov_id,city_id,isp_id from dn_ip_data_shengshi  where prov_id='"+prov+"' order by start;";
		cout<<mys<<endl;
	 s="view "+prov+" { match-clients {";
	}
	if(prov!=""&&city==""&&isp!="")
	{
	mys="select ippart,prov_id,city_id,isp_id from dn_ip_data_shengshi  where prov_id='"+prov+"' and isp_id= '"+isp+"' order  by start ;";
		cout<<mys<<endl;
	 s="view "+prov+"-"+isp+" { match-clients {";
	}
	if(prov!=""&&city!=""&&isp!="")
	{
	mys="select ippart,prov_id,city_id,isp_id from dn_ip_data_shengshi  where prov_id='"+prov+"' and city_id='"+city+"' and isp_id='"+isp+"' order by start ;";
		cout<<mys<<endl;

     s="view "+prov+"-"+city+"-"+isp+" { match-clients {";
		  }
	
	if(prov==""&&city==""&&isp!="")
	{
		mys="select ippart,prov_id,city_id,isp_id from dn_ip_data_shengshi  where isp_id='"+isp+"'  order by start ;";
		cout<<mys<<endl;

     //s="view -"+isp+" { match-clients {";
		  }
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	unsigned  long  num_rows;   
	int  ret;   
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
	//	printf("连接成功! \t");  
		mysql_query(&mysql,"set names utf8 ;");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			//printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{   
				//printf("存储结果成功!\t");   
				num_rows  =  mysql_num_rows(mysql_ret);   
				if(num_rows  !=  0)   
				{   
				//	printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
						printf("%s\n",mysql_row[0]); 
							s=s+mysql_row[0];
						//	viewfile("china",mysql_row[1],mysql_row[2],mysql_row[3],mysql_row[0]);
					//生成数据文件
					}   
				}   
				else   
				{
					mysql_close(&mysql);
					s="";
					return s ;

				}   
				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	mysql_close(&mysql);

//	return s ;
	return s+"};};";
*/


}



bool IpmstorI::x()
{
	//cout<<"x running !"<<endl;
	IPinfoList ipl; 
	//以下的方法初始化了startip endip ippart 的最终信息   
	ipl=read_ipdatautf8();
	for(int i=0;i<ipl.size();i++)
	{
		cout<<"vector "<<i<<" 内的值 :  "<<ipl[i].location<<endl;
		xadd(ipl[i]);
	}
	update_ipdatautf8();
	return true ;
}



bool xadd(const IPinfo& ipinfo)
{
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	unsigned  long ipistart;	
	unsigned  long ipiend;
	int  ret;   
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t");
		unsigned long ipnstart=htonl(inet_addr(ipinfo.startip.c_str()));
		unsigned long ipnend=htonl(inet_addr(ipinfo.endip.c_str()));
		stringstream stream1;
		stringstream stream2;
		string s1;//起始IP
		stream1 << ipnstart;
		stream1>>s1;
		string s2;//终止IP
		stream2 << ipnend;
		stream2>>s2;
		string mys="insert into  dn_ip_data(startip,endip,ippart,location,country_id,prov_id,city_id,isp_id,opflag,start,end)  values('"+ipinfo.startip+"','"+ipinfo.endip+"','"+ipinfo.ipmask+"','"+ipinfo.location+"','"+ipinfo.countryid+"','"+ipinfo.provid+"','"+ipinfo.cityid+"','"+ipinfo.ispid+"','"+ipinfo.opflag+"','"+s1+"','"+s2+"') ";
		cout<<mys<<endl;
		//ret  =  mysql_query(&mysql,"select  *  from  IPMS "); 
		//mysql_query(&mysql,"set names utf8 ;"); 
		//设置字符编码	
		mysql_query(&mysql,"set names utf8 ;");	
		ret  = mysql_query(&mysql,mys.c_str());
	}   
	else   
	{   
		printf("连接失败\n");
		mysql_close(&mysql);	
	}    
	mysql_close(&mysql);
	return true;	
}

//合并IP段 dn_ip_data 数据量太大 要进行省市的合并段处理 

bool updatednipdata(IPinfoList ipl)
{
	cout<<"用户更新"<<endl;
	return true ;
}



unsigned long getstart(unsigned long startinput)
{
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
   int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t");  
		string mys="select start  from dn_ip_data where "+longtostring(startinput)+" >= start  and "+longtostring(startinput)+" <= end ";
		mysql_query(&mysql,"set names utf8 ;");
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
			    num_rows  =  mysql_num_rows(mysql_ret);
				printf("存储结果成功!\t");  
			   cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
					printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
						printf("%s\n",mysql_row[0]);  
					} 
						mysql_close(&mysql);
						return atol(mysql_row[0]); 
				}   
				else   
				{  
					mysql_close(&mysql);
					return startinput ;
				}
				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
//	cout << num_rows << endl;
	mysql_close(&mysql);
}

unsigned long getend(unsigned long end )
{
	//为什么它会在这里

}



bool searchdnipdatach(IPinfo ii,IPinfo it,IPinfo userii)
{
	IPinfo ipl;
	cout<<"sreach  操作范围"<<endl;
	IPinfo  ipi=ii;
	cout<<ii.startip<<endl;
	cout<<ii.endip<<endl;
	bool bret = true ;
	//cout<<"返回："<<getstart(atol(ii.startip))<<endl;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows=0;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t");  
		//string mys="select *  from dn_ip_data where start>="+ii.start+" and end <= "+ii.end;
	    string mys="select startip,start,endip,end,location,country_id,prov_id,city_id,isp_id,opflag,isp_name  from Odie_OK  where start <= "+ii.start+" and end >= "+it.end;
		cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
			    num_rows  =  mysql_num_rows(mysql_ret);
				printf("存储结果成功!\t");  
			   cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
					printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
						printf("startip: %s\tstart:%s\t endip:%s\t end:%s\t \n",mysql_row[0],mysql_row[1],mysql_row[2],mysql_row[3]); 
						ipl.startip=mysql_row[0];
						ipl.start=mysql_row[1];
                        ipl.endip=mysql_row[2];
                        ipl.end=mysql_row[3];
                        ipl.location=mysql_row[4] ;
                        ipl.countryid=mysql_row[5];		 
                        ipl.provid=mysql_row[6];		 
                        ipl.cityid=mysql_row[7];		 
                        ipl.ispid=mysql_row[8];		 
                        ipl.opflag=mysql_row[9];
						ipl.ipmask=mysql_row[10];

					}   
				}   
				else   
				{


 string mys="select startip,start,endip,end,location,country_id,prov_id,city_id,isp_id,opflag,isp_name  from Odie_OK  where start >= "+ii.start+" and end <= "+it.end;
		cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
			    num_rows  =  mysql_num_rows(mysql_ret);
				printf("存储结果成功!\t");  
			   cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
					printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
						printf("startip: %s\tstart:%s\t endip:%s\t end:%s\t \n",mysql_row[0],mysql_row[1],mysql_row[2],mysql_row[3]); 
						ipl.startip=mysql_row[0];
						ipl.start=mysql_row[1];
                        ipl.endip=mysql_row[2];
                        ipl.end=mysql_row[3];
                        ipl.location=mysql_row[4] ;
                        ipl.countryid=mysql_row[5];		 
                        ipl.provid=mysql_row[6];		 
                        ipl.cityid=mysql_row[7];		 
                        ipl.ispid=mysql_row[8];		 
                        ipl.opflag=mysql_row[9];	
						ipl.ipmask=mysql_row[10];

					}   
				}   
				else   
				{
   
				}
				}
				mysql_free_result(mysql_ret);  
	mysql_close(&mysql);//断开mysql的连接
			}   
			else   
			{   
			}   
		}
}   
		else   
		{

		}   
	}   


	mysql_close(&mysql);//断开mysql的连接
	
}else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);
		bret=false;
		return bret;	
	}    
//以下代码进行事务的操作
	int t=-1;
	const char* query="" ;
	if(!mysql_real_connect(&mysql, ipaddress.c_str(), username.c_str(), password.c_str(), dbname.c_str(),3306,NULL,0))
	{
		printf( "Error connecting to database: %s\n",mysql_error(&mysql));
	}  else{
		printf("Connected mysqlserver \n");
	query="SET CHARACTER SET UTF8"; //设置编码  这行代码 有问题吧? 
	t=mysql_real_query(&mysql,query,(unsigned int)strlen(query));
	if(t)
	{
		printf("编码设置失败\n");
	}
	t=mysql_real_query(&mysql,"SET AUTOCOMMIT =0",(unsigned int)strlen("SET AUTOCOMMIT =0"));
	if(t!=0){
		printf("启用自定义事务失败\n");
		bret=false;
		return bret;
	}else{
		printf("启用自定义事务成功\n");
	}
	t=mysql_real_query(&mysql,"Begin ;",(unsigned int)strlen("Begin ;"));
int deleteno=0;
int insert1no=0;
int insert2no=0;
int insert3no=0;
int insert_result=0;
//////////////////////////////////////////////////////////
cout<<"查出记录："<<num_rows<<endl;
if(num_rows==0)
{
//如果执行到这一步是有问题的 不应该出现这种情况的
/*  
cout<<"直接插入新增的IP段数据,该数据来自客户端的的输入全新的数据"<<endl;
string queryin="insert into Odie_OK(startip,start,endip,end)values('"+ii.startip+"','"+longtostring(iptolong(ii.startip))+"','"+ii.endip+"','"+longtostring(iptolong(ii.endip))+"')";
insert_result=dninsert(&mysql,strdup(queryin.c_str()));
*/

}

///////////////////////////////拆分IP段过程//////////////////////////////////////

if(num_rows==1){
    cout<<"划分查出的一条IP段"<<endl;
    cout<<"该条数据的详细描述:"<<endl;
    cout<<"startip:"<<ipl.startip<<"endip:"<<ipl.endip<<"location:"<<ipl.location<<endl;
    string deletes1="delete  from Odie_OK where start='"+ipl.start+"' and end='"+ipl.end+"' ";
    cout<<"删除段:"<<deletes1<<endl;
    deleteno=dndelete(&mysql,(char*)deletes1.c_str());
    if(ipl.startip==userii.startip){}else{
        string inserts1="insert into Odie_OK(startip,start,endip,end,location,isp_name,country_id,prov_id,city_id,isp_id,opflag)values('"+ipl.startip+"','"+ipl.start+"','"+longtoip(iptolong(userii.startip)-1)+"','"+longtostring(iptolong(userii.startip)-1)+"','"+ipl.location+"','"+ipl.ipmask+"','"+ipl.countryid+"','"+ipl.provid+"','"+ipl.cityid+"','"+ipl.ispid+"','"+ipl.opflag+  "')";
        insert1no=dninsert(&mysql,(char*)inserts1.c_str());
        cout<<"第一段:"<<inserts1<<endl;
    }
    string inserts2="insert into Odie_OK(startip,start,endip,end,location,isp_name,country_id,prov_id,city_id,isp_id,opflag)values('"+userii.startip+"','"+longtostring(iptolong(userii.startip))+"','"+userii.endip+"','"+longtostring(iptolong(userii.endip))+"','"+userii.location+"','"+userii.ipmask+"','"+userii.countryid+"','"+userii.provid+"','"+userii.cityid+"','"+userii.ispid+"','"+userii.opflag+"')";
    insert2no=dninsert(&mysql,(char*)inserts2.c_str());
    cout<<"第二段:"<<inserts2<<endl;
    if(userii.endip==ipl.endip){}else{
        string inserts3="insert into Odie_OK(startip,start,endip,end,location,isp_name,country_id,prov_id,city_id,isp_id,opflag)values('"+longtoip(iptolong(userii.endip)+1)+"','"+longtostring(iptolong(userii.endip)+1)+"','"+ipl.endip+"','"+ipl.end+"','"+ipl.location+"','"+ipl.ipmask+"','"+ipl.countryid+"','"+ipl.provid+"','"+ipl.cityid+"','"+ipl.ispid+"','"+ipl.opflag+  "')";
        insert3no=dninsert(&mysql,(char*)inserts3.c_str());
        cout<<"第三段:"<<inserts3<<endl;
    }
}
//////////////////////////////////////////////////////////////
if(num_rows>1){
    string deletes1="delete  from Odie_OK where start>='"+ii.start+"' and end<='"+it.end+"'";
    deleteno=dndelete(&mysql,(char*)deletes1.c_str());
    cout<<"删除段:"<<deletes1<<endl;

    cout<<"合并查出的"<<num_rows<<"条Ip段的首尾,中间段若连续则合并，相关location prov_id city_id isp_id 也要相应的更改"<<endl;
    if(ii.startip==userii.startip){}else{
        string inserts1="insert into Odie_OK(startip,start,endip,end,location,isp_name,country_id,prov_id,city_id,isp_id,opflag)values('"+ii.startip+"','"+ii.start+"','"+longtoip(iptolong(userii.startip)-1)+"','"+longtostring(iptolong(userii.startip)-1)+"','"+ii.location+"','"+ii.ipmask+"','"+ii.countryid+"','"+ii.provid+"','"+ii.cityid+"','"+ii.ispid+"','"+ii.opflag+"')";
        insert1no=dninsert(&mysql,(char*)inserts1.c_str());
        cout<<"第si段:"<<inserts1<<endl;
    }

    string inserts2="insert into Odie_OK(startip,start,endip,end,location,isp_name,country_id,prov_id,city_id,isp_id,opflag)values('"+userii.startip+"','"+longtostring(iptolong(userii.startip))+"','"+userii.endip+"','"+longtostring(iptolong(userii.endip))+"','"+userii.location+"','"+userii.ipmask+"','"+userii.countryid+"','"+userii.provid+"','"+userii.cityid+"','"+userii.ispid+"','"+userii.opflag+"')";
    insert2no=dninsert(&mysql,(char*)inserts2.c_str());
    cout<<"第wu段:"<<inserts2<<endl;

    if(userii.endip==it.endip){}else{
        string inserts3="insert into Odie_OK(startip,start,endip,end,location,isp_name,country_id,prov_id,city_id,isp_id,opflag)values('"+longtoip(iptolong(userii.endip)+1)+"','"+longtostring(iptolong(userii.endip)+1)+"','"+it.endip+"','"+it.end+"','"+it.location+"','"+it.ipmask+"','"+it.countryid+"','"+it.provid+"','"+it.cityid+"','"+it.ispid+"','"+it.opflag+ "')";
        insert3no=dninsert(&mysql,(char*)inserts3.c_str());
        cout<<"第liu段:"<<inserts3<<endl;
    }
}
//sleep(100000000);
if(insert_result==0&&deleteno==0&&insert1no==0&&insert2no==0&&insert3no==0){
	printf("事务提交\n");
	t=mysql_real_query(&mysql,"COMMIT;",(unsigned int)strlen("COMMIT;"));
}else{
	printf("事务回滚\n");
	t=mysql_real_query(&mysql,"ROLLBACK;",(unsigned int)strlen("ROLLBACK;"));
	bret=false;
	return bret  ;
}
	mysql_close(&mysql);//断开mysql的连接
}
return bret;
}



IPinfo searchdnipdataend(IPinfo ii)
{
	IPinfo ipl;
	cout<<"sreach end ch_test—————－";
	IPinfo  ipi=ii;
	//cout<<"返回："<<getstart(atol(ii.startip))<<endl;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		//printf("连接成功! \t");  
	//	string mys="select *  from dn_ip_data where start<="+ii.end+" and end >= "+ii.end;
		string mys="select startip,endip,end,location,isp_id,country_id,prov_id,city_id,opflag,isp_name  from Odie_OK  where start<="+ii.end+" and end >= "+ii.end;
		cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
		//	printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
			    num_rows  =  mysql_num_rows(mysql_ret);
				//printf("存储结果成功!\t");  
			   //cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
				//	printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
			printf("%s\t end :%s\t%s\t%s\t%s\t\n",mysql_row[0],mysql_row[1],mysql_row[2],mysql_row[3],mysql_row[4]);  
						ipl.endip=mysql_row[1];
						ipl.end=mysql_row[2];
						ipl.location=mysql_row[3];
						ipl.countryid=mysql_row[5];
						ipl.provid=mysql_row[6];
						ipl.cityid=mysql_row[7];
						ipl.opflag=mysql_row[8];
						ipl.ipmask=mysql_row[9];
						ipl.ispid=mysql_row[4];
						cout<<"ipl.endip:"<<ipl.endip<<endl;
					}   
				}   
				else   
				{   
				}

				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
//	cout << num_rows << endl;
	mysql_close(&mysql);
	return ipl;
}


IPinfo searchdnipdatastart(IPinfo ii)
{
	IPinfo ipl;
	cout<<"sreach start ——————－";
    IPinfo  ipi=ii;
	//cout<<"返回："<<getstart(atol(ii.startip))<<endl;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
   int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		//printf("连接成dd功! \t");  
		//string mys="select *  from dn_ip_data where start<="+ii.start+" and end >= "+ii.start;
		string mys="select startip,endip,location,isp_id,start,country_id,prov_id,city_id,opflag,isp_name  from Odie_OK where start<="+ii.start+" and end >= "+ii.start;
		cout<<"sql: "<<mys<<endl;
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
		//	printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
			    num_rows  =  mysql_num_rows(mysql_ret);
		//		printf("存储结果成功!\t");  
			  // cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
		//			printf("一共查询出 %d 条记录 \n",num_rows);   
					
					
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
			printf("start :%s\t%s\t%s\t%s\t\n",mysql_row[0],mysql_row[1],mysql_row[2],mysql_row[3]);  
						ipl.startip=mysql_row[0];
						ipl.location=mysql_row[2];
						ipl.countryid=mysql_row[5];
						ipl.provid=mysql_row[6];
						ipl.cityid=mysql_row[7];
						ipl.opflag=mysql_row[8];
						ipl.ipmask=mysql_row[9];
						ipl.ispid=mysql_row[3];
						ipl.start=mysql_row[4];
					}   
				}   
				else   
				{   
				}
				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
//	cout << num_rows << endl;
	mysql_close(&mysql);
	return ipl;
}

bool IpmstorI::updatednipdata(const IPinfo& ipinfo  )
{
	IPinfo ii;
	bool bret=true ;
	ii.start=longtostring( iptolong(ipinfo.startip));
	ii.end=longtostring(iptolong(ipinfo.endip));
	if(iptolong(ipinfo.startip)>iptolong(ipinfo.endip))
	{
		cout<<"用户输入不合法"<<endl;
		return false;
	}else{
	cout<<"用户起始地址"<<longtoip(iptolong(ipinfo.startip))<<"start "<<ii.start<<endl;
	cout<<"用户终止地址"<<longtoip(iptolong(ipinfo.endip))<<"end "<<ii.end<<endl;
	if(ipinfo.startip!="")
	{
		ii.startip=ipinfo.startip;
	}else{}
	if(ipinfo.endip!="")
	{
		ii.endip=ipinfo.endip;
	}else{}
	if(ipinfo.ipmask!="")
	{
		ii.ipmask=ipinfo.ipmask;
	}else{}
	if(ipinfo.location!="")
	{
		ii.location=ipinfo.location;
	}else{}
	if(ipinfo.countryid!="")
	{
		ii.countryid=ipinfo.countryid;
	}else{}
	if(ipinfo.provid!="")
	{
		ii.provid=ipinfo.provid;
	}else{}
	if(ipinfo.cityid!="")
	{
		ii.cityid=ipinfo.cityid;
	}else{}
	if(ipinfo.ispid!="")
	{
		ii.ispid=ipinfo.ispid;
	}else{}
	if(ipinfo.opflag!="")
	{
		ii.opflag=ipinfo.opflag;
	}else{}
//该处的操作标志 决定了这个IP段 以后是否可以被再次修改
	cout<<"操作标志 opflag "<<ii.opflag<<endl;
cout<<"开始进行搜索,根据搜索结构适当的拆分合并I段数据"<<endl;
IPinfo ipinfo1=searchdnipdatastart(ii);
IPinfo ipinfo2=searchdnipdataend(ii);
//cout<<"ipinfo2 endip : "<<ipinfo2.endip <<endl;
IPinfo ipinfoin ;
if((ipinfo1.startip=="")&&(ipinfo2.endip==""))
{
ipinfoin=ii;
}
if(ipinfo1.startip!="")
{
ipinfoin.startip=ipinfo1.startip ;
ipinfoin.start=ipinfo1.start ;
}
if(ipinfo2.endip!="")
{
	ipinfoin.endip=ipinfo2.endip;
	ipinfoin.end=ipinfo2.end;
	cout<<"ipinfoin:  endip k "<<ipinfoin.endip<<endl;
}
cout<<"起始"<<ipinfoin.startip<<endl;
cout<<"终止"<<ipinfoin.endip<<endl;
cout<<"起始"<<ipinfo.start<<endl;
cout<<"终止"<<ipinfo.end<<endl;
//bret=searchdnipdatach(ipinfoin,ipinfo);
bret=searchdnipdatach(ipinfo1,ipinfo2,ipinfo);
}
return bret ;

}



IPinfoList  IpmstorI::dnipinfoadd(const IPinfo& ipinfo   )
{
	//该方法主要将 startip endip localtion 等信息由函数计算产生的一些结果传送至此 在此接受，并传递给数据库操作层，插入数据库的dn_ip_data 数据表
	//slice 定义的复杂数据结构向量vector的使用
	IPinfoList ipl ;
	/*
	IPinfo ii;
	ii.startip="1.1.1.0";
	ii.endip="1.1.1.255";
*/
	Ice::Current iv;	
	cout<<"调用者IP"<<iv.adapter<<endl;
	ipl.push_back(ipinfo);
	cout<<"vector的输出startip"<<ipl[0].startip<<"  endip:"<<ipl[0].endip<<endl;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	unsigned  long ipistart;	
	unsigned  long ipiend;
	int  ret;   
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t");
		string mys="insert into  dn_ip_data  values('"+ipl[0].startip+"','"+ipl[0].endip+"','"+ipl[0].ipmask+"','"+ipl[0].location+"','"+ipl[0].countryid+"','"+ipl[0].provid+"','"+ipl[0].cityid+"','"+ipl[0].ispid+"','"+ipl[0].opflag+"')";
		cout<<mys<<endl;
		//ret  =  mysql_query(&mysql,"select  *  from  IPMS "); 
		//mysql_query(&mysql,"set names utf8 ;"); 
		//设置字符编码	
		mysql_query(&mysql,"set names utf8 ;");	
		ret  = mysql_query(&mysql,mys.c_str());
	}   
	else   
	{   
		printf("连接失败\n");
		mysql_close(&mysql);	
	}    
	mysql_close(&mysql);
	return ipl ;
}



int update_ipdata(string startip, string endip,string location,string isp)
{
	string start = startip;
	string end = endip;
	cout<<"IP范围"<<start<<"---"<<end<<"地区信息"<<location<<"ISP信息"<<isp<<endl;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	unsigned  long  num_rows;   
	int  ret;   
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t");  
		string mys="select * from ip_data  where startip='"+start+"' and endip='"+end+"' ;";
		mysql_query(&mysql,"set names utf8 ;");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{   
				printf("存储结果成功!\t");   
				num_rows  =  mysql_num_rows(mysql_ret);   
				if(num_rows  !=  0)   
				{   
					printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
						printf("%s\t%s\t%s\t%s\t%s\n",mysql_row[0],mysql_row[1],mysql_row[2],mysql_row[3],mysql_row[4]);   
					}   
				}   
				else   
				{   
				}   
				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	cout << num_rows << endl;
	mysql_close(&mysql);
}

string compu_mask(char* network)
{
	char *p;
	int mask[32];
	string ipduan="";
	if(network==NULL)
		return 0;
	int first1,first2,first3,first4,maskno ;
	first1=first2=first3=first4=maskno=-1 ;
	sscanf(network,"%d.%d.%d.%d/%d",&first1,&first2,&first3,&first4,&maskno);
	for(int j=0 ;j <maskno ; j++)
	{
		mask[j]=1;
	}
	for(int j=maskno;j<32;j++)
	{
		mask[j]=0;
	}
	stringstream   sstrm;
	sstrm << btt(&mask[0])<<"."<<btt(&mask[8])<<"."<<btt(&mask[16])<<"."<<btt(&mask[24]);
	ipduan= sstrm.str(); 
	cout<<"子网掩码"<<ipduan<<endl ;
	return ipduan ;
}


string reprovpy(string location)
{
	string provpy;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t");  
		string mys=" select dn_prov.prov_py, dn_prov.prov_name  from dn_prov ; ";
		//mysql_query(&mysql,"set names utf8 ;");
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			printf("查询成功!\t");  
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
			    num_rows  =  mysql_num_rows(mysql_ret);
				printf("存储结果成功!\t");  
			   cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
					printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
						string str1=location;
						string::size_type loc=str1.find(mysql_row[1],0);
						if(loc != string::npos)
						{	
							provpy=mysql_row[0];
							mysql_close(&mysql);
							return provpy;
						}else{
							//citypy="not china";
						}

					}
				}   
				else   
				{   
				}   
				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}
	//cout << num_rows << endl;
	mysql_close(&mysql);
	return provpy;
}


string recitypy(string location)
{
	string citypy;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t");  
		string mys=" select  dn_city.city_py,dn_city.city_name  from dn_city ; ";
		//mysql_query(&mysql,"set names utf8 ;");
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			printf("查询成功!\t");  
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
			    num_rows  =  mysql_num_rows(mysql_ret);
				printf("存储结果成功!\t");  
			   cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
					printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
						string str1=location;
						string::size_type loc=str1.find(mysql_row[1],0);
						if(loc != string::npos)
						{	
							citypy=mysql_row[0];
							mysql_close(&mysql);
							return citypy;
						}else{
							//citypy="not china";
						}

					}
				}   
				else   
				{   
				}   
				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}
	//cout << num_rows << endl;
	mysql_close(&mysql);
	return citypy;
}



string reisppy(string location)
{
	string isppy;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		//printf("连接成功! \t");  
		string mys=" select isp_py ,isp_name  from isp ;";
		//mysql_query(&mysql,"set names utf8 ;");
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
		//	printf("查询成功!\t");  
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
			    num_rows  =  mysql_num_rows(mysql_ret);
				//printf("存储结果成功!\t");  
			   cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
					printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
						string str1=location;
						string::size_type loc=str1.find(mysql_row[1],0);
						if(loc != string::npos)
						{	
							isppy=mysql_row[0];
							mysql_close(&mysql);
							return isppy;
						}else{
							//citypy="not china";
						}

					}
				}   
				else   
				{   
				}   
				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}
	//cout << num_rows << endl;
	mysql_close(&mysql);
	return isppy;
}


bool update_ipdatautf8()
{
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		string mys="update  ip_data_20110707  set opflag=2  where opflag=0  limit 100 ; ";
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
				num_rows  =  mysql_num_rows(mysql_ret);
				printf("一共更新 %d 条记录 \n",num_rows);   
			}   
			else   
			{   
				}   
				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	cout << num_rows << endl;
	mysql_close(&mysql);
}



IPinfoList read_ipdatautf8()
{
	IPinfoList ipl;
	//cout<<"自动获取ip_data_utf8 数据 !"<<endl;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	int  ret;  
	int num_rows;	
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		//printf("连接成功! \t");  
	string mys="select startip,endip,location,isp from ip_data_20110707 where opflag=0  limit 100  ";
		mysql_query(&mysql,"set names utf8 ;");
		//mysql_query(&mysql,"");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			//printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{  
			    num_rows  =  mysql_num_rows(mysql_ret);
				//printf("存储结果成功!\t");  
			  // cout<<"num_rows"<<num_rows<<endl;	
				if(num_rows  !=  0)   
				{   
					//printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
						//printf("%s\t%s\t%s\t\n",mysql_row[0],mysql_row[1],mysql_row[2]);  
						//在这里组装我们的IPinfoList
						//首先合并IP段
						string ipduan="";  
						ipduan=get_network_ippart(mysql_row[0],mysql_row[1]); 
						//	cout<<"ipduan"<<ipduan<<endl;
						IPinfo ii;
						ii.startip=mysql_row[0];
						ii.endip=mysql_row[1];
						ii.location=mysql_row[2];
						ii.opflag="1";
						ii.cityid=recitypy(mysql_row[2]);
					//	cout<<"-----------------"<<ii.cityid<<endl;
						ii.countryid="china";
						ii.ipmask=ipduan;
						ii.ispid=reisppy(mysql_row[3]);
						ii.provid=reprovpy(mysql_row[2]);
						ipl.push_back(ii);
					}   
				}   
				else   
				{   
				}

				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{

		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	//	cout << num_rows << endl;
	mysql_close(&mysql);
	return ipl ;
}



IPpart  get_network_submask(char* network) //input 1.1.1.0/24 return FF FF FF 00
{
	IPpart iptsub;
	char *p;
	int net_len = strlen(network);
	int i =0;
	int sub_num=0;
	int mask[32];
	int netaddr[32];
	int ip[32];
	string ipduan="";
	if(network==NULL)
		return iptsub;
	int first1,first2,first3,first4,maskno ;
	first1=first2=first3=first4=maskno=-1 ;
	sscanf(network,"%d.%d.%d.%d/%d",&first1,&first2,&first3,&first4,&maskno);
	//printf("处理IP地址之前 ");
	//printf("%d.%d.%d.%d/%d \n",first1,first2,first3,first4,maskno);
	//printf("将十进制IP地址转换为二进制的IP地址:  ");
	const string s1=binto(first1);
	const string s2=binto(first2);
	const string s3=binto(first3);
	const string s4=binto(first4);
	sscanf(s1.c_str(),"%d.%d.%d.%d.%d.%d.%d.%d.",&ip[0],&ip[1],&ip[2],&ip[3],&ip[4],&ip[5],&ip[6],&ip[7]);
	sscanf(s2.c_str(),"%d.%d.%d.%d.%d.%d.%d.%d.",&ip[8],&ip[9],&ip[10],&ip[11],&ip[12],&ip[13],&ip[14],&ip[15]);
	sscanf(s3.c_str(),"%d.%d.%d.%d.%d.%d.%d.%d.",&ip[16],&ip[17],&ip[18],&ip[19],&ip[20],&ip[21],&ip[22],&ip[23]);
	sscanf(s4.c_str(),"%d.%d.%d.%d.%d.%d.%d.%d.",&ip[24],&ip[25],&ip[26],&ip[27],&ip[28],&ip[29],&ip[30],&ip[31]);
	//cout<<"返回字符串:"<<s1<<"-"<<s2<<"-"<<s3<<"-"<<s4<<endl;
//	cout<<"IP地址的二进制形式";
	for(int i=0 ;i<32 ;i++)
	{
//		printf("%d",ip[i]);
	}
//	cout<<endl ;
//	printf("子网掩码二进制形式");
	for(int j=0 ;j <maskno ; j++)
	{
		mask[j]=1;
	}
	for(int j=maskno;j<32;j++)
	{
		mask[j]=0;
	}
	for(int j=0;j<32;j++)
	{
//		printf("%d",mask[j]);
	}
//	printf("\n网络号的二进制形式");
	for(int j=0 ;j<32 ; j++)
	{
		netaddr[j]=ip[j]&&mask[j];
//		printf("%d",netaddr[j]);
	}

//	printf("\n广播号的二进制形式");
	int k =0 ;
	int bcast[32];
	for(k;k<32;k++)
	{
		if(mask[k]==0)
		{
			bcast[k]=1;
		}else
		{
			bcast[k]=ip[k];
		}
//		printf("%d",bcast[k]);
	}
//	printf("\nip地址二进制转十进制：");
//	printf(" %d.%d.%d.%d",btt(&ip[0]),btt(&ip[8]),btt(&ip[16]),btt(&ip[24]));
//	printf("\n子网掩码二进制转十进制：");
//	printf(" %d.%d.%d.%d",btt(&mask[0]),btt(&mask[8]),btt(&mask[16]),btt(&mask[24]));
//	printf("\n网络号二进制转十进制：");
//	printf(" %d.%d.%d.%d",btt(&netaddr[0]),btt(&netaddr[8]),btt(&netaddr[16]),btt(&netaddr[24]));
//	printf("\n广播地址二进制转十进制：");
//	printf(" %d.%d.%d.%d",btt(&bcast[0]),btt(&bcast[8]),btt(&bcast[16]),btt(&bcast[24]));
	stringstream   sstrm1;
	stringstream   sstrm2;
	//	cout<<"\n拆分出的IP段 ";
	sstrm1 << btt(&netaddr[0])<<"."<<btt(&netaddr[8])<<"."<<btt(&netaddr[16])<<"."<<btt(&netaddr[24]);
	sstrm2<<btt(&bcast[0])<<"."<<btt(&bcast[8])<<"."<<btt(&bcast[16])<<"."<<btt(&bcast[24]);
	ipduan= sstrm1.str(); 
	string ipduan2=sstrm2.str();
	iptsub.startip=ipduan;
	iptsub.endip=ipduan2;
	//cout<<ipduan<<endl ;
	//printf("\n\n");
	return iptsub ;
}


string get_cpc(string startip , string endip)
{
	cout<<"接受参数"<<startip<<":"<<endip<<":"<<endl;
	string result="默认";
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	unsigned  long  num_rows;   
	int  ret;   
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t");  
		string mys="select location  from ip_data  where startip='"+startip+"' and endip='"+endip+"' ;";
		mysql_query(&mysql,"set names utf8 ;");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{   
				printf("存储结果成功!\t");   
				num_rows  =  mysql_num_rows(mysql_ret);   
				if(num_rows  !=  0)   
				{   
					printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
//printf("%s\t%s\t%s\t%s\t%s\n",mysql_row[0],mysql_row[1],mysql_row[2],mysql_row[3],mysql_row[4]); 
						result=cstr_to_string(mysql_row[0]);
					}   
				}   
				else   
				{   
				}   
				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{   
		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	mysql_close(&mysql);
	return result ;

}

int add_data_viewinfo(int id,string ip_part,string country,string prov,string city,string isp_name,string ip_mask)
{
	cout<<id<<":"<<ip_part<<":"<<country<<":"<<prov<<":"<<city<<":"<<isp_name<<":"<<ip_mask<<":"<<endl;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	unsigned  long  num_rows; 
	unsigned  long ipistart;	
	unsigned  long ipiend;
	int  ret;   
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t");
		/*
		   stringstream streamstart;
		   stringstream streamend;	
		   string ipstart ;
		   string ipends;
		   ipistart=inet_addr(startip.c_str());
		   ipiend= inet_addr(endip.c_str());
		   streamstart << ipistart;        //从long型数据输入
		   streamstart >>ipstart;  	//转换为 string
		   streamend << ipiend ;
		   streamend >> ipends ;
		   cout<<"sdsd"<<ipstart<<endl ;
		   */
		string mys="insert into  viewinfo  values('1','"+ip_part+"','"+country+"','"+prov+"','"+city+"','"+isp_name+"','"+ip_mask+"')";
		cout<<mys<<endl;
		//ret  =  mysql_query(&mysql,"select  *  from  IPMS "); 
		//mysql_query(&mysql,"set names utf8 ;"); 
		//设置字符编码	
		mysql_query(&mysql,"set names utf8 ;");	
		ret  = mysql_query(&mysql,mys.c_str());
	}   
	else   
	{   
		printf("连接失败\n");
		mysql_close(&mysql);	
		return 0;  
	}    
	cout << num_rows << endl;
	mysql_close(&mysql);
	return 0;
}


string get_from_ipdata(const string startip,const string endip)
{
	cout<<"出口参数函数开始工作!\n";
	cout<<"接受参数"<<startip<<":"<<endip<<":"<<endl;
	string result="默认";
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	unsigned  long  num_rows;   
	int  ret;   
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t");  
		string mys="select * from ip_data  where startip='"+startip+"' and endip='"+endip+"' ;";
		mysql_query(&mysql,"set names utf8 ;");	
		ret  =  mysql_query(&mysql,mys.c_str());  
		if(!ret)   
		{   
			printf("查询成功!\t");   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{   
				printf("存储结果成功!\t");   
				num_rows  =  mysql_num_rows(mysql_ret);   
				if(num_rows  !=  0)   
				{   
					printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
						printf("%s\t%s\t%s\t%s\t%s\n",mysql_row[0],mysql_row[1],mysql_row[2],mysql_row[3],mysql_row[4]);   
						result=cstr_to_string(mysql_row[0])+":"+cstr_to_string(mysql_row[1])+":"+cstr_to_string(mysql_row[2])+":"+cstr_to_string(mysql_row[3])+":"+cstr_to_string(mysql_row[4]);
					}   
				}   
				else   
				{   
				}   
				mysql_free_result(mysql_ret);   
			}   
			else   
			{   
			}   
		}   
		else   
		{   
		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysql);	
	}    
	mysql_close(&mysql);
	return result;
}


string IpmstorI::getResult(const string& startip,const string& endip,int& resuetsource)
{
	cout<<"接受参数"<<startip<<":"<<endip<<":"<<resuetsource<<endl;
	string result="默认值";
	result = get_from_ipdata(startip,endip);
	return result ;
}



int IpmstorI::ipmerge(const string& startip,const string& endip)
{
	cout<<"ipmerge函数"<<endl;
	return 0;
}

int IpmstorI::ipvalid(const string& ipinfo)
{
	cout<<"IP段信息校验函数"<<endl ;
	return 0;
}


string get_network_ippart(string startip,string endip)
{
string sippart="get_network_ippart该方法置空";
cout<<sippart<<endl;
	return sippart ;
}

IPpart IpmstorI::ipsplit(const string& ipinfo )
{
	IPpart ipt ;
	char *p=new char[ipinfo.size()+1];
	strcpy(p,ipinfo.c_str()); 	
	cout<<"ip段开始划分"<<endl ;
	cout<<"ipsplit 接受的参数p:"<<p<<endl ;
	ipt = get_network_submask(p);
	cout<<"分解函数接受到的分解结果"<<ipt.startip<<"----"<<ipt.endip<<endl ;
	cout<<"接下来把分解结果交给查询函数处理，可能涉及到增加删除的操作"<<endl;
	return ipt;
}



int IpmstorI::ipmsadd(const string& startip,const string& endip,const string& location,const string& isp)
{
	printf("IP段信息添加函数调用开始 \n");
	cout<<"添加函数接收的函数参数："<<endl<<"startip:"<<startip<<endl<<"endip:"<<endip<<"location:"<<location<<"isp:"<<isp<<endl;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	unsigned  long  num_rows; 
	unsigned  long ipistart;	
	unsigned  long ipiend;
	int  ret;   
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t");
		stringstream streamstart;
		stringstream streamend;	
		string ipstart ;
		string ipends;
		ipistart=inet_addr(startip.c_str());
		ipiend= inet_addr(endip.c_str());
		streamstart << ipistart;        //从long型数据输入
		streamstart >>ipstart;  	//转换为 string
		streamend << ipiend ;
		streamend >> ipends ;
		//cout<<"sdsd"<<ipstart<<endl ;
		string mys="insert into  ip_data_utf8  values ('"+startip+"','"+endip+"','"+location+"')";
		cout<<mys<<endl;
		//ret  =  mysql_query(&mysql,"select  *  from  IPMS "); 
		mysql_query(&mysql,"set names utf8 ;"); 
		//设置字符编码	 
		ret  = mysql_query(&mysql,mys.c_str());
	}   
	else   
	{   
		printf("连接失败\n");
		mysql_close(&mysql);	
		return 0;  
	}    
	cout << num_rows << endl;
	mysql_close(&mysql);
	return 1 ;
}

int IpmstorI::ipmsdel(const string& start ,const string& end)
{
	printf("删除函数开始工作…… \n");
	cout<<"删除函数接收的函数参数："<<endl<<"start:"<<start<<endl<<"end:"<<end<<endl;
	MYSQL  mysql;   
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	unsigned  long  num_rows;   
	int  ret;   
	mysql_init(&mysql);   
	if(mysql_real_connect(&mysql,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
		printf("连接成功! \t");  
		string mys="delete from IPMS";
		mys=mys+" where STARTIP='"+start+"' and ENDIP='"+end+"'";
		cout<<mys<<endl;
		ret  = mysql_query(&mysql,mys.c_str());
		if(!ret)   
		{ 
			mysql_ret = mysql_store_result(&mysql);	
			if(mysql_ret != NULL)
			{
				num_rows = mysql_num_rows(mysql_ret);
				cout<<"共有"<<num_rows<<"条数据符合删除要求"<<endl;
			}
		}   
		else   
		{
			cout<<"木有找到要求的数据"<<endl;	
		}   
	}   
	else   
	{   
		printf("连接失败\n");   
	}    
	cout << num_rows << endl;
	return 0 ;
}

void LoadConfig(int iSize)
{
	printf("#############################################################################\n");
	printf("开始读取配置文件server.conf信息\n");
	Config configSettings(ConfigFile);  
	port = configSettings.Read("port", 0); 
	ipaddress = configSettings.Read("ipaddress", ipaddress);  
	username = configSettings.Read("username", username);  
	password = configSettings.Read("password", password); 
	dbname = configSettings.Read("dbname",dbname);
	server_adapter = configSettings.Read("server_adapter",server_adapter);
	adapter_address = configSettings.Read("adapter_address",adapter_address);	
	cout<<"远程调用适配器名称server_adapter"<<server_adapter<<endl ;
	cout<<"远程调用适配器地址端口号adapter_adress"<<adapter_address<<endl ;
	cout<<"mysql数据库端口号port:"<<port<<endl;  
	cout<<"mysql数据库地址ipAddress:"<<ipaddress<<endl;  
	cout<<"mysql数据库用户名username:"<<username<<endl;  
	cout<<"mysql该用户密码password:"<<password<<endl;  
	cout<<"mysql使用的数据库dbname:"<<dbname<<endl;
	printf("############################################################################\n");
}

string IpmstorI::ipmssea(const string& start ,const string& end)
{
	MYSQL_RES  *mysql_ret;   
	MYSQL_ROW  mysql_row;   
	unsigned  long  num_rows;   
	int  ret; 
	string result="";	
	string mys="";
	string sstart=longtostring(iptolong(start));
	//mys="select prov_id,city_id,isp_id,area from Odie_OK  where '"+sstart+"' >= start  and '"+sstart+"' <= end;";
mys="select isp_id,prov_id  from  Odie_OK  where "+sstart+" between start  and  end;";
	cout<<mys<<endl;
	mysql_query(&mysql,"set names utf8 ;");	
	ret  =  mysql_query(&mysql,mys.c_str());  
	if(!ret)   
	{   
			mysql_ret  =  mysql_store_result(&mysql);   
			if(mysql_ret  !=  NULL)   
			{   
				num_rows  =  mysql_num_rows(mysql_ret);   
				if(num_rows  !=  0)   
				{   
					printf("一共查询出 %d 条记录 \n",num_rows);   
					while(mysql_row  =  mysql_fetch_row(mysql_ret))   
					{   
//	printf("%s\t%s\t%s\t%s\t%s\n",mysql_row[0],mysql_row[1],mysql_row[2],mysql_row[3],mysql_row[4]);  
   //result="\t"+start+" : "+string(mysql_row[0])+" : "+string(mysql_row[1])+" : "+string(mysql_row[2])+" : "+string(mysql_row[3]) ;
   //result=result+string(mysql_row[0])+"  "+string(mysql_row[1])+"  "+string(mysql_row[2])+"   "+string(mysql_row[3])+";";
//if(string(mysql_row[1])!="sanxi")
{
result=string(mysql_row[1])+" "+string(mysql_row[0]);
}
//else{
//result=string(mysql_row[0]);	
//}
		}   
				}   
				else{ }   
				mysql_free_result(mysql_ret);   
			}   
			else{ }   
		}else{ }   
	return  result ; 
}

void IpmstorI::printString(const string& s)
{
	//ICE 服务端程序//自定义代码执行 目前主要返回ISP为其他的数据
	MYSQL  mysqlc;   
	MYSQL_RES  *mysql_retc;   
	MYSQL_ROW  mysql_rowc;   
	unsigned  long  num_rowsc;   
	int  retc; 
	string result="";	
	mysql_init(&mysqlc);   
	if(mysql_real_connect(&mysqlc,ipaddress.c_str(),username.c_str(),password.c_str(),dbname.c_str(),0,NULL,0))   
	{   
	   string mysc="";
	// mysc="select startip from Odie_OK    order by start ;";
mysc="select startip from Odie_OK where country_id='china' and isp_id='qita' order by start";	
	mysql_query(&mysqlc,"set names utf8 ;");	
		retc  =  mysql_query(&mysqlc,mysc.c_str());  
		if(!retc)   
		{   
			printf("查询成功!\t");   
			mysql_retc  =  mysql_store_result(&mysqlc);   
			if(mysql_retc  !=  NULL)   
			{   
				printf("存储结果成功!\t");   
				num_rowsc  =  mysql_num_rows(mysql_retc);   
				if(num_rowsc  !=  0)   
				{   
					printf("一共查询出 %d 条记录 \n",num_rowsc);   
					while(mysql_rowc  =  mysql_fetch_row(mysql_retc))   
					{
						cout<< mysql_rowc[0]<<endl;
					}   
				}   
				else   
				{   
				}   
				mysql_free_result(mysql_retc);   
			}   
			else   
			{   
			}   
		}   
		else   
		{   
		}   
	}   
	else   
	{   
		printf("连接失败\n"); 
		mysql_close(&mysqlc);	
	}    
	cout << num_rowsc << endl;
	mysql_close(&mysqlc);
}
