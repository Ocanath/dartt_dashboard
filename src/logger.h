#pragma once

#include "npy_writer.h"
#include "config.h"
#include <vector>
#include <thread>
#include <mutex>

class DataLogger
{
	public:
		
		int build_logging_list(std::vector<DarttField*> & subscribed_list);	//has to be guarded, callsite should be after sub list gets rebuilt under its own mutex
		void start();
		void stop();
		
		void package();	//

	private:

		void file_writer_loop();
		std::thread fwriter_thread_;
		std::atomic<bool> running_{false};
		std::vector<NpyWriter> writer_list_;	//derived from the DarttField flat subscribed list

		
};

