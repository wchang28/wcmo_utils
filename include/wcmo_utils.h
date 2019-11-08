#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <exception>
#include <memory>
#include <wul/string_manip.h>
#include <wul/lexical_cast.h>
#include <chrono>
#include <ctime>
#include <wcmoapi.h>

namespace intex { namespace wcmoapi{

class wcmo_exception: public std::exception
{
public:
	std::string function;
	int ret;
	inline wcmo_exception(const char* sz = nullptr, const char* func = nullptr, int ret_code = 0) : std::exception(sz),  function(func ? func : ""), ret(ret_code) {}
};

template <char delim = '\n'>
class wcmoarg
{
public:
	static const char delimiter = delim;
	typedef std::pair<std::string, std::string> kvp;
	typedef std::map<std::string, std::string> kvm;
private:
	mutable char* Handle;
	WCMOARG* _getArg() const {return reinterpret_cast<WCMOARG*>(&Handle);}
	static std::string makeKeyValuePairString(const kvm& keyValuePairs)
	{
		std::ostringstream os;
		for (auto p = keyValuePairs.begin(); p!= keyValuePairs.end(); p++)
			os << p->first << '=' << p->second << delim;
		return os.str();
	}
	static std::string makeKeyValuePairString(const kvp& keyValuePair)
	{
		std::ostringstream os;
		os << keyValuePair.first << '=' << keyValuePair.second << delim;
		return os.str();
	}
public:
	inline wcmoarg(const char* sz = nullptr) : Handle(nullptr) {if (sz != nullptr) wcmoarg_set_string(_getArg(), (char*)sz);}
	inline wcmoarg(const std::string& str) : Handle(nullptr) {wcmoarg_set_string(_getArg(), (char*)str.c_str());}
	inline wcmoarg(const wcmoarg& src) : Handle(nullptr)
	{
		if (src.valid()) wcmoarg_set_string(_getArg(), (char*)src.c_str());
	}
	inline wcmoarg(const kvm& keyValuePairs) : Handle(nullptr)
	{
		wcmoarg_set_string(_getArg(), (char*)makeKeyValuePairString(keyValuePairs).c_str());
	}
	// copy from WCMOARG
	inline wcmoarg(const WCMOARG& src) : Handle(nullptr)
	{
		if (src != nullptr)
			wcmoarg_set_string(_getArg(), wcmoarg_get_string((WCMOARG*)&src));
	}
	inline void free()
	{
		if (Handle != nullptr)
		{
			wcmoarg_free(_getArg());
			Handle=nullptr;
		}
	}
	inline void alloc()
	{
		free();
		wcmoarg_alloc(_getArg()); 
	}
	inline ~wcmoarg()
	{
		free();
	}
	inline WCMOARG* operator & () {return _getArg();}
	inline wcmoarg& operator = (const char* sz)
	{
		if (sz == nullptr)
			free();
		else
			wcmoarg_set_string(_getArg(), (char*)sz);
		return *this;
	}
	inline wcmoarg& operator = (const std::string& str) {return this->operator=(str.c_str());}
	inline wcmoarg& operator = (const wcmoarg& src) {return this->operator=(src.c_str());}
	inline wcmoarg& operator = (const WCMOARG& arg) {return this->operator=(reinterpret_cast<const char*>(arg));}
	inline wcmoarg& operator = (const kvm& keyValuePairs) {return this->operator=(makeKeyValuePairString(keyValuePairs));}
	inline const char* c_str() const {return (Handle == nullptr ? nullptr : (const char*)wcmoarg_get_string(_getArg()));}
	inline operator const char* () const {return this->c_str();}
	inline size_t length() const
	{
		if (Handle != nullptr)
			return std::string(this->c_str()).length();
		else
			throw std::exception("null string has no length");
	}

	inline bool operator == (const char* sz) const
	{
		if (this->c_str() == sz)
			return true;
		else // this->c_str() != sz
		{
			if (Handle == nullptr) // sz != nullptr
				return false;
			else if (sz == nullptr) // Handle != nullptr
				return false;
			else // Handle != nullptr && sz != nullptr && (this->c_str() != sz)
				return (std::string(this->c_str()) == sz);
		}
	}
	inline bool operator == (const std::string& str) const {return this->operator==(str.c_str());}
	inline bool operator == (const wcmoarg& arg) const {return this->operator==(arg.c_str());}
	inline bool operator == (const WCMOARG& arg) const {return this->operator==(reinterpret_cast<const char*>(arg));}
	
