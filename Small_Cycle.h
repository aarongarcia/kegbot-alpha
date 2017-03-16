/*
  Small_Cycle.h - Library for defining a Small Cycle.
  Created by Bo Bartlett, March 6, 2017.
*/
#ifndef Small_Cycle_h
#define Small_Cycle_h

class Small_Cycle
{
	private:
		byte _stage;
		byte _duration;
		String _display;
		bool _started;
		bool _completed;
		unsigned long _endtime;

	public:
		Small_Cycle();
		byte stage();
		void stage(byte);
		byte duration();
		void duration(byte duration);
		String display();
		void display(String display);
		bool started();
		void started(bool started);
		bool completed();
		void completed(bool completed);
		unsigned long endtime();
		void endtime(unsigned long endtime);
		void setEndtime(unsigned long nowSeconds);
};

#endif
