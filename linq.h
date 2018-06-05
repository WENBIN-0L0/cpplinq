#pragma once
#include <string>
#include <vector>
#include <iterator>
namespace wwb
{
	class linq_exception
	{
	public:
		std::string message;
		linq_exception(const std::string& message):message(message){ }
	};

	template<typename TValue>
	struct cleanup_type
	{
		typedef typename std::remove_const<typename std::remove_reference<TValue>::type>::type type;
	};

	template<typename TFUNC, typename TValue>
	struct func1_traits
	{
		static    TValue get_source();
		static    TFUNC get_func();
		typedef   decltype (get_func()(get_source()))   raw_value_type;
		typedef   typename cleanup_type<raw_value_type>::type  value_type;
	};

	namespace detail
	{
		template <typename TIterator>
		class from_range{
		public:
			typedef typename std::iterator_traits<TIterator>::value_type value_type;
		private:
			TIterator current;
			TIterator upcoming;
			TIterator end;
		public:
			from_range(TIterator start, TIterator end) :current(start), upcoming(start), end(end) { }
			bool next() { return upcoming == end ? false : (current = upcoming++, true); }
			value_type front() const { return *current; }
		};

		template <typename TIterator>
		from_range<TIterator> from(TIterator __first, TIterator __last)
		{
			return from_range<TIterator>(__first,__last);
		}

		template <typename TRange, typename TFUNC>
		class select_range{
		public:
			typedef typename func1_traits<TFUNC, typename TRange::value_type>::value_type value_type;
		private:
			TRange range;
			TFUNC func;
		public:
			select_range(const TRange& range,const TFUNC& func) : range(range),func(func) { }
			bool next() { return range.next(); } 
			value_type front() const { return func(range.front()); }
		};

		template <typename TRange, typename TFUNC>
		select_range<TRange,TFUNC> select(const TRange& range,const TFUNC& func)
		{
			return select_range<TRange,TFUNC>(range,func);
		}

		template <typename TRange,typename TPredicate>
		class where_range{
		public:
			typedef typename TRange::value_type value_type;
		private:
			TRange range;
			TPredicate predicate;
		public:
			where_range(const TRange& range,const TPredicate& predicate) : range(range),predicate(predicate) { }
			bool next() 
			{ 
				while(range.next()) if (predicate(range.front())) return true;
				return false;
			}
			const value_type front() const { return range.front(); }
		};

		template <typename TRange,typename TPredicate>
		where_range<TRange,TPredicate> where(const TRange& range,const TPredicate& predicate)
		{
			return where_range<TRange,TPredicate>(range,predicate);
		}

		template <typename TRange, typename TFUNC>
		class select_many_range{
		public:
			typedef typename func1_traits<TFUNC, typename TRange::value_type>::value_type TContainer;
			typedef typename TContainer::iterator iterator;
			typedef typename TContainer::value_type value_type;
		private:
			TContainer container;
			select_range<TRange,TFUNC> range;
			from_range<iterator> child_range;
		public:
			select_many_range(const TRange& range,const TFUNC& func) : range(select(range,func)),child_range(from(std::begin(container),std::end(container))) { }
			bool next() 
			{ 
				while(!child_range.next())
				{
					if(!range.next()) return false;
					container = range.front();
					child_range=from(std::begin(container),std::end(container));
				}
				return true;
			} 
			value_type front() { return child_range.front(); }
		};

		template <typename TRange, typename TFUNC>
		select_many_range<TRange,TFUNC> select_many(const TRange& range,const TFUNC& func)
		{
			return select_many_range<TRange,TFUNC>(range,func);
		}

		template <typename TValue>
		class number_range{
		public:
			typedef TValue value_type;
		private:
			value_type current;
			value_type upcoming;
			value_type end;
			value_type step;
		public:
			number_range(TValue start, TValue end, TValue step) :current(start), upcoming(start), end(end), step(step) { }
			bool next() 
			{ 
				current = upcoming;
				upcoming += step;
				return step>0 ? current < end : current > end;
			}
			value_type front() const { return current; }
		};

		template <typename TValue>
		class number_close_range{
		public:
			typedef TValue value_type;
		private:
			value_type current;
			value_type upcoming;
			value_type end;
			value_type step;
		public:
			number_close_range(TValue start, TValue end, TValue step) :current(start), upcoming(start), end(end), step(step) { }
			bool next() 
			{ 
				current = upcoming;
				upcoming += step;
				return step>0 ? current <= end : current >= end;
			}
			value_type front() const { return current; }
		};

	} 

	template<typename TRange>
	class linq
	{
	private:
		TRange range;
	public:
		linq(const TRange& range) : range(range) { }

		template <typename TFUNC>
		linq<typename detail::select_range<TRange,TFUNC>> select(const TFUNC& func)
		{
			return linq<detail::select_range<TRange,TFUNC>>(detail::select_range<TRange,TFUNC>(range,func));
		}

		template <typename TPredicate>
		linq<typename detail::where_range<TRange,TPredicate>> where(const TPredicate& predicate)
		{
			return linq<detail::where_range<TRange,TPredicate>>(detail::where_range<TRange,TPredicate>(range,predicate));
		}

		template <typename TFUNC>
		linq<typename detail::select_many_range<TRange,TFUNC>> select_many(const TFUNC& func)
		{
			return linq<detail::select_many_range<TRange,TFUNC>>(detail::select_many_range<TRange,TFUNC>(range,func));
		}

		template <typename TFUNC>
		typename TRange::value_type aggregate(const TFUNC& func)
		{
			if(!range.next()) throw linq_exception("Failed to get a value from an empty collection.");
			typename TRange::value_type acc = range.front();
			while(range.next()) acc = func(acc,range.front());
			return acc;
		}

		template <typename TValue,typename TFUNC>
		TValue aggregate(const TValue& init,const TFUNC& func)
		{
			TValue acc = init;
			while(range.next()) acc = func(acc,range.front());
			return acc;
		}

		typename TRange::value_type first()
		{
			if(!range.next()) throw linq_exception("Failed to get a value from an empty collection.");
			return range.front();
		}

		template<typename TValue>
		typename TRange::value_type firstOrDefault(const TValue& defaultv)
		{
			if(!range.next()) return defaultv;
			return range.front();
		}

		std::vector<typename TRange::value_type> to_vector()
		{
			std::vector<typename TRange::value_type> r;
			while(range.next()) r.push_back(range.front());
			return r;
		}

		template<typename TACTION>
		void foreach(TACTION action)
		{
			while(range.next()) action(range.front());
		}
	};

	template <typename TIterator>
	linq<typename detail::from_range<TIterator>> from(TIterator __first, TIterator __last)
	{
		return linq<detail::from_range<TIterator>>(detail::from_range<TIterator>(__first,__last));
	}

	template <typename TValue>
	linq<typename detail::number_close_range<TValue>> num_range(TValue start, TValue end, TValue step=1)
	{
		return linq<detail::number_close_range<TValue>>(detail::number_close_range<TValue>(start,end,step));
	}

	template <typename TValue>
	linq<typename detail::number_range<TValue>> num_range_until(TValue start, TValue end, TValue step=1)
	{
		return linq<detail::number_range<TValue>>(detail::number_range<TValue>(start,end,step));
	}
}