	inline bool operator != (const char* sz) const {return !this->operator==(sz);}
	inline bool operator != (const std::string& str) const {return !this->operator==(str);}
	inline bool operator != (const wcmoarg& arg) const {return !this->operator==(arg);}
	inline bool operator != (const WCMOARG& arg) const {return !this->operator==(arg);}

	inline wcmoarg operator + (const char* sz) const
	{
		if (Handle != nullptr)
		{
			std::string s = this->c_str();
			if (sz != nullptr) s += sz;
			return wcmoarg(s.c_str());
		}
		else if (sz != nullptr)	// Handle == nullptr
			return wcmoarg(sz);
		else // both are null
			return wcmoarg();
	}
	inline wcmoarg operator + (const std::string& str) const {return this->operator+(str.c_str());}
	inline wcmoarg operator + (const wcmoarg& arg) const {return this->operator+(arg.c_str());}
	inline wcmoarg operator + (const WCMOARG& arg) const {return this->operator+(reinterpret_cast<const char*>(arg));}
	inline wcmoarg operator + (const kvm& keyValuePairs) const {return this->operator+(makeKeyValuePairString(keyValuePairs));}
	inline wcmoarg& operator += (const char* sz)
	{
		*this = *this + sz;
		return *this;
	}
	inline wcmoarg& operator += (const std::string& str) {return this->operator+=(str.c_str());}
	inline wcmoarg& operator += (const wcmoarg& arg) {return this->operator+=(arg.c_str());}
	inline wcmoarg& operator += (const WCMOARG& arg) {return this->operator+=(reinterpret_cast<const char*>(arg));}
	inline wcmoarg& operator += (const kvm& keyValuePairs) {return this->operator+=(makeKeyValuePairString(keyValuePairs));}
	inline wcmoarg& operator += (const kvp& keyValuePair) {return this->operator+=(makeKeyValuePairString(keyValuePair));}

	inline wcmoarg& operator << (const char* sz) {return this->operator+=(sz);}
	inline wcmoarg& operator << (const std::string& str) {return this->operator+=(str);}
	inline wcmoarg& operator << (const wcmoarg& arg) {return this->operator+=(arg);}
	inline wcmoarg& operator << (const WCMOARG& arg) {return this->operator+=(arg);}
	inline wcmoarg& operator << (const kvm& keyValuePairs) {return this->operator+=(keyValuePairs);}
	inline wcmoarg& operator << (const kvp& keyValuePair) {return this->operator+=(keyValuePair);}
			
	inline bool valid () const {return (Handle != nullptr);}
	inline operator bool() const {return (Handle != nullptr);}
	inline bool operator ! () const {return (Handle == nullptr);}
	inline char& operator [] (size_t i) {return Handle[i];}
	inline const char& operator [] (size_t i) const {return Handle[i];}

	inline wcmoarg& append(const std::string& opt)
	{
		std::ostringstream os;
		os << opt << delim;
		return this->operator+=(os.str());
	}

	inline static kvp parseKeyValueString(const std::string& keyValueString)
	{
		size_t x = keyValueString.find("=");
		std::string key = (x != std::string::npos ? keyValueString.substr(0, x) : keyValueString);
		std::string value = (x != std::string::npos ? keyValueString.substr(x+1) : "");
		return kvp(key, value);
	}
	static std::shared_ptr<kvm> parse(const char* sz) 
	{
		std::shared_ptr<kvm> ret;
		if (!sz) return ret;
		std::string s = sz;
		std::string seperator = " ";
		seperator[0]=delim;
		auto pTokens = wul::string_manip::cast_to(s).tokenize(seperator);
		if (pTokens != nullptr)
		{
			for (auto p = pTokens->begin(); p != pTokens->end(); p++)
			{
				if (p->length() != 0 && p->operator[](0) != '#')
				{
					kvp pair = parseKeyValueString(*p);
					if (ret == nullptr) ret.reset(new kvm());
					(*ret)[pair.first] = pair.second;
				}
			}
		}
		return ret;
	}
	inline std::shared_ptr<kvm> parse() const {return parse(this->c_str());}
};

template <char delim = '\n'>
class wcmo_instance
{
public:
	typedef wcmoarg<delim> arg_type;
	typedef typename arg_type::kvp kvp;
	typedef typename arg_type::kvm kvm;
	arg_type HandleArg;
protected:
	std::string _instanceHandle;
	arg_type _DataOut;
	arg_type _UserArg;
	arg_type _ErrOut;
	std::shared_ptr<kvm> _pInstanceData;

