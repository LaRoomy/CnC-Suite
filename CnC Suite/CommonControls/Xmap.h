#pragma once
#include"ItemCollection.h"

template<class Y, class Z> class Xmap
{
public:
	Xmap(){}
	Xmap(const Xmap& map) {
		this->copy(map);
	}
	Xmap& operator= (const Xmap& map) {
		this->copy(map);
		return *this;
	}
	void Release() {
		delete this;
	}
	void Clear() {
		this->key.Clear();
		this->data.Clear();
	}
	void Add(Y Key, Z Data);
	void Remove(Y Key);
	bool Replace(Y Key, Z Data);

	int GetCount() const {
		return this->key.GetCount();
	}

	Z GetDataForKey(Y Key) const;
	bool GetPairAtIndex(int index, Y& Key, Z& Data) const;

private:
	itemCollection<Y> key;
	itemCollection<Z> data;

	void copy(const Xmap& map) {

		Y _key;
		Z _data;

		auto nItems = map.GetCount();

		for (int i = 0; i < nItems; i++)
		{
			if (map.GetPairAtIndex(i, _key, _data))
			{
				this->Add(_key, _data);
			}
		}
	}
};

template<class Y, class Z>
inline void Xmap<Y, Z>::Add(Y Key, Z Data)
{
	this->key.AddItem(Key);
	this->data.AddItem(Data);
}

template<class Y, class Z>
inline void Xmap<Y, Z>::Remove(Y Key)
{
	auto nItems = this->key.GetCount();

	for (int i = 0; i < nItems; i++)
	{
		auto k = this->key.GetAt(i);
		if (k == Key)
		{
			this->key.RemoveAt(i);
			this->data.RemoveAt(i);
			break;
		}
	}
}

template<class Y, class Z>
inline bool Xmap<Y, Z>::Replace(Y Key, Z Data)
{
	auto nItems = this->key.GetCount();
	for (int i = 0; i < nItems; i++)
	{
		auto k = this->key.GetAt(i);
		if (k == Key)
		{
			this->data.ReplaceAt(i, Data);
			return true;
		}
	}
	return false;
}

template<class Y, class Z>
inline Z Xmap<Y, Z>::GetDataForKey(Y Key) const
{
	Z element(0);

	auto nItems = this->key.GetCount();

	for (int i = 0; i < nItems; i++)
	{
		auto k = this->key.GetAt(i);
		if (k == Key)
		{
			return this->data.GetAt(i);
		}
	}
	return element;
}

template<class Y, class Z>
inline bool Xmap<Y, Z>::GetPairAtIndex(int index, Y & Key, Z & Data) const
{
	if (index < this->GetCount())
	{
		Key = this->key.GetAt(index);
		Data = this->data.GetAt(index);

		return true;
	}
	return false;
}
