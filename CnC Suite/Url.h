#pragma once
#include<Windows.h>
#include<StringClass.h>
#include"cObject.h"

class Url
	: public ClsObject<Url>
{
public:
	Url(){}
	Url(const iString& url) {
		this->_url_ = url;
	}
	Url(LPCTSTR url) {
		this->_url_ = url;
	}
	Url(const Url& url) {
		this->_url_.Replace(
			url.GetUrl()
		);
	}

	void SetUrlFromLocalPath(iString path) {
		this->SetUrlFromLocalPath(
			path.GetData()
		);
	}
	void SetUrlFromLocalPath(LPCTSTR path);

	void SetUrl(iString url) {
		this->SetUrl(
			url.GetData()
		);
	}
	void SetUrl(LPCTSTR url) {
		this->_url_.Replace(url);
	}

	int GetLength() {
		return this->_url_.GetLength();
	}

	const wchar_t* GetUrl() const {
		return this->_url_.GetData();
	}

	// ClsObject Base >>
	const wchar_t* ToString() {
		return _url_.GetData();
	};
	void FromString(const wchar_t* stringRepresentation) {
		this->_url_.Replace(stringRepresentation);
	}

private:
	iString _url_;

};