#pragma once
//using namespace std;

template<class C>
class iCollectable
{
	virtual C& operator= (const C&) = 0;
	//virtual C* getInstance() = 0;				// this is not necessary for the collection (never used)
	virtual bool operator== (const C&) = 0;	// this is necessary for the RemoveOnComparison-Method !
};

template<class E>
class CollectionEvents
{
public:
	virtual void OnItemAdded(const E& item, int itemIndex) = 0;
	virtual void OnItemRemoved(int removedAt) = 0;
	virtual void OnItemsCleared() = 0;
};

template<class T> class itemCollection
{
public:
	itemCollection();
	itemCollection(CollectionEvents<T>* events);
	itemCollection(const itemCollection<T>&);
	~itemCollection();

	void setEventListenter(CollectionEvents<T>* events) {
		this->Events = events;
	}

	//void AddItem(T& item);
	void AddItem(const T& item);
	void AddItem(T* item);
	// Watch out: if the collection contains no items the access via GetAt(...) is illegal
	// -> call GetCount() first to check the content
	T GetAt(int index) const;
	void RemoveAt(int index);
	void RemoveByComparison(T item);
	// Watch out: usage could cause major problems!
	void ReplaceAt(int index, const T& item);
	void InsertAt(int index,const T& item);
	void Clear();
	int GetCount() const;
	static itemCollection<T>* Copy(itemCollection<T>*);
	itemCollection<T>* getInstance() { return this; }

	// watch out! this is a dangerous method which can corrupt the collection!
	T* getObjectCoreReferenceAt(int a) { return &this->_Items_[a]; }

	void Release() { delete this; }

	itemCollection<T>& operator= (const itemCollection<T>& iC)
	{
		this->Clear();

		for (int i = 0; i < iC.GetCount(); i++)
		{
			T t = iC.GetAt(i);
			this->AddItem(t);
		}
		return *this;
	}

	void operator+= (const T& item)
	{
		this->AddItem(item);
	}

private:
	int itemCount;
	T* _Items_;
	CollectionEvents<T>* Events;
};

template<class T>
inline itemCollection<T>::itemCollection()
	: itemCount(0), Events(nullptr), _Items_(nullptr)
{
	// ...
}

template<class T>
inline itemCollection<T>::itemCollection(CollectionEvents<T>* events)
	: itemCount(0), _Items_(nullptr)
{
	this->Events = events;

	// ...	
}

template<class T>
inline itemCollection<T>::itemCollection(const itemCollection<T>& toCopy)
{
	this->itemCount = 0;
	this->Events = nullptr;

	int countofCopy = toCopy.GetCount();

	if (toCopy.GetCount() == 0)
	{
		this->_Items_ = nullptr;
	}
	else
	{
		for (int i = 0; i < countofCopy; i++)
		{
			T t = toCopy.GetAt(i);
			this->AddItem(t);
		}
	}
}

template<class T>
inline itemCollection<T>::~itemCollection()
{
	if (this->GetCount() == 1)
	{
		if (this->_Items_ != nullptr)
			delete this->_Items_;
	}
	else
	{
		if (this->GetCount() > 1)
		{
			if (this->_Items_ != nullptr)
			{
				delete[] this->_Items_;
			}
		}
	}
}

template<class T>
inline void itemCollection<T>::AddItem(const T& item)
{
	if (this->itemCount == 0)
	{
		this->_Items_ = new T();
		if (this->_Items_ != nullptr)
		{
			this->_Items_[0] = item;
		}
	}
	else
	{
		if (this->itemCount == 1)
		{
			T holder = *this->_Items_;

			delete this->_Items_;
			this->_Items_ = nullptr;
			this->_Items_ = new T[2];
			if (this->_Items_ != nullptr)
			{
				this->_Items_[0] = holder;
				this->_Items_[1] = item;
			}
		}
		else
		{
			T* holder = new T[this->itemCount];
			if (holder != nullptr)
			{
				for (int i = 0; i < this->itemCount; i++)
				{
					holder[i] = this->_Items_[i];
				}
				delete[] this->_Items_;
				this->_Items_ = new T[itemCount + 1];
				if (this->_Items_ != nullptr)
				{
					int i;
					for (i = 0; i < itemCount; i++)
					{
						this->_Items_[i] = holder[i];
					}
					this->_Items_[i] = item;
				}
				delete[] holder;
			}
		}
	}
	// fire added-event
	if (this->Events != nullptr)
	{
		this->Events->OnItemAdded(item, this->itemCount);
	}
	this->itemCount++;
}

