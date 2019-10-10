#pragma once
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

template <typename T>
class threadsafe_arr_of_queue
{
		//массив очередей
		std::vector<std::queue<T>> data_queue(0);
		//число очередей в векторе
		int queue_num;
		//максимальный размер
		int long max_size;
		mutable std::mutex mut;
		//переменная отвечающая за текущий размер 
		int current_size;
		//условные переменные
		std::condition_variable EmptyCond, FullCond;

	public:

		threadsafe_arr_of_queue()
		{
			queue_num = 0;
			max_size = 0;
			current_size = 0;
		}

		threadsafe_arr_of_queue(const long int queue_size, const long int num_of_queue)
		{
			max_size = queue_size;
			queue_num = num_of_queue;
			data_queue.resize(num_of_queue);
		}

		void set_size(const long int queue_size, const long int num_of_queue)
		{
			max_size = queue_size;
			queue_num = num_of_queue;
			data_queue.resize(num_of_queue);
		}

		//избавляемся от оператора присваивания, чтобы не передавать ссылку вне класса
		threadsafe_arr_of_queue & operator = (const threadsafe_arr_of_queue &) = delete;

		threadsafe_arr_of_queue (threadsafe_arr_of_queue &other)
		{
			std::lock_guard<std::mutex> first_mut(other.mut);
			max_size = other.max_size;
			data_queue = other.data_queue;
		}

	
		bool full_i(const int num)
		{
			//возвращает true если нельзя добавить и false - в противном случае 
			std::lock_guard<std::mutex> block(mut);
			if (data_queue[num].size() == max_size)
				return true;
			else
				return false;
		}

		bool empty_i(const int num)
		{
			//возвращает true если нельзя вытолкнуть и false - в противном случае 
			std::lock_guard<std::mutex> block(mut);
			return data_queue[num].empty();
		}

		//в случае успешного добавления - true, иначе - false
		bool try_put(T new_value, const int start_num)
		{
			std::lock_guard<std::mutex> lk(mut);
			if (full_i(start_num))
				return false;
			data_queue[start_num].push(new_value);
			return true;
		}


		bool try_get(T &return_value, const int start_num)
		{
			std::lock_guard<std::mutex> lk(mut);
			if (empty_i(start_num))
				return false; 
			return_value = data_queue[start_num].front();
			data_queue[start_num].pop();
			return true;
		}

		//
		void get(T &return_value, const int start_num)
		{
			std::unique_lock<std::mutex> get_mut(mut);
			bool result = false;
			if (empty_i(start_num))
			{
				//пытаемся вытолкнуть из соседнего 
				for (int indx = 0; indx < queue_num; indx++)
				{
					if (!(empty_i(indx)))
					{
						result = true;
						return_value = data_queue[indx].front();
						data_queue[indx].pop();
						FullCond.notify_all();
					}
				}	
				if (!result)
				{
					EmptyCond.wait(get_mut, false);
				}
			}
			else
			{
				return_value = data_queue[start_num].front();
				data_queue[start_num].pop();
				FullCond.notify_all();
			}
		}

		void put(T new_value, const int start_num)
		{
			std::unique_lock<std::mutex> put_mut(mut);
			bool result = false;
			if (full_i(start_num))
			{
				//пытаемся добавить в соседний
				for (int indx = 0; indx < queue_num; indx++)
				{
					if (!(full_i(indx)))
					{
						result = true;
						data_queue[indx].push(new_value);
						EmptyCond.notify_all();
					}
				}
				if (!result)
				{
					FullCond.wait(put_mut, false);
				}
			}
			else
			{
				data_queue[start_num].push(new_value);
				EmptyCond.notify_all();
			}
		}
};