#pragma once

template<class B>
class iBox
{
public:
	iBox()
		:b(nullptr)
	{
		this->b = new B();
	}
	iBox(const B& b_)
		:b(nullptr)
	{
		this->b = new B(b_);
	}
	iBox(const iBox<B>& box)
		:b(nullptr)
	{
		this->b
			= new B(
				box.get()
			);
	}
	~iBox() {
		if(this->b != nullptr)
			delete this->b;
	}
	void set(B b_) {
		if (this->b != nullptr)
			delete this->b;

		this->b = new B(b_);
	}
	B get() {
		return *this->b;
	}
	void Release() {
		delete this;
	}

private:
	B *b;
};