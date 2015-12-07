#include "tue/control/generic_controller.h"

namespace tue
{

namespace control
{

GenericController::GenericController() : gain_(0), filters_(Filters())
{

}

GenericController::~GenericController()
{
}

void GenericController::configure(tue::Configuration& config, double sample_time)
{
    //! Clear the filters (required for reconfiguring)
    filters_.clear();

    //! Get the gain
    config.value("gain", gain_);

    //! Get the filters
    if (config.readGroup("filters"))
    {
        if (config.readGroup("weak_integrator"))
        {
            double fz;
            config.value("fz", fz);

            if (fz < 0)
                config.addError("fz < 0");

            if (!config.hasError())
                filters_.weak_integrator = new DWeakIntegrator(fz, sample_time);

            config.endGroup();
        }

        if (config.readGroup("lead_lag"))
        {
            double fz, fp;
            config.value("fz", fz);
            config.value("fp", fp);

            if (fz < 0 || fp < 0)
                config.addError("fz < 0 || fp < 0");

            if (!config.hasError())
                filters_.lead_lag = new DLeadLag(fz, fp, sample_time);

            config.endGroup();
        }

        if (config.readGroup("skewed_notch"))
        {
            double fz, dz, fp, dp;
            config.value("fz", fz);
            config.value("dz", dz);
            config.value("fp", fp);
            config.value("dp", dp);

            if (fz < 0 || dz < 0 || fp < 0 || dp < 0)
                config.addError("fz < 0 || dz < 0 || fp < 0 || dp < 0");

            if (!config.hasError())
                filters_.skewed_notch = new DSkewedNotch(fz, dz,fp, dp, sample_time);

            config.endGroup();
        }

        if (config.readGroup("second_order_low_pass"))
        {
            double fp, dp;
            config.value("fp", fp);
            config.value("dp", dp);

            if (fp < 0 || dp < 0)
                config.addError("fp < 0 || dp < 0");

            if (!config.hasError())
                filters_.second_order_low_pass = new DSecondOrderLowpass(fp, dp, sample_time);

            config.endGroup();
        }

        // end filters
        config.endGroup();
    }
}

void GenericController::update(const ControllerInput& input)
{
    if (!is_set(input.pos_reference) || !is_set(input.pos_measurement))
        return;

    //! 1) Calculate the error

    output_ = input.pos_reference - input.pos_measurement;

    //! 2) Apply gain

    output_ *= gain_;

    //! 3) Check what filters are configured and apply these

    // Apply weak_integrator
    if (filters_.weak_integrator)
    {
        filters_.weak_integrator->update( output_ );
        output_ = filters_.weak_integrator->getOutput();
    }

    // Apply lead_lag
    if (filters_.lead_lag)
    {
        filters_.lead_lag->update( output_ );
        output_ = filters_.lead_lag->getOutput();
    }

    // Apply skewed_notch
    if (filters_.skewed_notch)
    {
        filters_.lead_lag->update( output_ );
        output_ = filters_.lead_lag->getOutput();
    }

    // Apply second_order_low_pass
    if (filters_.second_order_low_pass)
    {
        filters_.second_order_low_pass->update( output_ );
        output_ = filters_.second_order_low_pass->getOutput();
    }

//    //! 4) Apply feed forward
//    output_ += feed_forward;

    return;
}

}

}
