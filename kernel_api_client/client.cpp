/*
 * =====================================================================================
 *
 *       Filename:  client.cpp
 *
 *    Description:  DNION IPMS 客户端 
 *
 *        Version:  1.0
 *        Created:  2013_07_02修改
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  GarfieldLinux (zjf), susan_jin(jdm)
 *        Company:  江苏天联信息科技发展有限公司
 *
 * =====================================================================================
 */

#include <Ice/Ice.h>
#include "../kernel_ice/Ipmstor.h"
#include "../kernel_api_server/Config.h"
#include <iconv.h>
#include <iostream>
#include <fstream>
#include <string>
//#include <curses.h>
#include <ncursesw/ncurses.h>
#include <locale.h> 


using namespace std;
using namespace IPMS;
using namespace jk;
using namespace dx;
#define OUTLEN 255 


class CodeConverter {
	private:
		iconv_t cd;
	public:
		// 构造
		CodeConverter(const char *from_charset,const char *to_charset) {
			cd = iconv_open(to_charset,from_charset);
		}

		// 析构
		~CodeConverter() {
			iconv_close(cd);
		}

		// 转换输出
		int convert(char *inbuf,int inlen,char *outbuf,int outlen) {
			char **pin = &inbuf;
			char **pout = &outbuf;

			memset(outbuf,0,outlen);
			return iconv(cd,pin,(size_t *)&inlen,pout,(size_t *)&outlen);
		}
};


const char ConfigFile[]="client.conf"  ;
string client_conn = "";

void StringSplit(string s,char splitchar,vector<string>& vec)
{
	if(vec.size()>0)//保证vec是空的
		vec.clear();
	int length = s.length();
	int start=0;
	for(int i=0;i<length;i++)
	{
		if(s[i] == splitchar && i == 0)//第一个就遇到分割符
		{
			start += 1;
		}
		else if(s[i] == splitchar)
		{
			vec.push_back(s.substr(start,i - start));
			start = i+1;
		}
		else if(i == length-1)//到达尾部
		{
			vec.push_back(s.substr(start,i+1 - start));

		}
	}
}

void LoadConfig(int iSize)
{
	//printf("**********************************************************************************\n");
	//printf("开始读取配置文件client.conf信息\n"); 
	Config configSettings(ConfigFile);  

	client_conn = configSettings.Read("client_conn", client_conn);  
    //	cout<<"客户端远程调用连接字符串client_conn: "<<client_conn<<endl ;
    //	printf("**********************************************************************************\n");
}

int  readline()
{
	ifstream in;
	string ipinfo = "";
	in.open("client.conf");

	if (!in)
	{
		cout << "打开文件失败！" << endl;
		return -1;
	}

	while(!in.eof())
	{
		in >>ipinfo ;
		cout << ipinfo << endl;
	}
	in.close();
	return 0;

}


