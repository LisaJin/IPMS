
#ifndef __Ipmstor_h__
#define __Ipmstor_h__


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


typedef ::std::vector<SSIsp> SSIspList ;
typedef ::std::vector<IPhebing> IPhebingList;
typedef ::std::vector<AREAinfo> AREAinfoList;
typedef ::std::vector<IPinfo> IPinfoList;
typedef ::std::vector<SSinfo> SSinfoList;
typedef ::std::vector<ISPinfo> ISPinfoList;
typedef ::std::vector<PRinfo> PRinfoList;
typedef ::std::vector<IPpart> IPpartList;
typedef ::std::vector<Odie> OdieList;

#endif