//template<class T>
//inline void itemCollection<T>::AddItem(const T& item)
//{
//	T _item_ = item;
//	this->AddItem(_item_);
//}

template<class T>
inline void itemCollection<T>::AddItem(T * item)
{
	T sItem = *item;
	this->AddItem(sItem);
}

template<class T>
inline T itemCollection<T>::GetAt(int index) const
{
	if (index < this->itemCount)
		return this->_Items_[index];
	else
		return *this->_Items_;
}

template<class T>
inline void itemCollection<T>::RemoveAt(int index)
{
	try
	{
		if (index < this->itemCount)
		{
			if (this->itemCount == 1)
			{
				this->Clear();
				this->itemCount = 0;
			}
			else
			{
				if (this->itemCount == 2)
				{
					T holder;

					if (index == 0)holder = this->_Items_[1];
					else holder = this->_Items_[0];

					this->Clear();
					this->itemCount = 1;
					this->_Items_ = new T(holder);
					//this->_Items_ = &holder;
				}
				else
				{
					T* holder = new T[this->itemCount];
					int cntHolder = this->itemCount;

					if (holder != nullptr)
					{
						for (int i = 0; i < this->itemCount; i++)
						{
							holder[i] = this->_Items_[i];
						}
						this->Clear();
						this->itemCount = cntHolder - 1;

						this->_Items_ = new T[cntHolder - 1];
						if (this->_Items_ != nullptr)
						{
							int k = 0;

							for (int j = 0; j < cntHolder; j++)
							{
								if (j != index)
								{
									this->_Items_[k] = holder[j];
									k++;
								}
							}
						}
						delete[] holder;
					}
				}
				// launch removed-event
				if (this->Events != NULL)
				{
					this->Events->OnItemRemoved(index);
				}
			}
		}
	}
	catch (...)
	{
		//UNREFERENCED_PARAMETER(e);
		return;
	}
}

template<class T>
inline void itemCollection<T>::RemoveByComparison(T item)
{
	int i;
	bool removable = false;

	for (i = 0; i < this->itemCount; i++)
	{
		if (this->_Items_[i] == item)
		{
			removable = true;
			break;
		}
	}
	if (removable)this->RemoveAt(i);
}

template<class T>
inline void itemCollection<T>::ReplaceAt(int index, const T& item)
{
	if (index < this->itemCount && index >= 0)
	{
		this->_Items_[index] = item;
	}
}

template<class T>
inline void itemCollection<T>::InsertAt(int index, const T & item)
{
	if ((index >= 0) && (index < this->itemCount))
	{
		int initialItemCount = this->itemCount;

		// at first take the last item and add it as a new item
		this->AddItem(
			this->GetAt(itemCount - 1)
		);

		// if there were more than 1 initial item, realign all items after the requested insertion index
		if (initialItemCount > 1)
		{
			for (int i = (initialItemCount - 1); i > index; i--)
			{
				this->ReplaceAt(
					i,
					this->GetAt(i - 1)
				);
			}
		}
		// insert the item
		this->ReplaceAt(index, item);
	}
	else if (index == this->itemCount)
	{
		// if the insertion-index is on the end of the collection, we only have to add it on the end of the collection
		this->AddItem(item);
	}
}

template<class T>
inline void itemCollection<T>::Clear()
{
	if (this->_Items_ != nullptr)
	{
		if (this->GetCount() == 1)
			delete this->_Items_;
		else if (this->GetCount() > 1)
			delete[] this->_Items_;

		this->_Items_ = nullptr;
		this->itemCount = 0;

		if (this->Events != nullptr)
		{
			this->Events->OnItemsCleared();
		}
	}
}

template<class T>
inline int itemCollection<T>::GetCount() const
{
	return itemCount;
}

template<class T>
inline itemCollection<T>* itemCollection<T>::Copy(itemCollection<T>* iC)
{
	if (iC != nullptr)
	{
		itemCollection<T>* newIC = new itemCollection<T>(iC->Events);
		if (newIC != nullptr)
		{
			for (int i = 0; i < iC->GetCount(); i++)
			{
				auto val = iC->GetAt(i);

				newIC->AddItem(val);
			}
			return newIC;
		}
	}
	return NULL;
}
