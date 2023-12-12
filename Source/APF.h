//#ifndef LOUIS_APFILTER_H
//#define LOUIS_APFILTER_H

#include <Stk.h>
#include <Delay.h>

namespace stk {

/***************************************************/
/*! \class APFilter
    \brief 
    This class implements a non-standard AllPass Filter.

    by Louis Boyu Deng, 2022
*/
/***************************************************/

class APF : public Stk
{
public: 

    //! The default constructor
    APF( unsigned long maximumDelay = (unsigned long) Stk::sampleRate() );

    //! The destructor
    ~APF();

    //! Reset and clear all internal state.
    void clear();

    //! Set the maximum delay line length in samples.
    void setMaximumDelay( unsigned long delay );

    //! Set combfilter delay and gain(gc), set lowpassfilter damp(d)
    void loadParams( unsigned long delay, StkFloat ga );

    //! Return the last computed output value.
    StkFloat lastOut( void ) const { return lastoutsample_; };

    //! Return the delayline delay length
    unsigned long getDelay( void ) const { return apdelayline_.getDelay(); };

    //! Input one sample to the effect and return one output.
    StkFloat tick( StkFloat input );

protected: 

    /* Building blocks of APFilter: */
    // Delay Line for AP
    Delay apdelayline_;
    // Max length for AP-Delay
    unsigned long length_;
    //feed factor ga => TF: H(z) = [-ga+z^-n] / [1 - ga*z^-n]
    StkFloat ga_;

    StkFloat lastoutsample_;
};

inline StkFloat APF :: tick( StkFloat input )
{
    //TF: H(z) = [-ga+z^-n] / [1 - ga*z^-n]
    // tmp is v[n] -> the input right before the delay unit
    
    //StkFloat tmp = input + ga_ * apdelayline_.lastOut();
    //lastoutsample_ = apdelayline_.lastOut() - ga_ * tmp;
    
    lastoutsample_ = apdelayline_.lastOut() - ga_*input;
    StkFloat tmp = ga_*lastoutsample_ + input;
    
    apdelayline_.tick(tmp);
    return lastoutsample_;
}

inline APF :: APF( unsigned long maximumDelay )
{
    // Default setting
    this->setMaximumDelay( maximumDelay );
    apdelayline_.setDelay( length_ >> 1 );
    this->clear();
}

inline APF :: ~APF()
{
}

inline void APF :: clear()
{
    apdelayline_.clear();
    lastoutsample_ = 0.0;
}

inline void APF :: setMaximumDelay( unsigned long delay )
{
    if ( delay == 0 ) {
    oStream_ << "Echo::setMaximumDelay: parameter cannot be zero!";
    handleError( StkError::WARNING ); return;
    }

    length_ = delay;
    apdelayline_.setMaximumDelay( delay );
}

inline void APF :: loadParams ( unsigned long delay, StkFloat ga )
{
    if ( delay > length_ ) {
        oStream_ << "LBCF::loadParams: delay is greater than maximum delay length!";
        handleError( StkError::WARNING ); return;
    }
    if ( abs(ga) >= 1.0 ) {
        oStream_ << "LBCF::loadParams: abs(ga) higher than 1.0 will cause instability";
        handleError( StkError::WARNING ); return;
    }

    apdelayline_.setDelay( delay );
    ga_ = ga;

}

}   // stk namespace

//#endif
