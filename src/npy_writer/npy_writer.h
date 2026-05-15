#include <fstream>
#include <string>

class NpyWriter
{
	public:
		enum type {
			UINT8,
			UINT16,
			UINT32,
			UINT64,
			INT8,
			INT16,
			INT32,
			INT64,
			FLOAT32,
			DOUBLE64
		};
		int open(std::string name, type dtype);
		int add_sample(void * data, size_t size, type type);
		int close();
	private:
		uint64_t sample_count;
		std::string _filename;
		type _dtype;
};

