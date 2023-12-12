//#ifndef LOUIS_LBCF_H
//#define LOUIS_LBCF_H

#include <Stk.h>
#include <Delay.h>
#include <OnePole.h>

namespace stk {

/***************************************************/
/*! \class LBCF
    \brief
    This class implements Lowpass-feedBack Comb Filter.

    by Louis Boyu Deng, 2022
*/
/***************************************************/

class LBCF : public Stk
{
public: 

    //! The default constructor
    LBCF( unsigned long maximumDelay = (unsigned long) Stk::sampleRate() );

    //! The destructor
    ~LBCF();

    //! Reset and clear all internal state.
    void clear();

    //! Set the maximum delay line length in samples.
    void setMaximumDelay( unsigned long delay );

    //! Set combfilter delay and gain(gc), set lowpassfilter damp(d)
    void loadParams( unsigned long delay, StkFloat gc, StkFloat damp );

    //! Return the last computed output value.
    StkFloat lastOut( void ) const { return lastoutsample_; };

    //! Return the delayline delay length
    unsigned long getDelay( void ) const { return cfdelayline_.getDelay(); };

    //! Input one sample to the effect and return one output.
    StkFloat tick( StkFloat input );

protected: 

    /* Building blocks of LBCF: */
    // Delay Line for combfilter
    Delay cfdelayline_;
    // Max length of LBCF/combfilter
    unsigned long length_;
    // Gain at the end of LBCF/combfilter right before summing with input
    StkFloat gc_;
    // Onepole lowpass filter
    OnePole lowpassfilter_;
    // Damping factor of LBCF/lowpass filter
    // as part of lowpassfilter_.setCoefficient()

    StkFloat lastoutsample_;

};

inline StkFloat LBCF :: tick( StkFloat input )
{
    StkFloat tmp = cfdelayline_.tick(input);
    tmp = gc_*lowpassfilter_.tick(tmp);
    lastoutsample_ = tmp;
    return lastoutsample_;
}

inline LBCF :: LBCF( unsigned long maximumDelay )
{
    // Default setting
    this->setMaximumDelay( maximumDelay );
    cfdelayline_.setDelay( length_ >> 1 );
    this->clear();
}

inline LBCF :: ~LBCF()
{
}

inline void LBCF :: clear( void )
{
    cfdelayline_.clear();
    lastoutsample_ = 0.0;
}

inline void LBCF :: setMaximumDelay( unsigned long delay )
{
  if ( delay == 0 ) {
    oStream_ << "Echo::setMaximumDelay: parameter cannot be zero!";
    handleError( StkError::WARNING ); return;
  }

    length_ = delay;
    cfdelayline_.setMaximumDelay( delay );
}

inline void LBCF :: loadParams( unsigned long delay, StkFloat gc, StkFloat damp )
{
  if ( delay > length_ ) {
    oStream_ << "LBCF::loadParams: delay is greater than maximum delay length!";
    handleError( StkError::WARNING ); return;
  }
  if ( gc >= 1.0 ) {
    oStream_ << "LBCF::loadParams: gc higher than 1.0 will cause instability";
    handleError( StkError::WARNING ); return;
  }
  if ( damp <= 0.0 ) {
    oStream_ << "LBCF::loadParams: damping lower than 0.0 will cause instability";
    handleError( StkError::WARNING ); return;
  }

    cfdelayline_.setDelay( delay );
    gc_ = gc;
    lowpassfilter_.setCoefficients( 1.0-damp, -damp );
    

}

}   // stk namespace

//#endif
