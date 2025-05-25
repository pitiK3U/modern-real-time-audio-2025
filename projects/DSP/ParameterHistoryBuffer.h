#pragma once

#include <vector>
#include <juce_core/juce_core.h>

namespace DSP
{

/** 
    A lock‐free, single‐writer (audio thread) / single‐reader (GUI thread)
    circular buffer for storing a short history of parameter values.
 
    - call pushSample() on the audio thread as often as you like
    - call getHistory() on the GUI thread (e.g. in a timer) to grab & clear everything
*/
template<typename FloatType>
class ParameterHistoryBuffer
{
public:
    /** 
        Create with a fixed capacity (must be > 0).  
        @param capacity  number of samples to buffer
    */
    explicit ParameterHistoryBuffer (int capacity = 512)
      : fifo   (capacity),
        buffer (static_cast<size_t> (capacity))
    {
    }

    /** 
        Change the history capacity at runtime (clears existing data).  
        @param newCapacity  new buffer size (> 0)
    */
    void setCapacity (int newCapacity)
    {
        jassert (newCapacity > 0);
        juce::ScopedLock lock (resizeMutex);  // protect resize operation
        fifo.reset();                         // reset FIFO indices
        buffer.assign (static_cast<size_t> (newCapacity), FloatType{}); // realloc buffer
    }

    /** 
        Store one new sample (called on audio thread).  
        Never blocks—will overwrite oldest data once full.
        @param sample  the value to push into the buffer
    */
    void pushSample (FloatType sample) noexcept
    {
        int start1, size1, start2, size2;
        fifo.prepareToWrite (1, start1, size1, start2, size2);

        // place sample into the available slice
        if (size1 > 0)
            buffer[(size_t) start1] = sample;
        else if (size2 > 0)
            buffer[(size_t) start2] = sample;

        fifo.finishedWrite (1);  // commit write
    }

    /** 
        Retrieve & clear *all* stored samples (called on GUI thread).  
        After this, the FIFO is empty.
        @param out  vector to receive the samples, in chronological order
    */
    void getHistory (std::vector<FloatType>& out)
    {
        int ready = fifo.getNumReady();
        if (ready <= 0)
        {
            out.clear();
            return;
        }

        int start1, size1, start2, size2;
        fifo.prepareToRead  (ready, start1, size1, start2, size2);

        out.clear();
        out.reserve (ready);

        // copy first contiguous block
        for (int i = 0; i < size1; ++i)
            out.push_back (buffer[(size_t)(start1 + i)]);

        // copy wrap-around block, if any
        for (int i = 0; i < size2; ++i)
            out.push_back (buffer[(size_t)(start2 + i)]);

        fifo.finishedRead (ready);  // commit read
    }

    /** @returns number of samples currently buffered (waiting to read). */
    int getNumReady() const noexcept           { return fifo.getNumReady(); }

    /** @returns the total capacity of the buffer. */
    int getCapacity() const noexcept           { return fifo.getTotalSize(); }

private:
    juce::AbstractFifo     fifo;          // manages read/write indices
    std::vector<FloatType> buffer;        // raw storage for samples
    juce::CriticalSection  resizeMutex;   // protects buffer reallocation

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterHistoryBuffer)
};

}