	static std::string getDelimAscii()
	{
		std::ostringstream os;
		os << (int)delim;
		return os.str();
	}
public:
	static const char delimiter = delim;
	wcmo_instance()
	{
		arg_type wcmoarg_Handle;
		wcmoarg_Handle += arg_type::kvp("KEYVAL_DELIM_ASCII", getDelimAscii());
		int ret = wcmo_init(&wcmoarg_Handle, &_UserArg, &_DataOut, &_ErrOut);
		if (ret) throw wcmo_exception(_ErrOut.c_str(), "wcmo_init", ret);
		_pInstanceData = _DataOut.parse();
		_instanceHandle = (*_pInstanceData)["INSTANCE_HANDLE"];
		HandleArg << kvp("INSTANCE_HANDLE", _instanceHandle);
	}
	wcmo_instance(arg_type& wcmoarg_Handle)
	{
		wcmoarg_Handle += arg_type::kvp("KEYVAL_DELIM_ASCII", getDelimAscii());
		int ret = wcmo_init(&wcmoarg_Handle, &_UserArg, &_DataOut, &_ErrOut);
		if (ret) throw wcmo_exception(_ErrOut.c_str(), "wcmo_init", ret);
		_pInstanceData = _DataOut.parse();
		_instanceHandle = (*_pInstanceData)["INSTANCE_HANDLE"];
		HandleArg << kvp("INSTANCE_HANDLE", _instanceHandle);
	}
	~wcmo_instance()
	{
		HandleArg << kvp("EXIT_ONLY_INSTANCE", "1");
		arg_type wcmoarg_DataOut;
		wcmo_exit(&HandleArg, &_UserArg, &wcmoarg_DataOut, &_ErrOut);
	}
	const arg_type& getDataOut() const {return _DataOut;}
	std::string getInstanceHandle() const {return _instanceHandle;}
	std::shared_ptr<kvm> getInstanceData() const {return _pInstanceData;}
	std::string version() const
	{
		if (_pInstanceData && _pInstanceData->find("ICMO_VERSION") != _pInstanceData->end())
			return _pInstanceData->at("ICMO_VERSION");
		else
			return "";
	}
	std::string product_version() const
	{
		if (_pInstanceData && _pInstanceData->find("DLL_PRODUCT_VERSION") != _pInstanceData->end())
			return _pInstanceData->at("DLL_PRODUCT_VERSION");
		else
			return "";
	}
	arg_type deal(arg_type& wcmoarg_Options, arg_type& wcmoarg_Deal)
	{
		arg_type wcmoarg_DataOut;
		int ret = wcmo_deal(&HandleArg, &_UserArg, &wcmoarg_Options, &wcmoarg_Deal, &wcmoarg_DataOut, &_ErrOut);
		if (ret) throw wcmo_exception(_ErrOut.c_str(), "wcmo_deal", ret);
		return wcmoarg_DataOut;
	}

	arg_type deal_info(arg_type& wcmoarg_Options)
	{
		arg_type wcmoarg_NotUsed;
		arg_type wcmoarg_DataOut;
		int ret = wcmo_deal_info(&HandleArg, &_UserArg, &wcmoarg_Options, &wcmoarg_NotUsed, &wcmoarg_DataOut, &_ErrOut);
		if (ret) throw wcmo_exception(_ErrOut.c_str(), "wcmo_deal_info", ret);
		return wcmoarg_DataOut;
	}

	arg_type extinfo(arg_type& wcmoarg_DataIn)
	{
		arg_type wcmoarg_DataOut;
		int ret = wcmo_extinfo(&HandleArg, &wcmoarg_DataIn, &wcmoarg_DataOut, &_ErrOut);
		if (ret) throw wcmo_exception(_ErrOut.c_str(), "wcmo_extinfo", ret);
		return wcmoarg_DataOut;
	}

	arg_type collat(arg_type& wcmoarg_Options)
	{
		arg_type wcmoarg_NotUsed;
		arg_type wcmoarg_DataOut;
		int ret = wcmo_collat(&HandleArg, &_UserArg, &wcmoarg_Options, &wcmoarg_NotUsed, &wcmoarg_DataOut, &_ErrOut);
		if (ret) throw wcmo_exception(_ErrOut.c_str(), "wcmo_collat", ret);
		return wcmoarg_DataOut;
	}