int main(int argc ,char *argv[])
{
	int status = 0;
	int num_rows ;
	LoadConfig(1);
	Ice::CommunicatorPtr ic;
	try {
		ic = Ice::initialize(argc, argv);
		Ice::ObjectPrx base = ic->stringToProxy(client_conn);
		IpmstorPrx ipmstor = IpmstorPrx::checkedCast(base);
		if (!ipmstor)
			throw "Invalid proxy";
            PRinfoList pril;
            ISPinfoList ispil ;
            SSinfoList ssil;
            SSinfoList shengil;
            SSinfoList haiwai;
            AREAinfoList alist ;

            ssil =	ipmstor->feedbackss();
            shengil = ipmstor->feedbacks();
            haiwai = ipmstor->feedbackhaiwai();
            ispil = ipmstor->feedbackisp();
            
			SSIspList ssisp;
            SSIspList oldview;



//生成贵州电信
ipmstor->feedbackviewarea("guizhoudx");
ipmstor->feedbackviewarea("chinajyw");



/*  
//生成除电信联通之外的isp view 
ipmstor->feedbackviewarea("shanghaiedu");
ipmstor->feedbackviewarea("huananedu");
ipmstor->feedbackviewarea("zhejiangedu");
ipmstor->feedbackviewarea("edu");
ipmstor->feedbackviewarea("hunanhengyangtietong");
ipmstor->feedbackviewarea("beijingtietong");
ipmstor->feedbackviewarea("tietong");
ipmstor->feedbackviewarea("hk");
ipmstor->feedbackviewarea("tw");
ipmstor->feedbackviewarea("mo");
ipmstor->feedbackviewarea("cmc");
ipmstor->feedbackviewarea("youxiantong");
ipmstor->feedbackviewarea("US");
ipmstor->feedbackviewarea("japan");
*/

			
/*7 view   
ipmstor->feedbackviewarea("changchun");
ipmstor->feedbackviewarea("dalian");
ipmstor->feedbackviewarea("haerbin");
ipmstor->feedbackviewarea("kunming");
ipmstor->feedbackviewarea("shijiazhuang");
ipmstor->feedbackviewarea("taiyuan");
ipmstor->feedbackviewarea("xian");
*/


/*
//生成全国七大区域的view  
string largearea[7]={"dongbei","huabei","huadong","huanan","huazhong","xibei","xinan"};
string largeisppy[4]={"dianxin","liantong","tietong","changchengkuandai"};
for(int i=0;i<7;i++)
{
for(int j=0;j<4;j++)
{
cout<<"大区域:"<<largearea[i]<<" isppy:"<<largeisppy[j]<<endl;
ipmstor->feedbackviewlargearea(largearea[i],largeisppy[j]);
}
}
*/

//返回32个省py
/*  
SSinfoList pusheng;
pusheng = ipmstor->feedbackpushengpy();
for(int i=0;i<pusheng.size();i++)
{
	cout<<pusheng[i].provname<<"-----------"<<pusheng[i].provpy<<endl;
}
 
*/

/*

 
//生成5min数据文件的调用处 by lisa

//5min 杨宁 王亚南  需要的配置文件 生成后  需要将配置名更改    
//// 
////      越南 --->       VietNam
////           泰国  --->      Thailand
////                马来西亚 ---> Malaysia
////                     新加坡 --->    Singapore
////日本(haiwai-Japan-qita )  ----》 Japan
////    韩国(haiwai-SouthKorea-qita) -----》 SouthKorea
////    haiwai-Korea-qita   ----》 Korea
////       香港(hk） 澳门台湾（ motw)  （港澳台请自行合并）  维持原样
////          美国 haiwai-UnitedStates-qita  -----》   UnitedStates
////
///


oldview=ipmstor->feedbackoldss();
for(int i=0;i<oldview.size();i++)
{
string s=ipmstor->feedbackview(oldview[i].shengpy,oldview[i].shipy,oldview[i].isppy,"viewinfo");//生成省市数据
if(s=="")
    {}else{ cout<<s<<endl;   }
}
  
{
	{
		string s="";
		if(s=="")
		{
		}
		else
		{
		   	cout<<s<<endl;
	   	}
	}
	
}



*/


//更新操作调用处 by lisa
//用户更新操作，该操作比较复杂
/*
IPinfo ii;
ii.startip="202.101.72.95";
ii.endip="202.101.72.95";
ii.ipmask="电信";
ii.location="贵州省贵阳市";
ii.countryid="china";
ii.provid="guizhou";
ii.cityid="guiyang";
ii.ispid="dianxin";
//ii.area="guizhoudx"
ii.opflag="2";

ipmstor->updatednipdata(ii);//更新操作   该函数能够正常工作

*/

VIEWinfo viewinfo;


	} catch (const Ice::Exception& ex) {
		cerr << "connection refused -_-! mail:garfield@dnion.com" << endl<<endl;
		status = 1;
	}
	if (ic)
		ic->destroy();
	return status;
	}

