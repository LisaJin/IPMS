#ifndef __ipmstor_h__
#define __ipmstor_h__

#include <vector>
#include <string>
using namespace std ;

struct IPhebing
{
    ::std::string start;
    ::std::string end;
    ::std::string provid;
    ::std::string cityid;
};

struct AREAinfo
{
    ::std::string areapy;
    ::std::string areaname;

};

struct SSIsp
{
    ::std::string shengpy;
    ::std::string shipy;
    ::std::string isppy;

};

struct VIEWinfo
{
    ::std::string viewname;
    ::std::string viewareaid;
    ::std::string viewispid;
    ::std::string viewpci;

};

struct IPpart
{
    ::std::string ipparts;
    ::std::string startip;
    ::std::string start;
    ::std::string end;
    ::std::string endip;

};

struct PRinfo
{
    ::std::string provname;
    ::std::string provpy;

};

struct SSinfo
{
    ::std::string provname;
    ::std::string provpy;
    ::std::string cityname;
    ::std::string citypy;

};

struct ISPinfo
{
    ::std::string ispname;
    ::std::string isppy;

};

struct IPinfo
{
    ::std::string startip;
    ::std::string start;
    ::std::string endip;
    ::std::string end;
    ::std::string ipmask;
    ::std::string location;
    ::std::string countryid;
    ::std::string provid;
    ::std::string cityid;
    ::std::string ispid;
    ::std::string opflag;

};

struct Odie
{
    ::std::string start;
    ::std::string end;
    ::std::string location;
    ::std::string ispname;
    ::std::string countryid;
    ::std::string provid;
    ::std::string cityid;
    ::std::string ispid;

};


typedef vector<SSIsp> SSIspList ;
typedef vector<IPhebing> IPhebingList;
typedef vector<AREAinfo> AREAinfoList;
typedef vector<IPinfo> IPinfoList;
typedef vector<SSinfo> SSinfoList;
typedef vector<ISPinfo> ISPinfoList;
typedef vector<PRinfo> PRinfoList;
typedef vector<IPpart> IPpartList;
typedef vector<Odie> OdieList;

class IpmstorI  {
	public:
		virtual void printString(const string& s);
		virtual string ipmssea(const string& start,const string& end);
		virtual int ipmsdel(const string& start ,const string& end );
		virtual int ipmsadd(const string& startip,const string& endip,const string& location,const string& isp );
		virtual IPpart ipsplit(const string& ipinfo);
		virtual string ipmerge();
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
		virtual bool updateOdie();

};



#endif