	arg_type cashflow(arg_type& wcmoarg_Options, arg_type& wcmoarg_CF, arg_type& wcmoarg_Loanscens)
	{
		arg_type wcmoarg_NotUsed;
		arg_type wcmoarg_NotUsed2;
		arg_type wcmoarg_DataOut;
		int ret = wcmo_cashflow(&HandleArg, &_UserArg, &wcmoarg_Options, &wcmoarg_NotUsed, &wcmoarg_CF, &wcmoarg_Loanscens, &wcmoarg_NotUsed2, &wcmoarg_DataOut, &_ErrOut);
		if (ret) throw wcmo_exception(_ErrOut.c_str(), "wcmo_cashflow", ret);
		return wcmoarg_DataOut;
	}
	arg_type cashflow(arg_type& wcmoarg_Options, arg_type& wcmoarg_CF)
	{
		arg_type wcmoarg_Loanscens;
		return cashflow(wcmoarg_Options, wcmoarg_CF, wcmoarg_Loanscens);
	}

	arg_type stats(arg_type& wcmoarg_Options, arg_type& wcmoarg_Stats)
	{
		arg_type wcmoarg_NotUsed;
		arg_type wcmoarg_NotUsed2;
		arg_type wcmoarg_DataOut;
		int ret = wcmo_stats(&HandleArg, &_UserArg, &wcmoarg_Options, &wcmoarg_NotUsed, &wcmoarg_NotUsed2, &wcmoarg_Stats, &wcmoarg_DataOut, &_ErrOut);
		if (ret) throw wcmo_exception(_ErrOut.c_str(), "wcmo_stats", ret);
		return wcmoarg_DataOut;
	}

	arg_type abs_summary(arg_type& wcmoarg_Options)
	{
		arg_type wcmoarg_NotUsed;
		arg_type wcmoarg_DataOut;
		int ret = wcmo_abs_summary(&HandleArg, &_UserArg, &wcmoarg_Options, &wcmoarg_NotUsed, &wcmoarg_DataOut, &_ErrOut);
		if (ret) throw wcmo_exception(_ErrOut.c_str(), "wcmo_abs_summary", ret);
		return wcmoarg_DataOut;
	}

	arg_type history(arg_type& wcmoarg_Options, arg_type& wcmoarg_Deal)
	{
		arg_type wcmoarg_NotUsed;
		arg_type wcmoarg_DataOut;
		int ret = wcmo_history(&HandleArg, &_UserArg, &wcmoarg_Options, &wcmoarg_Deal, &wcmoarg_DataOut, &_ErrOut);
		if (ret) throw wcmo_exception(_ErrOut.c_str(), "wcmo_history", ret);
		return wcmoarg_DataOut;
	}

	arg_type business_date_adj(arg_type& wcmoarg_Options)
	{
		arg_type wcmoarg_DataOut;
		int ret = wcmo_business_date_adj(&HandleArg, &_UserArg, &wcmoarg_Options, &wcmoarg_DataOut, &_ErrOut);
		if (ret) throw wcmo_exception(_ErrOut.c_str(), "wcmo_business_date_adj", ret);
		return wcmoarg_DataOut;
	}

	int daycount(int older_date, int recent_date, const std::string& method)
	{
		arg_type wcmoarg_Options;
		wcmoarg_Options += kvp("DAYCOUNT_METHOD", method);
		return wcmo_daycount(&HandleArg, &wcmoarg_Options, recent_date, older_date);
	}
};

// get today's date in YYYYMMDD format
template <typename RT>
inline RT todayYYYYMMDD()
{
	std::time_t t = std::time(0);   // get time now
	std::tm* now = std::localtime(&t);
	auto year = now->tm_year + 1900;
	auto month = now->tm_mon + 1;
	auto mday = now->tm_mday;
	auto n = year * 10000 + month * 100 + mday;
	return wul::lexical_cast<RT>(n);
}

template <typename _Elem = char>
class pipe_delimited
{
public:
	using string_t = std::basic_string<_Elem, std::char_traits<_Elem>, std::allocator<_Elem>>;
	using ostringstream_t = std::basic_ostringstream<_Elem, std::char_traits<_Elem>, std::allocator<_Elem>>;

	static inline std::shared_ptr<std::vector<string_t>> parse(const string_t& s)
	{
		const wul::basic_string_manip<_Elem>& sm = wul::basic_string_manip<_Elem>::cast_to(s);
		string_t delim(1, '|');
		auto p = sm.tokenize(delim);
		if (s[s.length() - 1] == '|') {
			p->pop_back();
		}
		return p;
	}

	static inline string_t stringify(const std::vector<string_t>& v)
	{
		ostringstream_t os;
		for (auto iter = v.begin(); iter != v.end(); iter++)
			os << (*iter) << '|';
		return os.str();
	}
};

}}

template <char delim>
inline std::ostream& operator << (std::ostream& os, const intex::wcmoapi::wcmoarg<delim>& arg)
{
	if (arg) os << arg.c_str();
	return os;
}