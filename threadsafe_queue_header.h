#pragma once
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>


template <typename T>
class threadsafe_queue
{
	private:
		int max_size;//переменная, отвечающая за максимальный размер 
		
		mutable std::mutex mut;
		std::queue<T> data_queue;
		std::condition_variable EmptyCond, FullCond;
	public:
		int current_size;//переменная отвечающая за текущий размер 
		//конструктор
		threadsafe_queue()
		{
			max_size = 0;
			current_size = 0;
		}

		void set_size(const long int num)
		{
			max_size = num;
		}

		//избавляемся от оператора присваивания, чтобы не передавать ссылку вне класса
		threadsafe_queue & operator = (const threadsafe_queue &) = delete;

		//конструктор копирования
		threadsafe_queue(threadsafe_queue &other)
		{
			std::lock_guard<std::mutex> first_mut(other.mut);
			max_size = other.max_size;
			data_queue = other.data_queue;
		}

		bool full() const
		{
			//возвращает true если нельзя добавить и false - в противном случае 
			std::lock_guard<std::mutex> block(mut);
			if (data_queue.size() == max_size)
				return true;
			else
				return false;
		}

		bool empty() const
		{
			//возвращает true если нельзя добавить и false - в противном случае 
			std::lock_guard<std::mutex> block(mut);
			return data_queue.empty();
		}

		void wait_and_pop(T &return_value)
		{
			std::unique_lock<std::mutex> pop_mut(mut);
			EmptyCond.wait(pop_mut, [this] { return !(data_queue.empty()); });
			return_value = data_queue.front();
			data_queue.pop();
			FullCond.notify_all();
		}

		
		void wait_and_push(T new_value)
		{
			std::unique_lock<std::mutex> push_mut(mut);
			FullCond.wait(push_mut, [this] { return !(data_queue.size() == max_size); });
			data_queue.push(new_value);
			EmptyCond.notify_all();
		}

		size_t size()
		{
			std::lock_guard<std::mutex> block(mut);
			return data_queue.size();
		}
};