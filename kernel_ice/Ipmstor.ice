module IPMS {
	module yc{
		exception IPUploadError{};
		exception IPDataTooLargerError{};
		exception IPDownloadError{};
	};

	module dx {
		struct IPhebing {
		string start;
		string end;
		string provid;
		string cityid;	
		};
		struct AREAinfo
		{
		string areapy; 
		string areaname;
		};
		struct SSIsp{
		string shengpy;
		string shipy;
		string isppy;
		};
		struct VIEWinfo
		{
		string viewname;
		int viewareaid;
		int viewispid;
		string viewpci;
		};
		struct IPpart
		{
		string ipparts;
		string startip;
		string start;
		string end;
		string endip;
		};
		struct PRinfo
		{
		string provname;
		string provpy;
		};
		struct SSinfo 
		{
		string provname;
		string provpy;
		string cityname;
		string citypy;
		};
		struct ISPinfo
		{
		string ispname;
		string isppy;
		};
		struct IPinfo
		{
			string startip;
			string start;
			string endip;
			string end;
			string ipmask;
			string location;
			string countryid;
			string provid;
			string cityid;
			string ispid;
			string opflag;

		};

                struct Odie
		{
			string start;
			string end;
			string location;
			string ispname;
			string countryid;
			string provid;
			string cityid;
			string ispid;
		};

	};
	module jk {
		sequence <dx::SSIsp>  SSIspList;	
		sequence <dx::IPhebing> IPhebingList;	
		sequence <dx::AREAinfo> AREAinfoList;
		sequence <dx::IPinfo> IPinfoList;
		sequence <dx::SSinfo> SSinfoList;
		sequence <dx::ISPinfo> ISPinfoList;
		sequence <dx::PRinfo> PRinfoList;
		sequence <dx::IPpart> IPpartList;
		sequence <dx::Odie> OdieList;
		interface Ipmstor {
			string ipmssea(string start,string end);//查询IP段信息
			bool updatednipdata(dx::IPinfo ipinfo);//用户更新dn_ip_data 数据表
		string updatewhois(string ip);//提取数据库Whois_OK；里的数据更新Odie_OK数据来自于whois 
string feedbackview(string prov, string city, string isp ,string opflag );//获取省市服务提供商的view	
		        string feedbackqitalianxu(string prov, string city);//找出其它的连续	
			string feedbackareaview(string areapy );//获取省市服务提供商的view	
			PRinfoList feedbacksp();
                        SSinfoList feedbacks();//返回省的结果集
			SSinfoList feedbackss();//返回省市结果集
			SSIspList feedbackoldss();//兼容老版本的妥协
			SSinfoList feedbackhaiwai();//返回海外地区的py代码
			ISPinfoList feedbackisp();//返回ISP 信息
			SSIspList feedbackssisp();
			SSinfoList feedbackpushengpy();//返回32省的py代码
			bool updateprovpy(string provname,string provpy);
			AREAinfoList feedbackarea();//返回区域 为大的VIEW 服务的	
string feedbackviewarea(string area);//获取大区域的view信息
string feedbackipinfo(string ip);//feedback ip xinxi  sheng-shi-isp  .so   
string feedbackviewsheng(string sheng,string isppy);//获取大区域的view信息
string feedbackviewlargearea(string largearea,string isppy);	
bool updatecitypy(string provname,string provpy,string cityname,string citypy);//更新数据表省市代码 -- 整表更新
bool updatehaiwaipy(string provname,string provpy,string cityname,string citypy);//更新数据表省市代码 -- 整表更新

		};
	};
};
