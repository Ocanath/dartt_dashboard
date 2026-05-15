# NPY Writer

This translation unit implements a .npy file writer for individual datastreams. The interface is simple - 
you simply append data to the file in a binary format with a size and type argument. The class has open/close semantics, where the header information is written (such as size) on close. 

This TU is designed specifically for the dartt-dashboard. Each file inherits the symbol name of the `DarttField` it's logging. Each log entry is written as raw data (not display value/scaled) by the callback. The file is written to on close, or on a subscribed falling edge.

