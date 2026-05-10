# Architectural Concerns

Currently display/plot information is shared via read callback and (afaik) is guarded with two mutexes: `periph_buf_mutex` and `plot_mutex`.

The read thread populates an internal periph backing store. This callback is the one location in which it is shared with the DarttField tree, which is what the rendering loop uses to display the information to the user.

In order to not starve the read callback, the copy-to-display-tree and copy-to-plotting-queue operations are guarded with a mutex which **does not block** - it tries the lock once, and if it can't grab the lock, it drops the sharing of that information with the render loop.

My lingering question - is there a way we could starve the rendering loop of necessary information, if we had a fucking badass insane computer that could somehow run the rendering loop much faster than the read/io loop? we could theoretically be in a situation where IO is slower than rendering, starving the render of information since a large percentage of its event loop is spent with those locks active. Splitting it into two separate locks helps... but it does feel a bit concerning. 

