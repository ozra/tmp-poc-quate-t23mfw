#ifdef INTERFACE
/**
* Created:  2014-08-13
* Author:   Oscar Campbell
* Licence:  MIT (Expat) - http://opensource.org/licenses/mit-license.html
**/

#include "QuantBase.hh"
#include "QuantBuffersBase.hh"
#include "QuantBuffersSynchronizedHeap.hh"
#include "QuantBuffersSynchronizedBufferAbstract.hh"
#include "QuantFeedAbstract.hh"
#include "QuantPeriodizationAbstract.hh"
#include "QuantStudyContextBase.hh"
#include "QuantExecutionContext.hh"

enum retro_mode {
    BACKTEST = 0,
    FULL
};

class QuantExecutionRetroactive : public QuantExecutionContext {
  public:
    QuantExecutionRetroactive (
        QuantTime start_date,
        QuantTime end_date,
        bool buffers_output_enabled,
        bool is_optimization
    );
    ~QuantExecutionRetroactive ();

    void add ( QuantStudyContextAbstract *study );
    void add ( QuantFeedAbstract *feed );
    void add ( QuantPeriodizationAbstract *periodization );

    int run ( void );

  private:
    vector<QuantStudyContextAbstract *>     studies;
    vector<QuantFeedAbstract *>             feeds;
    vector<QuantPeriodizationAbstract *>            periodizations;

    //Str                         symbol;
    retro_mode                  mode;

};


#endif

#include "QuantExecutionRetroactive.hh"

QuantExecutionRetroactive::QuantExecutionRetroactive (
    QuantTime start_date,
    QuantTime end_date,
    bool buffers_output_enabled,
    bool is_optimization
)
//: study ( study )
//, symbol ( symbol )
:
    QuantExecutionContext {
        start_date,
        end_date,
        buffers_output_enabled,
        is_optimization
    },
    mode { BACKTEST }
{}

QuantExecutionRetroactive::~QuantExecutionRetroactive ()
{}

void QuantExecutionRetroactive::add ( QuantStudyContextAbstract *study ) {
    studies.push_back ( study );
}

void QuantExecutionRetroactive::add ( QuantFeedAbstract *feed ) {
    //feed->setDateRange( start_date, end_date );
    feeds.push_back( feed );
}

void QuantExecutionRetroactive::add ( QuantPeriodizationAbstract *periodization) {
    //periodization->setDateRange( start_date, end_date );
    periodizations.push_back( periodization );
}

int QuantExecutionRetroactive::run ( void ) {
    cerr << "RetroPracdelic::run - init\n";

    vector<QuantFeedAbstract *> study_feeds;  // so that we can remove feeds as they reach their last values - might change the implementation to check them every loop - will only affect computations in the very last minute of trading data...  - 2014-09-11/ORC(20:51)

    for ( auto study : studies ) {
        study->prepareRun();
        study->init();

        for ( auto per : study->the_jar.periodizations ) {
            per->buffer_heap.allocate();
        }

        study_feeds.insert( study_feeds.end(), study->the_jar.feeds.begin(), study->the_jar.feeds.end() );
    }

    // Prime feeds by fetching next available tick in each of them
    for ( auto f : study_feeds ) {
        cerr << "calls readNext on each feed for priming\n";
        f->setDateRange( start_date, end_date );
        f->readNext();               // This will cause loads for each feed, unfortunately waiting sequentially for each before calling fs for the next..     - 2014-09-11/ORC(20:54)
    }

    cerr << "Feed count is " << study_feeds.size() << "\n";

    //if (state == HISTORICAL )     // There's ALWAYS historical data reading at first, nothing else makes sense really (the pre-load may be short, but will always be there) -  - 2014-09-11/ORC(20:56)

    //# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
    //# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
    // Historical / backtest event loop  - 2014-09-11/ORC(21:03)
    QuantFeedAbstract *nearest_feed = nullptr;
    QuantTime lowest_time;

    while ( true ) {
        lowest_time = pxt::max_date_time;
        nearest_feed = nullptr;

        //cerr << "\nBacktest Eventloop : Find closest tick in time\n";

        // Find the tick closest in time
        for ( auto f : study_feeds ) {
            if ( ! f ) continue;

            if ( f->ticks().time < lowest_time || lowest_time == pxt::not_a_date_time ) {
                lowest_time = f->ticks().time;
                nearest_feed = f;
            }
        }

        if ( ! nearest_feed ) {
            cerr << "No feed left available" << "\n";
            break;
        }

        if ( lowest_time > end_date ) {
            cerr << "Past end date of backtesting date range - quits." << "\n";
            cerr << (nearest_feed->ticks().time) << " vs " << (nearest_feed->ticks().time) << " vs " << end_date << "\n";
            break;

        }

        //cerr << "Got it. emit it\n";
        if ( lowest_time >= start_date ) {   // *TODO* change to pre-main-loop "spooling"
            nearest_feed->emit();            // Let it kick of its' chain of events
        }
        //cerr << "read next value before next selection iteration\n";
        if ( nearest_feed->readNext() == true ) {         // More _historical_ data in the feed?
            continue;

        } else {
            cerr << "retval was false, no more data\n";

            int count = study_feeds.size();

            for ( int ix = 0; ix < count; ++ix ) {
                if ( study_feeds[ix] == nearest_feed ) {
                    study_feeds[ix] = nullptr;
                    //study_feeds.erase ( ix );
                    break;
                }
            }

            //if ( study_feeds.size() == 0 ) {
            //    break;  // Break out of history/backtest event-loop
            //}
        }
    }

    //# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
    // Change "mode" to live _if_ not pure backtest that is
    if ( mode == BACKTEST ) {
        return 0;
    }

    for ( auto f : feeds ) {
        f->setLiveMode();
    }

    //# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
    // Live event loop  - 2014-09-11/ORC(21:10)

    for (;;) {
        if ( true ) {   // AKA "TODO"
            break;
        }
    }

    /*

    const int MULTER = 1;

    QuantTime start = 1409986376737L * MULTER;
    QuantTime end = 1410301589077L * MULTER;
    QuantTime duration = end - start;

    QuantTime pos, prev_pos = 0;
    QuantTime quarterly = 7776000 * MULTER;

    cerr << duration << "\n";

    for (QuantTime t = start; t < end; ++t) {
        // run zmq-polls and such

        if(t % quarterly == 0) {
            pos = ((t - start) * 100) / (end - start);
            if(prev_pos != pos)
                cerr << pos << "\n";
            prev_pos = pos;
        }
    }
    */

    /*
    cerr << "RetroPracdelic::run deinit";
    for ( auto study : studies ){
        study->deinit();
    }
    */

    cerr << "RetroPracdelic::run ends";
    return 0;
}
